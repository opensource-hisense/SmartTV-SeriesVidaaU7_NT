/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/*-----------------------------------------------------------------------------
                        include files
-----------------------------------------------------------------------------*/
#include <stdio.h>

#include "mtk_bt_service_mesh.h"
#include "mtk_bt_service_mesh_handle.h"
#include "mtk_bt_service_mesh_ipcrpc_struct.h"
#include "u_bt_mw_mesh.h"
#include "ri_common.h"

//#include "ble_mesh_interface.h"
#include "util_dlist.h"

#define BT_RH_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("[Handle]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static VOID *g_access_msg_cb_list = NULL;
void *g_mesh_health_client_evt_pvtag = NULL;

static BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T * _bt_mesh_rpc_find_access_msg_cb_entry(UINT16 model_handle, UINT16 opcode, UINT16 company_id)
{
    BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T *entry = NULL;

    entry = (BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T *)util_dlist_first(g_access_msg_cb_list);
    while (entry != NULL)
    {
        if (entry->model_handle == model_handle)
        {
            if (entry->cb_cnt == 0) //The model registered only one message handler
            {
                BT_RH_LOG("callback of model [0x%x] is found", model_handle);
                return entry;
            }
            else if ((entry->opcode == opcode) && (entry->company_id == company_id))  //The model registered message handler for every operation
            {
                BT_RH_LOG("callback of model [0x%x] opcode [0x%x] company_id [0x%x] is found", model_handle, opcode, company_id);
                return entry;
            }

        }
        entry = (BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T *)util_dlist_next(g_access_msg_cb_list, entry);
    }
    BT_RH_LOG("callback of model [0x%x] opcode [0x%x] company_id [0x%x] is NOT found", model_handle, opcode, company_id);
    return NULL;
}

static void _bt_mesh_rpc_add_access_msg_cb_entry(BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T *new_entry)
{
    BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T *entry = NULL;
    BOOL found = FALSE;

    if (NULL == new_entry)
    {
        BT_RH_LOG("%s fail @ L %d",__func__,__LINE__);
        return;
    }

    entry = _bt_mesh_rpc_find_access_msg_cb_entry(new_entry->model_handle, new_entry->opcode, new_entry->company_id);
    if (NULL == entry)
    {
        entry = (BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T *)util_dlist_entry_alloc(sizeof(BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T));
        if (NULL == entry)
        {
            BT_RH_LOG("%s fail @ L %d",__func__,__LINE__);
            return;
        }
    }
    else
    {
        found = TRUE;
    }

    entry->model_handle = new_entry->model_handle;
    entry->cb_cnt = new_entry->cb_cnt;
    entry->opcode = new_entry->opcode;
    entry->company_id = new_entry->company_id;
    entry->cb_index = new_entry->cb_index;
    entry->pv_cb = new_entry->pv_cb;
    entry->pv_tag = new_entry->pv_tag;

    BT_RH_LOG("add_access_msg_cb_entry: m_handle[0x%x], cb_cnt[%d], opcode[0x%x][0x%x], cb_index[%d], pv_cb:%p, pv_tag:%p",\
        entry->model_handle,entry->cb_cnt,entry->opcode,entry->company_id,entry->cb_index,entry->pv_cb,entry->pv_tag);

    if (FALSE == found)
    {
        util_dlist_insert(g_access_msg_cb_list, entry);
    }
}

static VOID bt_app_mesh_bt_event_cbk_wrapper(BT_MESH_BT_EVENT_T *bt_event, VOID *pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, bt_event, RPC_DESC_BT_MESH_BT_EVENT_T, NULL));
    if (bt_event->evt_id == BT_MESH_EVT_PROV_FACTOR)
    {
        RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(bt_event->evt.mesh_evt.mesh.prov_factor), RPC_DESC_BT_MESH_PROV_FACTOR_T, NULL));
        RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, bt_event->evt.mesh_evt.mesh.prov_factor.buf, bt_event->evt.mesh_evt.mesh.prov_factor.buf_len));
    }
    else if (bt_event->evt_id == BT_MESH_EVT_OTA_EVENT)
    {
        RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(bt_event->evt.mesh_evt.mesh.ota_evt), RPC_DESC_BT_MESH_EVT_OTA_T, NULL));
        RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, bt_event->evt.mesh_evt.mesh.ota_evt.nodes_status, \
                bt_event->evt.mesh_evt.mesh.ota_evt.nodes_num * sizeof(BT_MESH_OTA_NODE_UPDATE_STATUS_INFO_T)));
    }
    else if (bt_event->evt_id == BT_MESH_EVT_PROV_SCAN_UD_RESULT)
    {
        if (NULL != bt_event->evt.mesh_evt.mesh.prov_scan_ud.ud.uri_hash)
        {
            RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(bt_event->evt.mesh_evt.mesh.prov_scan_ud.ud), RPC_DESC_BT_MESH_PROV_UNPROVISIONED_DEVICE_T, NULL));
            RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, bt_event->evt.mesh_evt.mesh.prov_scan_ud.ud.uri_hash, 4));
        }
    }
    RPC_ARG_INP(ARG_TYPE_REF_DESC, bt_event);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    if (bt_event->evt_id != BT_MESH_EVT_SEQ_CHANGE) //This event is too frequent and does not make much sense
    {
        BT_RH_LOG("[_hndlr_]evnet id = %d, pv_cb_addr = %p", bt_event->evt_id, pt_nfy_tag->pv_cb_addr);
    }

    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_mesh_bt_event_cbk", pt_nfy_tag->pv_cb_addr);

    RPC_RETURN_VOID;
}

static VOID bt_mesh_access_msg_cbk_wrapper(BT_MESH_CBK_ACCESS_MSG_T *msg)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = NULL;
    BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T *cb_entry = NULL;

    //call corresponding callback function based on model_handle and opcode
    cb_entry = _bt_mesh_rpc_find_access_msg_cb_entry(msg->model_handle, msg->msg->opcode.opcode, msg->msg->opcode.company_id);
    if (NULL == cb_entry)
    {
        BT_RH_LOG("bt_mesh_access_msg_cbk_wrapper, no callback is found, return");
        return;
    }
    pt_nfy_tag = (RPC_CB_NFY_TAG_T *)cb_entry->pv_tag;

    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, msg, RPC_DESC_BT_MESH_CBK_ACCESS_MSG_T, NULL));
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, msg->msg, RPC_DESC_BT_MESH_ACCESS_MESSAGE_RX_T, NULL));
    if (msg->msg->buf_len > 0)
    {
        RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->msg->buf, msg->msg->buf_len));
    }
    //RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->arg, 2));  //the sizeof arg field is implemetation specific. It is not used currently. Therefore, do not notify it now. T.B.D
    RPC_ARG_INP(ARG_TYPE_REF_DESC, msg);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BT_RH_LOG("[_hndlr_]bt_mesh_access_msg_cbk_wrapper, model_handle=0x%x, opcode[0x%x][0x%x], buf=%p, pv_cb=%p",
               cb_entry->model_handle, cb_entry->opcode, cb_entry->company_id, msg->msg->buf, cb_entry->pv_cb);

    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_mesh_access_msg_cbk", cb_entry->pv_cb);
    RPC_RETURN_VOID;
}

static void bt_mesh_health_client_evt_cbk_wrapper(BT_MESH_CBK_HEALTH_CLIENT_EVT_T *event)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)g_mesh_health_client_evt_pvtag;

    if ((NULL == event) || (NULL == event->msg) || (NULL == event->event))
    {
        BT_RH_LOG("[_hndlr_] %s invalid ARG, return!!!", __func__);
        return;
    }

    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, event, RPC_DESC_BT_MESH_CBK_HEALTH_CLIENT_EVT_T, NULL));
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, event->msg, RPC_DESC_BT_MESH_ACCESS_MESSAGE_RX_T, NULL));
    RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, event->msg->buf, event->msg->buf_len));
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, event->event, RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_T, NULL));
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(event->event->data), RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_T_data, NULL));
    switch (event->msg->opcode.opcode)
    {
        case BT_MESH_ACCESS_MSG_HEALTH_CURRENT_STATUS:
            RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(event->event->data.current_status), RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_CURRENT_STATUS_T, NULL));
            RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, event->event->data.current_status.fault_array, event->event->data.current_status.fault_array_length));
            break;
        case BT_MESH_ACCESS_MSG_HEALTH_FAULT_STATUS:
            RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(event->event->data.fault_status), RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_FAULT_STATUS_T, NULL));
            RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, event->event->data.fault_status.fault_array, event->event->data.fault_status.fault_array_length));
            break;
        case BT_MESH_ACCESS_MSG_HEALTH_PERIOD_STATUS:
            RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(event->event->data.period_status), RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_PERIOD_STATUS_T, NULL));
            break;
        case BT_MESH_ACCESS_MSG_HEALTH_ATTENTION_STATUS:
            RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(event->event->data.attention_status), RPC_DESC_BT_MESH_HEALTH_CLIENT_EVT_ATTENTION_STATUS_T, NULL));
            break;
        default:
            break;
    }

    RPC_ARG_INP(ARG_TYPE_REF_DESC, event);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BT_RH_LOG("[_hndlr_]bt_mesh_health_client_evt_cbk_wrapper, pt_nfy_tag->pv_cb_addr = %p",
               pt_nfy_tag->pv_cb_addr);

    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_mesh_health_client_evt_cbk", pt_nfy_tag->pv_cb_addr);
    RPC_RETURN_VOID;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_init(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    MTKRPCAPI_BT_APP_MESH_CB_FUNC_T *p_bt_app_cb_func = NULL;
    MTKRPCAPI_BT_APP_MESH_CB_FUNC_T bt_app_cb_func;
    MTKRPCAPI_BT_MESH_CB_FUNC_T internal_cb;
    RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
    VOID * apv_cb_addr[1] = {0};

    if ((ui4_num_args != 2) || (NULL == pt_args) || (NULL == pt_return))
    {
        return RPCR_INV_ARGS;
    }
    memset(&bt_app_cb_func, 0, sizeof(MTKRPCAPI_BT_APP_MESH_CB_FUNC_T));
    p_bt_app_cb_func = (MTKRPCAPI_BT_APP_MESH_CB_FUNC_T*)pt_args[0].u.pv_desc;
    BT_RH_LOG("bt_mesh_init, pt_args[0].u.pv_desc = %p bt_mesh_bt_event_cb= %p",
        pt_args[0].u.pv_desc, p_bt_app_cb_func->bt_mesh_bt_event_cb);

    if (p_bt_app_cb_func->bt_mesh_bt_event_cb != NULL)
    {
        apv_cb_addr[0] = p_bt_app_cb_func->bt_mesh_bt_event_cb;
        bt_app_cb_func.bt_mesh_bt_event_cb = bt_app_mesh_bt_event_cbk_wrapper;
    }
    //allocate notify tag for app callback
    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, pt_args[1].u.pv_arg);
    if (NULL == pt_nfy_tag)
    {
        return RPCR_OUTOFMEMORY;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_init(&bt_app_cb_func, pt_nfy_tag);
    BT_RH_LOG("bt_mesh_init, pt_nfy_tag->pv_cb_addr = %p", pt_nfy_tag->pv_cb_addr);
    if (pt_return->u.i4_arg)
    {
        BT_RH_LOG("Error!!!");
        ri_free_cb_tag(pt_nfy_tag);
    }

    //Register callback fct for rpc internal use, Allocate notify tag later
    memset(&internal_cb, 0, sizeof(MTKRPCAPI_BT_MESH_CB_FUNC_T));
    internal_cb.bt_mesh_access_msg_cb = bt_mesh_access_msg_cbk_wrapper;
    internal_cb.bt_mesh_health_client_evt_cb = bt_mesh_health_client_evt_cbk_wrapper;
    mtkrpcapi_btm_mesh_register_internal_cbk_entry(&internal_cb);

    if (NULL == g_access_msg_cb_list)
    {
        g_access_msg_cb_list = (void *)util_dlist_alloc();
    }

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_deinit(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_deinit , arg_1 = %d", pt_args[0].u.i4_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_deinit();

    if (NULL != g_access_msg_cb_list)
    {
        util_dlist_free(g_access_msg_cb_list);
        g_access_msg_cb_list = NULL;
    }

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_set_model_data(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_MESH_MODEL_DATA_T *md = (BT_MESH_MODEL_DATA_T *)pt_args[0].u.pv_desc;

    BT_RH_LOG("[_hndlr_]bt_mesh_set_model_data , arg_1 = %p, opcode:0x%x", pt_args[0].u.pv_desc, md->opcode);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    //pt_return->e_type   = ARG_TYPE_INT32;
    //pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);

    /*pickup and save all the callback functions for the adding models.*/
    switch (md->opcode)
    {
        case BT_MESH_MD_OP_SET_COMPOSITION_DATA_HEADER:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(md);
            break;
        }
        case BT_MESH_MD_OP_ADD_ELEMENT:
        {
            BT_RH_LOG("BT_MESH_MD_OP_ADD_ELEMENT, element_index=%p, location:%d",md->data.element.element_index, md->data.element.location);
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(md);
            break;
        }
        case BT_MESH_MD_OP_SET_ELEMENT_ADDR:
        {
            BT_RH_LOG("BT_MESH_MD_OP_SET_ELEMENT_ADDR");
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(md);
            break;
        }
        case BT_MESH_MD_OP_ADD_CONFIGURATION_SERVER:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(md);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry;
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[1] = {0};

                BT_RH_LOG("BT_MESH_MD_OP_ADD_CONFIGURATION_SERVER, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
                if(md->data.configuration_server.callback != NULL)
                {
                    apv_cb_addr[0] = md->data.configuration_server.callback;
                    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);
                    if (NULL != pt_nfy_tag)
                    {
                        cb_entry.model_handle = *(md->data.configuration_server.model_handle);
                        cb_entry.cb_cnt = 0;
                        cb_entry.opcode = 0;
                        cb_entry.company_id = 0;
                        cb_entry.cb_index = 0;
                        cb_entry.pv_cb = md->data.configuration_server.callback;
                        cb_entry.pv_tag = pt_nfy_tag;
                        _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                    }
                    else
                    {
                        BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                    }
                }
            }
            break;
        }
        case BT_MESH_MD_OP_ADD_CONFIGURATION_CLIENT:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry;
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[1] = {0};

                BT_RH_LOG("BT_MESH_MD_OP_ADD_CONFIGURATION_CLIENT, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
                if(md->data.configuration_client.callback != NULL)
                {
                    apv_cb_addr[0] = md->data.configuration_client.callback;
                    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);
                    if (NULL != pt_nfy_tag)
                    {
                        cb_entry.model_handle = *(md->data.configuration_client.model_handle);
                        cb_entry.cb_cnt = 0;
                        cb_entry.opcode = 0;
                        cb_entry.company_id = 0;
                        cb_entry.cb_index = 0;
                        cb_entry.pv_cb = md->data.configuration_client.callback;
                        cb_entry.pv_tag = pt_nfy_tag;
                        _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                    }
                    else
                    {
                        BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                    }
                }
            }
            break;
        }
        case BT_MESH_MD_OP_ADD_HEALTH_SERVER:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry;
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[1] = {0};

                BT_RH_LOG("BT_MESH_MD_OP_ADD_HEALTH_SERVER, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
                if(md->data.health_server.callback != NULL)
                {
                    apv_cb_addr[0] = md->data.health_server.callback;
                    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);
                    if (NULL != pt_nfy_tag)
                    {
                        cb_entry.model_handle = *(md->data.health_server.model_handle);
                        cb_entry.cb_cnt = 0;
                        cb_entry.opcode = 0;
                        cb_entry.company_id = 0;
                        cb_entry.cb_index = 0;
                        cb_entry.pv_cb = md->data.health_server.callback;
                        cb_entry.pv_tag = pt_nfy_tag;
                        _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                    }
                    else
                    {
                        BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                    }
                }
            }
            break;
        }
        case BT_MESH_MD_OP_ADD_HEALTH_CLIENT:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[1] = {0};

                BT_RH_LOG("BT_MESH_MD_OP_ADD_HEALTH_CLIENT, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
                if(md->data.health_client.callback != NULL)
                {
                    apv_cb_addr[0] = md->data.health_client.callback;
                    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);
                    if (NULL != pt_nfy_tag)
                    {
                        g_mesh_health_client_evt_pvtag = pt_nfy_tag;
                    }
                    else
                    {
                        BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                    }
                }
            }
            break;
        }
        case BT_MESH_MD_OP_ADD_GENERIC_ONOFF_SERVER:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry;
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[1] = {0};

                BT_RH_LOG("BT_MESH_MD_OP_ADD_GENERIC_ONOFF_SERVER, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
                if(md->data.generic_onoff_server.callback != NULL)
                {
                    apv_cb_addr[0] = md->data.generic_onoff_server.callback;
                    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);
                    if (NULL != pt_nfy_tag)
                    {
                        cb_entry.model_handle = *(md->data.generic_onoff_server.model_handle);
                        cb_entry.cb_cnt = 0;
                        cb_entry.opcode = 0;
                        cb_entry.company_id = 0;
                        cb_entry.cb_index = 0;
                        cb_entry.pv_cb = md->data.generic_onoff_server.callback;
                        cb_entry.pv_tag = pt_nfy_tag;
                        _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                    }
                    else
                    {
                        BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                    }
                }
            }
            break;
        }
        case BT_MESH_MD_OP_ADD_CTL_SETUP_SERVER:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry;
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[1] = {0};

                BT_RH_LOG("BT_MESH_MD_OP_ADD_CTL_SETUP_SERVER, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
                if(md->data.ctl_setup_server.callback != NULL)
                {
                    apv_cb_addr[0] = md->data.ctl_setup_server.callback;
                    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);

                    UINT16 ctl_temp_element_idx = 0;
                    x_mtkapi_bt_mesh_light_model_get_element_index(BT_MESH_SIG_MODEL_ID_LIGHT_CTL_TEMPERATURE_SERVER, &ctl_temp_element_idx);

                    BT_MESH_EXTEND_MODEL_ELEMENT_INFO_T extend_model_element_info[] = {
                        {md->data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_CTL_SETUP_SERVER},
                        {md->data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_CTL_SERVER},
                        {md->data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_SERVER},
                        {md->data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SERVER},
                        {md->data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_ONOFF_SERVER},
                        {md->data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER},
                        {ctl_temp_element_idx, BT_MESH_SIG_MODEL_ID_LIGHT_CTL_TEMPERATURE_SERVER},
                        {ctl_temp_element_idx, BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER},
                        {md->data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_SETUP_SERVER},
                        {md->data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SETUP_SERVER},
                        {md->data.ctl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_DEFAULT_TRANSITION_TIME_SERVER},
                    };

                    if (NULL != pt_nfy_tag)
                    {
                        for (int i = 0; i < (sizeof(extend_model_element_info) / sizeof(BT_MESH_EXTEND_MODEL_ELEMENT_INFO_T)); i++)
                        {
                            cb_entry.model_handle = x_mtkapi_bt_mesh_get_model_handle_by_elementIdx_and_modeId(extend_model_element_info[i].model_id,
                                                                                                               extend_model_element_info[i].element_idx);
                            cb_entry.cb_cnt = 0;
                            cb_entry.opcode = 0;
                            cb_entry.company_id = 0;
                            cb_entry.cb_index = 0;
                            cb_entry.pv_cb = md->data.ctl_setup_server.callback;
                            cb_entry.pv_tag = pt_nfy_tag;
                            _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                        }

                    }
                    else
                    {
                        BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                    }
                }
            }
            break;
        }
        case BT_MESH_MD_OP_ADD_GENERIC_POWER_ONOFF_CLIENT:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry;
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[1] = {0};

                BT_RH_LOG("BT_MESH_MD_OP_ADD_GENERIC_POWER_ONOFF_CLIENT, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
                if(md->data.generic_power_onoff_client.callback != NULL)
                {
                    apv_cb_addr[0] = md->data.generic_power_onoff_client.callback;
                    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);
                    if (NULL != pt_nfy_tag)
                    {
                        cb_entry.model_handle = *(md->data.generic_power_onoff_client.model_handle);
                        cb_entry.cb_cnt = 0;
                        cb_entry.opcode = 0;
                        cb_entry.company_id = 0;
                        cb_entry.cb_index = 0;
                        cb_entry.pv_cb = md->data.generic_power_onoff_client.callback;
                        cb_entry.pv_tag = pt_nfy_tag;
                        _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                    }
                    else
                    {
                        BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                    }
                }
            }
            break;
        }
        case BT_MESH_MD_OP_ADD_LIGHTNESS_CLIENT:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry;
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[1] = {0};

                BT_RH_LOG("BT_MESH_MD_OP_ADD_LIGHTNESS_CLIENT, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
                if(md->data.lightness_client.callback != NULL)
                {
                    apv_cb_addr[0] = md->data.lightness_client.callback;
                    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);
                    if (NULL != pt_nfy_tag)
                    {
                        cb_entry.model_handle = *(md->data.lightness_client.model_handle);
                        cb_entry.cb_cnt = 0;
                        cb_entry.opcode = 0;
                        cb_entry.company_id = 0;
                        cb_entry.cb_index = 0;
                        cb_entry.pv_cb = md->data.lightness_client.callback;
                        cb_entry.pv_tag = pt_nfy_tag;
                        _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                    }
                    else
                    {
                        BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                    }
                }
            }
            break;
        }
        case BT_MESH_MD_OP_ADD_CTL_CLIENT:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry;
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[1] = {0};

                BT_RH_LOG("BT_MESH_MD_OP_ADD_CTL_CLIENT, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
                if(md->data.ctl_client.callback != NULL)
                {
                    apv_cb_addr[0] = md->data.ctl_client.callback;
                    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);
                    if (NULL != pt_nfy_tag)
                    {
                        cb_entry.model_handle = *(md->data.ctl_client.model_handle);
                        cb_entry.cb_cnt = 0;
                        cb_entry.opcode = 0;
                        cb_entry.company_id = 0;
                        cb_entry.cb_index = 0;
                        cb_entry.pv_cb = md->data.ctl_client.callback;
                        cb_entry.pv_tag = pt_nfy_tag;
                        _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                    }
                    else
                    {
                        BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                    }
                }
            }
            break;
        }
        case BT_MESH_MD_OP_ADD_HSL_SETUP_SERVER:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry;
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[1] = {0};

                BT_RH_LOG("BT_MESH_MD_OP_ADD_HSL_SETUP_SERVER, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
                if(md->data.hsl_setup_server.callback != NULL)
                {
                    apv_cb_addr[0] = md->data.hsl_setup_server.callback;
                    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);

                    UINT16 hsl_hue_element_idx = 0;
                    UINT16 hsl_saturation_element_idx = 0;
                    x_mtkapi_bt_mesh_light_model_get_element_index(BT_MESH_SIG_MODEL_ID_LIGHT_HSL_HUE_SERVER, &hsl_hue_element_idx);
                    x_mtkapi_bt_mesh_light_model_get_element_index(BT_MESH_SIG_MODEL_ID_LIGHT_HSL_SATURATION_SERVER, &hsl_saturation_element_idx);

                    BT_MESH_EXTEND_MODEL_ELEMENT_INFO_T extend_model_element_info[] = {
                        {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_HSL_SETUP_SERVER},
                        {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_HSL_SERVER},
                        {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_SERVER},
                        {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SERVER},
                        {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_ONOFF_SERVER},
                        {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER},
                        {hsl_hue_element_idx, BT_MESH_SIG_MODEL_ID_LIGHT_HSL_HUE_SERVER},
                        {hsl_hue_element_idx, BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER},
                        {hsl_saturation_element_idx, BT_MESH_SIG_MODEL_ID_LIGHT_HSL_SATURATION_SERVER},
                        {hsl_saturation_element_idx, BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER},
                        {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_SETUP_SERVER},
                        {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SETUP_SERVER},
                        {md->data.hsl_setup_server.element_index, BT_MESH_SIG_MODEL_ID_GENERIC_DEFAULT_TRANSITION_TIME_SERVER},
                    };

                    if (NULL != pt_nfy_tag)
                    {
                        for (int i = 0; i < (sizeof(extend_model_element_info) / sizeof(BT_MESH_EXTEND_MODEL_ELEMENT_INFO_T)); i++)
                        {
                            cb_entry.model_handle = x_mtkapi_bt_mesh_get_model_handle_by_elementIdx_and_modeId(extend_model_element_info[i].model_id,
                                                                                                               extend_model_element_info[i].element_idx);
                            cb_entry.cb_cnt = 0;
                            cb_entry.opcode = 0;
                            cb_entry.company_id = 0;
                            cb_entry.cb_index = 0;
                            cb_entry.pv_cb = md->data.hsl_setup_server.callback;
                            cb_entry.pv_tag = pt_nfy_tag;
                            _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                        }

                    }
                    else
                    {
                        BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                    }
                }
            }
            break;
        }
        case BT_MESH_MD_OP_ADD_HSL_CLIENT:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry;
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[1] = {0};

                BT_RH_LOG("BT_MESH_MD_OP_ADD_HSL_CLIENT, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
                if(md->data.hsl_client.callback != NULL)
                {
                    apv_cb_addr[0] = md->data.hsl_client.callback;
                    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);
                    if (NULL != pt_nfy_tag)
                    {
                        cb_entry.model_handle = *(md->data.hsl_client.model_handle);
                        cb_entry.cb_cnt = 0;
                        cb_entry.opcode = 0;
                        cb_entry.company_id = 0;
                        cb_entry.cb_index = 0;
                        cb_entry.pv_cb = md->data.hsl_client.callback;
                        cb_entry.pv_tag = pt_nfy_tag;
                        _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                    }
                    else
                    {
                        BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                    }
                }
            }
            break;
        }
        case BT_MESH_MD_OP_ADD_MODEL:
        {
            pt_return->e_type   = ARG_TYPE_INT32;
            pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_model_data(pt_args[0].u.pv_desc);
            if (pt_return->u.i4_arg == TRUE)    //success
            {
                BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry[25];
                UINT8 entry_cnt = md->data.model_data.model_params->opcode_count;
                RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
                VOID * apv_cb_addr[25] = {0};
                UINT8 cb_tag_cnt = md->data.model_data.model_params->opcode_count/25 + 1;
                UINT32 i = 0, j = 0;

                BT_RH_LOG("BT_MESH_MD_OP_ADD_MODEL, pt_args[0].u.pv_desc = %p, entry_cnt = %d", pt_args[0].u.pv_desc, entry_cnt);
                for (i = 0; i < cb_tag_cnt; i++)
                {
                    if (entry_cnt > 25)
                    {
                        for (j = 0; j < 25; j++)
                        {
                            cb_entry[j].model_handle = *(md->data.model_data.model_handle);
                            cb_entry[j].cb_cnt = 25;
                            cb_entry[j].opcode = md->data.model_data.model_params->opcode_handlers[i*25+j].opcode.opcode;
                            cb_entry[j].company_id = md->data.model_data.model_params->opcode_handlers[i*25+j].opcode.company_id;
                            cb_entry[j].cb_index = j;
                            cb_entry[j].pv_cb = md->data.model_data.model_params->opcode_handlers[i*25+j].handler;
                            apv_cb_addr[j] = md->data.model_data.model_params->opcode_handlers[i*25+j].handler;
                        }
                        pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 25, NULL);
                        if (NULL != pt_nfy_tag)
                        {
                            for (j = 0; j < 25; j++)
                            {
                                cb_entry[j].pv_tag = pt_nfy_tag;
                                _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry[j]);
                            }
                        }
                        else
                        {
                            BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                        }
                        entry_cnt = entry_cnt - 25;
                    }
                    else
                    {
                        for (j = 0; j < entry_cnt; j++)
                        {
                            cb_entry[j].model_handle = *(md->data.model_data.model_handle);
                            cb_entry[j].cb_cnt = entry_cnt;
                            cb_entry[j].opcode = md->data.model_data.model_params->opcode_handlers[i*25+j].opcode.opcode;
                            cb_entry[j].company_id = md->data.model_data.model_params->opcode_handlers[i*25+j].opcode.company_id;
                            cb_entry[j].cb_index = j;
                            cb_entry[j].pv_cb = md->data.model_data.model_params->opcode_handlers[i*25+j].handler;
                            apv_cb_addr[j] = md->data.model_data.model_params->opcode_handlers[i*25+j].handler;
                        }
                        pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, j, NULL);
                        if (NULL != pt_nfy_tag)
                        {
                            for (j = 0; j < entry_cnt; j++)
                            {
                                cb_entry[j].pv_tag = pt_nfy_tag;
                                _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry[j]);
                            }
                        }
                        else
                        {
                            BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                        }
                    }
                }
            }
            break;
        }
        default:
            BT_RH_LOG("invalid opcode!!!");
            return RPCR_INV_ARGS;
    }

    if (TRUE == pt_return->u.i4_arg)
    {
        pt_return->u.i4_arg = BT_SUCCESS;
        return RPCR_OK;
    }
    else
    {
        return RPCR_ERROR;
    }
}

static INT32 _hndlr_x_mtkapi_bt_mesh_enable(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_enable , arg_1 = %p", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_enable(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_disable(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_disable , arg_1 = %d", pt_args[0].u.i4_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_disable();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_set_netkey(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_set_netkey , arg_1 = %p", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_netkey(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_set_appkey(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_set_appkey , arg_1 = %p", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_appkey(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static VOID _hndlr_x_mtkapi_bt_mesh_unprov_dev_scan(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_unprov_dev_scan , arg_1 = %d, arg_2 = %d", pt_args[0].u.b_arg, pt_args[1].u.ui4_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return;
    }

    pt_return->e_type   = ARG_TYPE_VOID;

    x_mtkapi_bt_mesh_unprov_dev_scan(pt_args[0].u.b_arg, pt_args[1].u.ui4_arg);

    return;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_invite_provisioning(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_invite_provisioning , arg_1 = %p, arg_2 = %p", pt_args[0].u.pui1_arg, pt_args[1].u.pv_desc);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_invite_provisioning(pt_args[0].u.pui1_arg, pt_args[1].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_start_provisioning(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_start_provisioning , arg_1 = %p, arg_2 = %d", pt_args[0].u.pv_desc, pt_args[1].u.ui1_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_start_provisioning(pt_args[0].u.pv_desc, pt_args[1].u.ui1_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_set_prov_factor(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_set_prov_factor , arg_1 = %p", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_prov_factor(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_model_cc_msg_tx(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_model_cc_msg_tx , arg_1 = %p", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_model_cc_msg_tx(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_send_packet(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_send_packet , arg_1 = %p", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_send_packet(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_send_packet_ex(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_send_packet , arg_1 = %p, arg_2 = %p", pt_args[0].u.pv_desc, pt_args[1].u.pv_desc);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_send_packet_ex(pt_args[0].u.pv_desc, pt_args[1].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_data_reset(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_data_reset , arg_1 = %d", pt_args[0].u.ui4_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_data_reset(pt_args[0].u.ui4_arg);

    if (TRUE == pt_return->u.i4_arg)
    {
        return RPCR_OK;
    }
    else
    {
        return RPCR_ERROR;
    }

}

static INT32 _hndlr_x_mtkapi_bt_mesh_data_save(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_data_save , arg_1 = %d", pt_args[0].u.i4_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_data_save();

    if (TRUE == pt_return->u.i4_arg)
    {
        return RPCR_OK;
    }
    else
    {
        return RPCR_ERROR;
    }
}

static INT32 _hndlr_x_mtkapi_bt_mesh_data_set(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_data_set , arg_1 = %p", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_data_set(pt_args[0].u.pv_desc);

    return RPCR_OK;

}

static INT32 _hndlr_x_mtkapi_bt_mesh_version(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_version , arg_1 = %s", pt_args[0].u.pc_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_version(pt_args[0].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_dump(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_dump , arg_1 = %d", pt_args[0].u.i4_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_VOID;
    x_mtkapi_bt_mesh_dump(pt_args[0].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_get_element_address(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_get_element_address , arg_1 = %d", pt_args[0].u.ui2_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_UINT16;
    pt_return->u.ui2_arg = x_mtkapi_bt_mesh_get_element_address(pt_args[0].u.ui2_arg);

    return RPCR_OK;
}
static INT32 _hndlr_x_mtkapi_bt_mesh_set_default_ttl(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_set_default_ttl , arg_1 = %d", pt_args[0].u.ui1_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_VOID;
    x_mtkapi_bt_mesh_set_default_ttl(pt_args[0].u.ui1_arg);

    return RPCR_OK;
}
static INT32 _hndlr_x_mtkapi_bt_mesh_get_default_ttl(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_get_default_ttl , arg_1 = %d", pt_args[0].u.i4_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_UINT8;
    pt_return->u.ui1_arg = x_mtkapi_bt_mesh_get_default_ttl();

    return RPCR_OK;
}
static INT32 _hndlr_x_mtkapi_bt_mesh_model_app_bind(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_model_app_bind , arg_1 = %d, arg_2 = %d", pt_args[0].u.ui2_arg, pt_args[1].u.ui2_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_model_app_bind(pt_args[0].u.ui2_arg, pt_args[1].u.ui2_arg);

    return RPCR_OK;
}
static INT32 _hndlr_x_mtkapi_bt_mesh_access_model_reply(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_access_model_reply , arg_1 = %d, arg_2 = %p, arg_3 = %p", pt_args[0].u.ui2_arg, pt_args[1].u.pv_desc, pt_args[2].u.pv_desc);

    if (ui4_num_args != 3)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_access_model_reply(pt_args[0].u.ui2_arg, \
                                                            (BT_MESH_ACCESS_MESSAGE_RX_T *)(pt_args[1].u.pv_desc), \
                                                            (BT_MESH_ACCESS_MESSAGE_TX_T *)(pt_args[2].u.pv_desc));

    return RPCR_OK;
}
static INT32 _hndlr_x_mtkapi_bt_mesh_bearer_adv_set_params(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_bearer_adv_set_params , arg_1 = %p, arg_2 = %p", pt_args[0].u.pv_desc, pt_args[1].u.pv_desc);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_bearer_adv_set_params((BT_MESH_BEARER_ADV_PARAMS_T *)(pt_args[0].u.pv_desc), \
                                                                (BT_MESH_BEARER_SCAN_PARAMS_T *)(pt_args[1].u.pv_desc));

    return RPCR_OK;
}
static INT32 _hndlr_x_mtkapi_bt_mesh_switch_adv(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_switch_adv , arg_1 = %d", pt_args[0].u.b_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_VOID;
    x_mtkapi_bt_mesh_switch_adv(pt_args[0].u.b_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_log_setlevel(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_log_setlevel , arg_1 = %d", pt_args[0].u.ui4_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_VOID;
    x_mtkapi_bt_mesh_log_setlevel(pt_args[0].u.ui4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_generate_uuid(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_generate_uuid , arg_1 = %p", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_generate_uuid(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_set_device_info(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_set_device_info , arg_1 = %p", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_device_info(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_set_mesh_mode(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_set_mesh_mode , arg_1 = %d", pt_args[0].u.i4_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_mesh_mode(pt_args[0].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_set_heartbeat_period(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_set_heartbeat_period , arg_1 = %d, arg_2 = %d", pt_args[0].u.ui1_arg, pt_args[1].u.ui4_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_heartbeat_period(pt_args[0].u.ui1_arg, pt_args[1].u.ui4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_ota_initiator_operation(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_MESH_OTA_OPERATION_PARAMS_T *params = (BT_MESH_OTA_OPERATION_PARAMS_T *)pt_args[0].u.pv_desc;
    BT_RH_LOG("[_hndlr_]bt_mesh_ota_initiator_operation , arg_1 = %p", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    switch (params->opcode)
    {
        case BT_MESH_OTA_INITIATOR_OP_REG_MSG_HANDLER:
        {
            UINT16 dist_client_handle = 0, update_client_handle = 0;
            BT_MESH_RPC_ACCESS_MSG_CB_ENTRY_T cb_entry;
            RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
            VOID * apv_cb_addr[1] = {0};

            BT_RH_LOG("BT_MESH_OTA_INITIATOR_OP_REG_MSG_HANDLER, pt_args[0].u.pv_desc = %p", pt_args[0].u.pv_desc);
            x_mtkapi_bt_mesh_ota_get_client_model_handle(&dist_client_handle, &update_client_handle);

            if(dist_client_handle != 0)
            {
                apv_cb_addr[0] = params->params.msg_handler.ota_msg_handler;
                pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);
                if (NULL != pt_nfy_tag)
                {
                    cb_entry.model_handle = dist_client_handle;
                    cb_entry.cb_cnt = 0;
                    cb_entry.opcode = 0;
                    cb_entry.company_id = 0;
                    cb_entry.cb_index = 0;
                    cb_entry.pv_cb = params->params.msg_handler.ota_msg_handler;
                    cb_entry.pv_tag = pt_nfy_tag;
                    _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                }
                else
                {
                    BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                }
            }
            if(update_client_handle != 0)
            {
                apv_cb_addr[0] = params->params.msg_handler.ota_msg_handler;
                pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1, NULL);
                if (NULL != pt_nfy_tag)
                {
                    cb_entry.model_handle = update_client_handle;
                    cb_entry.cb_cnt = 0;
                    cb_entry.opcode = 0;
                    cb_entry.company_id = 0;
                    cb_entry.cb_index = 0;
                    cb_entry.pv_cb = params->params.msg_handler.ota_msg_handler;
                    cb_entry.pv_tag = pt_nfy_tag;
                    _bt_mesh_rpc_add_access_msg_cb_entry(&cb_entry);
                }
                else
                {
                    BT_RH_LOG("%s create cb tag fail @ L %d",__func__, __LINE__);
                }
            }
            break;
        }
        default:
            BT_RH_LOG("%s invalid opcode %d @ L %d",__func__, params->opcode, __LINE__);
            break;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_ota_initiator_operation(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_set_special_pkt_params(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_MESH_SPECIAL_PKT_PARAMS_T *pkt_params = (BT_MESH_SPECIAL_PKT_PARAMS_T *)pt_args[0].u.pv_desc;
    BT_RH_LOG("[_hndlr_]bt_mesh_set_special_pkt_params , arg_1 = %p", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_set_special_pkt_params(pt_args[0].u.pv_desc);

    return RPCR_OK;
}


#ifdef MTK_GATT_BEARER_SUPPORT
static INT32 _hndlr_x_mtkapi_bt_mesh_gatt_connect(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_gatt_connect , arg_1 = %p, arg_2 = %d", pt_args[0].u.pv_desc, pt_args[1].u.i4_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_gatt_connect(pt_args[0].u.pv_desc, pt_args[1].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mesh_gatt_disconnect(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_mesh_gatt_disconnect , arg_1 = %d",pt_args[0].u.i4_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mesh_gatt_disconnect();

    return RPCR_OK;
}
#endif

INT32 c_rpc_reg_mtk_bt_service_mesh_op_hndlrs(VOID)
{
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_init);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_deinit);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_set_model_data);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_enable);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_disable);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_set_netkey);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_set_appkey);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_unprov_dev_scan);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_invite_provisioning);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_start_provisioning);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_set_prov_factor);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_model_cc_msg_tx);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_send_packet);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_send_packet_ex);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_data_reset);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_data_save);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_data_set);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_version);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_dump);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_get_element_address);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_set_default_ttl);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_get_default_ttl);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_model_app_bind);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_access_model_reply);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_bearer_adv_set_params);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_switch_adv);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_log_setlevel);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_generate_uuid);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_set_device_info);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_set_mesh_mode);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_set_heartbeat_period);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_ota_initiator_operation);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_set_special_pkt_params);
#ifdef MTK_GATT_BEARER_SUPPORT
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_gatt_connect);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_mesh_gatt_disconnect);
#endif

    return RPCR_OK;
}



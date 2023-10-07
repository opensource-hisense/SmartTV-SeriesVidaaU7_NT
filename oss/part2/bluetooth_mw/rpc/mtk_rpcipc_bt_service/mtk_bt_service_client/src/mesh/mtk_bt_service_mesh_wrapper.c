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

#include "mtk_bt_service_mesh_wrapper.h"
#include "mtk_bt_service_mesh_ipcrpc_struct.h"
#include "client_common.h"
#include "ri_common.h"
#include "u_bt_mw_mesh.h"

#define BT_RW_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("[Client]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static INT32 _hndlr_bt_app_mesh_bt_event_cbk(
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    void          *pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return)
{
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    if (((BT_MESH_BT_EVENT_T *)pt_args[0].u.pv_desc)->evt_id != BT_MESH_EVT_SEQ_CHANGE) //This event is too frequent and does not make much sense
    {
        BT_RW_LOG("pv_cb_addr = %p", pv_cb_addr);
    }
    pt_return->e_type   = ARG_TYPE_VOID;

    ((mtkrpcapi_BtAppMeshBtEventCbk)pv_cb_addr)((BT_MESH_BT_EVENT_T *)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

//access message callback for models
static INT32 _hndlr_bt_app_mesh_access_msg_cbk(
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    void          *pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return)
{
    BT_MESH_CBK_ACCESS_MSG_T * msg = (BT_MESH_CBK_ACCESS_MSG_T *)pt_args[0].u.pv_desc;
    BT_RW_LOG("bt_app_mesh_access_msg_cbk, pv_cb_addr = %p, model_handle = 0x%x", pv_cb_addr, msg->model_handle);

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    //call corresponding callback function based on model_handle and opcode

    ((bt_mesh_access_msg_handler)pv_cb_addr)((BT_MESH_CBK_ACCESS_MSG_T *)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

//event callback for health client model
static INT32 _hndlr_bt_app_mesh_health_client_evt_cbk(
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    void          *pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_mesh_health_client_evt_cbk, pv_cb_addr = %p", pv_cb_addr);

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;

    ((bt_mesh_health_client_evt_handler)pv_cb_addr)((BT_MESH_CBK_HEALTH_CLIENT_EVT_T *)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);

    return RPCR_OK;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_init (MTKRPCAPI_BT_APP_MESH_CB_FUNC_T *func, void *pv_tag)
{
    RPC_CLIENT_DECL(2, INT32);

    BT_RW_LOG("a_mtkapi_bt_mesh_init start\n");
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    func,
                    RPC_DESC_MTKRPCAPI_BT_APP_MESH_CB_FUNC_T,
                    NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, func);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_init");
    BT_RW_LOG("a_mtkapi_bt_mesh_init end\n");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);

}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_deinit (VOID)
{
    //Adding a dummy value for passing IPC/RPC, no other use
    INT32 i4_InvalidValue = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_deinit");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_set_model_data(BT_MESH_MODEL_DATA_T *md)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    md,
                    RPC_DESC_BT_MESH_MODEL_DATA_T,
                    NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    &(md->data),
                    RPC_DESC_BT_MESH_MODEL_DATA_T_data,
                    NULL));
    switch(md->opcode)
    {
        case BT_MESH_MD_OP_SET_COMPOSITION_DATA_HEADER:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.composition_data),
                            RPC_DESC_BT_MESH_COMPOSITION_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.composition_data.buf,
                            md->data.composition_data.buf_len));
            break;
        }
        case BT_MESH_MD_OP_ADD_ELEMENT:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.element),
                            RPC_DESC_BT_MESH_ELEMENT_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.element.element_index,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_CONFIGURATION_SERVER:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.configuration_server),
                            RPC_DESC_BT_MESH_CONFIGURATION_SERVER_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.configuration_server.model_handle,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_CONFIGURATION_CLIENT:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.configuration_client),
                            RPC_DESC_BT_MESH_CONFIGURATION_CLIENT_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.configuration_client.model_handle,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_HEALTH_SERVER:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.health_server),
                            RPC_DESC_BT_MESH_HEALTH_SERVER_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.health_server.model_handle,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_HEALTH_CLIENT:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.health_client),
                            RPC_DESC_BT_MESH_HEALTH_CLIENT_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.health_client.model_handle,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_GENERIC_ONOFF_SERVER:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.generic_onoff_server),
                            RPC_DESC_BT_MESH_GENERIC_ONOFF_SERVER_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.generic_onoff_server.model_handle,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_CTL_SETUP_SERVER:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.ctl_setup_server),
                            RPC_DESC_BT_MESH_CTL_SETUP_SERVER_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.ctl_setup_server.model_handle,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_GENERIC_POWER_ONOFF_CLIENT:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.generic_power_onoff_client),
                            RPC_DESC_BT_MESH_GENERIC_POWER_ONOFF_CLIENT_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.generic_power_onoff_client.model_handle,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_LIGHTNESS_CLIENT:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.lightness_client),
                            RPC_DESC_BT_MESH_LIGHTNESS_CLIENT_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.lightness_client.model_handle,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_CTL_CLIENT:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.ctl_client),
                            RPC_DESC_BT_MESH_CTL_CLIENT_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.ctl_client.model_handle,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_HSL_SETUP_SERVER:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.hsl_setup_server),
                            RPC_DESC_BT_MESH_CTL_SETUP_SERVER_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.hsl_setup_server.model_handle,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_HSL_CLIENT:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.hsl_client),
                            RPC_DESC_BT_MESH_CTL_CLIENT_DATA_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.hsl_client.model_handle,
                            2));
            break;
        }
        case BT_MESH_MD_OP_ADD_MODEL:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            &(md->data.model_data),
                            RPC_DESC_BT_MESH_MODEL_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                            md->data.model_data.model_handle,
                            2));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                            md->data.model_data.model_params,
                            RPC_DESC_BT_MESH_MODEL_ADD_PARAMS_T,
                            NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc_arr(RPC_DEFAULT_ID,
                            md->data.model_data.model_params->opcode_count,
                            md->data.model_data.model_params->opcode_handlers,
                            RPC_DESC_BT_MESH_ACCESS_OPCODE_HANDLER_T,
                            NULL));
            break;
        }
        default:
            BT_RW_LOG("a_mtkapi_bt_mesh_set_model_data, no this opcode\n");
            break;

    }
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, md);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_set_model_data");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_enable (const BT_MESH_INIT_PARAMS_T *init_params)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    init_params,
                    RPC_DESC_BT_MESH_INIT_PARAMS_T,
                    NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    init_params->provisionee,
                    RPC_DESC_BT_MESH_PROV_PROVISIONEE_PARAMS_T,
                    NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    init_params->config,
                    RPC_DESC_BT_MESH_CONFIG_INIT_PARAMS_T,
                    NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    init_params->friend_params,
                    RPC_DESC_BT_MESH_FRIEND_INIT_PARAMS_T,
                    NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    init_params->debug,
                    RPC_DESC_BT_MESH_DEBUG_INIT_PARAMS_T,
                    NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    init_params->debug,
                    RPC_DESC_BT_MESH_CUSTOMIZE_PARA_T,
                    NULL));
    if (NULL != init_params->config->uri)
    {
        RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                        init_params->config->uri,
                        strlen(init_params->config->uri) + 1));
    }
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, init_params);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_enable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_disable (VOID)
{
    //Adding a invalid value for passing IPC/RPC, no other use
    INT32 i4_InvalidValue = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_disable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_set_netkey(BT_MESH_NETKEY_T *netkey)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    netkey,
                    RPC_DESC_BT_MESH_NETKEY_T,
                    NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                    netkey->network_key,
                    BT_MESH_KEY_SIZE));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, netkey);
    BT_RW_LOG("a_mtkapi_bt_mesh_set_netkey, netkey->network_key = [%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]",
               netkey->network_key[0], netkey->network_key[1], netkey->network_key[2], netkey->network_key[3],
               netkey->network_key[4], netkey->network_key[5], netkey->network_key[6], netkey->network_key[7],
               netkey->network_key[8], netkey->network_key[9], netkey->network_key[10], netkey->network_key[11],
               netkey->network_key[12], netkey->network_key[13], netkey->network_key[14], netkey->network_key[15]);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_set_netkey");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_set_appkey(BT_MESH_APPKEY_T *appkey)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    appkey,
                    RPC_DESC_BT_MESH_APPKEY_T,
                    NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                    appkey->application_key,
                    BT_MESH_KEY_SIZE));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, appkey);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_set_appkey");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL VOID a_mtkapi_bt_mesh_unprov_dev_scan(BOOL start, UINT32 duration)
{
    RPC_CLIENT_DECL_VOID(2);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, start);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT32, duration);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_unprov_dev_scan");
    RPC_RETURN_VOID;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_invite_provisioning(const UINT8 *target_uuid, const BT_MESH_PROV_INVITE_T *invite)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                    target_uuid,
                    BT_MESH_UUID_SIZE));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, target_uuid);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    invite,
                    RPC_DESC_BT_MESH_PROV_INVITE_T,
                    NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, invite);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_invite_provisioning");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_start_provisioning(const BT_MESH_PROV_PROVISIONER_PARAMS_T *params, UINT8 mode)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    params,
                    RPC_DESC_BT_MESH_PROV_PROVISIONER_PARAMS_T,
                    NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, params);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, mode);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_start_provisioning");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL extern INT32 a_mtkapi_bt_mesh_set_prov_factor(const BT_MESH_PROV_FACTOR_T *factor)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    factor,
                    RPC_DESC_BT_MESH_PROV_FACTOR_T,
                    NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                    factor->buf,
                    factor->buf_len));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, factor);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_set_prov_factor");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_model_cc_msg_tx(const BT_MESH_CONFIGURATION_MSG_TX_T *msg)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    msg,
                    RPC_DESC_BT_MESH_CONFIGURATION_MSG_TX_T,
                    NULL));
    switch (msg->opcode)
    {
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_SET:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_VIRTUAL_ADDRESS_SET:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(msg->data.model_pub_set), RPC_DESC_BT_MESH_CONFIG_MODEL_PUB_SET_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->data.model_pub_set.state, sizeof(BT_MESH_MODEL_PUBLICATION_STATE_T)));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(msg->data.model_pub_set.state->publish_address), RPC_DESC_BT_MESH_ADDRESS_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->data.model_pub_set.state->publish_address.virtual_uuid, BT_MESH_UUID_SIZE));
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_ADD:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_ADD:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(msg->data.model_sub_add.address), RPC_DESC_BT_MESH_ADDRESS_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->data.model_sub_add.address.virtual_uuid, BT_MESH_UUID_SIZE));
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_DELETE:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_DELETE:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(msg->data.model_sub_del.address), RPC_DESC_BT_MESH_ADDRESS_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->data.model_sub_del.address.virtual_uuid, BT_MESH_UUID_SIZE));
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE:
        case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_OVERWRITE:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(msg->data.model_sub_ow.address), RPC_DESC_BT_MESH_ADDRESS_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->data.model_sub_ow.address.virtual_uuid, BT_MESH_UUID_SIZE));
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_ADD:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(msg->data.netkey_add), RPC_DESC_BT_MESH_CONFIG_NETKEY_ADD_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->data.netkey_add.netkey, BT_MESH_KEY_SIZE));
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_NETKEY_UPDATE:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(msg->data.netkey_update), RPC_DESC_BT_MESH_CONFIG_NETKEY_UPDATE_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->data.netkey_update.netkey, BT_MESH_KEY_SIZE));
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_ADD:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(msg->data.appkey_add), RPC_DESC_BT_MESH_CONFIG_APPKEY_ADD_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->data.appkey_add.appkey, BT_MESH_KEY_SIZE));
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_APPKEY_UPDATE:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(msg->data.appkey_update), RPC_DESC_BT_MESH_CONFIG_APPKEY_UPDATE_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->data.appkey_update.appkey, BT_MESH_KEY_SIZE));
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_SET:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(msg->data.hb_pub_set), RPC_DESC_BT_MESH_CONFIG_HB_PUB_SET_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->data.hb_pub_set.publication, sizeof(BT_MESH_HEARTBEAT_PUBLICATION_T)));
            break;
        }
        case BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_SET:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(msg->data.hb_sub_set), RPC_DESC_BT_MESH_CONFIG_HB_SUB_SET_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->data.hb_sub_set.subscription, sizeof(BT_MESH_HEARTBEAT_SUBSCRIPTION_T)));
            break;
        }
        default:
            break;
    }
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, msg);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_model_cc_msg_tx");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_send_packet(const BT_MESH_TX_PARAMS_T *params)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    params,
                    RPC_DESC_BT_MESH_TX_PARAMS_T,
                    NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                    params->data,
                    params->data_len));
    if (NULL != params->security.device_key)
    {
        RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, params->security.device_key, BT_MESH_KEY_SIZE));
    }
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, params);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_send_packet");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_send_packet_ex(const BT_MESH_TX_PARAMS_T *params, BT_MESH_MODEL_OPERATION_T *model_op)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    params,
                    RPC_DESC_BT_MESH_TX_PARAMS_T,
                    NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID,
                    params->data,
                    params->data_len));
    if (NULL != params->security.device_key)
    {
        RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, params->security.device_key, BT_MESH_KEY_SIZE));
    }

    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    model_op,
                    RPC_DESC_BT_MESH_MODEL_OPERATION_T,
                    NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, params);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, model_op);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_send_packet_ex");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_data_reset(UINT32 record)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT32, record);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_data_reset");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_data_save(VOID)
{
    //Adding a invalid value for passing IPC/RPC, no other use
    INT32 i4_InvalidValue = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_data_save");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_data_set(BT_MESH_RECORD_T *mesh_data)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                     mesh_data,
                     RPC_DESC_BT_MESH_RECORD_T,
                     NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, mesh_data);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_data_set");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_version(CHAR *buf)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, buf, BT_MESH_VERSION_LEN));
    RPC_CLIENT_ARG_OUT(ARG_TYPE_REF_DESC, buf);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_version");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_dump(BT_MESH_DUMP_TYPE_T type)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, type);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_dump");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL UINT16 a_mtkapi_bt_mesh_get_element_address(UINT16 element_index)
{
    RPC_CLIENT_DECL(1, UINT16);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT16, element_index);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_get_element_address");
    RPC_CLIENT_RETURN(ARG_TYPE_UINT16, 0);//There is no special invaid address defined by SPEC. This API is considered to be always successful. RPC always returns RPC_OK.
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_set_default_ttl(UINT8 def_ttl)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, def_ttl);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_set_default_ttl");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL UINT8 a_mtkapi_bt_mesh_get_default_ttl(VOID)
{
    //Adding a dummy value for passing IPC/RPC, no other use
    INT32 i4_InvalidValue = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_get_default_ttl");
    RPC_CLIENT_RETURN(ARG_TYPE_UINT8, 0xFF);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_model_app_bind(UINT16 model_handle, UINT16 appkey_index)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT16, model_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT16, appkey_index);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_model_app_bind");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_access_model_reply(UINT16 model_handle, const BT_MESH_ACCESS_MESSAGE_RX_T *msg, const BT_MESH_ACCESS_MESSAGE_TX_T *reply)
{
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, msg, RPC_DESC_BT_MESH_ACCESS_MESSAGE_RX_T, NULL));
    if (msg->buf_len > 0)
    {
        RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, msg->buf, msg->buf_len));
    }
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, reply, RPC_DESC_BT_MESH_ACCESS_MESSAGE_TX_T, NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, reply->p_buffer, reply->length));
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT16, model_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, msg);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, reply);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_access_model_reply");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_bearer_adv_set_params(const BT_MESH_BEARER_ADV_PARAMS_T *adv_params, const BT_MESH_BEARER_SCAN_PARAMS_T *scan_params)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, adv_params, RPC_DESC_BT_MESH_BEARER_ADV_PARAMS_T, NULL));
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, scan_params, RPC_DESC_BT_MESH_BEARER_SCAN_PARAMS_T, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, adv_params);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, scan_params);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_bearer_adv_set_params");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_switch_adv(BOOL enable)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, enable);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_switch_adv");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL VOID a_mtkapi_bt_mesh_log_setlevel(UINT32 level)
{
    RPC_CLIENT_DECL_VOID(1);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT32, level);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_log_setlevel");
    RPC_RETURN_VOID;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_generate_uuid(UINT8 *uuid_buf)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, uuid_buf, BT_MESH_UUID_SIZE));
    RPC_CLIENT_ARG_OUT(ARG_TYPE_REF_DESC, uuid_buf);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_generate_uuid");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_set_device_info(BT_MESH_DEVICE_INFO_T *info)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    info,
                    RPC_DESC_BT_MESH_DEVICE_INFO_T,
                    NULL));
    switch (info->opcode)
    {
        case BT_MESH_DEV_INFO_OP_ADD_DEVKEY:
        case BT_MESH_DEV_INFO_OP_GET_DEVKEY:
        case BT_MESH_DEV_INFO_OP_DELETE_DEVKEY:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    &(info->data),
                    RPC_DESC_BT_MESH_DEVKEY_INFO_T,
                    NULL));
        }
        default:
            break;
    }
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, info);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_set_device_info");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_set_mesh_mode(BT_MESH_MODE_T mode)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, mode);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_set_mesh_mode");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_set_heartbeat_period(UINT8 num, UINT32 hb_timeout)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, num);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT32, hb_timeout);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_set_heartbeat_period");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_ota_initiator_operation(const BT_MESH_OTA_OPERATION_PARAMS_T *params)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    params,
                    RPC_DESC_BT_MESH_OTA_OPERATION_PARAMS_T,
                    NULL));
    switch (params->opcode)
    {
        case BT_MESH_OTA_INITIATOR_OP_START_DISTRIBUTION:
        {
            RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, &(params->params.start_params), RPC_DESC_BT_MESH_OTA_INITIATOR_START_PARAMS_T, NULL));
            RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, \
                            params->params.start_params.updaters, \
                            params->params.start_params.updaters_num * 2));
            break;
        }
        default:
            break;
    }
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, params);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_ota_initiator_operation");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_set_special_pkt_params(const BT_MESH_SPECIAL_PKT_PARAMS_T *pkt_params)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pkt_params, RPC_DESC_BT_MESH_SPECIAL_PKT_PARAMS_T, NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, pkt_params);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_set_special_pkt_params");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

#ifdef MTK_GATT_BEARER_SUPPORT
EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_gatt_connect(BT_MESH_BLE_ADDR_T *addr, BT_MESH_GATT_SERVICE_T type)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    addr,
                    RPC_DESC_BT_MESH_BLE_ADDR_T,
                    NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, type);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_gatt_connect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mesh_gatt_disconnect(VOID)
{
    INT32 i4_InvalidValue = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mesh_gatt_disconnect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}
#endif

INT32 c_rpc_reg_mtk_bt_service_mesh_cb_hndlrs(void)
{
    int i4_ret = RPCR_OK;
    RPC_REG_CB_HNDLR(bt_app_mesh_bt_event_cbk);
    RPC_REG_CB_HNDLR(bt_app_mesh_access_msg_cbk);
    RPC_REG_CB_HNDLR(bt_app_mesh_health_client_evt_cbk);
    return i4_ret;
}


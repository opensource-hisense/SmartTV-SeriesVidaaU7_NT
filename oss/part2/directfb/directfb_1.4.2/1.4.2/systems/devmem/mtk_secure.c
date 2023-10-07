/*----------------------------------------------------------------------------*
 * Copyright Statement:                                                       *
 *                                                                            *
 *   This software/firmware and related documentation ("MediaTek Software")   *
 * are protected under international and related jurisdictions'copyright laws *
 * as unpublished works. The information contained herein is confidential and *
 * proprietary to MediaTek Inc. Without the prior written permission of       *
 * MediaTek Inc., any reproduction, modification, use or disclosure of        *
 * MediaTek Software, and information contained herein, in whole or in part,  *
 * shall be strictly prohibited.                                              *
 * MediaTek Inc. Copyright (C) 2010. All rights reserved.                     *
 *                                                                            *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND     *
 * AGREES TO THE FOLLOWING:                                                   *
 *                                                                            *
 *   1)Any and all intellectual property rights (including without            *
 * limitation, patent, copyright, and trade secrets) in and to this           *
 * Software/firmware and related documentation ("MediaTek Software") shall    *
 * remain the exclusive property of MediaTek Inc. Any and all intellectual    *
 * property rights (including without limitation, patent, copyright, and      *
 * trade secrets) in and to any modifications and derivatives to MediaTek     *
 * Software, whoever made, shall also remain the exclusive property of        *
 * MediaTek Inc.  Nothing herein shall be construed as any transfer of any    *
 * title to any intellectual property right in MediaTek Software to Receiver. *
 *                                                                            *
 *   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its *
 * representatives is provided to Receiver on an "AS IS" basis only.          *
 * MediaTek Inc. expressly disclaims all warranties, expressed or implied,    *
 * including but not limited to any implied warranties of merchantability,    *
 * non-infringement and fitness for a particular purpose and any warranties   *
 * arising out of course of performance, course of dealing or usage of trade. *
 * MediaTek Inc. does not provide any warranty whatsoever with respect to the *
 * software of any third party which may be used by, incorporated in, or      *
 * supplied with the MediaTek Software, and Receiver agrees to look only to   *
 * such third parties for any warranty claim relating thereto.  Receiver      *
 * expressly acknowledges that it is Receiver's sole responsibility to obtain *
 * from any third party all proper licenses contained in or delivered with    *
 * MediaTek Software.  MediaTek is not responsible for any MediaTek Software  *
 * releases made to Receiver's specifications or to conform to a particular   *
 * standard or open forum.                                                    *
 *                                                                            *
 *   3)Receiver further acknowledge that Receiver may, either presently       *
 * and/or in the future, instruct MediaTek Inc. to assist it in the           *
 * development and the implementation, in accordance with Receiver's designs, *
 * of certain softwares relating to Receiver's product(s) (the "Services").   *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MediaTek Inc. with respect  *
 * to the Services provided, and the Services are provided on an "AS IS"      *
 * basis. Receiver further acknowledges that the Services may contain errors  *
 * that testing is important and it is solely responsible for fully testing   *
 * the Services and/or derivatives thereof before they are used, sublicensed  *
 * or distributed. Should there be any third party action brought against     *
 * MediaTek Inc. arising out of or relating to the Services, Receiver agree   *
 * to fully indemnify and hold MediaTek Inc. harmless.  If the parties        *
 * mutually agree to enter into or continue a business relationship or other  *
 * arrangement, the terms and conditions set forth herein shall remain        *
 * effective and, unless explicitly stated otherwise, shall prevail in the    *
 * event of a conflict in the terms in any agreements entered into between    *
 * the parties.                                                               *
 *                                                                            *
 *   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and    *
 * cumulative liability with respect to MediaTek Software released hereunder  *
 * will be, at MediaTek Inc.'s sole discretion, to replace or revise the      *
 * MediaTek Software at issue.                                                *
 *                                                                            *
 *   5)The transaction contemplated hereunder shall be construed in           *
 * accordance with the laws of Singapore, excluding its conflict of laws      *
 * principles.  Any disputes, controversies or claims arising thereof and     *
 * related thereto shall be settled via arbitration in Singapore, under the   *
 * then current rules of the International Chamber of Commerce (ICC).  The    *
 * arbitration shall be conducted in English. The awards of the arbitration   *
 * shall be final and binding upon both parties and shall be entered and      *
 * enforceable in any court of competent jurisdiction.                        *
 *---------------------------------------------------------------------------*/

 #include <directfb.h>
#include <misc/conf.h>
#include <direct/debug.h>
 
#include "tee_client_api.h"
#include "utpa2_XC.h"


TEEC_Session m_session;
TEEC_Context m_context;
const TEEC_UUID XCTA_UUID = {0x4dd53ca0, 0x0248, 0x11e6, \
                            {0x86, 0xc0, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b} };

static const char* gtzDevname = "opteearmtz00";
static u32 pipeline_id = 0;
static u32 secure_buffer_cnt = 0;


void deinitTee()
{
    TEEC_CloseSession(&m_session);
    TEEC_FinalizeContext(&m_context);
}

DFBResult InitTee()
{
    u32 ret = 0;

    //----Initial context
    memset(&m_context, 0, sizeof(TEEC_Context));
    if (TEEC_SUCCESS != TEEC_InitializeContext(gtzDevname, &m_context)) {
            D_ERROR("[DFB][%s] TEEC_InitializeContext fail\n", __FUNCTION__);
            goto error;
    }

    //---- Open session
    memset(&m_session, 0, sizeof(TEEC_Session));
    if (TEEC_SUCCESS != TEEC_OpenSession(&m_context, &m_session, &XCTA_UUID,
            TEEC_LOGIN_PUBLIC, NULL, NULL, &ret)) {
            D_ERROR("[DFB][%s] TEEC_OpenSession fail\n", __FUNCTION__);
            goto error;
    }

    //secure_buffer_cnt = 0;
    return DFB_OK;
error:
    deinitTee();
    return DFB_FAILURE;
}

DFBResult _dfb_secure_create_pipeline(u32 *u32CompPipelineID)
{
    XC_STI_OPTEE_HANDLER optee_handler;
    TEEC_Operation op = {0};
    u32 ret_orig;
    memset(&optee_handler, 0, sizeof(XC_STI_OPTEE_HANDLER));
    memset(&op, 0, sizeof(TEEC_Operation));

    /* create disp pipeline */
    optee_handler.u16Version = XC_STI_OPTEE_HANDLER_VERSION;
    optee_handler.u16Length = sizeof(XC_STI_OPTEE_HANDLER);
    optee_handler.enAID = E_XC_STI_AID_S;
    optee_handler.enCompHWIP = E_XC_COMP_GE;
    op.params[0].tmpref.buffer = (void *)&optee_handler;
    op.params[0].tmpref.size = sizeof(XC_STI_OPTEE_HANDLER);
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,TEEC_NONE, TEEC_NONE, TEEC_NONE);

    if (TEEC_SUCCESS == TEEC_InvokeCommand(&m_session, E_XC_OPTEE_CREATE_COMP_PIPELINE, &op,&ret_orig))
    {
        *u32CompPipelineID = optee_handler.u32CompPipelineID;
        return DFB_OK;
    }

    return DFB_FAILURE;
}

DFBResult _dfb_secure_release_pipeline(u32 *u32CompPipelineID)
{
    XC_STI_OPTEE_HANDLER optee_handler;
    TEEC_Operation op = {0};
    u32 ret_orig;
    memset(&optee_handler, 0, sizeof(XC_STI_OPTEE_HANDLER));
    memset(&op, 0, sizeof(TEEC_Operation));

    /* create disp pipeline */
    optee_handler.u16Version = XC_STI_OPTEE_HANDLER_VERSION;
    optee_handler.u16Length = sizeof(XC_STI_OPTEE_HANDLER);
    optee_handler.enAID = E_XC_STI_AID_S;
    optee_handler.enCompHWIP = E_XC_COMP_GE;
    optee_handler.u32CompPipelineID = *u32CompPipelineID;
    op.params[0].tmpref.buffer = (void *)&optee_handler;
    op.params[0].tmpref.size = sizeof(XC_STI_OPTEE_HANDLER);
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,TEEC_NONE, TEEC_NONE, TEEC_NONE);

    if (TEEC_SUCCESS == TEEC_InvokeCommand(&m_session, E_XC_OPTEE_DESTROY_COMP_PIPELINE, &op,&ret_orig))
    {
        *u32CompPipelineID = 0;
        pipeline_id = 0;
        return DFB_OK;
    }

    return DFB_FAILURE;
}

DFBResult _dfb_secure_get_pipeline_id (u32 *u32CompPipelineID)
{
    if (pipeline_id == 0) {
        // init tee.
        DBG_SECURE_MSG("[DFB] %s, InitTee()\n", __FUNCTION__);
        if(InitTee() != DFB_OK)
        {
            D_ERROR("[DFB][%s] InitTee failed!\n", __FUNCTION__);
            return DFB_FAILURE;
        }
        // get pipe line id.
        DBG_SECURE_MSG("[DFB] %s, create pipeline()\n", __FUNCTION__);
        if (DFB_OK != _dfb_secure_create_pipeline(&pipeline_id))
        {
             D_ERROR("[DFB][%s] _dfb_secure_create_pipeline failed!\n", __FUNCTION__);
             deinitTee();
             return DFB_FAILURE;
        }
    }
    DBG_SECURE_MSG("[DFB] %s, pipeline_id = %x\n", __FUNCTION__, pipeline_id);
    *u32CompPipelineID = pipeline_id;

    return DFB_OK;
}

void _dfb_secure_buf_lock()
{
    secure_buffer_cnt++;
    DBG_SECURE_MSG("[DFB] %s, secure buffer cnt : %d\n", __FUNCTION__, secure_buffer_cnt);
}

void _dfb_secure_buf_unlock()
{
    if (secure_buffer_cnt > 0) {
        secure_buffer_cnt--;
        DBG_SECURE_MSG("[DFB] %s, secure buffer cnt : %d\n", __FUNCTION__, secure_buffer_cnt);

        if (secure_buffer_cnt == 0) {
            DBG_SECURE_MSG("[DFB] %s, reset pipeline id\n", __FUNCTION__);
            pipeline_id = 0;
        }
    }
}


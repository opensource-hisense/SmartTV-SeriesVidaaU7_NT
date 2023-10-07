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
/* FILE NAME:  linuxbt_a2dp_sink_if.c
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 *
 *
 */
/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/un.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "bluetooth.h"
#include "bt_mw_common.h"
#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"
#include "bt_mw_a2dp_common.h"
#include "linuxbt_a2dp_common_if.h"

#include "mtk_bluetooth.h"
#if defined(MTK_LINUX_A2DP_PLUS) && (MTK_LINUX_A2DP_PLUS == TRUE)
#include "mtk_bt_av.h"
#else
#include "bt_av.h"
#endif
#include "c_mw_config.h"

#include "bt_mw_message_queue.h"

#define A2D_MEDIA_CT_SBC        0x00    /* SBC media codec type */
#define A2D_MEDIA_CT_M12        0x01    /* MPEG-1, 2 Audio media codec type */
#define A2D_MEDIA_CT_M24        0x02    /* MPEG-2, 4 AAC media codec type */
#define A2D_MEDIA_CT_STE        0x03    /*Stereo media codec type*/
#define A2D_MEDIA_CT_NON        0xFF    /* currently it means lhdc */

// [Octet 0-3] Vendor ID
#define LINUX_BT_A2DP_LHDC_VENDOR_ID 0x0000053A
// [Octet 4-5] Vendor Specific Codec ID
#define LINUX_BT_A2DP_LHDC_CODEC_ID 0x4C32
#define LINUX_BT_A2DP_LHDC_LL_CODEC_ID 0x4C4C


#define LINUXBT_A2DP_COMMON_SET_MSG_LEN(msg) do{   \
    msg.hdr.len = sizeof(tBT_MW_A2DP_MSG);      \
    }while(0)

#if 0
static INT32 linuxbt_a2dp_convert_aac_codec(tA2D_AAC_CIE *tmp_aac_config, BT_A2DP_AAC_CONF *aac_config)
{
    UINT32 tmp_bitrate = 0;

    if ((NULL == tmp_aac_config) || (NULL == aac_config))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "aac config NULL");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (AAC_MPEG2_LC & tmp_aac_config->object_type)
    {
        aac_config->object_type = BT_A2DP_AAC_OBJ_TYPE_MPEG2_LC;
    }
    else if (AAC_MPEG4_LC & tmp_aac_config->object_type)
    {
        aac_config->object_type = BT_A2DP_AAC_OBJ_TYPE_MPEG4_LC;
    }
    else if (AAC_MPEG4_LTP & tmp_aac_config->object_type)
    {
        aac_config->object_type = BT_A2DP_AAC_OBJ_TYPE_MPEG4_LTP;
    }
    else
    {
        aac_config->object_type = BT_A2DP_AAC_OBJ_TYPE_MPEG4_SCA;
    }

    if (tmp_aac_config->samp_freq == AAC_SAMPLE_FREQ_48k)
    {
        aac_config->samp_freq = 48000;
    }
    else
    {
        aac_config->samp_freq = 44100;
    }

    if (tmp_aac_config->channels & AAC_CHANNEL_1)
    {
        aac_config->channels = 1;
    }
    else
    {
        aac_config->channels = 2;
    }

    aac_config->vbr = (tmp_aac_config->bit_rate_high & 0x80) ? TRUE : FALSE;
    tmp_bitrate  = (tmp_aac_config->bit_rate_high & ~0x80) << 16;
    tmp_bitrate +=  tmp_aac_config->bit_rate_mid  << 8;
    tmp_bitrate +=  tmp_aac_config->bit_rate_low;
    aac_config->bitrate = tmp_bitrate;

    return BT_SUCCESS;
}

static INT32 linuxbt_a2dp_convert_lhdc_codec(tA2D_LHDC_CIE *codec_config,
    BT_A2DP_LHDC_CONF *lhdc_config)
{
    if ((NULL == codec_config) || (NULL == lhdc_config))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "lhdc config NULL");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (codec_config->samp_freq & LHDC_SAMPLE_FREQ_96k)
    {
        lhdc_config->samp_freq = 96000;
    }
    else if (codec_config->samp_freq & LHDC_SAMPLE_FREQ_88k)
    {
        lhdc_config->samp_freq = 88200;
    }
    else if (codec_config->samp_freq & LHDC_SAMPLE_FREQ_48k)
    {
        lhdc_config->samp_freq = 48000;
    }
    else
    {
        lhdc_config->samp_freq = 44100;
    }

    if (codec_config->bit_depth & LHDC_BITDEPTH_24)
    {
        lhdc_config->bit_depth = 24;
    }
    else
    {
        lhdc_config->bit_depth = 16;
    }

    lhdc_config->obj_type = codec_config->obj_type;

    if (0 == codec_config->max_rate)
    {
        lhdc_config->max_rate = 900;
    }
    else if (1 == codec_config->max_rate)
    {
        lhdc_config->max_rate = 560;
    }
    else if (2 == codec_config->max_rate)
    {
        lhdc_config->max_rate = 400;
    }

    if ((codec_config->channels == 0)
        || (codec_config->channels & LHDC_CHANNEL_STEREO))
    {
        lhdc_config->channels = 2;
    }
    else if (codec_config->channels & LHDC_CHANNEL_MONO)
    {
        lhdc_config->channels = 1;
    }
    else if (codec_config->channels & LHDC_CHANNEL_CH5_1)
    {
        lhdc_config->channels = 6;
    }

    lhdc_config->compressor_output_fmt = codec_config->compressor_output_fmt;

    return BT_SUCCESS;
}

static INT32 linuxbt_a2dp_convert_vendor_codec(UINT8 *codec_config, BT_A2DP_CONF *config)
{
    tA2D_VENDOR_CIE *vendor_cie = NULL;

    if ((NULL == codec_config) || (NULL == config))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "vendor config NULL");
        return BT_ERR_STATUS_NOT_READY;
    }
    vendor_cie = (tA2D_VENDOR_CIE *)codec_config;

    config->codec_conf.vendor_conf.vendor_id = vendor_cie->vendor_id;
    config->codec_conf.vendor_conf.codec_id = vendor_cie->codec_id;
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "vendor_id 0x%x, codec_id 0x%x",
        config->codec_conf.vendor_conf.vendor_id,
        config->codec_conf.vendor_conf.codec_id);

    switch(vendor_cie->vendor_id)
    {
    case LINUX_BT_A2DP_LHDC_VENDOR_ID:
        if (vendor_cie->codec_id == LINUX_BT_A2DP_LHDC_CODEC_ID)
        {
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "codec is lhdc");
            config->codec_type = BT_A2DP_CODEC_TYPE_LHDC;
        }
        else if (vendor_cie->codec_id == LINUX_BT_A2DP_LHDC_LL_CODEC_ID)
        {
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "codec is lhdc-ll");
            config->codec_type = BT_A2DP_CODEC_TYPE_LHDC_LL;
        }
        linuxbt_a2dp_convert_lhdc_codec((tA2D_LHDC_CIE*)codec_config,
            &config->codec_conf.vendor_conf.conf.lhdc_conf);
        break;
    default:
        break;
    }


    return BT_SUCCESS;
}

static INT32 linuxbt_a2dp_convert_sbc_codec(tA2D_SBC_CIE *tmp_sbc_config, BT_A2DP_SBC_CONF *sbc_config)
{
    if ((NULL == tmp_sbc_config) || (NULL == sbc_config))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, " sbc config NULL");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (tmp_sbc_config->samp_freq & A2D_SBC_IE_SAMP_FREQ_44)
    {
        sbc_config->samp_freq = 44100;
    }
    else
    {
        sbc_config->samp_freq = 48000;
    }

    if (tmp_sbc_config->num_subbands & A2D_SBC_IE_SUBBAND_4)
    {
        sbc_config->num_subbands = 4;
    }
    else
    {
        sbc_config->num_subbands = 8;
    }

    if (tmp_sbc_config->ch_mode & A2D_SBC_IE_CH_MD_MONO)
    {
        sbc_config->ch_mode = BT_A2DP_CHANNEL_MODE_MONO;
    }
    else if (tmp_sbc_config->ch_mode & A2D_SBC_IE_CH_MD_DUAL)
    {
        sbc_config->ch_mode = BT_A2DP_CHANNEL_MODE_DUAL;
    }
    else if (tmp_sbc_config->ch_mode & A2D_SBC_IE_CH_MD_STEREO)
    {
        sbc_config->ch_mode = BT_A2DP_CHANNEL_MODE_STEREO;
    }
    else
    {
        sbc_config->ch_mode = BT_A2DP_CHANNEL_MODE_JOINT;
    }

    if (tmp_sbc_config->block_len & A2D_SBC_IE_BLOCKS_4)
    {
        sbc_config->block_len = 4;
    }
    else if (tmp_sbc_config->block_len & A2D_SBC_IE_BLOCKS_8)
    {
        sbc_config->block_len = 8;
    }
    else if (tmp_sbc_config->block_len & A2D_SBC_IE_BLOCKS_12)
    {
        sbc_config->block_len = 12;
    }
    else
    {
        sbc_config->block_len = 16;
    }

    if (tmp_sbc_config->alloc_mthd & A2D_SBC_IE_ALLOC_MD_S)
    {
        sbc_config->alloc_mthd = 0;
    }
    else
    {
        sbc_config->alloc_mthd = 1;
    }

    sbc_config->min_bitpool =   tmp_sbc_config->min_bitpool;
    sbc_config->max_bitpool =   tmp_sbc_config->max_bitpool;

    return BT_SUCCESS;
}
#endif

#if 0 //need refactor
INT32 linuxbt_a2dp_ext_init(VOID)
{
    INT32 ret = 0;

    g_bt_a2dp_ext_interface = (btav_ext_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_EXT_ID);
    if (NULL == g_bt_a2dp_ext_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "Failed to get A2DP extended interface");
        return -1;
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "get A2DP extended interface successfully");
    }
    // Init A2DP SRC ext interface
    ret = g_bt_a2dp_ext_interface->init(&g_bt_a2dp_ext_callbacks);
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "init A2DP extended interface successfully");
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "Fail to init A2DP extended interface");
    }

    return linuxbt_return_value_convert(ret);
}
#endif

INT32 linuxbt_a2dp_change_thread_priority_handler(INT32 priority)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "priority[%d]", priority);

    #if 0 //need refactor
    if (g_bt_a2dp_ext_interface != NULL)
    {
        g_bt_a2dp_ext_interface->change_thread_priority(priority);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP ext interface");
    }
    #endif

    return BT_SUCCESS;
}


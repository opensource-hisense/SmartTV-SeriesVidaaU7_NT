/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <string.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <experimental/optional>

#include "bt_mw_common.h"
#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"
#include "linuxbt_leaudio_bmr_if.h"
#include "bt_mw_message_queue.h"
#include "bluetooth.h"
#include "bt_le_audio.h"
#include "bt_mw_leaudio_bmr.h"
#include "u_bt_mw_leaudio_bmr.h"

using bluetooth::le_audio::LeAudioBroadcastReceiverInterface;
using bluetooth::le_audio::LeAudioBroadcastReceiverCallbacks;
using bluetooth::le_audio::LeAudioBroadcastAudioSource;
using bluetooth::le_audio::LeAudioBroadcastReceiverInitParams;
using bluetooth::le_audio::ConnectionState;
using bluetooth::le_audio::BasicAudioAnnouncementSubgroup_t;
using bluetooth::le_audio::BasicAudioAnnouncementBisConfig_t;

static bool b_close_done = false;
static std::mutex close_mtx;
static std::condition_variable close_cv;

class LeAudioBroadcastReceiverCallbacksImpl : public LeAudioBroadcastReceiverCallbacks {
public:
    LeAudioBroadcastReceiverCallbacksImpl() = default;

    void OnConnectionState(ConnectionState state, const RawAddress& address) override {
        tBTMW_MSG btmw_msg;

        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "addr=%s, state=%d", address.ToString().c_str(), (int)state);
        memset(&btmw_msg, 0, sizeof(btmw_msg));
        btmw_msg.hdr.event = BTMW_BMR_CONNECTION_STATE_CB_EVT;
        btmw_msg.hdr.len = sizeof(BT_BMR_EVENT_PARAM);
        btmw_msg.data.bmr_event.event = BT_BMR_EVENT_CONN_STATE;
        snprintf(btmw_msg.data.bmr_event.data.conn_state.bt_addr, MAX_BDADDR_LEN, "%s", address.ToString().c_str());
        btmw_msg.data.bmr_event.data.conn_state.state = (BT_BMR_CONN_STATE_E)state;
        linuxbt_send_msg(&btmw_msg);
    }

    void OnSolicitationRequestState(uint8_t state) override {
        tBTMW_MSG btmw_msg;

        memset(&btmw_msg, 0, sizeof(btmw_msg));
        btmw_msg.hdr.event = BTMW_BMR_SOLICITATION_REQUEST_STATE_CB_EVT;
        btmw_msg.hdr.len = sizeof(BT_BMR_EVENT_PARAM);
        btmw_msg.data.bmr_event.event = BT_BMR_EVENT_SOLICITATION_REQUEST_STATE;
        btmw_msg.data.bmr_event.data.solicitation_req_state = (BT_BMR_SOLICITATION_REQUEST_STATE_E)state;
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "state=%d", (int)btmw_msg.data.bmr_event.data.solicitation_req_state);
        linuxbt_send_msg(&btmw_msg);
    }

    void OnRemoteScanEvent(uint8_t event, const RawAddress& address) override {
        tBTMW_MSG btmw_msg;

        memset(&btmw_msg, 0, sizeof(btmw_msg));
        btmw_msg.hdr.event = BTMW_BMR_SOURCE_DISCOVERY_STATE_CB_EVT;
        btmw_msg.hdr.len = sizeof(BT_BMR_EVENT_PARAM);
        btmw_msg.data.bmr_event.event = BT_BMR_EVENT_SCAN_STATE;
        btmw_msg.data.bmr_event.data.scan_state.state = (BT_BMR_SCAN_STATE_E)event;
        snprintf(btmw_msg.data.bmr_event.data.scan_state.bt_addr, MAX_BDADDR_LEN, "%s", address.ToString().c_str());
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "state=%d, addr=%s", (int)btmw_msg.data.bmr_event.data.scan_state.state,
            btmw_msg.data.bmr_event.data.scan_state.bt_addr);
        linuxbt_send_msg(&btmw_msg);
    }

    void OnAudioSourceInfo(uint8_t action, const LeAudioBroadcastAudioSource& source) override {
        tBTMW_MSG btmw_msg;
        BT_BMR_SRC_INFO_T *p_src = nullptr;

        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "action=%d, source_id=%d", action, source.source_id);
        memset(&btmw_msg, 0, sizeof(btmw_msg));
        btmw_msg.hdr.event = BTMW_BMR_SOURCE_INFO_CB_EVT;
        btmw_msg.hdr.len = sizeof(BT_BMR_EVENT_PARAM);
        btmw_msg.data.bmr_event.event = BT_BMR_EVENT_SRC_INFO;
        btmw_msg.data.bmr_event.data.source_info.action = (BT_BMR_SRC_ACTION_E)action;
        p_src = &(btmw_msg.data.bmr_event.data.source_info.info);
        p_src->source_id = source.source_id;
        p_src->broadcast_id = source.broadcast_id;
        p_src->encryption = (BT_BMR_BIG_ENCRYPTION_E)source.encryption;
        snprintf(p_src->addr, MAX_BDADDR_LEN, "%s", source.source_addr.ToString().c_str());
        p_src->addr_type = source.source_addr_type;
        p_src->adv_sid = source.adv_sid;
        p_src->state = (BT_BMR_SRC_STATE_E)source.state;
        p_src->bis_sync_req_mask = source.bis_sync_req_mask;
        p_src->presentation_delay = source.BASE.presentation_delay;
        p_src->num_subgroups = source.BASE.num_subgroups;

        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "source params, num_subgroups = %d, bis_sync_req_mask: 0x%x", \
            p_src->num_subgroups, p_src->bis_sync_req_mask);
        if (p_src->num_subgroups > 0) {
            p_src->subgroups = (BT_BMR_SUBGROUP_T *)calloc(sizeof(UINT8), sizeof(BT_BMR_SUBGROUP_T) * p_src->num_subgroups);
            if (p_src->subgroups == nullptr) {
                BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "malloc for subgroup fail !!!");
                return;
            }
            #if 0
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "memory of subgroups: %p", p_src->subgroups);
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "source_id: %d", p_src->source_id);
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "  broadcast_id: 0x%08x", p_src->broadcast_id);
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "  encryption: %d", p_src->encryption);
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "  addr: %s", p_src->addr);
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "  addr_type: %d", p_src->addr_type);
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "  adv_sid: %d", p_src->adv_sid);
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "  state: %d", p_src->state);
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "  presentation_delay: %d", p_src->presentation_delay);
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "  num_subgroups: %d", p_src->num_subgroups);
            #endif

            for (size_t i = 0; i < p_src->num_subgroups; i++) {
                BT_BMR_SUBGROUP_T *p_cur_subgroup = p_src->subgroups + i;
                auto const& source_subgroup = source.BASE.subgroup_config[i];
                p_cur_subgroup->subgroup_id = i;
                p_cur_subgroup->num_bis = source_subgroup.num_bis;
                p_cur_subgroup->codec_id.coding_format = source_subgroup.codec_configuration.codec_format;
                p_cur_subgroup->codec_id.company_id = source_subgroup.codec_configuration.company_id;
                p_cur_subgroup->codec_id.vendor_codec_id = source_subgroup.codec_configuration.vendor_codec_id;
                p_cur_subgroup->codec_configs.sampling_freq = source_subgroup.codec_configuration.codec_configs.sampling_freq;
                p_cur_subgroup->codec_configs.frame_duration = source_subgroup.codec_configuration.codec_configs.frame_duration;
                p_cur_subgroup->codec_configs.channel_allocation = source_subgroup.codec_configuration.codec_configs.channel_allocation;
                p_cur_subgroup->codec_configs.octets_per_codec_frame = source_subgroup.codec_configuration.codec_configs.octets_per_codec_frame;
                p_cur_subgroup->codec_configs.blocks_per_sdu = source_subgroup.codec_configuration.codec_configs.blocks_per_sdu;
                p_cur_subgroup->metadata.prefered_audio_contexts = source_subgroup.metadata.metadata.prefered_audio_contexts;
                p_cur_subgroup->metadata.streaming_audio_contexts = source_subgroup.metadata.metadata.streaming_audio_contexts;
                p_cur_subgroup->metadata.vendor_metadata_length = source_subgroup.metadata.metadata.vendor_specific_metadata.size();
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "vendor_metadata_length=%d", p_cur_subgroup->metadata.vendor_metadata_length);
                if (p_cur_subgroup->metadata.vendor_metadata_length > 0) {
                    p_cur_subgroup->metadata.vendor_metadata = (UINT8 *)calloc(sizeof(UINT8), p_cur_subgroup->metadata.vendor_metadata_length);
                    if (p_cur_subgroup->metadata.vendor_metadata != nullptr) {
                      memcpy(p_cur_subgroup->metadata.vendor_metadata,
                          source_subgroup.metadata.metadata.vendor_specific_metadata.data(),
                          p_cur_subgroup->metadata.vendor_metadata_length);
                    } else {
                        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "malloc for vendor_metadata fail!!!");
                        for (size_t idx = 0; idx < p_src->num_subgroups; idx++) {
                            if (p_src->subgroups[idx].metadata.vendor_metadata != nullptr)
                                free(p_src->subgroups[idx].metadata.vendor_metadata);
                        }
                        free(p_src->subgroups);
                        return;
                    }
                }
                #if 0
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "memory of vendor_specific_metadata: %p", p_cur_subgroup->metadata.vendor_metadata);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, " subgroup #%d:", p_cur_subgroup->subgroup_id);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   num_bis: %d", p_cur_subgroup->num_bis);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   coding_format: 0x%x", p_cur_subgroup->codec_id.coding_format);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   company_id: 0x%x", p_cur_subgroup->codec_id.company_id);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   vendor_codec_id: 0x%x", p_cur_subgroup->codec_id.vendor_codec_id);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   sampling_freq: 0x%x", p_cur_subgroup->codec_configs.sampling_freq);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   frame_duration: 0x%x", p_cur_subgroup->codec_configs.frame_duration);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   channel_allocation: 0x%x", p_cur_subgroup->codec_configs.channel_allocation);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   octets_per_codec_frame: 0x%x", p_cur_subgroup->codec_configs.octets_per_codec_frame);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   blocks_per_sdu: 0x%x", p_cur_subgroup->codec_configs.blocks_per_sdu);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   prefered_audio_contexts: 0x%x", p_cur_subgroup->metadata.prefered_audio_contexts);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   streaming_audio_contexts: 0x%x", p_cur_subgroup->metadata.streaming_audio_contexts);
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   vendor_metadata_length: 0x%x", p_cur_subgroup->metadata.vendor_metadata_length);
                #endif
                BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "subgroup %d, num_bis = %d, bis_config.size = %d", \
                    i, p_cur_subgroup->num_bis, source_subgroup.bis_config.size());
                if (p_cur_subgroup->num_bis > 0) {
                    for (size_t j = 0; j < p_cur_subgroup->num_bis; j++) {
                        BasicAudioAnnouncementBisConfig_t source_bis = source_subgroup.bis_config.at(j);
                        p_cur_subgroup->bis_info[j].bis_index = source_bis.bis_index;
                        p_cur_subgroup->bis_info[j].subgroup_id = p_cur_subgroup->subgroup_id;
                        p_cur_subgroup->bis_info[j].codec_configs.sampling_freq = source_bis.codec_configs.sampling_freq;
                        p_cur_subgroup->bis_info[j].codec_configs.frame_duration = source_bis.codec_configs.frame_duration;
                        p_cur_subgroup->bis_info[j].codec_configs.channel_allocation = source_bis.codec_configs.channel_allocation;
                        p_cur_subgroup->bis_info[j].codec_configs.octets_per_codec_frame = source_bis.codec_configs.octets_per_codec_frame;
                        p_cur_subgroup->bis_info[j].codec_configs.blocks_per_sdu = source_bis.codec_configs.blocks_per_sdu;
                        p_cur_subgroup->bis_info[j].b_playable = source_bis.b_renderable;
                        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "   BIS index #%d:", p_cur_subgroup->bis_info[j].bis_index);
                        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "     sampling_freq: 0x%x", p_cur_subgroup->bis_info[j].codec_configs.sampling_freq);
                        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "     frame_duration: 0x%x", p_cur_subgroup->bis_info[j].codec_configs.frame_duration);
                        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "     channel_allocation: 0x%x", p_cur_subgroup->bis_info[j].codec_configs.channel_allocation);
                        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "     octets_per_codec_frame: 0x%x", p_cur_subgroup->bis_info[j].codec_configs.octets_per_codec_frame);
                        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "     blocks_per_sdu: 0x%x", p_cur_subgroup->bis_info[j].codec_configs.blocks_per_sdu);
                        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "     b_playable: %d", p_cur_subgroup->bis_info[j].b_playable);
                    }
                }
            }
        }
        linuxbt_send_msg(&btmw_msg);
    }

    void OnSourceStateChange(uint8_t source_id, uint8_t new_state, uint8_t reason) override {
        tBTMW_MSG btmw_msg;

        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "source_id=%d, new_state=%d, reason=%d", source_id, new_state, reason);
        memset(&btmw_msg, 0, sizeof(btmw_msg));
        btmw_msg.hdr.event = BTMW_BMR_SOURCE_STATE_CHANGE_CB_EVT;
        btmw_msg.hdr.len = sizeof(BT_BMR_EVENT_PARAM);
        btmw_msg.data.bmr_event.event = BT_BMR_EVENT_SRC_STATE_CHANGE;
        btmw_msg.data.bmr_event.data.source_state.source_id = source_id;
        btmw_msg.data.bmr_event.data.source_state.state = (BT_BMR_SRC_STATE_E)new_state;
        btmw_msg.data.bmr_event.data.source_state.err_code = (BT_BMR_ERROR_CODE_E)reason;
        linuxbt_send_msg(&btmw_msg);
    }

    void OnStreamingEvent(const bluetooth::le_audio::BroadcastReceiverStreamingEventData& data, uint8_t event, uint8_t reason) override {
        tBTMW_MSG btmw_msg;

        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "bisnum=%d, event=%d, reason=%d", data.bis_num, event, reason);
        memset(&btmw_msg, 0, sizeof(btmw_msg));
        btmw_msg.hdr.event = BTMW_BMR_STREAMING_EVENT_CB_EVT;
        btmw_msg.hdr.len = sizeof(BT_BMR_EVENT_PARAM);
        btmw_msg.data.bmr_event.event = BT_BMR_EVENT_STREAMING_EVENT;
        //btmw_msg.data.bmr_event.data.streaming_event.data = source_id;
        memcpy(&btmw_msg.data.bmr_event.data.streaming_event.data, &data, sizeof(bluetooth::le_audio::BroadcastReceiverStreamingEventData));
        btmw_msg.data.bmr_event.data.streaming_event.event = (BT_BMR_STREAMING_EVENT_E)event;
        btmw_msg.data.bmr_event.data.streaming_event.err_code = (BT_BMR_ERROR_CODE_E)reason;
        linuxbt_send_msg(&btmw_msg);
    }

    void OnCloseDone(void) override {
        BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "b_close_done: %d", b_close_done);
        b_close_done = true;
        close_cv.notify_one();
    }
};


#if MTK_LEAUDIO_BMR
static LeAudioBroadcastReceiverInterface *g_bt_bmr_interface = NULL;
static LeAudioBroadcastReceiverCallbacksImpl g_bt_bmr_callbacks;

INT32 linuxbt_bmr_init(void)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "");
    g_bt_bmr_interface =
        (LeAudioBroadcastReceiverInterface *)linuxbt_gap_get_profile_interface(BT_PROFILE_LE_AUDIO_BROADCAST_RECEIVER_ID);
    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "Failed to get BMR interface");
        return BT_ERR_STATUS_FAIL;
    }

    g_bt_bmr_interface->Initialize(&g_bt_bmr_callbacks);
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_deinit(void)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "");
    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }
    b_close_done = false;
    g_bt_bmr_interface->Close(false);
    //shall wait for close done !!!
    std::unique_lock <std::mutex> lck(close_mtx);
    while (!b_close_done) {
        if (std::cv_status::timeout == close_cv.wait_for(lck, std::chrono::seconds(3))) {
            BT_DBG_WARNING(BT_DEBUG_LEAUDIO_BMR, "bmr close timeout");
            break;
        } else {
            BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "bmr close normally done: %d", b_close_done);
        }
    }
    b_close_done = false;
    g_bt_bmr_interface->Cleanup();
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_discover(BT_BMR_DISCOVERY_PARAMS *params)
{
    std::vector<uint32_t> bl;

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "op_code:%d, scan_duration:%d", params->op_code, params->scan_duration);
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "blacklist: [0x%08x] [0x%08x] [0x%08x]",
        params->black_list[0], params->black_list[1], params->black_list[2]);

    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }

    for (size_t i = 0; i < BMR_MAX_BLACKLIST_SIZE; i++) {
        if (params->black_list[i] != 0)
            bl.push_back(params->black_list[i]);
    }

    g_bt_bmr_interface->Discover(params->op_code, params->scan_duration, bl);
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_solicitation_request(BT_BMR_SOLICITATION_REQUEST_PARAMS *params)
{
    std::vector<uint8_t> data;

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "op_code:%d, adv_duration:%d", params->op_code, params->adv_duration);
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "user_data_len: %d", params->user_data_len);

    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }
    if (params->user_data != NULL && params->user_data_len > 0)
        data.assign(params->user_data, params->user_data + params->user_data_len);

    g_bt_bmr_interface->SolicitationRequest(params->op_code, params->adv_duration, data);
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_disconnect(char *bt_addr)
{
    RawAddress addr = RawAddress::kEmpty;

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "bt_addr=%s", bt_addr);
    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }

    LINUXBT_CHECK_AND_CONVERT_ADDR(bt_addr, addr);

    g_bt_bmr_interface->Disconnect(addr);
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_refresh_source(UINT8 source_id)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "source_id:%d", source_id);
    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_bmr_interface->SourceRefresh(source_id);
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_remove_source(BOOL all, UINT8 source_id)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "all: %d, source_id: %d", all, source_id);
    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_bmr_interface->SourceRemove(all, source_id);
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_set_broadcast_code(BT_BMR_SET_BROADCAST_CODE_PARAMS *params)
{
    std::vector<uint8_t> brcst_code;

    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "source_id: %d, broadcast_code: %p", params->source_id, params->broadcast_code);
    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }
    if (params->broadcast_code != NULL)
        brcst_code.assign(params->broadcast_code, params->broadcast_code + BMR_BROADCAST_CODE_SIZE);

    g_bt_bmr_interface->SetBroadcastCode(params->source_id, brcst_code);
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_streaming_start(UINT8 source_id, UINT32 bis_mask)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "source_id:%d, bis:0x%x", source_id, bis_mask);
    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_bmr_interface->StreamingStart(source_id, bis_mask);
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_streaming_stop(UINT8 source_id, UINT32 bis_mask)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "source_id:%d, bis:0x%x", source_id, bis_mask);
    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_bmr_interface->StreamingStop(source_id, bis_mask);
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_set_pac_config(UINT8 pac_type, UINT32 value)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "pac_type:%d, value:0x%x", pac_type, value);
    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_bmr_interface->SetPACConfig(pac_type, value);
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_close(BOOL open_to_bsa)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "open_to_bsa:%d", open_to_bsa);
    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_bmr_interface->Close(open_to_bsa);
    return BT_SUCCESS;
}

INT32 linuxbt_bmr_dump(BOOL all, UINT8 source_id, BOOL full_info)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "all: %d, source_id: %d, full_info: %d", all, source_id, full_info);
    if (NULL == g_bt_bmr_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_LEAUDIO_BMR, "g_bt_bmr_interface not init");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_bmr_interface->Dump(all, source_id, full_info);
    return BT_SUCCESS;
}
#else
INT32 linuxbt_bmr_init(void)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_deinit(void)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_discover(BT_BMR_DISCOVERY_PARAMS *params)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_solicitation_request(BT_BMR_SOLICITATION_REQUEST_PARAMS *params)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_disconnect(char *bt_addr)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_refresh_source(UINT8 source_id)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_remove_source(BOOL all, UINT8 source_id)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_set_broadcast_code(BT_BMR_SET_BROADCAST_CODE_PARAMS *params)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_streaming_start(UINT8 source_id, UINT32 bis_mask)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_streaming_stop(UINT8 source_id, UINT32 bis_mask)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_set_pac_config(UINT8 pac_type, UINT32 value)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_close(BOOL open_to_bsa)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}

INT32 linuxbt_bmr_dump(BOOL all, UINT8 source_id, BOOL full_info)
{
    BT_DBG_NORMAL(BT_DEBUG_LEAUDIO_BMR, "not support");
    return BT_ERR_STATUS_FAIL;
}
#endif

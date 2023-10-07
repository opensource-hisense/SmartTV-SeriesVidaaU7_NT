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

/* FILE NAME:  linuxbt_avrcp_tg_if.c
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
#include <unistd.h>
#include "bluetooth.h"

#include "bt_mw_common.h"
#include "linuxbt_gap_if.h"
#include "linuxbt_avrcp_tg_if.h"
//#include "bt_base_types.h" //need refactor

#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"
#include "bt_mw_message_queue.h"

//#define MTK_LINUX_AVRCP_PLUS TRUE
#define MTK_LINUX_AVRCP_PLUS FALSE

#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
#include "mtk_bt_rc.h"
#else
#include "bt_rc.h"
#endif
#include "avrcp/avrcp.h"

// new avrcp
#include <string.h>
using std::string;

#define USE_NEW_AVRCP_TG  /*Pls make sure 'persist.bluetooth.enablenewavrcp' is true*/


/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */

#define LINUXBT_AVRCP_TG_SET_MSG_LEN(msg) do{       \
    msg.hdr.len = sizeof(tBT_MW_AVRCP_MSG);         \
    }while(0)

/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
CHAR g_bt_avrcp_addr[MAX_BDADDR_LEN];
/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

#ifdef USE_NEW_AVRCP_TG
#include <map>
#include <base/bind.h>
#include <base/callback.h>
#include <hardware/avrcp/avrcp.h>
#include <stdlib.h>
#include <stdio.h>

using namespace bluetooth::avrcp;
extern const bt_interface_t *g_bt_interface;
static ServiceInterface* sServiceInterface;
static MediaCallbacks* mServiceCallbacks;
BT_AVRCP_PLAYER_MEDIA_INFO g_media_info;
BT_AVRCP_PLAYER_STATUS g_play_status;

#endif

#ifdef USE_NEW_AVRCP_TG
class AvrcpMediaInterfaceImpl : public MediaInterface {
    public:
        void SendKeyEvent(const RawAddress& bdaddr, uint8_t key, KeyState state) {
            BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Media CB : key = 0x%x, KeyState = %d ", key, (int)state);
            tBTMW_MSG btmw_msg;
            memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
            RawAddress *p_btaddr = NULL;
            RawAddress bt_addr;
            memset(&bt_addr, 0, sizeof(RawAddress));
            memcpy(&bt_addr, &bdaddr, sizeof(RawAddress));
            p_btaddr = &bt_addr;

            BT_AVRCP_CMD_TYPE bt_cmd_type = BT_AVRCP_CMD_TYPE_MAX;
            int i = 0;

            INT32 key_map[BT_AVRCP_CMD_TYPE_MAX] = {
                AVRC_ID_PLAY,
                AVRC_ID_PAUSE,
                AVRC_ID_FORWARD,
                AVRC_ID_BACKWARD,
                AVRC_ID_FAST_FOR,
                AVRC_ID_REWIND,
                AVRC_ID_STOP,
                AVRC_ID_VOL_UP,
                AVRC_ID_VOL_DOWN,
                AVRC_ID_CHAN_UP,
                AVRC_ID_CHAN_DOWN,
                AVRC_ID_MUTE,
                AVRC_ID_POWER };

            linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr); //need refactor
            BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s key=%d, is_press=%d",
                btmw_msg.data.avrcp_msg.addr, key, (int)state);
            for (i = 0;i < BT_AVRCP_CMD_TYPE_MAX;i ++)
            {
                if (key_map[i] == key)
                {
                    bt_cmd_type = (BT_AVRCP_CMD_TYPE)i;
                    break;
                }
            }

            if (BT_AVRCP_CMD_TYPE_MAX == bt_cmd_type)
            {
                return;
            }

            btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;
            btmw_msg.hdr.event = BTMW_AVRCP_PASSTHROUGH_CMD_REQ;
            btmw_msg.data.avrcp_msg.data.passthrough_cmd_req.cmd_type = bt_cmd_type;
            btmw_msg.data.avrcp_msg.data.passthrough_cmd_req.action =
                ((int)state == 0) ? BT_AVRCP_KEY_STATE_PRESS : BT_AVRCP_KEY_STATE_RELEASE;
            LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);

            linuxbt_send_msg(&btmw_msg);
        }

    void GetSongInfo(SongInfoCallback cb) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "");
        CHAR temp[10];
        int len = 0;
        SongInfo mediaInfo;
        mediaInfo.media_id = g_media_info.media_info.media_id;
        mediaInfo.attributes.insert(AttributeEntry(Attribute::TITLE, std::string(g_media_info.media_info.title)));
        mediaInfo.attributes.insert(AttributeEntry(Attribute::ARTIST_NAME, std::string(g_media_info.media_info.artist)));
        mediaInfo.attributes.insert(AttributeEntry(Attribute::ALBUM_NAME, std::string(g_media_info.media_info.album)));
        len = sprintf(temp, "%d", g_media_info.media_info.current_track_number);
        if (len > 0)
        {
            mediaInfo.attributes.insert(AttributeEntry(Attribute::TRACK_NUMBER, std::string(temp)));
        }

        len = sprintf(temp, "%d", g_media_info.media_info.number_of_tracks);
        if (len > 0)
        {
            mediaInfo.attributes.insert(AttributeEntry(Attribute::TOTAL_NUMBER_OF_TRACKS, std::string(temp)));
        }
        mediaInfo.attributes.insert(AttributeEntry(Attribute::GENRE, std::string(g_media_info.media_info.genre)));
        len = sprintf(temp, "%d", g_media_info.media_info.position);
        if (len > 0)
        {
            mediaInfo.attributes.insert(AttributeEntry(Attribute::PLAYING_TIME, std::string(temp)));
        }
#ifdef USE_TG_CB_IN_STACK
        sServiceInterface->SongInfoCb(cb, mediaInfo);
#else
        cb.Run(mediaInfo);
#endif
  }

    void GetPlayStatus(PlayStatusCallback cb) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "GetPlayStatus Media CB");
        PlayStatus status;
        status.duration = g_play_status.song_len;
        status.position = g_play_status.song_pos;
        status.state = (PlayState)g_play_status.play_status;
#ifdef USE_TG_CB_IN_STACK
         aa
        sServiceInterface->PlayStatusCb(cb, status);
#else
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "before cb GetPlayStatus Media CB");
        cb.Run(status);
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "after cb GetPlayStatus Media CB");
#endif
    }

    void GetNowPlayingList(NowPlayingCallback cb) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Media CB");
        CHAR temp[10];
        int len = 0;
        std::vector<SongInfo> SongInfoList;
        SongInfo mediaInfo;
        SongInfoList.clear();
        std::string curr_song_id = g_media_info.media_info.media_id;
        mediaInfo.media_id = g_media_info.media_info.media_id;
        mediaInfo.attributes.insert(AttributeEntry(Attribute::TITLE, std::string(g_media_info.media_info.title)));
        mediaInfo.attributes.insert(AttributeEntry(Attribute::ARTIST_NAME, std::string(g_media_info.media_info.artist)));
        mediaInfo.attributes.insert(AttributeEntry(Attribute::ALBUM_NAME, std::string(g_media_info.media_info.album)));
        len = sprintf(temp, "%d", g_media_info.media_info.current_track_number);
        if (len > 0)
        {
            mediaInfo.attributes.insert(AttributeEntry(Attribute::TRACK_NUMBER, std::string(temp)));
        }
        len = sprintf(temp, "%d", g_media_info.media_info.number_of_tracks);
        if (len > 0)
        {
            mediaInfo.attributes.insert(AttributeEntry(Attribute::TOTAL_NUMBER_OF_TRACKS, std::string(temp)));
        }
        mediaInfo.attributes.insert(AttributeEntry(Attribute::GENRE, std::string(g_media_info.media_info.genre)));
        len = sprintf(temp, "%d", g_media_info.media_info.position);
        if (len > 0)
        {
            mediaInfo.attributes.insert(AttributeEntry(Attribute::PLAYING_TIME, std::string(temp)));
        }
        SongInfoList.push_back(mediaInfo);

        cb.Run(curr_song_id, std::move(SongInfoList));
      }

    void GetMediaPlayerList(MediaListCallback cb) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Media CB");
        //uint16_t current_player = getCurrentPlayerId();
        //auto player_list = getMediaPlayerList();
        //cb.Run(current_player, std::move(player_list));
    }

    void GetFolderItems(uint16_t player_id, std::string media_id,
                      FolderItemsCallback folder_cb) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Media CB");

        //getFolderItems(player_id, media_id, folder_cb);
    }

    void SetBrowsedPlayer(uint16_t player_id,
                        SetBrowsedPlayerCallback browse_cb) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Media CB");
        //setBrowsedPlayer(player_id, browse_cb);
    }

    void RegisterUpdateCallback(MediaCallbacks* callback) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Media CB");

        mServiceCallbacks = callback;
    }

    void UnregisterUpdateCallback(MediaCallbacks* callback) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Media CB");
        mServiceCallbacks = nullptr;
    }

    void PlayItem(uint16_t player_id, bool now_playing,
                std::string media_id) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Media CB");

        //playItem(player_id, now_playing, media_id);
    }

    void SetActiveDevice(const RawAddress& address) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Media CB");

        //SetActiveDevice(address);
    }

//#if defined(MTK_COMMON) && (MTK_COMMON == TRUE) // [MTWTV-5819] To support battery change
    void GetCapabilitiesCallback(const RawAddress& address, std::vector<uint8_t> elements) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Capabilities CB: size = %d", elements.size());
        for (unsigned int i = 0; i <  (unsigned int)(elements.size()); i++) {
            BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Capabilities CB: data = 0x%x", elements[i]);
        }
    }

    void BatteryStatusChanged(const RawAddress& address, uint8_t status) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Media CB: status = %d", status);
        tBTMW_MSG btmw_msg;
        memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
        RawAddress *p_btaddr = NULL;
        RawAddress bt_addr;
        memset(&bt_addr, 0, sizeof(RawAddress));
        memcpy(&bt_addr, &address, sizeof(RawAddress));
        p_btaddr = &bt_addr;

        linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "bd_addr = %s", btmw_msg.data.avrcp_msg.addr);
        btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;

        strncpy(g_bt_avrcp_addr, btmw_msg.data.avrcp_msg.addr, sizeof(g_bt_avrcp_addr));
        g_bt_avrcp_addr[MAX_BDADDR_LEN-1] = '\0';
        btmw_msg.hdr.event = BTMW_AVRCP_BATTERY_CHANGE;
        btmw_msg.data.avrcp_msg.data.battery_change.battery_status = (BT_AVRCP_BATTERY_STATUS)status;

        LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);
        linuxbt_send_msg(&btmw_msg);
    }
//#endif

// Mediatek Android Patch Begin
#if defined(MTK_AVRCP_APP_SETTINGS) && (MTK_AVRCP_APP_SETTINGS == TRUE)
    /** M: Media interface for app settings @{ */
    //PDU ID 0x11
    void ListAppSettingAttrs(GetListAttributeCb cb) override {
        //getListPlayerAttribute(cb);
    }

    //PDU ID 0x12
    void ListAppSettingValues(uint8_t player_att,
                                GetListAttributeValuesCb cb) override {
        //getListPlayerAttributeValues(player_att, cb);
    }

    //PDU ID 0x13
    void GetAppSettingValues(std::vector<BtrcPlayerAttr> p_attrs,
                               GetAttributeValueCb cb) override {
        //getPlayerAttributeValue(p_attrs, cb);
    }

    //PDU ID 0x14
    void SetAppSettingValues(BtrcPlayerSettings attr,
                               SetPlayerAppSettingCb cb) override {
        //setPlayerAppSetting(attr, cb);
    }

    //PDU ID 0x15
    void GetAppSettingAttrsText(std::vector<BtrcPlayerAttr> att,
                                  GetAttributeTextCb cb) override {
        //getPlayerAttributeText(att, cb);
    }

    //PDU ID 0x16
    void GetAppSettingValuesText(uint8_t attr_id,
                                   std::vector<uint8_t> value,
                                   GetAttributeTextValueCb cb) override {
        //getPlayerAttributeValueText(attr_id, value, cb);
    }

    //Get app settings update
    void GetAppSettingChange(GetAppSettingChangeCb cb) override {
        //getAppSettingChange(cb);
    }
      /** @} */
#endif
// Mediatek Android Patch End
};
static AvrcpMediaInterfaceImpl mAvrcpInterface;

class VolumeInterfaceImpl : public VolumeInterface {
    std::map<RawAddress, VolumeChangedCb> volumeCallbackMap;

    public:
        void DeviceConnected(const RawAddress& bdaddr) override {
        tBTMW_MSG btmw_msg;
        memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "TG CB - addr=%s", bdaddr.ToString().c_str());
        RawAddress *p_btaddr = NULL;
        RawAddress bt_addr;
        memset(&bt_addr, 0, sizeof(RawAddress));
        memcpy(&bt_addr, &bdaddr, sizeof(RawAddress));
        p_btaddr = &bt_addr;

        linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "bd_addr = %s", btmw_msg.data.avrcp_msg.addr);
        btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;

        strncpy(g_bt_avrcp_addr, btmw_msg.data.avrcp_msg.addr, sizeof(g_bt_avrcp_addr));
        g_bt_avrcp_addr[MAX_BDADDR_LEN-1] = '\0';
        btmw_msg.hdr.event = BTMW_AVRCP_CONNECTED;

        LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);
        linuxbt_send_msg(&btmw_msg);

     }

    void DeviceConnected(const RawAddress& bdaddr, VolumeChangedCb cb) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "TG CB abs cb - addr=%s", bdaddr.ToString().c_str());
        volumeCallbackMap.emplace(bdaddr, cb);

        tBTMW_MSG btmw_msg;
        memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
        RawAddress *p_btaddr = NULL;
        RawAddress bt_addr;
        memset(&bt_addr, 0, sizeof(RawAddress));
        memcpy(&bt_addr, &bdaddr, sizeof(RawAddress));
        p_btaddr = &bt_addr;


        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "abs support");
        linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "bd_addr = %s", btmw_msg.data.avrcp_msg.addr);
        btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;

        strncpy(g_bt_avrcp_addr, btmw_msg.data.avrcp_msg.addr, sizeof(g_bt_avrcp_addr));
        g_bt_avrcp_addr[MAX_BDADDR_LEN-1] = '\0';

        btmw_msg.hdr.event = BTMW_AVRCP_ABS_SUPPORTED;
        LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);
        linuxbt_send_msg(&btmw_msg);

        btmw_msg.hdr.event = BTMW_AVRCP_CONNECTED;
        LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);
        linuxbt_send_msg(&btmw_msg);

    }

    void DeviceDisconnected(const RawAddress& bdaddr) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "TG CB - addr=%s", bdaddr.ToString().c_str());
        volumeCallbackMap.erase(bdaddr);

        tBTMW_MSG btmw_msg;
        memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
        RawAddress *p_btaddr = NULL;
        RawAddress bt_addr;
        memset(&bt_addr, 0, sizeof(RawAddress));
        memcpy(&bt_addr, &bdaddr, sizeof(RawAddress));
        p_btaddr = &bt_addr;

        linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr);
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "bd_addr = %s", btmw_msg.data.avrcp_msg.addr);
        btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;

        strncpy(g_bt_avrcp_addr, btmw_msg.data.avrcp_msg.addr, sizeof(g_bt_avrcp_addr));
        g_bt_avrcp_addr[MAX_BDADDR_LEN-1] = '\0';
        btmw_msg.hdr.event = BTMW_AVRCP_DISCONNECTED;

        LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);
        linuxbt_send_msg(&btmw_msg);
    }

    void SetVolume(const RawAddress& bdaddr, int8_t volume) override {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "TG CB - volume=%d", volume);
        tBTMW_MSG btmw_msg;
        memset(&btmw_msg, 0, sizeof(tBTMW_MSG));
        RawAddress *p_btaddr = NULL;
        RawAddress bt_addr;
        memset(&bt_addr, 0, sizeof(RawAddress));
        memcpy(&bt_addr, &bdaddr, sizeof(RawAddress));
        p_btaddr = &bt_addr;

        linuxbt_btaddr_htos(p_btaddr, btmw_msg.data.avrcp_msg.addr); //need refactor

        BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s volume = %x ",
            btmw_msg.data.avrcp_msg.addr, volume);
        btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;

        btmw_msg.hdr.event = BTMW_AVRCP_VOLUME_CHANGE;
        btmw_msg.data.avrcp_msg.data.volume_change.abs_volume = volume;
        LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);

        linuxbt_send_msg(&btmw_msg);
    }

  //Provide sending interface
    void SetAbsVolumeNative(RawAddress& bd_addr, int8_t volume) {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "");
        if (volumeCallbackMap.empty())
        {
            BT_DBG_NORMAL(BT_DEBUG_AVRCP, "no support abs");
        }
        for (const auto& cb : volumeCallbackMap)
        {
            if (memcmp(&(cb.first), &bd_addr, sizeof(RawAddress)) == 0)
            {
                cb.second.Run(volume & 0x7F);
            }
        }
    }
};
static VolumeInterfaceImpl mVolumeInterface;

int linuxbt_rc_tg_send_media_change(BT_AVRCP_PLAYER_MEDIA_INFO *player_media_info, bool track_changed)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");
    /* save media info for getNowPlayingList */
    memcpy(&g_media_info, player_media_info, sizeof(BT_AVRCP_PLAYER_MEDIA_INFO));
    if (mServiceCallbacks == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  "[AVRCP] Failed to get AVRCP TG service \n");
        return -1;
    }
    mServiceCallbacks->SendMediaUpdate(track_changed, FALSE, FALSE);
    return 0;
}

int linuxbt_rc_tg_send_play_status_change(BT_AVRCP_PLAYER_STATUS *player_status, bool play_status_change)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");
    /* save media info for getNowPlayingList */
    memcpy(&g_play_status, player_status, sizeof(BT_AVRCP_PLAYER_STATUS));
    if (mServiceCallbacks == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  "[AVRCP] Failed to get AVRCP TG service \n");
        return -1;
    }
    mServiceCallbacks->SendMediaUpdate(FALSE, play_status_change, FALSE);
    return 0;
}


int linuxbt_rc_tg_init(void)
{
    int ret = 0;

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");
#ifdef USE_NEW_AVRCP_TG
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,"New Interface");
    /* init g_media_info */
    memset(&g_media_info, 0, sizeof(BT_AVRCP_PLAYER_MEDIA_INFO));
    memset(&g_play_status, 0, sizeof(BT_AVRCP_PLAYER_STATUS));
    if (g_bt_interface == NULL) {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  "[AVRCP] Failed to get BT interface \n");
        return -1;
    }
    sServiceInterface = g_bt_interface->get_avrcp_service();
    sServiceInterface->Init(&mAvrcpInterface, &mVolumeInterface);
#endif
    return ret;
}

int linuxbt_rc_tg_deinit(void)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");
//#ifdef USE_NEW_AVRCP_TG
    if (sServiceInterface)
    {
        sServiceInterface->Cleanup();
    }
    sServiceInterface = nullptr;
    mServiceCallbacks = nullptr;
//#endif
    return 0;
}

int linuxbt_rc_tg_set_abs_volume(char *addr, UINT8 volume)
{
    RawAddress bdaddr;

    LINUXBT_CHECK_AND_CONVERT_ADDR(addr, bdaddr);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);

    if (mServiceCallbacks == NULL)
    {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, "Failed to get AVRCP TG service");
        return -1;
    }

    BT_DBG_NORMAL(BT_DEBUG_AVRCP, "end volume change: %d", volume);
    mVolumeInterface.SetAbsVolumeNative(bdaddr, volume);
    return 0;
}
#endif


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


/* FILE NAME:  u_bt_mw_a2dp.h
 * AUTHOR: Hongliang Hu
 * PURPOSE:
 *      It provides bluetooth a2dp structure to APP.
 * NOTES:
 */


#ifndef _U_BT_MW_A2DP_H_
#define _U_BT_MW_A2DP_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_bt_mw_common.h"
/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

#define BT_A2DP_MAX_DEVICE_NUM  (10)
#define CODEC_NUM_MAX (9)
#define BT_A2DP_CONNECTED_MAX_DEVICES  (4) /* 3 INT + 1 ACP */

#define BT_MW_A2DP_MAX_DEVICES  BT_A2DP_CONNECTED_MAX_DEVICES



typedef enum
{
    BT_A2DP_STREAM_STATE_SUSPEND,   /* Stream is suspend */
    BT_A2DP_STREAM_STATE_PLAYING,   /* Stream is playing */
    BT_A2DP_STREAM_STATE_MAX,
} BT_A2DP_STREAM_STATE;


typedef enum
{
    BT_A2DP_ROLE_SRC,   /* Source role */
    BT_A2DP_ROLE_SINK,  /* Sink role   */
    BT_A2DP_ROLE_MAX
}BT_A2DP_ROLE;


typedef enum
{
    BT_A2DP_EVENT_CONNECTED,        /* A2DP connected             */
    BT_A2DP_EVENT_DISCONNECTED,     /* A2DP disconnected          */
    BT_A2DP_EVENT_CONNECT_TIMEOUT,  /* A2DP Connectin Timeout     */
    BT_A2DP_EVENT_STREAM_SUSPEND,   /* A2DP suspend               */
    BT_A2DP_EVENT_STREAM_START,     /* A2DP start                 */
    BT_A2DP_EVENT_CONNECT_COMING,   /* A2DP connect comming       */
    BT_A2DP_EVENT_PLAYER_EVENT,     /* A2DP Local playback status */
    BT_A2DP_EVENT_ROLE_CHANGED,     /* A2DP sink/src role change  */
    BT_A2DP_EVENT_STREAM_DATA_IND,  /* A2DP stream data indication */
    BT_A2DP_EVENT_DELAY,            /* A2DP src delay report */
    BT_A2DP_EVENT_SRC_AUDIO_CONFIG,     /* A2DP audio config */
    BT_A2DP_EVENT_SINK_AUDIO_CONFIG,     /* A2DP SINK audio config */
    BT_A2DP_EVENT_ACTIVE_CHANGED,     /* A2DP src active change */
    BT_A2DP_EVENT_MAX
} BT_A2DP_EVENT;

typedef enum
{
    BT_A2DP_DBG_SET_HW_AUDIO_LOG_LV,    /* modify BT audio device log level */
    BT_A2DP_DBG_SHOW_INFO,              /* show info */
    BT_A2DP_DBG_SET_PLAYER_WAIT_TIME_MS,/* set player wait time after open */
    BT_A2DP_DBG_SET_PLAYER_ZERO_TIME_MS,/* fill zero data after open */
    BT_A2DP_DBG_SET_PLAYER_START_TIME_MS,/* start threshold */
    BT_A2DP_DBG_MAX,
} BT_A2DP_DBG_FLAG;

typedef enum
{
    BT_A2DP_ALSA_PB_EVENT_STOP,         /* local player STOPED      */
    BT_A2DP_ALSA_PB_EVENT_STOP_FAIL,    /* local player stop fail   */
    BT_A2DP_ALSA_PB_EVENT_START,        /* local player STARTED     */
    BT_A2DP_ALSA_PB_EVENT_START_FAIL,   /* local player start fail  */
    BT_A2DP_ALSA_PB_EVENT_DATA_COME,    /* remote audio data come from remote SRC device */
    BT_A2DP_ALSA_PB_EVENT_MAX,
    BT_A2DP_PB_EVENT_STOP = BT_A2DP_ALSA_PB_EVENT_STOP,
    BT_A2DP_PB_EVENT_STOP_FAIL = BT_A2DP_ALSA_PB_EVENT_STOP_FAIL,
    BT_A2DP_PB_EVENT_START = BT_A2DP_ALSA_PB_EVENT_START,
    BT_A2DP_PB_EVENT_START_FAIL = BT_A2DP_ALSA_PB_EVENT_START_FAIL,
    BT_A2DP_PB_EVENT_DATA_COME = BT_A2DP_ALSA_PB_EVENT_DATA_COME,
    BT_A2DP_PB_EVENT_MAX = BT_A2DP_ALSA_PB_EVENT_MAX,
} BT_A2DP_PLAYER_EVENT;

typedef enum
{
    BT_A2DP_CODEC_TYPE_SBC,
    BT_A2DP_CODEC_TYPE_AAC,
    BT_A2DP_CODEC_TYPE_LDAC,
    BT_A2DP_CODEC_TYPE_APTX,
    BT_A2DP_CODEC_TYPE_APTX_HD,
    BT_A2DP_CODEC_TYPE_LHDC_LL,
    BT_A2DP_CODEC_TYPE_LHDC,
    BT_A2DP_CODEC_TYPE_STE,
    BT_A2DP_CODEC_TYPE_MAX,
} BT_A2DP_CODEC_TYPE;

typedef enum
{
    BT_A2DP_CHANNEL_MODE_MONO,
    BT_A2DP_CHANNEL_MODE_DUAL,
    BT_A2DP_CHANNEL_MODE_STEREO,
    BT_A2DP_CHANNEL_MODE_JOINT,
}BT_A2DP_CHANNEL_MODE;

typedef enum
{
    BT_A2DP_AAC_OBJ_TYPE_MPEG2_LC,
    BT_A2DP_AAC_OBJ_TYPE_MPEG4_LC,
    BT_A2DP_AAC_OBJ_TYPE_MPEG4_LTP,
    BT_A2DP_AAC_OBJ_TYPE_MPEG4_SCA,
}BT_A2DP_AAC_OBJ_TYPE;

typedef enum
{
    BT_A2DP_SINK_DUMP_PLAYER_STAT,
    BT_A2DP_SINK_DUMP_PLAYER_STAT_2_FILE,
    BT_A2DP_SINK_DUMP_PLAYER_PCM_START,
    BT_A2DP_SINK_DUMP_PLAYER_PCM_STOP,
} BT_A2DP_SINK_DUMP_FLAG;


typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];      /* history connected device address */
    CHAR name[MAX_NAME_LEN];
    BT_A2DP_ROLE role;
}BT_A2DP_CONNECTED_DEVICE;

typedef struct
{
    UINT32                   dev_num;   /* history A2DP connected device count */
    BOOL                     is_snklp;
    BT_A2DP_CONNECTED_DEVICE dev[BT_A2DP_MAX_DEVICE_NUM];
}BT_A2DP_DEVICE_LIST;

typedef struct
{
    UINT32  samp_freq;                      /* Sampling frequency */
    BT_A2DP_CHANNEL_MODE ch_mode;           /* Channel mode */
    UINT8   block_len;                      /* Block length */
    UINT8   num_subbands;                   /* Number of subbands */
    UINT8   alloc_mthd;                     /* 0-SNR, 1-Loundness */
    UINT8   max_bitpool;                    /* Maximum bitpool */
    UINT8   min_bitpool;                    /* Minimum bitpool */
} BT_A2DP_SBC_CONF;

typedef struct
{
    UINT8 layer;                    /* 0-layer1, 1-layer2, 2-layer3 */
    UINT8 crc;                      /* 0-protection not support, 1-support */
    BT_A2DP_CHANNEL_MODE ch_mode;   /* 0-mono, 1-dual, 2-stereo, 3-joint stereo */
    UINT8 mpf;                      /* 0-mpf-1, 1-mpf-2 */
    UINT32 sample_rate;             /* sample rate */
    BOOL vbr;                       /* vbr support */
    UINT32 bit_rate;                /* bit rate, unit: Kbps */
} BT_A2DP_MP3_CONF;                 /* all MPEG-1,2 Audio */

typedef struct
{
    BT_A2DP_AAC_OBJ_TYPE object_type;
    UINT32 samp_freq;
    UINT8  channels;
    BOOL   vbr;
    UINT32 bitrate;
} BT_A2DP_AAC_CONF;

typedef struct
{
    UINT32 sample_rate;
    BT_A2DP_CHANNEL_MODE channel_mode;
}BT_A2DP_LDAC_CONF;

typedef struct
{
    UINT32  samp_freq;
    UINT32  max_rate;
    UINT16  bit_depth;
    UINT16  channels;
    UINT8 obj_type;
    UINT8 compressor_output_fmt;
} BT_A2DP_LHDC_CONF;

typedef struct
{
    UINT32 vendor_id;
    UINT16 codec_id;
    union
    {
        BT_A2DP_LDAC_CONF ldac_conf;
        BT_A2DP_LHDC_CONF lhdc_conf;
    } conf;
} BT_A2DP_VENDOR_CONF;

typedef union
{
    BT_A2DP_SBC_CONF sbc_conf;
    BT_A2DP_MP3_CONF mp3_conf;
    BT_A2DP_AAC_CONF aac_conf;
    BT_A2DP_SBC_CONF  ste_conf;
    BT_A2DP_VENDOR_CONF vendor_conf;
} BT_A2DP_CODEC_CONF;

//#if defined(MTK_STACK_SINK_LP) && (MTK_STACK_SINK_LP == TRUE)
typedef struct
{
    UINT16 acl_hdl;
    UINT16 lcid;
} BT_A2DP_LP_INFO;
//#endif
typedef struct
{
    BT_A2DP_CODEC_TYPE codec_type; // SBC, MP3,AAC,etc
    BT_A2DP_CODEC_CONF codec_conf;
//#if defined(MTK_STACK_SINK_LP) && (MTK_STACK_SINK_LP == TRUE)
    BT_A2DP_LP_INFO    lowpower_info;
//#endif
} BT_A2DP_CONF;


typedef union
{
    INT32 hw_audio_log_lv;  /* BT audio device log level */
    UINT32 wait_time_ms;
    UINT32 zero_time_ms;
} BT_A2DP_DBG_PARAM;

/* when APP will register a Player into BT MW, it is used to report player
 * status to MW and MW will report it to APP
 */
typedef VOID (*BT_A2DP_PLAYER_EVENT_CB)(BT_A2DP_PLAYER_EVENT event);

/* prepare player, like: open audio device, set playback parameter
 */
typedef INT32 (*BT_A2DP_PLAYER_START)(INT32 trackFreq, INT32 channelType, INT32 bitDepth);

/* play player, after this, BT streaming data can be pushed into player.
 */
typedef INT32 (*BT_A2DP_PLAYER_PLAY)(VOID);

/* pause player
 */
typedef INT32 (*BT_A2DP_PLAYER_PAUSE)(VOID);

/* init player, when A2DP sink is enable, this will be called once.
 */
typedef INT32 (*BT_A2DP_PLAYER_INIT)(BT_A2DP_PLAYER_EVENT_CB event_cb);

/* deinit player, when A2DP sink is disable, this will be called once.
 */
typedef INT32 (*BT_A2DP_PLAYER_DEINIT)(VOID);

/* stop player, like: close audio device
 */
typedef INT32 (*BT_A2DP_PLAYER_STOP)(VOID);

/* push A2DP streaming data into player to playback
 */
typedef INT32 (*BT_A2DP_PLAYER_WRITE)(VOID *audioBuffer, INT32 bufferLen);

/* adjust player buffer time
 */
typedef INT32 (*BT_A2DP_PLAYER_ADJUST_BUFFER_TIME)(UINT32 buffer_time);

/* dump player
 */
typedef INT32 (*BT_A2DP_PLAYER_DUMP)(BT_A2DP_SINK_DUMP_FLAG flag);

/* app can implement a BT player and register it into BT MW.
 * player is used when it's a A2DP sink device.
 */
typedef struct
{
    CHAR name[32];
    UINT32 support_codec_mask;
    BT_A2DP_PLAYER_INIT init;
    BT_A2DP_PLAYER_DEINIT deinit;
    BT_A2DP_PLAYER_START start;
    BT_A2DP_PLAYER_STOP stop;
    BT_A2DP_PLAYER_PLAY play;
    BT_A2DP_PLAYER_PAUSE pause;
    BT_A2DP_PLAYER_WRITE write;
    BT_A2DP_PLAYER_ADJUST_BUFFER_TIME adjust_buf_time;
    BT_A2DP_PLAYER_DUMP dump;
}BT_A2DP_PLAYER;

/**/
typedef struct
{
    /** get player:alsa ;stereo; avs; */
    BT_A2DP_PLAYER * (*get_player)(VOID);

} BT_A2DP_GET_PLAYER_METHODS;

/**
 * Every player module must have a data structure named PLAYER_MODULE_INFO_SYM
 * and the fields of this data structure must begin with BT_A2DP_PLAYER_MODULE
 * followed by module specific information.
 */
typedef struct
{
    /*player version*/
    UINT16 version_major;
    UINT16 version_minor;

    /** Modules methods */
    BT_A2DP_GET_PLAYER_METHODS* methods;

} BT_A2DP_PLAYER_MODULE;


/* init uploader.
 * freq: sample rate
 * channel: channel number
 */
typedef INT32 (*BT_A2DP_UPLOADER_INIT)(INT32 freq, INT32 channel);

/* start uploader, it is used to ask uploader to get PCM data and send out to
 * BT MW.
 * delay_ms: after this time, uploader begin to get PCM data.
 */
typedef INT32 (*BT_A2DP_UPLOADER_START)(INT32 delay_ms);

/* stop uploader, it is used to stop get PCM data and sending.
 */
typedef INT32 (*BT_A2DP_UPLOADER_STOP)(VOID);

/* suspend uploader, it is used to pause get PCM data and send SUSPEND to the
 * remote device
 */
typedef INT32 (*BT_A2DP_UPLOADER_PAUSE)(VOID *param);

/* resume uploader, it's used to get pcm data and send START to the remote device
 */
typedef INT32 (*BT_A2DP_UPLOADER_RESUME)(VOID *param);

/* mute uploader
 */
typedef INT32 (*BT_A2DP_UPLOADER_MUTE)(BOOL mute);

/* deinit uploader
 */
typedef INT32 (*BT_A2DP_UPLOADER_DEINIT)(VOID);

/* app can implement a BT uploader and register it into BT MW.
 * uploader is used when it's a A2DP Source device.
 */
typedef struct
{
    CHAR name[32];
    BT_A2DP_UPLOADER_INIT init;
    BT_A2DP_UPLOADER_START start;
    BT_A2DP_UPLOADER_STOP stop;
    BT_A2DP_UPLOADER_DEINIT deinit;
    BT_A2DP_UPLOADER_PAUSE pause;
    BT_A2DP_UPLOADER_RESUME resume;
    BT_A2DP_UPLOADER_MUTE mute;
}BT_A2DP_UPLOADER;

typedef INT32 (*BT_A2DP_MTAL_OUTPUT_CALLBACK)(const CHAR *data, INT32 len);
/**/
typedef struct
{
    /** get uploader:mtal, alsa, etc */
    BT_A2DP_UPLOADER * (*get_uploader)(VOID);
} BT_A2DP_GET_UPLOADER_METHODS;

/**
 * Every uploader module must have a data structure named PLAYER_MODULE_INFO_SYM
 * and the fields of this data structure must begin with BT_A2DP_UPLOADER_MODULE
 * followed by module specific information.
 */
typedef struct
{
    /*uploader version*/
    UINT16 ver_major;
    UINT16 ver_minor;

    /** Modules methods */
    BT_A2DP_GET_UPLOADER_METHODS* methods;

} BT_A2DP_UPLOADER_MODULE;

/* when report a BT_A2DP_EVENT_CONNECTED, app can get the audio parameter.
 */
typedef struct
{
    BT_A2DP_ROLE local_role;
    UINT32 sample_rate;
    UINT32 channel_num;
    BT_A2DP_CONF config;
} BT_A2DP_EVENT_CONNECTED_DATA;


/* when disable a role, it may be pending because of connection exist, so
 * BT MW will disconnect all connection and then report a event to app.
 */
typedef struct
{
    BT_A2DP_ROLE role;
    BOOL         enable;
} BT_A2DP_EVENT_ROLE_CHANGE_DATA;

/* when local device is src, sonme event will report delay data
*/
typedef struct
{
    UINT16 delay;    /* delay value */
} BT_A2DP_EVENT_DELAY_DATA;

typedef struct
{
    UINT8 *data;            /* media data pointer */
    UINT16 total_length;    /* the total length of Media packet */
    UINT16 media_offset;    /* the offset of media payload from the whole packet */
} BT_A2DP_EVENT_STREAM_DATA;

/* q migration add
 */

typedef enum {
  // Disable the codec.
  // NOTE: This value can be used only during initialization
  BT_A2DP_CODEC_PRIORITY_DISABLED = -1,

  // Reset the codec priority to its default value.
  BT_A2DP_CODEC_PRIORITY_DEFAULT = 0,

  // Highest codec priority.
  BT_A2DP_CODEC_PRIORITY_HIGHEST = 1000 * 1000
} BT_A2DP_CODEC_PRIORITY;

typedef enum {
  BT_A2DP_CODEC_SAMPLE_RATE_NONE = 0x0,
  BT_A2DP_CODEC_SAMPLE_RATE_44100 = 0x1 << 0,
  BT_A2DP_CODEC_SAMPLE_RATE_48000 = 0x1 << 1,
  BT_A2DP_CODEC_SAMPLE_RATE_88200 = 0x1 << 2,
  BT_A2DP_CODEC_SAMPLE_RATE_96000 = 0x1 << 3,
  BT_A2DP_CODEC_SAMPLE_RATE_176400 = 0x1 << 4,
  BT_A2DP_CODEC_SAMPLE_RATE_192000 = 0x1 << 5,
  BT_A2DP_CODEC_SAMPLE_RATE_16000 = 0x1 << 6,
  BT_A2DP_CODEC_SAMPLE_RATE_24000 = 0x1 << 7
} BT_A2DP_CODEC_SAMPLE_RATE;

typedef enum {
  BT_A2DP_CODEC_BITS_PER_SAMPLE_NONE = 0x0,
  BT_A2DP_CODEC_BITS_PER_SAMPLE_16 = 0x1 << 0,
  BT_A2DP_CODEC_BITS_PER_SAMPLE_24 = 0x1 << 1,
  BT_A2DP_CODEC_BITS_PER_SAMPLE_32 = 0x1 << 2
} BT_A2DP_CODEC_BITS_PER_SAMPLE;

typedef enum {
  BT_A2DP_CODEC_CHANNEL_MODE_NONE = 0x0,
  BT_A2DP_CODEC_CHANNEL_MODE_MONO = 0x1 << 0,
  BT_A2DP_CODEC_CHANNEL_MODE_STEREO = 0x1 << 1
} BT_A2DP_CODEC_CHANNEL_MODE;

typedef enum {
  BT_A2DP_CODEC_CONFIG_CODEC_TYPE = 0x01,
  BT_A2DP_CODEC_CONFIG_CODEC_PRIORITY = 0x1 << 1,
  BT_A2DP_CODEC_CONFIG_CH_MODE = 0x1 << 2,
  BT_A2DP_CODEC_CONFIG_SAMPLE_RATE = 0x1 << 3,
  BT_A2DP_CODEC_CONFIG_BITS_PER_SAMPLE = 0x1 << 4
} BT_A2DP_CODEC_CONFIG_MASK;


typedef struct
{
    BT_A2DP_CODEC_TYPE  codec_type;
    BT_A2DP_CODEC_CHANNEL_MODE channel_mode;
    BT_A2DP_CODEC_PRIORITY codec_priority;
    BT_A2DP_CODEC_SAMPLE_RATE sample_rate;
    BT_A2DP_CODEC_BITS_PER_SAMPLE bits_per_sample;
    INT32 channel_num;      /*for sink*/
    INT64 codec_specific_1;
    INT64 codec_specific_2;
    INT64 codec_specific_3;
    INT64 codec_specific_4;
} BT_A2DP_CODEC_CONFIG;

typedef struct
{
    UINT32                  codec_config_num;
    BT_A2DP_CODEC_CONFIG  codec_config_list[CODEC_NUM_MAX];
}BT_A2DP_CODEC_CONFIG_LIST;

typedef struct
{
    INT32                        max_connected_audio_devices;
    BT_A2DP_CODEC_CONFIG_LIST  codec_config_list;
}BT_A2DP_SRC_INIT_CONFIG;

typedef struct
{
    BT_A2DP_CODEC_TYPE  codec_type;
    // bit 0 :codec priority is use; bit 1 ch_mode  is use;
    //bit 2:sample_rate is use; bit 3:bits_per_sample is use
    UINT32              bit_mask;
    UINT32 codec_priority; //BT_A2DP_CODEC_PRIORITY
    BT_A2DP_CODEC_CHANNEL_MODE ch_mode;
    BT_A2DP_CODEC_SAMPLE_RATE sample_rate;
    BT_A2DP_CODEC_BITS_PER_SAMPLE bits_per_sample;
} BT_A2DP_SET_CODEC_CONFIG;


typedef struct
{
    BT_A2DP_CODEC_CONFIG audio_config;
    BT_A2DP_CODEC_CONFIG_LIST local_codec_capabilities;
    BT_A2DP_CODEC_CONFIG_LIST codec_selectable_capabilities;
}BT_A2DP_EVENT_AUDIO_CONFIG_DATA;

typedef struct
{
    UINT8 codec_type;
    INT32 sample_rate;
    INT32 channel_num;
}BT_A2DP_EVENT_SINK_AUDIO_CONFIG_DATA;

typedef struct
{
    BT_A2DP_ROLE local_role;
} BT_A2DP_EVENT_ACTIVE_CHANGED_DATA;

typedef enum
{
    BT_A2DP_CONNECT_STATUS_DISCONNECTED,
    BT_A2DP_CONNECT_STATUS_DISCONNECTING,
    BT_A2DP_CONNECT_STATUS_CONNECTING,
    BT_A2DP_CONNECT_STATUS_CONNECTED,
    BT_A2DP_CONNECT_STATUS_MAX
} BT_A2DP_CONNECT_STATUS;

typedef struct
{
    UINT8 codec_type;
    INT32 sample_rate;
    INT32 channel_mode;
} BT_A2DP_SINK_CODEC_CONFIG;

typedef struct
{
    UINT16 delay;
} BT_A2DP_DELAY;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_A2DP_STREAM_STATE stream_status;
    BT_A2DP_CONNECT_STATUS conn_status;
    BT_A2DP_ROLE local_role;
    BT_A2DP_SINK_CODEC_CONFIG codec_config;   // for sink
    BT_A2DP_CODEC_CONFIG curr_codec_info; // save cuur select codec info for src
    BT_A2DP_DELAY delay;
    BT_A2DP_CONF config;
    BOOL in_use;
    BOOL is_coming; /* TRUE: connect from remote */
    BOOL is_silence_mode;
} BT_A2DP_CONNECT_DEV_INFO;

typedef struct
{
    UINT32 dev_num;
    BT_A2DP_CONNECT_DEV_INFO a2dp_connected_dev_list[BT_A2DP_CONNECTED_MAX_DEVICES];
}BT_A2DP_CONNECT_DEV_INFO_LIST;

/* when report a BT_A2DP_EVENT_ to app, some event will report some data
 */
typedef union
{
    BT_A2DP_EVENT_CONNECTED_DATA connected_data;  /* connected data */
    BT_A2DP_PLAYER_EVENT player_event;            /* player event   */
    BT_A2DP_EVENT_ROLE_CHANGE_DATA role_change;   /* change role  */
    BT_A2DP_EVENT_STREAM_DATA stream_data;        /* stream data */
    BT_A2DP_EVENT_DELAY_DATA delay;               /* delay value */
    BT_A2DP_EVENT_AUDIO_CONFIG_DATA audio_config;       /* src audio_config */
    BT_A2DP_EVENT_ACTIVE_CHANGED_DATA active_changed;   /* active_changed */
    BT_A2DP_EVENT_SINK_AUDIO_CONFIG_DATA sink_audio_config;       /* sink audio_config */
} BT_A2DP_EVENT_DATA;

/* when report a BT_A2DP_EVENT_ to app, this paramter is passed to APP
 */
typedef struct
{
    BT_A2DP_EVENT event;        /* event id */
    CHAR addr[MAX_BDADDR_LEN];  /* which device report this event */
    BT_A2DP_EVENT_DATA data;    /* event data */
}BT_A2DP_EVENT_PARAM;

/* app should provide a A2DP event handler and register to BT MW
 */
typedef void (*BT_A2DP_EVENT_HANDLE_CB)(BT_A2DP_EVENT_PARAM *param);

#endif /*  _U_BT_MW_A2DP_H_ */


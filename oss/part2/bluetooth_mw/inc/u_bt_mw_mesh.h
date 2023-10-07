/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2013
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

/* FILE NAME:  u_bt_mw_mesh.h
 * AUTHOR: Houxian Shi
 * PURPOSE:
 *      It provides MESH structure to APP.
 * NOTES:
 */


#ifndef _U_BT_MW_MESH_H_
#define _U_BT_MW_MESH_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

//#include "u_common.h"
#include "u_bt_mw_common.h"
#define BT_MESH_VERSION_LEN (30)
#define BT_MESH_UUID_SIZE   (16)
#define BT_MESH_KEY_SIZE    (16)
#define BT_MESH_PUBLIC_KEY_SIZE    (16)
#define BT_MESH_BLE_ADDR_LEN     (6)
#define BT_MESH_TTL_MAX     (0x7F)
#define BT_MESH_AUTHENTICATION_SIZE  (16)
#define BT_MESH_DEVKEY_SIZE BT_MESH_KEY_SIZE
#define BT_MESH_COMPOSITION_DATA_FIXED_FIELD_LEN  (10)
#define BT_MESH_EVENT_MESH   (200)
#define BT_MESH_EVENT_ADV_REPORT (0x1003)
#define BT_MESH_ADV_DATA_SIZE (62)

#define BT_MESH_FEATURE_NONE     0x00    /**< A bit field indicating no feature. */
#define BT_MESH_FEATURE_RELAY    0x01    /**< A bit field indicating feature relay. */
#define BT_MESH_FEATURE_PROXY    0x02    /**< A bit field indicating feature proxy. */
#define BT_MESH_FEATURE_FRIEND   0x04    /**< A bit field indicating feature friend. */
#define BT_MESH_FEATURE_LPN      0x08    /**< A bit field indicating feature low power node. */

#define BT_MESH_FLASH_INVALID_DATA  0xFF
#define BT_MESH_FLASH_VALID_DATA    0x0F

#define BT_MESH_NET_KEY_RECORD_NUMBER              10  /**< The maximum number of network keys stored in flash.*/
#define BT_MESH_APP_KEY_RECORD_NUMBER              50  /**< The maximum number of application keys stored in flash.*/
#define BT_MESH_MODEL_RECORD_NUMBER                50  /**< The maximum number of model records stored in flash.*/
#define BT_MESH_MODEL_PUBLICATION_RECORD_NUMBER    10  /**< The maximum number of model publication records stored in flash.*/
#define BT_MESH_MODEL_SUBSCRIPTION_RECORD_NUMBER   10  /**< The maximum number of model subscription records stored in flash.*/
#define BT_MESH_HEALTH_SERVER_RECORD_NUMBER        10  /**< The maximum number of health server records stored in flash.*/
#define BT_MESH_FRIEND_RECORD_NUMBER               15  /**< The max number of friend stored in flash.*/
#define BT_MESH_LOCAL_DEVICE_INFO_RECORD_NUMBER    1   /**< The max number of local device information stored in flash.*/
#define BT_MESH_REMOTE_DEVICE_INFO_RECORD_NUMBER   50  /**< The max number of remote device information stored in flash.*/
#define BT_MESH_CONFIGURATION_SERVER_RECORD_NUMBER 1   /**< The max number of configuration server record stored in flash.*/
#define BT_MESH_SEQUENCE_NUMBER_RECORD_NUMBER      1   /**< The max number of sequence number stored in memory.*/

//Configuration model messages
#define BT_MESH_ACCESS_MSG_CONFIG_BEACON_GET 0x8009
#define BT_MESH_ACCESS_MSG_CONFIG_BEACON_SET 0x800A
#define BT_MESH_ACCESS_MSG_CONFIG_BEACON_STATUS  0x800B
#define BT_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_GET   0x8008
#define BT_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_STATUS    0x02
#define BT_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_GET    0x800C
#define BT_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_SET    0x800D
#define BT_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_STATUS 0x800E
#define BT_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_GET 0x8012
#define BT_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_SET 0x8013
#define BT_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_STATUS  0x8014
#define BT_MESH_ACCESS_MSG_CONFIG_FRIEND_GET 0x800F
#define BT_MESH_ACCESS_MSG_CONFIG_FRIEND_SET 0x8010
#define BT_MESH_ACCESS_MSG_CONFIG_FRIEND_STATUS  0x8011
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_GET  0x8018
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_SET  0x03
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_STATUS   0x8019
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_VIRTUAL_ADDRESS_SET  0x801A
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_ADD 0x801B
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_DELETE  0x801C
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL  0x801D
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE   0x801E
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_STATUS  0x801F
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_ADD 0x8020
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_DELETE  0x8021
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_OVERWRITE   0x8022
#define BT_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_GET   0x8023
#define BT_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_SET   0x8024
#define BT_MESH_ACCESS_MSG_CONFIG_NETWORK_TRANSMIT_STATUS    0x8025
#define BT_MESH_ACCESS_MSG_CONFIG_RELAY_GET  0x8026
#define BT_MESH_ACCESS_MSG_CONFIG_RELAY_SET  0x8027
#define BT_MESH_ACCESS_MSG_CONFIG_RELAY_STATUS   0x8028
#define BT_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_SUBSCRIPTION_GET 0x8029
#define BT_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_SUBSCRIPTION_LIST    0x802A
#define BT_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_SUBSCRIPTION_GET  0x802B
#define BT_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_SUBSCRIPTION_LIST 0x802C
#define BT_MESH_ACCESS_MSG_CONFIG_LOW_POWER_NODE_POLL_TIMEOUT_GET    0x802D
#define BT_MESH_ACCESS_MSG_CONFIG_LOW_POWER_NODE_POLL_TIMEOUT_STATUS 0x802E
#define BT_MESH_ACCESS_MSG_CONFIG_NETKEY_ADD 0x8040
#define BT_MESH_ACCESS_MSG_CONFIG_NETKEY_DELETE  0x8041
#define BT_MESH_ACCESS_MSG_CONFIG_NETKEY_GET 0x8042
#define BT_MESH_ACCESS_MSG_CONFIG_NETKEY_LIST    0x8043
#define BT_MESH_ACCESS_MSG_CONFIG_NETKEY_STATUS  0x8044
#define BT_MESH_ACCESS_MSG_CONFIG_NETKEY_UPDATE  0x8045
#define BT_MESH_ACCESS_MSG_CONFIG_APPKEY_ADD 0x00
#define BT_MESH_ACCESS_MSG_CONFIG_APPKEY_UPDATE  0x01
#define BT_MESH_ACCESS_MSG_CONFIG_APPKEY_DELETE  0x8000
#define BT_MESH_ACCESS_MSG_CONFIG_APPKEY_GET 0x8001
#define BT_MESH_ACCESS_MSG_CONFIG_APPKEY_LIST    0x8002
#define BT_MESH_ACCESS_MSG_CONFIG_APPKEY_STATUS  0x8003
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_APP_BIND 0x803D
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_APP_STATUS   0x803E
#define BT_MESH_ACCESS_MSG_CONFIG_MODEL_APP_UNBIND   0x803F
#define BT_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_APP_GET  0x804B
#define BT_MESH_ACCESS_MSG_CONFIG_SIG_MODEL_APP_LIST 0x804C
#define BT_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_APP_GET   0x804D
#define BT_MESH_ACCESS_MSG_CONFIG_VENDOR_MODEL_APP_LIST  0x804E
#define BT_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_GET  0x8046
#define BT_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_SET  0x8047
#define BT_MESH_ACCESS_MSG_CONFIG_NODE_IDENTITY_STATUS   0x8048
#define BT_MESH_ACCESS_MSG_CONFIG_NODE_RESET 0x8049
#define BT_MESH_ACCESS_MSG_CONFIG_NODE_RESET_STATUS  0x804A
#define BT_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_GET  0x8015
#define BT_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_SET  0x8016
#define BT_MESH_ACCESS_MSG_CONFIG_KEY_REFRESH_PHASE_STATUS   0x8017
#define BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_GET  0x8038
#define BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_SET  0x8039
#define BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_PUBLICATION_STATUS   0x06
#define BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_GET 0x803A
#define BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_SET 0x803B
#define BT_MESH_ACCESS_MSG_CONFIG_HEARTBEAT_SUBSCRIPTION_STATUS  0x803C

#define BT_MESH_ACCESS_MSG_HEALTH_CURRENT_STATUS 0x04
#define BT_MESH_ACCESS_MSG_HEALTH_FAULT_STATUS   0x05
#define BT_MESH_ACCESS_MSG_HEALTH_FAULT_CLEAR    0x802F
#define BT_MESH_ACCESS_MSG_HEALTH_FAULT_CLEAR_UNACKNOWLEDGED 0x8030
#define BT_MESH_ACCESS_MSG_HEALTH_FAULT_GET  0x8031
#define BT_MESH_ACCESS_MSG_HEALTH_FAULT_TEST 0x8032
#define BT_MESH_ACCESS_MSG_HEALTH_FAULT_TEST_UNACKNOWLEDGED  0x8033
#define BT_MESH_ACCESS_MSG_HEALTH_PERIOD_GET 0x8034
#define BT_MESH_ACCESS_MSG_HEALTH_PERIOD_SET 0x8035
#define BT_MESH_ACCESS_MSG_HEALTH_PERIOD_SET_UNACKNOWLEDGED  0x8036
#define BT_MESH_ACCESS_MSG_HEALTH_PERIOD_STATUS  0x8037
#define BT_MESH_ACCESS_MSG_HEALTH_ATTENTION_GET  0x8004
#define BT_MESH_ACCESS_MSG_HEALTH_ATTENTION_SET  0x8005
#define BT_MESH_ACCESS_MSG_HEALTH_ATTENTION_SET_UNACKNOWLEDGED   0x8006
#define BT_MESH_ACCESS_MSG_HEALTH_ATTENTION_STATUS   0x8007

/*!
     @name Lighting model ID
     @{
 */
#define BT_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_SERVER    0x1300
#define BT_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_SETUP_SERVER    0x1301
#define BT_MESH_SIG_MODEL_ID_LIGHT_LIGHTNESS_CLIENT    0x1302
#define BT_MESH_SIG_MODEL_ID_LIGHT_CTL_SERVER    0x1303
#define BT_MESH_SIG_MODEL_ID_LIGHT_CTL_SETUP_SERVER    0x1304
#define BT_MESH_SIG_MODEL_ID_LIGHT_CTL_CLIENT    0x1305
#define BT_MESH_SIG_MODEL_ID_LIGHT_CTL_TEMPERATURE_SERVER    0x1306
#define BT_MESH_SIG_MODEL_ID_LIGHT_HSL_SERVER    0x1307
#define BT_MESH_SIG_MODEL_ID_LIGHT_HSL_SETUP_SERVER    0x1308
#define BT_MESH_SIG_MODEL_ID_LIGHT_HSL_CLIENT    0x1309
#define BT_MESH_SIG_MODEL_ID_LIGHT_HSL_HUE_SERVER    0x130a
#define BT_MESH_SIG_MODEL_ID_LIGHT_HSL_SATURATION_SERVER    0x130b
#define BT_MESH_SIG_MODEL_ID_LIGHT_XYL_SERVER    0x130c
#define BT_MESH_SIG_MODEL_ID_LIGHT_XYL_SETUP_SERVER    0x130d
#define BT_MESH_SIG_MODEL_ID_LIGHT_XYL_CLIENT    0x130e
#define BT_MESH_SIG_MODEL_ID_LIGHT_LC_SERVER    0x130f
#define BT_MESH_SIG_MODEL_ID_LIGHT_LC_LIGHT_ONOFF_SERVER    0x1310
#define BT_MESH_SIG_MODEL_ID_LIGHT_LC_SETUP_SERVER    0x1311
#define BT_MESH_SIG_MODEL_ID_LIGHT_LC_CLIENT    0x1312
/*!  @} */


/*!
     @name Generic model ID
     @{
 */
#define BT_MESH_SIG_MODEL_ID_GENERIC_ONOFF_SERVER    0x1000
#define BT_MESH_SIG_MODEL_ID_GENERIC_ONOFF_CLIENT    0x1001
#define BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER    0x1002
#define BT_MESH_SIG_MODEL_ID_GENERIC_LEVEL_CLIENT    0x1003
#define BT_MESH_SIG_MODEL_ID_GENERIC_DEFAULT_TRANSITION_TIME_SERVER    0x1004
#define BT_MESH_SIG_MODEL_ID_GENERIC_DEFAULT_TRANSITION_TIME_CLIENT    0x1005
#define BT_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SERVER    0x1006
#define BT_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SETUP_SERVER    0x1007
#define BT_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_CLIENT    0x1008
#define BT_MESH_SIG_MODEL_ID_GENERIC_POWER_LEVEL_SERVER    0x1009
#define BT_MESH_SIG_MODEL_ID_GENERIC_POWER_LEVEL_SETUP_SERVER    0x100a
#define BT_MESH_SIG_MODEL_ID_GENERIC_POWER_LEVEL_CLIENT    0x100b
#define BT_MESH_SIG_MODEL_ID_GENERIC_BATTERY_SERVER    0x100c
#define BT_MESH_SIG_MODEL_ID_GENERIC_BATTERY_CLIENT    0x100d
#define BT_MESH_SIG_MODEL_ID_GENERIC_LOCATION_SERVER    0x100e
#define BT_MESH_SIG_MODEL_ID_GENERIC_LOCATION_SETUP_SERVER    0x100f
#define BT_MESH_SIG_MODEL_ID_GENERIC_LOCATION_CLIENT    0x1010
#define BT_MESH_SIG_MODEL_ID_GENERIC_ADMIN_PROPERTY_SERVER    0x1011
#define BT_MESH_SIG_MODEL_ID_GENERIC_MANUFACTURER_PROPERTY_SERVER    0x1012
#define BT_MESH_SIG_MODEL_ID_GENERIC_USER_PROPERTY_SERVER    0x1013
#define BT_MESH_SIG_MODEL_ID_GENERIC_CLIENT_PROPERTY_SERVER    0x1014
#define BT_MESH_SIG_MODEL_ID_GENERIC_PROPERTY_CLIENT    0x1015
/*!  @} */

typedef struct {
    UINT8 number_of_elements;   /**< Number of elements supported by the device */
    UINT16 algorithms;          /**< Supported algorithms and other capabilities */
    UINT8 public_key_type;      /**< Supported public key types */
    UINT8 static_oob_type;      /**< Supported static OOB Types */
    UINT8 output_oob_size;      /**< Maximum size of Output OOB supported */
    UINT16 output_oob_action;   /**< Supported Output OOB Actions */
    UINT8 input_oob_size;       /**< Maximum size in octets of Input OOB supported */
    UINT16 input_oob_action;    /**< Supported Input OOB Actions */
} BT_MESH_PROV_CAPABILITIES_T;

typedef struct {
    BT_MESH_PROV_CAPABILITIES_T cap;     /**< The capabilities detail value. */
} BT_MESH_EVT_PROV_CAPABILITIES_T;

typedef struct {
    UINT8 method;   /**< Authentication Method used */
    UINT8 action;   /**< Selected Output OOB Action or Input OOB Action or 0x00 */
    UINT8 size;     /**< Size of the Output OOB used or size of the Input OOB used or 0x00 */
} BT_MESH_EVT_PROV_REQUEST_AUTH_T;

typedef struct {
    UINT8 *pk;    /**< The public key received. */
} BT_MESH_EVT_PROV_SHOW_PK_T;

typedef struct {
    UINT8 auth[BT_MESH_AUTHENTICATION_SIZE];  /**< The authentication value received. */
} BT_MESH_EVT_PROV_SHOW_AUTH_T;

/* Provisioning error code used for #BT_MESH_PROV_ERROR_T */
typedef enum {
    BT_MESH_PROV_SUCCESS = 0,         /**< Provisioning success */
    BT_MESH_PROV_FAILED_ERROR_CODE_INVALID_PDU = 1,        /**< The provisioning protocol PDU is not recognized by the device */
    BT_MESH_PROV_FAILED_ERROR_CODE_INVALID_FORMAT = 2,     /**< The arguments of the protocol PDUs are outside expected values or the length of the PDU is different than expected */
    BT_MESH_PROV_FAILED_ERROR_CODE_UNEXPECTED_PDU = 3,     /**< The PDU received was not expected at this moment of the procedure */
    BT_MESH_PROV_FAILED_ERROR_CODE_CONFIRMATION_FAILED = 4, /**< The computed confirmation value was not successfully verified */
    BT_MESH_PROV_FAILED_ERROR_CODE_OUT_OF_RESOURCES = 5,   /**< The provisioning protocol cannot be continued due to insufficient resources in the device */
    BT_MESH_PROV_FAILED_ERROR_CODE_DECRYPTION_FAILED = 6,  /**< The Data block was not successfully decrypted */
    BT_MESH_PROV_FAILED_ERROR_CODE_UNEXPECTED_ERROR = 7,   /**< An unexpected error occurred that may not be recoverable */
    BT_MESH_PROV_FAILED_ERROR_CODE_CANNOT_ASSIGN_ADDR = 8, /**< The device cannot assign consecutive unicast addresses to all elements */
    BT_MESH_PROV_FAILED_ERROR_CODE_TRANSACTION_TIMEOUT = 200, /**MTK private enum field, reserve 200 elements for original SDK */
    BT_MESH_PROV_FAILED_ERROR_CODE_PROVISION_TIMEOUT = 201,
    BT_MESH_PROV_FAILED_ERROR_CODE_AUTHENTICATION_FAILED = 202,
    BT_MESH_PROV_FAILED_ERROR_INVALID_PARAMS = 203,
} BT_MESH_PROV_ERROR_T;

typedef struct {
    BT_MESH_PROV_ERROR_T reason;       /**< Indicate the provisioning process success or failed reason. */
    UINT16 address;      /**< Indicate the target unicast address. */
    UINT8 device_key[BT_MESH_DEVKEY_SIZE]; /**< Indicate the device key. */
    BOOL success;       /**< Indicate the provisioning process is successfull or not. */
    BOOL gatt_bearer;
} BT_MESH_EVT_PROV_DONE_T;

typedef struct {
    UINT8 uuid[BT_MESH_UUID_SIZE];   /**< The unprovisioned device UUID. */
    UINT16 oob_info;                  /**< The OOB information of unprovisioned device. */
    UINT8 *uri_hash;                  /**< The Uri hash value of unprovisioned device, may be NULL. */
} BT_MESH_PROV_UNPROVISIONED_DEVICE_T;

typedef struct {
    UINT32 total_count;   //total number of the scanned ud
    UINT32 current_index; //the index of current notified ud, start from 1. (<= total_count)
    BT_MESH_PROV_UNPROVISIONED_DEVICE_T ud;  //current ud info
} BT_MESH_EVT_PROV_SCAN_UD_T;

typedef enum {
    BT_MESH_PROV_FACTOR_CONFIRMATION_KEY,
    BT_MESH_PROV_FACTOR_RANDOM_PROVISIONER,
    BT_MESH_PROV_FACTOR_RANDOM_DEVICE,
    BT_MESH_PROV_FACTOR_CONFIRMATION_PROVISIONER,
    BT_MESH_PROV_FACTOR_CONFIRMATION_DEVICE,
    BT_MESH_PROV_FACTOR_PUB_KEY,
    BT_MESH_PROV_FACTOR_AUTHEN_VALUE,
    BT_MESH_PROV_FACTOR_AUTHEN_RESULT,
} BT_MESH_PROV_FACTOR_TYPE_T;

//This is a common structure for provisioning factor data, which may be set from APP or notified to APP. It is implementation dependent.
typedef struct {
    BT_MESH_PROV_FACTOR_TYPE_T type;
    UINT8 *buf;
    UINT16 buf_len;
} BT_MESH_PROV_FACTOR_T;

typedef enum
{
    BT_MESH_REPORT_TYPE_IND = 0x00,                 ///< Type for ADV_IND found (passive)
    BT_MESH_REPORT_TYPE_DIRECT_IND = 0x01,          ///< Type for ADV_DIRECT_IND found (passive)
    BT_MESH_REPORT_TYPE_SCAN_IND    = 0x02,         ///< Type for ADV_SCAN_IND found (passive)
    BT_MESH_REPORT_TYPE_NONCONN_IND  = 0x03,        ///< Type for ADV_NONCONN_IND found (passive)
    BT_MESH_REPORT_TYPE_SCAN_RSP = 0x04             ///< Type for SCAN_RSP found (active)
} BT_MESH_REPORT_TYPE;

typedef enum {
    BT_MESH_BLE_ADDR_TYPE_PUBLIC = 0,                /**< public address */
    BT_MESH_BLE_ADDR_TYPE_RANDOM_STATIC = 1,         /**< random static address */
    BT_MESH_BLE_ADDR_TYPE_RANDOM_RESOLVABLE = 2,     /**< random resolvable addresss */
    BT_MESH_BLE_ADDR_TYPE_RANDOM_NON_RESOLVABLE = 3, /**< random non resolvable address */
} BT_MESH_BLE_ADDR_TYPE_T;

typedef struct {
    BT_MESH_BLE_ADDR_TYPE_T addr_type;               /**< address type */
    CHAR addr[MAX_BDADDR_LEN];                      /**< address byte array */
} BT_MESH_BLE_ADDR_T;

typedef struct
{
    BT_MESH_BLE_ADDR_T    peer_addr;
    INT32                 rssi;
    BT_MESH_REPORT_TYPE   type;
    UINT8                 dlen;
    UINT8                 data[BT_MESH_ADV_DATA_SIZE];
} BT_MESH_EVT_ADV_REPORT_T;

typedef enum {
    BT_MESH_KEY_REFRESH_STATE_NONE = 0,    /**< Key refresh phase 0. Indicates normal device operation. */
    BT_MESH_KEY_REFRESH_STATE_1 = 1,       /**< Key refresh phase 1. Old keys are used for packet transmission, but new keys can be used to receive messages. */
    BT_MESH_KEY_REFRESH_STATE_2 = 2,       /**< Key refresh phase 2. New keys are used for packet transmission, but old keys can be used to receive messages. */
    BT_MESH_KEY_REFRESH_STATE_3 = 3,       /**< Key refresh phase 3. Used to complete a key refresh procedure and transition back to phase 0. */
} BT_MESH_KEY_REFRESH_STATE_T;

typedef struct {
    UINT16 netkey_index;  /**< The network key index. */
    BT_MESH_KEY_REFRESH_STATE_T phase;          /**< Current key refresh phase. */
} BT_MESH_EVT_KEY_REFRESH_T;

typedef enum {
    BT_MESH_IV_UPDATE_STATE_NORMAL = 0,        /**< Indicates IV update is in normal operation. */
    BT_MESH_IV_UPDATE_STATE_IN_PROGRESS = 1,   /**< Indicates IV update is in progress. */
} BT_MESH_IV_UPDATE_STATE_T;

typedef struct {
    UINT32 iv_index;  /**< The IV index currently used for sending messages. */
    BT_MESH_IV_UPDATE_STATE_T state;      /**< Current IV update state.*/
    UINT8 iv_phase;
}BT_MESH_EVT_IV_UPDATE_T;

typedef struct {
    UINT32 seq_num;
}BT_MESH_EVT_SEQ_CHANGE_T;

typedef struct {
    UINT16 address;
    UINT8 active;
}BT_MESH_EVT_HEARTBEAT_T;

typedef enum
{
    BT_MESH_OTA_ERROR_CODE_SUCCESS,                  /**< Error code of indicating success.*/
    BT_MESH_OTA_ERROR_CODE_WRONG_FIRMWARE_ID,        /**< Error code of inidcating wrong firmware id.*/
    BT_MESH_OTA_ERROR_CODE_BUSY,                     /**< Error code of inidcating busy of distributor*/
    BT_MESH_OTA_ERROR_CODE_NO_RESPONSE,              /**< Error code of inidcating no response of distributor*/
    BT_MESH_OTA_ERROR_CODE_USER_STOP,                /**< Error code of inidcating user interuption*/
} BT_MESH_OTA_ERROR_CODE_T;

typedef enum
{
    BT_MESH_OTA_EVENT_DISTRIBUTION_STARTING, /**< Event id for informing status of a new distribution was starting.*/
    BT_MESH_OTA_EVENT_DISTRIBUTION_STARTED,  /**< Event id for informing status of a new distribution was started.*/
    BT_MESH_OTA_EVENT_DISTRIBUTION_ONGOING,  /**< Event id for informing status of the distribution was ongoing.*/
    BT_MESH_OTA_EVENT_DISTRIBUTION_STOP,     /**< Event id for informing status of the distirbution was stopped.*/
    BT_MESH_OTA_EVENT_DISTRIBUTION_QUEUED,   /**< Event id for informing status of a new distribution was queued.*/
    BT_MESH_OTA_EVENT_DISTRIBUTION_DFU_READY,  /**< Event id for informing status of the distribution was dfu ready.*/
} BT_MESH_OTA_EVENT_T;

typedef enum
{
    BT_MESH_OTA_NODE_UPDATE_STATUS_SUCCESS = 0,
    BT_MESH_OTA_NODE_UPDATE_STATUS_IN_PROGRESS,
    BT_MESH_OTA_NODE_UPDATE_STATUS_CANCELED,    //failed with some problem
    BT_MESH_OTA_NODE_UPDATE_STATUS_DFU_READY,
} BT_MESH_OTA_NODE_UPDATE_STATUS_T;

typedef struct
{
    UINT16 addr;
    BT_MESH_OTA_NODE_UPDATE_STATUS_T status;
} BT_MESH_OTA_NODE_UPDATE_STATUS_INFO_T;

typedef struct {
    BT_MESH_OTA_EVENT_T event_id; /** Event id*/
    BT_MESH_OTA_ERROR_CODE_T error_code; /**< Status code*/
    UINT32 serial_number; /**< Serial number*/
    UINT32 firmware_id;   /**< Firmware id*/
    UINT32 time_escaped; /**< time escaped from started*/
    UINT16 nodes_num;
    UINT8 curr_block;
    UINT8 total_block;
    UINT16 curr_chunk;
    UINT16 chunk_mask;
    BT_MESH_OTA_NODE_UPDATE_STATUS_INFO_T *nodes_status; //this is an array with element number of nodes_num
} BT_MESH_EVT_OTA_T;

typedef enum {
    BT_MESH_FRIENDSHIP_TERMINATED = 0,         /**< The friendship is terminated. */
    BT_MESH_FRIENDSHIP_ESTABLISHED = 1,        /**< The friendship is successfully established. */
    BT_MESH_FRIENDSHIP_ESTABLISH_FAILED = 2,       /**< The friendship is not established. */
    BT_MESH_FRIENDSHIP_REQUEST_FRIEND_TIMEOUT = 3, /**< Request friend procedure timeout. The status is only received when low power feature in use. */
    BT_MESH_FRIENDSHIP_SELECT_FRIEND_TIMEOUT = 4,  /**< Select friend procedure timeout. The status is only received when low power feature in use. */
} BT_MESH_FRIDSHIP_STATUS_T;

typedef struct {
    UINT16 address;                                   /**< Indicates the friend or the low-power node unicast address. */
    BT_MESH_FRIDSHIP_STATUS_T status;                /**< Indicates the friendship status between the nodes. */
} BT_MESH_EVT_FRIDSHIP_STATUS_T;

typedef enum {
    BT_MESH_ERROR_NO_RESOURCE_TO_ADD_REPLAYPROTECTION,
} BT_MESH_ERROR_CODE_TYPE_T;

typedef enum {
    BT_MESH_BEARER_GATT_STATUS_CONNECTED = 0,      /**< Bearer GATT is connected. */
    BT_MESH_BEARER_GATT_STATUS_DISCONNECTED = 1,   /**< Bearer GATT is disconnected. */
    BT_MESH_BEARER_GATT_STATUS_NO_SERVICE = 2,     /**< Bearer GATT failed to be established because the specified service was not found. */
    BT_MESH_BEARER_GATT_STATUS_NO_CHARACTERISTIC = 3,  /**< Bearer GATT failed to be established because the specified characteristics were not found. */
    BT_MESH_BEARER_GATT_STATUS_WRITE_CCCD_FAILED = 4,  /**< Bearer GATT failed to be established because writing the CCCD failed. */
} BT_MESH_BEARER_GATT_STATUS_T;

typedef struct {
    UINT32 handle;  /**< The handle of this connection. */
    BT_MESH_BEARER_GATT_STATUS_T status;      /**< The status of bearer GATT. */
} BT_MESH_EVT_BEARER_GATT_STATUS_T;

typedef struct {
    BT_MESH_ERROR_CODE_TYPE_T type;
} BT_MESH_EVT_ERROR_CODE_T;

typedef struct {
    union {
        BT_MESH_EVT_PROV_CAPABILITIES_T      prov_cap;           /**<  parameter of mesh event @ref BT_MESH_EVT_PROV_CAPABILITIES */
        BT_MESH_EVT_PROV_REQUEST_AUTH_T      prov_request_auth;  /**<  parameter of mesh event @ref BT_MESH_EVT_PROV_REQUEST_OOB_AUTH_VALUE */
        BT_MESH_EVT_PROV_SHOW_PK_T           prov_show_pk;       /**<  parameter of mesh event @ref BT_MESH_EVT_PROV_SHOW_OOB_PUBLIC_KEY */
        BT_MESH_EVT_PROV_SHOW_AUTH_T         prov_show_auth;     /**<  parameter of mesh event @ref BT_MESH_EVT_PROV_SHOW_OOB_AUTH_VALUE */
        BT_MESH_EVT_PROV_DONE_T              prov_done;          /**<  parameter of mesh event @ref BT_MESH_EVT_PROV_DONE */
        BT_MESH_EVT_PROV_SCAN_UD_T           prov_scan_ud;       /**<  parameter of mesh event @ref BT_MESH_EVT_PROV_SCAN_UD_RESULT */
        BT_MESH_PROV_FACTOR_T                prov_factor;
        BT_MESH_EVT_ADV_REPORT_T             adv_report;
        BT_MESH_EVT_KEY_REFRESH_T            key_refresh;
        BT_MESH_EVT_IV_UPDATE_T              iv_update;          /**<  parameter of mesh event @ref BLE_MESH_EVT_IV_UPDATE */
        BT_MESH_EVT_SEQ_CHANGE_T             seq_change;
        BT_MESH_EVT_HEARTBEAT_T              heartbeat;
        BT_MESH_EVT_OTA_T                    ota_evt;
        BT_MESH_EVT_FRIDSHIP_STATUS_T        friendship_status;
        BT_MESH_EVT_BEARER_GATT_STATUS_T     bearer_gatt_status;
        BT_MESH_EVT_ERROR_CODE_T             error_code;
    } mesh;
} BT_MESH_EVT_T;

typedef enum {
    BT_MESH_EVT_INIT_DONE = BT_MESH_EVENT_MESH,
    BT_MESH_EVT_PROV_CAPABILITIES,
    BT_MESH_EVT_PROV_REQUEST_OOB_PUBLIC_KEY,
    BT_MESH_EVT_PROV_REQUEST_OOB_AUTH_VALUE,
    BT_MESH_EVT_PROV_SHOW_OOB_PUBLIC_KEY,
    BT_MESH_EVT_PROV_SHOW_OOB_AUTH_VALUE,
    BT_MESH_EVT_PROV_DONE,
    BT_MESH_EVT_PROV_SCAN_UD_RESULT,
    BT_MESH_EVT_CONFIG_RESET,
    BT_MESH_EVT_FRIENDSHIP_STATUS,                     /**< Event for mesh friendship status change. */
    BT_MESH_EVT_LPN_FRIEND_OFFER,                      /**< Event for mesh LPN receiving friend offer. */
    BT_MESH_EVT_LPN_FRIEND_SUBSCRIPTION_LIST_CONFRIM,  /**< Event for mesh LPN receiving friend subscription list confirm. */
    BT_MESH_EVT_HEARTBEAT,                             /**< Event for mesh heartbeat. */
    BT_MESH_EVT_IV_UPDATE,                             /**< Event for mesh IV index update. */
    BT_MESH_EVT_KEY_REFRESH,                           /**< Event for mesh key refresh. */
    BT_MESH_EVT_BEARER_GATT_STATUS,                    /**< Event for the mesh bearer GATT status. */

    //The above enum value match with SDK
    BT_MESH_EVT_UD_RESULT_COMPLETE = BT_MESH_EVENT_MESH+200,
    BT_MESH_EVT_PROV_FACTOR,
    BT_MESH_EVT_SEQ_CHANGE,
    BT_MESH_EVT_OTA_EVENT,
    BT_MESH_EVT_ERROR_CODE,
    BT_MESH_EVT_ADV_REPORT = BT_MESH_EVENT_ADV_REPORT,
} BT_MESH_EVENT_ID;

typedef struct {
    UINT32 evt_id; ///< Event ID
    union
    {
      BT_MESH_EVT_T mesh_evt; ///< MESH event structure
    }evt;
} BT_MESH_BT_EVENT_T;

typedef enum {
    BT_MESH_ADDRESS_TYPE_UNASSIGNED = 0,   /**< unassigned address */
    BT_MESH_ADDRESS_TYPE_UNICAST,          /**< unicast address */
    BT_MESH_ADDRESS_TYPE_VIRTUAL,          /**< virtual address */
    BT_MESH_ADDRESS_TYPE_GROUP,            /**< group address */
} BT_MESH_ADDRESS_TYPE_T;

typedef struct {
    BT_MESH_ADDRESS_TYPE_T type;   /**< the address type of this address */
    UINT16 value;                 /**< address value */
    const UINT8 *virtual_uuid;    /**< virtual uuid value, must be NULL unless address type is #BT_MESH_ADDRESS_TYPE_VIRTUAL */
} BT_MESH_ADDRESS_T;

typedef struct {
    UINT16 netidx;                /**< index of network key */
    UINT16 appidx;                /**< index of application key, if 0xFFFF means using device key */
    UINT8 *device_key;            /**< device key value, can't be NULL if appidx is 0xFFFF. */
} BT_MESH_SECURITY_T;

typedef struct {
    BT_MESH_ADDRESS_T dst;         /**< destination address information  */
    UINT16 src;                   /**< source unicast address */
    UINT8 ttl;                    /**< ttl value */
    const UINT8 *data;            /**< data buffer to be sent */
    UINT16 data_len;              /**< data buffer length */
    BT_MESH_SECURITY_T security;   /**< security information */
} BT_MESH_TX_PARAMS_T;

typedef struct {
    UINT16 opcode;        /**< Operation code. */
    UINT16 company_id;    /**< Company id, use #MESH_MODEL_COMPANY_ID_NONE if this is a SIG access message */
} BT_MESH_ACCESS_OPCODE_T;

typedef struct {
    UINT16 src_addr;      /**< The source address of this message. */
    UINT16 dst_addr;      /**< The destination address of this message */
    UINT16 appkey_index;        /**< The application key index used for this message. */
    UINT16 netkey_index;     /**< The network key index used for this message. */
    UINT8 rssi;           /**< The RSSI value . */
    UINT8 ttl;            /**< The received TTL value . */
} BT_MESH_ACCESS_MESSAGE_RX_META_T;

typedef struct {
    BT_MESH_ACCESS_OPCODE_T opcode;    /**< The operation code information. */
    UINT8 *buf;                       /**< The received message buffer. */
    UINT16 buf_len;                   /**< The length of received message. */
    BT_MESH_ACCESS_MESSAGE_RX_META_T meta_data;    /**< The metadata of this message. */
} BT_MESH_ACCESS_MESSAGE_RX_T;

//typedef VOID (*bt_mesh_access_msg_handler) (UINT16 model_handle, const BT_MESH_ACCESS_MESSAGE_RX_T *msg, const VOID *arg);
typedef struct {
    UINT16 model_handle;
    BT_MESH_ACCESS_MESSAGE_RX_T *msg;
    VOID *arg;
} BT_MESH_CBK_ACCESS_MSG_T;

typedef VOID (*bt_mesh_access_msg_handler)(BT_MESH_CBK_ACCESS_MSG_T *msg, VOID *pv_tag);

typedef struct {
    BT_MESH_ACCESS_OPCODE_T opcode;            /**< The operation code information. */
    bt_mesh_access_msg_handler  handler;    /**< The message handler for this opcode. */
} BT_MESH_ACCESS_OPCODE_HANDLER_T;

typedef struct {
    UINT32 model_id;                      /**< The model id, could be SIG defined value or Vendor defined value. */
    UINT16 element_index;                 /**< The target element index to add model. */
    const BT_MESH_ACCESS_OPCODE_HANDLER_T *opcode_handlers;    /**< The access message handler for this model. */
    UINT8 opcode_count;                   /**< Indicate how many opcode need to handle in this model. */
} BT_MESH_MODEL_ADD_PARAMS_T;

typedef enum {
    BT_MESH_ROLE_PROVISIONEE,      /**< act as a provisionee */
    BT_MESH_ROLE_PROVISIONER,      /**< act as a provisioner */
} BT_MESH_ROLE_T;

typedef struct {
    BT_MESH_PROV_CAPABILITIES_T cap;   /**< The capabilities parameters. */
} BT_MESH_PROV_PROVISIONEE_PARAMS_T;

typedef struct {
    UINT8 device_uuid[BT_MESH_UUID_SIZE];    /**< The device UUID. */
    UINT16 oob_info;                      /**< The OOB information of this device. Please reference to Provision OOB Info*/
    UINT8 default_ttl;                    /**< The default TTL value. */
    const CHAR *uri;                        /**< The uri infomation of this device, can be NULL */
} BT_MESH_CONFIG_INIT_PARAMS_T;

typedef enum {
    BT_MESH_FEATURE_HEARTBEAT = 1 << 0,   /**heartbeat feature */
    BT_MESH_FEATURE_OTA = 1 << 1,          /**OTA feature */
} BT_MESH_FEATURE_MASK_T;

typedef struct {
    UINT8 lpn_number;                     /**< The max number of low power nodes to be friends with. The default and maximum value is 2. */
    UINT8 queue_size;                     /**< The size of messages that can store for a Low Power node. The default and maximum value is 2. */
    UINT8 subscription_list_size;         /**< The size of subscription list that supported for a Low Power node. The default and maximum value is 5. */
} BT_MESH_FRIEND_INIT_PARAMS_T;

/** @brief The initialization parameters for mesh debug module. This parameter is used in #ble_mesh_init. */
typedef struct {
    UINT16 verbose_level;           /**< default debug level, verbose output */
    UINT16 info_level;            /**< default info level, rich output */
    UINT16 notify_level;          /**< default notify level, few output */
    UINT16 warning_level;         /**< default warning level, rare output */
} BT_MESH_DEBUG_INIT_PARAMS_T;

typedef struct {
    UINT16 max_remote_node_cnt;       /**< the max number of remote node*/
    UINT8 save2flash;             /**< automatically save mesh data to flash or not*/
} BT_MESH_CUSTOMIZE_PARA_T;

typedef struct {
    BT_MESH_ROLE_T role;           /**< mesh role */
    BT_MESH_PROV_PROVISIONEE_PARAMS_T *provisionee;       /**< initialization parameters of provision, can't be NULL when role is #BT_MESH_ROLE_PROVISIONEE */
    BT_MESH_CONFIG_INIT_PARAMS_T *config;   /**< initialization parameters of config */
    UINT32 feature_mask;    /**new feature mask **/
    BT_MESH_FRIEND_INIT_PARAMS_T *friend_params;              /**< initialization parameters of friend. The parameters is available only when friend feature is supported and enabled. Default value will be used when init_params is NULL. */
    BT_MESH_DEBUG_INIT_PARAMS_T *debug;                /**< initialization parameters of debug */
    BT_MESH_CUSTOMIZE_PARA_T *customize_params;
} BT_MESH_INIT_PARAMS_T;

typedef struct {
    UINT8 attention_duration;  /**< Attention Timer state */
} BT_MESH_PROV_INVITE_T;

typedef struct {
    UINT8 algorithm;               /**< The algorithm used for provisioning */
    UINT8 public_key;              /**< Public Key used */
    UINT8 authentication_method;   /**< Authentication Method used */
    UINT8 authentication_action;   /**< Selected Output OOB Action or Input OOB Action or 0x00 */
    UINT8 authentication_size;     /**< Size of the Output OOB used or size of the Input OOB used or 0x00 */
} BT_MESH_PROV_START_T;

typedef struct {
    UINT8 netkey[BT_MESH_KEY_SIZE];  /**< Network key value. */
    UINT16 netkey_index;          /**< Network key index. */
    UINT32 iv_index;              /**< IV Index */
    UINT16 address;               /**< Unicast address. */
    UINT8 flags;                  /**< Flags indicate current states of IV update and key refresh */
} BT_MESH_PROV_DATA_T;

typedef struct {
    BT_MESH_PROV_START_T start;     /**< The start parameters. */
    BT_MESH_PROV_DATA_T data;       /**< The data parameters. */
} BT_MESH_PROV_PROVISIONER_PARAMS_T;

typedef struct {
    UINT16 element_address;               /**< Address of the element. */
    BT_MESH_ADDRESS_T publish_address;     /**< Value of the publish address. */
    UINT16 appkey_index;                  /**< Index of the application key. */
    BOOL friendship_credential_flag;         /**< Value of the Friendship Credential Flag. */
    UINT8 publish_ttl;                    /**< Default TTL value for the outgoing messages. */
    UINT8 publish_period;                 /**< Period for periodic status publishing. */
    UINT8 retransmit_count;               /**< Number of retransmissions for each published message. */
    UINT8 retransmit_interval_steps;      /**< Number of 50-millisecond steps between retransmissions. */
    UINT32 model_id;                      /**< SIG Model ID or Vendor Model ID. */
} BT_MESH_MODEL_PUBLICATION_STATE_T;

typedef struct {
    UINT16 destination;                   /**< Destination address for Heartbeat messages. */
    UINT8 count_log;                      /**< Destination address for Heartbeat messages. */
    UINT8 period_log;                     /**< Period for sending Heartbeat messages. */
    UINT8 ttl;                            /**< TTL to be used when sending Heartbeat messages. */
    UINT16 features;                      /**< Bit field indicating features that trigger Heartbeat messages when changed. */
    UINT16 netkey_index;                  /**< Network key index. */
} BT_MESH_HEARTBEAT_PUBLICATION_T;

typedef struct {
    UINT16 source;                        /**< Source address for Heartbeat messages. */
    UINT16 destination;                   /**< Destination address for Heartbeat messages. */
    UINT8 period_log;                     /**< Period for receiving Heartbeat messages. */
} BT_MESH_HEARTBEAT_SUBSCRIPTION_T;

typedef struct {
    UINT8 isValidData;
    UINT16 keyidx;
    UINT8 key[BT_MESH_KEY_SIZE];
    UINT8 ivphase;
    UINT32 ivIndex;
    UINT8 phase;
    UINT8 node_identity;
    UINT8 tmpkey[BT_MESH_KEY_SIZE];
} BT_MESH_NETKEY_FLASH_DATA_T;

typedef struct {
    UINT8 isValidData;
    UINT16 netkeyIdx;
    UINT8 lpn_addr[2];
    UINT8 friend_addr[2];
    UINT8 lpn_counter[2];
    UINT8 friend_counter[2];
} BT_MESH_FRIEND_FLASH_DATA_T;

typedef struct {
    UINT8 isValidData;
    UINT16 appkeyIdx;
    UINT8 key[BT_MESH_KEY_SIZE];
    UINT16 netkeyIdx;
    UINT8 phase;
    UINT8 tmpkey[BT_MESH_KEY_SIZE];
} BT_MESH_APPKEY_FLASH_DATA_T;

typedef struct {
    UINT8 isValidData;
    UINT16 appkeyIdx;
    UINT8 idLength;
    UINT32 model_id;
    UINT16 unicast_addr;
} BT_MESH_MODEL_FLASH_DATA_T;

typedef struct {
    UINT16 addr;                           /**< publish address */
    UINT16 appkey_index;              /**< AppKey index */
    UINT16 flag;                       /**< friendship credentials flag */
    UINT16 rfu;                        /**< reserve for future use. */
    UINT8 ttl;                             /**< publish TTL */
    UINT8 period;                          /**< publish period */
    UINT8 retransmit_count;            /**< publish retransmit count */
    UINT8 retransmit_interval_steps;   /**< Publish retransmit interval steps */
} BT_MESH_BLE_MESH_MODEL_PUBLICATION_T;

typedef struct {
    UINT8 isValidData;
    UINT32 model_id;
    UINT16 unicast_addr;
    BT_MESH_BLE_MESH_MODEL_PUBLICATION_T model_publication;
} BT_MESH_MESH_MODEL_PUBLICATION_FLASH_T;

typedef struct {
    UINT8 isValidData;
    UINT32 model_id;
    UINT16 unicast_addr;
    UINT16 subscriptionAddr;
} BT_MESH_MODEL_SUBSCRIPTION_FLASH_DATA_T;

typedef struct {
    UINT8 isValidData;
    UINT8 uuid[BT_MESH_UUID_SIZE];
    UINT8 deviceKey[BT_MESH_KEY_SIZE];
    UINT16 unicast_addr;
} BT_MESH_DEVICE_FLASH_DATA_T;

typedef struct {
    UINT8 network_transmit_count;             /**< Network transmit count */
    UINT8 network_transmit_interval_steps;    /**< Network transmit interval steps */
} BT_MESH_NETWORK_TRANSMIT_T;

typedef struct {
    UINT8 relay_retransmit_count;             /**< Relay retransmit count */
    UINT8 relay_retransmit_interval_steps;    /**< Relay retransmit interval steps */
} BT_MESH_RELAY_TRANSMIT_T;

typedef struct {
    UINT8 isValidData;
    UINT8 secureNetworkBeacon;
    UINT8 defaultTTL;
    UINT8 gattProxy;
    UINT8 is_friend;
    UINT8 relay;
    UINT8 nodeIdentity;
    UINT8 keyRefreshPhase;
    UINT16 unicastAddr;
    BT_MESH_HEARTBEAT_PUBLICATION_T heartbeatPublication;
    BT_MESH_HEARTBEAT_SUBSCRIPTION_T heartbeatSubscription;
    BT_MESH_NETWORK_TRANSMIT_T networkTransmit;
    BT_MESH_RELAY_TRANSMIT_T relayRetransmit;
} BT_MESH_CONFIGURATION_SERVER_FLASH_DATA_T;

typedef struct {
    UINT8 fastPeriod;
} BT_MESH_HEALTH_PERIOD_T;

typedef struct {
    UINT8 isValidData;
    BT_MESH_HEALTH_PERIOD_T healthPeriod;
    UINT8 attention;
    UINT16 appkeyIdx;
    UINT16 unicastAddr;
} BT_MESH_HEALTH_SERVER_FLASH_DATA_T;

typedef struct {
    UINT8 isValidData;
    UINT32 seq_num;
} BT_MESH_SEQNUM_FLASH_DATA_T;

typedef struct {
    BT_MESH_NETKEY_FLASH_DATA_T               netkey[BT_MESH_NET_KEY_RECORD_NUMBER];
    BT_MESH_FRIEND_FLASH_DATA_T               friend_info[BT_MESH_FRIEND_RECORD_NUMBER];
    BT_MESH_APPKEY_FLASH_DATA_T               appkey[BT_MESH_APP_KEY_RECORD_NUMBER];
    BT_MESH_MODEL_FLASH_DATA_T                model[BT_MESH_MODEL_RECORD_NUMBER];
    BT_MESH_MESH_MODEL_PUBLICATION_FLASH_T    publication[BT_MESH_MODEL_PUBLICATION_RECORD_NUMBER];
    BT_MESH_MODEL_SUBSCRIPTION_FLASH_DATA_T   subscription[BT_MESH_MODEL_SUBSCRIPTION_RECORD_NUMBER];
    BT_MESH_DEVICE_FLASH_DATA_T               local_deviceInfo[BT_MESH_LOCAL_DEVICE_INFO_RECORD_NUMBER];
    BT_MESH_SEQNUM_FLASH_DATA_T               seq_info[BT_MESH_SEQUENCE_NUMBER_RECORD_NUMBER];
} BT_MESH_RECORD_T;

////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    UINT16      src_addr;
    UINT16      dst_addr;
    UINT8       ttl;
    UINT16      msg_netkey_index;
} BT_MESH_CONFIG_META_T;

typedef struct {
    BT_MESH_CONFIG_META_T meta;
} BT_MESH_CONFIG_BEACON_GET_T,  BT_MESH_CONFIG_DEFAULT_TTL_GET_T,
BT_MESH_CONFIG_GATT_PROXY_GET_T, BT_MESH_CONFIG_FRIEND_GET_T,
BT_MESH_CONFIG_RELAY_GET_T, BT_MESH_CONFIG_NETWORK_TRANSMIT_GET_T,
BT_MESH_CONFIG_NETKEY_GET_T, BT_MESH_CONFIG_NODE_RESET_T,
BT_MESH_CONFIG_HB_PUB_GET_T, BT_MESH_CONFIG_HB_SUB_GET_T;

typedef struct {
    BT_MESH_CONFIG_META_T meta;
    UINT8       beacon;
} BT_MESH_CONFIG_BEACON_SET_T;

typedef struct {
    BT_MESH_CONFIG_META_T meta;
    UINT8       page;
} BT_MESH_CONFIG_COMPOSITION_DATA_GET_T;

typedef struct {
    BT_MESH_CONFIG_META_T meta;
    UINT8       TTL;
} BT_MESH_CONFIG_DEFAULT_TTL_SET_T;

typedef struct {
    BT_MESH_CONFIG_META_T meta;
    UINT8       gatt_proxy;
} BT_MESH_CONFIG_GATT_PROXY_SET_T;

typedef struct {
    BT_MESH_CONFIG_META_T meta;
    UINT8       mesh_friend;
} BT_MESH_CONFIG_FRIEND_SET_T;

typedef struct {
    BT_MESH_CONFIG_META_T meta;
    UINT8       relay;
    UINT8       retransmit_count;
    UINT8       retransmit_interval_steps;
} BT_MESH_CONFIG_RELAY_SET_T;

typedef struct {
    BT_MESH_CONFIG_META_T meta;
    UINT16      element_addr;
    UINT32      model_id;
} BT_MESH_CONFIG_MODEL_PUB_GET_T;

typedef struct {
    BT_MESH_CONFIG_META_T meta;
    BT_MESH_MODEL_PUBLICATION_STATE_T *state;
} BT_MESH_CONFIG_MODEL_PUB_SET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    BT_MESH_ADDRESS_T       address;
    UINT32      model_id;
} BT_MESH_CONFIG_MODEL_SUB_ADD_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    BT_MESH_ADDRESS_T       address;
    UINT32      model_id;
} BT_MESH_CONFIG_MODEL_SUB_DEL_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    BT_MESH_ADDRESS_T       address;
    UINT32      model_id;
} BT_MESH_CONFIG_MODEL_SUB_OW_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT32      model_id;
} BT_MESH_CONFIG_MODEL_SUB_DEL_ALL_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT16      model_id;
} BT_MESH_CONFIG_SIG_MODEL_SUB_GET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT32      model_id;
} BT_MESH_CONFIG_VENDOR_MODEL_SUB_GET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
    UINT8           *netkey;
} BT_MESH_CONFIG_NETKEY_ADD_T, BT_MESH_CONFIG_NETKEY_UPDATE_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
} BT_MESH_CONFIG_NETKEY_DEL_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
    UINT16      appkey_index;
    UINT8           *appkey;
} BT_MESH_CONFIG_APPKEY_ADD_T, BT_MESH_CONFIG_APPKEY_UPDATE_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
    UINT16      appkey_index;
} BT_MESH_CONFIG_APPKEY_DEL_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
} BT_MESH_CONFIG_APPKEY_GET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT16      appkey_index;
    UINT32      model_id;
} BT_MESH_CONFIG_MODEL_APP_BIND_T, BT_MESH_CONFIG_MODEL_APP_UNBIND_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT16      model_id;
} BT_MESH_CONFIG_SIG_MODEL_APP_GET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      element_addr;
    UINT32      model_id;
} BT_MESH_CONFIG_VENDOR_MODEL_APP_GET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
} BT_MESH_CONFIG_NODE_IDENTITY_GET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
    UINT8           identity;
} BT_MESH_CONFIG_NODE_IDENTITY_SET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
} BT_MESH_CONFIG_KEY_REFRESH_PHASE_GET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT16      netkey_index;
    UINT8           transition;
} BT_MESH_CONFIG_KEY_REFRESH_PHASE_SET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    BT_MESH_HEARTBEAT_PUBLICATION_T *publication;
} BT_MESH_CONFIG_HB_PUB_SET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    BT_MESH_HEARTBEAT_SUBSCRIPTION_T *subscription;
} BT_MESH_CONFIG_HB_SUB_SET_T;

typedef struct {
    BT_MESH_CONFIG_META_T   meta;
    UINT8           count;
    UINT8           interval_steps;
} BT_MESH_CONFIG_NETWORK_TRANSMIT_SET_T;

/** @brief configuration msg structure */
typedef struct {
    UINT16                            opcode;                                /**<The operation code information */
    union {
        BT_MESH_CONFIG_BEACON_GET_T             beacon_get;                            /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_BEACON_GET*/
        BT_MESH_CONFIG_BEACON_SET_T             beacon_set;                            /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_BEACON_SET */
        BT_MESH_CONFIG_COMPOSITION_DATA_GET_T   composition_data_get;                  /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_COMPOSITION_DATA_GET */
        BT_MESH_CONFIG_DEFAULT_TTL_GET_T        default_ttl_get;                       /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_GET */
        BT_MESH_CONFIG_DEFAULT_TTL_SET_T        default_ttl_set;                       /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_DEFAULT_TTL_SET */
        BT_MESH_CONFIG_GATT_PROXY_GET_T         gatt_proxy_get;                        /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_GET */
        BT_MESH_CONFIG_GATT_PROXY_SET_T         gatt_proxy_set;                        /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_GATT_PROXY_SET */
        BT_MESH_CONFIG_FRIEND_GET_T             friend_get;                            /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_FRIEND_GET */
        BT_MESH_CONFIG_FRIEND_SET_T             friend_set;                            /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_FRIEND_SET */
        BT_MESH_CONFIG_RELAY_GET_T              relay_get;                             /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_RELAY_GET */
        BT_MESH_CONFIG_RELAY_SET_T              relay_set;                             /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_RELAY_SET */
        BT_MESH_CONFIG_MODEL_PUB_GET_T          model_pub_get;                         /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_GET */
        BT_MESH_CONFIG_MODEL_PUB_SET_T          model_pub_set;                         /**<  parameter of config msg @ref BLE_MESH_ACCESS_MSG_CONFIG_MODEL_PUBLICATION_SET */
        BT_MESH_CONFIG_MODEL_SUB_ADD_T          model_sub_add;
        BT_MESH_CONFIG_MODEL_SUB_DEL_T          model_sub_del;
        BT_MESH_CONFIG_MODEL_SUB_OW_T           model_sub_ow;
        BT_MESH_CONFIG_MODEL_SUB_DEL_ALL_T      model_sub_del_all;
        BT_MESH_CONFIG_SIG_MODEL_SUB_GET_T      sig_model_sub_get;
        BT_MESH_CONFIG_VENDOR_MODEL_SUB_GET_T   vendor_model_sub_get;
        BT_MESH_CONFIG_NETKEY_ADD_T             netkey_add;
        BT_MESH_CONFIG_NETKEY_UPDATE_T          netkey_update;
        BT_MESH_CONFIG_NETKEY_DEL_T             netkey_del;
        BT_MESH_CONFIG_NETKEY_GET_T             netkey_get;
        BT_MESH_CONFIG_APPKEY_ADD_T             appkey_add;
        BT_MESH_CONFIG_APPKEY_UPDATE_T          appkey_update;
        BT_MESH_CONFIG_APPKEY_DEL_T             appkey_del;
        BT_MESH_CONFIG_APPKEY_GET_T             appkey_get;
        BT_MESH_CONFIG_MODEL_APP_BIND_T         model_app_bind;
        BT_MESH_CONFIG_MODEL_APP_UNBIND_T       model_app_unbind;
        BT_MESH_CONFIG_SIG_MODEL_APP_GET_T      sig_model_app_get;
        BT_MESH_CONFIG_VENDOR_MODEL_APP_GET_T   vendor_model_app_get;
        BT_MESH_CONFIG_NODE_IDENTITY_GET_T      node_identity_get;
        BT_MESH_CONFIG_NODE_IDENTITY_SET_T      node_identity_set;
        BT_MESH_CONFIG_NODE_RESET_T             node_reset;
        BT_MESH_CONFIG_KEY_REFRESH_PHASE_GET_T  key_ref_pha_get;
        BT_MESH_CONFIG_KEY_REFRESH_PHASE_SET_T  key_ref_pha_set;
        BT_MESH_CONFIG_HB_PUB_GET_T             hb_pub_get;
        BT_MESH_CONFIG_HB_PUB_SET_T             hb_pub_set;
        BT_MESH_CONFIG_HB_SUB_GET_T             hb_sub_get;
        BT_MESH_CONFIG_HB_SUB_SET_T             hb_sub_set;
        BT_MESH_CONFIG_NETWORK_TRANSMIT_GET_T   net_trans_get;
        BT_MESH_CONFIG_NETWORK_TRANSMIT_SET_T   net_trans_set;
    } data;
} BT_MESH_CONFIGURATION_MSG_TX_T;

/** @brief Mesh opcode of model data */
typedef enum {
    BT_MESH_MD_OP_SET_COMPOSITION_DATA_HEADER,
    BT_MESH_MD_OP_ADD_ELEMENT,
    BT_MESH_MD_OP_SET_ELEMENT_ADDR,
    BT_MESH_MD_OP_ADD_CONFIGURATION_SERVER,
    BT_MESH_MD_OP_ADD_CONFIGURATION_CLIENT,
    BT_MESH_MD_OP_ADD_HEALTH_SERVER,
    BT_MESH_MD_OP_ADD_HEALTH_CLIENT,
    BT_MESH_MD_OP_ADD_GENERIC_ONOFF_SERVER,
    BT_MESH_MD_OP_ADD_CTL_SETUP_SERVER,
    BT_MESH_MD_OP_ADD_GENERIC_POWER_ONOFF_CLIENT,
    BT_MESH_MD_OP_ADD_LIGHTNESS_CLIENT,
    BT_MESH_MD_OP_ADD_CTL_CLIENT,
    BT_MESH_MD_OP_ADD_HSL_SETUP_SERVER,
    BT_MESH_MD_OP_ADD_HSL_CLIENT,
    BT_MESH_MD_OP_ADD_GENERIC_LEVEL_SERVER,
    BT_MESH_MD_OP_ADD_GENERIC_LEVEL_CLIENT,
    BT_MESH_MD_OP_ADD_GENERIC_ONOFF_CLIENT,
    BT_MESH_MD_OP_ADD_MODEL = 500,
} BT_MESH_MODEL_DATA_OPCODE_T;

typedef struct {
    UINT8 *buf;
    UINT8 buf_len;
} BT_MESH_COMPOSITION_DATA_T;

typedef struct {
    UINT16 location;
    UINT16 *element_index;
} BT_MESH_ELEMENT_DATA_T;

typedef struct {
    UINT16 *model_handle;
    UINT16 element_index;
    bt_mesh_access_msg_handler callback;
} BT_MESH_HEALTH_SERVER_DATA_T,
BT_MESH_CONFIGURATION_CLIENT_DATA_T, BT_MESH_GENERIC_ONOFF_SERVER_DATA_T,
BT_MESH_CTL_SETUP_SERVER_DATA_T, BT_MESH_GENERIC_POWER_ONOFF_CLIENT_DATA_T,
BT_MESH_LIGHTNESS_CLIENT_DATA_T, BT_MESH_CTL_CLIENT_DATA_T,
BT_MESH_HSL_SETUP_SERVER_DATA_T, BT_MESH_HSL_CLIENT_DATA_T;

typedef struct {
    UINT8 test_id;            /**< Identifier of a most recently performed test. */
    UINT16 company_id;        /**< 16-bit Bluetooth assigned Company Identifier. */
    UINT8 *fault_array;       /**< An array contains a sequence of 1-octet fault values. */
    UINT8 fault_array_length; /**< Length of the fault array. */
} BT_MESH_HEALTH_CLIENT_EVT_CURRENT_STATUS_T;

typedef struct {
    UINT8 test_id;            /**< Identifier of a most recently performed test. */
    UINT16 company_id;        /**< 16-bit Bluetooth assigned Company Identifier. */
    UINT8 *fault_array;       /**< An array contains a sequence of 1-octet fault values. */
    UINT8 fault_array_length; /**< Length of the fault array. */
} BT_MESH_HEALTH_CLIENT_EVT_FAULT_STATUS_T;

typedef struct {
    UINT8 fast_period_divisor;    /**< Divider for the Publish Period. Modified Publish Period is used for sending Current Health Status messages when there are active faults to communicate.*/
} BT_MESH_HEALTH_CLIENT_EVT_PERIOD_STATUS_T;

typedef struct {
    UINT8 attention; /**< Value of the Attention Timer state, which represents the remaining duration of the attention state of a server in seconds. */
} BT_MESH_HEALTH_CLIENT_EVT_ATTENTION_STATUS_T;

typedef struct {
    union {
        BT_MESH_HEALTH_CLIENT_EVT_CURRENT_STATUS_T current_status;
        BT_MESH_HEALTH_CLIENT_EVT_FAULT_STATUS_T fault_status;
        BT_MESH_HEALTH_CLIENT_EVT_PERIOD_STATUS_T period_status;
        BT_MESH_HEALTH_CLIENT_EVT_ATTENTION_STATUS_T attention_status;
    } data;
} BT_MESH_HEALTH_CLIENT_EVT_T;

//typedef void (*bt_mesh_health_client_evt_handler)(UINT16 model_handle, const BT_MESH_ACCESS_MESSAGE_RX_T *msg, const BT_MESH_HEALTH_CLIENT_EVT_T *event);
typedef struct {
    UINT16 model_handle;
    BT_MESH_ACCESS_MESSAGE_RX_T *msg;
    BT_MESH_HEALTH_CLIENT_EVT_T *event;
} BT_MESH_CBK_HEALTH_CLIENT_EVT_T;

typedef void (*bt_mesh_health_client_evt_handler)(BT_MESH_CBK_HEALTH_CLIENT_EVT_T *event, VOID *pv_tag);

typedef struct {
    UINT16 *model_handle;
    UINT16 element_index;
    bt_mesh_health_client_evt_handler callback;
}BT_MESH_HEALTH_CLIENT_DATA_T;

typedef struct {
    UINT16 *model_handle;
    BT_MESH_MODEL_ADD_PARAMS_T *model_params;
} BT_MESH_MODEL_T;

typedef struct {
    UINT16 unicast_addr;
} BT_MESH_ELEMENT_ADDR_T;

typedef struct {
    UINT16 *model_handle;
    bt_mesh_access_msg_handler callback;
} BT_MESH_CONFIGURATION_SERVER_DATA_T;

/** @brief model data structure */
typedef struct {
    BT_MESH_MODEL_DATA_OPCODE_T                  opcode;                                    /**<  The operation code information @ref BT_MESH_MODEL_DATA_OPCODE_T */
    union {
        BT_MESH_COMPOSITION_DATA_T               composition_data;                          /**<  parameter of model data @ref MODEL_DATA_OP_SET_COMPOSITION_DATA_HEADER*/
        BT_MESH_ELEMENT_DATA_T                   element;                              /**<  parameter of model data @ref MODEL_DATA_OP_ADD_ELEMENT */
        BT_MESH_ELEMENT_ADDR_T                   element_addr;                              /**<  parameter of model data @ref MODEL_DATA_OP_SET_ELEMENT_ADDR */
        BT_MESH_CONFIGURATION_SERVER_DATA_T      configuration_server;
        BT_MESH_CONFIGURATION_CLIENT_DATA_T      configuration_client;                                  /**<  parameter of model data @ref MODEL_DATA_OP_ADD_CONFIGURATION_CLIENT */
        BT_MESH_HEALTH_SERVER_DATA_T             health_server;                        /**<  parameter of model data @ref MODEL_DATA_OP_ADD_HEALTH_SERVER */
        BT_MESH_HEALTH_CLIENT_DATA_T             health_client;
        BT_MESH_GENERIC_ONOFF_SERVER_DATA_T      generic_onoff_server;
        BT_MESH_CTL_SETUP_SERVER_DATA_T          ctl_setup_server;
        BT_MESH_GENERIC_POWER_ONOFF_CLIENT_DATA_T generic_power_onoff_client;
        BT_MESH_LIGHTNESS_CLIENT_DATA_T           lightness_client;
        BT_MESH_CTL_CLIENT_DATA_T                 ctl_client;
        BT_MESH_HSL_SETUP_SERVER_DATA_T          hsl_setup_server;
        BT_MESH_HSL_CLIENT_DATA_T                hsl_client;
        BT_MESH_MODEL_T                          model_data;                                /**<  parameter of model data @ref MODEL_DATA_OP_ADD_MODEL */
    } data;
} BT_MESH_MODEL_DATA_T;

/** @brief key opcode */
typedef enum {
    BT_MESH_KEY_ADD,         /**< add a netkey or appkey */
    BT_MESH_KEY_UPDATE,      /**< update a netkey or appkey */
    BT_MESH_KEY_USE_NEW_NETKEY,  //Key refreh phase2 - swithing to the new keys
    BT_MESH_KEY_REVOKE_OLD_NETKEY,   //Key refresh phase3 - revoking old keys
} BT_MESH_KEY_OPCODE_T;

/** @brief MESH netkey structure.*/
typedef struct {
    BT_MESH_KEY_OPCODE_T opcode;    /**< The operation code information */
    UINT8 *network_key;            /**< network key */
    UINT16 key_index;              /**< index of network key */
} BT_MESH_NETKEY_T;

/** @brief MESH appkey structure.*/
typedef struct {
    BT_MESH_KEY_OPCODE_T opcode;    /**< The operation code information */
    UINT8 *application_key;        /**< applicatin key */
    UINT16 appkey_index;           /**< index of applicatin key */
    UINT16 netkey_index;           /**< index of network key */
} BT_MESH_APPKEY_T;

/** @brief This structure defines a access message for sending. */
typedef struct {
    BT_MESH_ACCESS_OPCODE_T opcode;    /**< The operation code information. */
    UINT8 *p_buffer;                  /**< The message buffer for sending. */
    UINT16 length;                    /**< The length of this message. */
} BT_MESH_ACCESS_MESSAGE_TX_T;

/** @brief This struct defines the advertising parameters*/
typedef struct {
    UINT32 adv_period;            /**< adv period in milliseconds*/
    UINT16 min_interval;          /**< adv minimum interval in 625 us units*/
    UINT16 max_interval;          /**< adv maximum interval in 625 us units*/
    UINT8 resend;                 /**< resend number of times, 0xFF means resending forever until user discard it*/
} BT_MESH_BEARER_ADV_PARAMS_T;

/** @brief This struct defines the scanner parameters*/
typedef struct {
    UINT32 scan_period;           /**< scan period in milliseconds*/
    UINT16 scan_interval;         /**< scan interval in 625 us units*/
    UINT16 scan_window;           /**< scan window in 625 us units*/
} BT_MESH_BEARER_SCAN_PARAMS_T;

typedef enum {
    //the bit31 indicates role, 0 for provisionee, 1 for provisioner
    BT_MESH_FLASH_RECORD_ROLE_PROVISIONEE   = 0x00000000,
    BT_MESH_FLASH_RECORD_ROLE_PROVISIONER   = 0x80000000,

    BT_MESH_FLASH_RECORD_DATA          = 0x00000001,
    BT_MESH_FLASH_RECORD_SEQ_NUM       = 0x00000002,
    BT_MESH_FLASH_RECORD_REMOTE_NODE   = 0x00000004,
    //TODO Add more...
    BT_MESH_FLASH_RECORD_ALL           = 0x7FFFFFFF,
} BT_MESH_FLASH_RECORD_T;

typedef VOID (*BtAppMeshBtEventCbk)(BT_MESH_BT_EVENT_T *event);
typedef VOID (*BtAppMeshAccessMsgCbk)(BT_MESH_CBK_ACCESS_MSG_T *msg);
typedef VOID (*BtAppMeshHealthClientEvtCbk)(BT_MESH_CBK_HEALTH_CLIENT_EVT_T *event);

typedef struct
{
    BtAppMeshBtEventCbk bt_mesh_bt_event_cb;
    BtAppMeshAccessMsgCbk bt_mesh_acces_msg_cb;
    BtAppMeshHealthClientEvtCbk bt_mesh_health_client_evt_cb;
    //...
} BT_APP_MESH_CB_FUNC_T;

typedef enum {
    BT_MESH_DUMP_TYPE_UUID = 0,
    BT_MESH_DUMP_TYPE_NETWORK,
    BT_MESH_DUMP_TYPE_TRANSPORT,
    BT_MESH_DUMP_TYPE_CONFIG,
    BT_MESH_DUMP_TYPE_MODEL,
    BT_MESH_DUMP_TYPE_LPN,
    BT_MESH_DUMP_TYPE_PROXY,
    BT_MESH_DUMP_TYPE_ADV,
    BT_MESH_DUMP_TYPE_ALL,
} BT_MESH_DUMP_TYPE_T;

typedef enum {
    BT_MESH_GATT_SERVICE_PROXY,                      /**< Mesh proxy service */
    BT_MESH_GATT_SERVICE_PROVISION,                  /**< Mesh provisioning service */
} BT_MESH_GATT_SERVICE_T;

typedef struct {
    UINT16 cid;                           /**< Contains a 16-bit company identifier assigned by the Bluetooth SIG */
    UINT16 pid;                           /**< Contains a 16-bit vendor-assigned product identifier */
    UINT16 vid;                           /**< Contains a 16-bit vendor-assigned product version identifier */
    UINT16 crpl;                          /**< Contains a 16-bit value representing the minimum number of replay protection list entries in a device */
    UINT16 features;                      /**< Contains a bit field indicating the device features */
    VOID *elements;                         /**< Contains a double-linked list of #ble_mesh_composition_element_t */
    UINT8 element_length;                 /**< The number of elements */
} BT_MESH_COMP_DATA_T;

typedef struct {
    UINT16 loc;               /**< The location of this element. */
    UINT8 num_s;              /**< The number of SIG models in this element. */
    UINT8 num_v;              /**< The number of vendor models in this element. */
    UINT16 *sig_models;       /**< SIG model IDs. */
    UINT32 *vendor_models;    /**< Vendor model IDs. */
} BT_MESH_COMP_ELEMENT_T;

typedef enum
{
    BT_MESH_MODEL_GENERIC_ONOFF_SET,
    BT_MESH_MODEL_GENERIC_ONOFF_GET,
    BT_MESH_MODEL_LIGHT_CTL_SET,
    BT_MESH_MODEL_LIGHT_CTL_GET,
    BT_MESH_MODEL_LIGHT_CTL_TEMPERATURE_SET,
    BT_MESH_MODEL_LIGHT_CTL_TEMPERATURE_GET,
    BT_MESH_MODEL_LIGHT_CTL_TEMPERATURE_RANGE_SET,
    BT_MESH_MODEL_LIGHT_CTL_TEMPERATURE_RANGE_GET,
    BT_MESH_MODEL_LIGHT_CTL_DEFAULT_SET,
    BT_MESH_MODEL_LIGHT_CTL_DEFAULT_GET,
} BT_MESH_MODEL_OPERATION_TYPE_T;

typedef struct
{
    BT_MESH_MODEL_OPERATION_TYPE_T op_type;
    UINT16 model_handle;
    BOOL reliable;
} BT_MESH_MODEL_OPERATION_T;

typedef struct
{
    UINT16 element_idx;
    UINT32 model_id;
}BT_MESH_EXTEND_MODEL_ELEMENT_INFO_T;

/** @brief MESH device key info structure.*/
typedef struct {
    UINT16 unicast_addr;
    UINT8  deviceKey[BT_MESH_KEY_SIZE];
    UINT8  uuid[BT_MESH_UUID_SIZE];
} BT_MESH_DEVKEY_INFO_T;

typedef enum {
    BT_MESH_DEV_INFO_OP_ADD_DEVKEY = 0,
    BT_MESH_DEV_INFO_OP_GET_DEVKEY,
    BT_MESH_DEV_INFO_OP_DELETE_DEVKEY,
    //...
} BT_MESH_DEVICE_INFO_OPCODE_T;

typedef struct {
    BT_MESH_DEVICE_INFO_OPCODE_T  opcode;
    union {
        BT_MESH_DEVKEY_INFO_T           devkey;    //For ADD_DEVKEY, all the fields shall be valid. For GET_DEVKEY, deviceKey[] and uuid[] are output. For DELETE_DEVKEY, only unicast_addr is used.
    } data;
} BT_MESH_DEVICE_INFO_T;

typedef enum
{
    BT_MESH_OTA_INITIATOR_OP_REG_MSG_HANDLER = 0,
    BT_MESH_OTA_INITIATOR_OP_FW_INFO_GET,
    BT_MESH_OTA_INITIATOR_OP_START_DISTRIBUTION,
    BT_MESH_OTA_INITIATOR_OP_STOP_DISTRIBUTION,
    BT_MESH_OTA_INITIATOR_OP_APPLY_DISTRIBUTION,
} BT_MESH_OTA_INITIATOR_OPCODE_T;

typedef struct
{
    UINT16 appkey_index;
    bt_mesh_access_msg_handler ota_msg_handler;
}BT_MESH_OTA_INITIATOR_MSG_HANDLER_T;

typedef struct
{
    UINT16 node_addr;
}BT_MESH_OTA_INITIATOR_FW_INFO_GET_T;

typedef struct
{
    CHAR obj_file[128];      //the path of the new firmware image object. e.g. /data/new_firmware.bin
    UINT32 obj_size;       //size of the image data which should be transferred
    UINT8 obj_id[8];       //unique object id
    UINT32 new_fw_id;      //fw version
    UINT16 appkey_index;      //the appkey index used for ota related msg, this appkey shall have been added to updaters.
    UINT16 distributor_addr;  //The address of the distributor node, currently, it is the same with initiator(self)
    UINT16 group_addr;        //the group address for the updaters
    UINT16 updaters_num;       //Number of updaters
    UINT16 *updaters;         //the address list of all the updaters in the group
    BOOL manual_apply;  //manual_apply or auto apply immediately when dfu ready
} BT_MESH_OTA_INITIATOR_START_PARAMS_T;

typedef struct
{
    UINT32 new_fw_id;
    UINT16 distributor_addr;  //The address of the distributor node, currently, it is the same with initiator
} BT_MESH_OTA_INITIATOR_STOP_PARAMS_T;

typedef struct
{
    UINT16 opcode;    /**< The operation code information */
    union
    {
        BT_MESH_OTA_INITIATOR_MSG_HANDLER_T msg_handler;
        BT_MESH_OTA_INITIATOR_FW_INFO_GET_T fw_info_get;
        BT_MESH_OTA_INITIATOR_START_PARAMS_T start_params;
        BT_MESH_OTA_INITIATOR_STOP_PARAMS_T stop_params;
    }params;
} BT_MESH_OTA_OPERATION_PARAMS_T;

typedef enum
{
    BT_MESH_MODE_OFF = 0,  //normal mode
    BT_MESH_MODE_ON = 1,   //mesh working mode
    BT_MESH_MODE_STANDBY = 2,  //standby mode
} BT_MESH_MODE_T;

typedef struct {
    UINT8 sn_increase_flag;            /**< 0: sn not increase,  1: sn increase*/
    UINT32 sn_increase_interval;          /**< seq number increase interval, unit: ms*/
    UINT16 adv_interval;          /**< adv  interval in 625 us units*/
    UINT32 adv_period;            /**< adv period in milliseconds*/
} BT_MESH_SPECIAL_PKT_PARAMS_T;

#endif /*  _U_BT_MW_MESH_H_ */





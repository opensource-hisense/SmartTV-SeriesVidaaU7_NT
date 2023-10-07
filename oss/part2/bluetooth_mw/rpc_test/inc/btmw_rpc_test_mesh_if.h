#ifndef __BTMW_RPC_TEST_MESH_IF_H__
#define __BTMW_RPC_TEST_MESH_IF_H__

#include <time.h>

#include "u_rpcipc_types.h"
#include "u_bt_mw_mesh.h"

#define BTMW_RPC_TEST_CMD_KEY_MESH   "MW_RPC_MESH"

#define BTMW_RPC_TEST_MESH_INVALID_ELEMENT_INDEX 0xFFFF

/*The Algorithm values*/
#define BTMW_RPC_TEST_MESH_PROV_CAPABILITY_ALGORITHM_FIPS_P256_ELLIPTIC_CURVE    (1<<0)    /**< Capabilities bit indicating that the FIPS P256 Elliptic Curve algorithm is supported. */

/*The Supported Public key OOB type */
#define BTMW_RPC_TEST_MESH_PROV_CAPABILITY_OOB_PUBLIC_KEY_TYPE_INBAND            (0)       /**< Capabilities bit indicating that the public key is available in-band. If no public key type is set, this is the default */
#define BTMW_RPC_TEST_MESH_PROV_CAPABILITY_OOB_PUBLIC_KEY_TYPE_OOB               (1<<0)    /**< Capabilities bit indicating that the public key is available OOB. */

/*Provision OOB Info */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_OTHER                          (1 << 0x00)   /**< Other location */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_ELECTRONIC_URI                 (1 << 0x01)   /**< Electronic / URI. */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_2D_MACHINE_READABLE_CODE       (1 << 0x02)   /**< 2D machine-readable code. */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_BAR_CODE                       (1 << 0x03)   /**< Bar code */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_NFC                            (1 << 0x04)   /**< Near Field Communication (NFC) */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_NUMBER                         (1 << 0x05)   /**< Number */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_STRING                         (1 << 0x06)   /**< String */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_ON_BOX                         (1 << 0x0B)   /**< On box */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_INSIDE_BOX                     (1 << 0x0C)   /**< Inside box */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_ON_PIECE_OF_PAPER              (1 << 0x0D)   /**< On piece of paper */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_INSIDE_MANUAL                  (1 << 0x0E)   /**< Inside manual */
#define BTMW_RPC_TEST_MESH_PROV_OOB_INFO_FIELD_ON_DEVICE                      (1 << 0x0F)   /**< On device */

/*OOB type description */
#define BTMW_RPC_TEST_MESH_PROV_START_ALGORITHM_FIPS_P256_ELLIPTIC_CURVE      (0x00)    /**< FIPS P256 Elliptic Curve */
#define BTMW_RPC_TEST_MESH_PROV_START_PUBLIC_KEY_NO_OOB                       (0x00)    /**< No OOB Public Key is used */
#define BTMW_RPC_TEST_MESH_PROV_START_PUBLIC_KEY_OOB                          (0x01)    /**< OOB Public Key is used */

/*The Authentication Method values */
#define BTMW_RPC_TEST_MESH_PROV_START_AUTHEN_METHOD_NO_OOB                    (0x00)    /**< No OOB authentication is used */
#define BTMW_RPC_TEST_MESH_PROV_START_AUTHEN_METHOD_STATIC_OOB                (0x01)    /**< Static OOB authentication is used */
#define BTMW_RPC_TEST_MESH_PROV_START_AUTHEN_METHOD_OUTPUT_OOB                (0x02)    /**< Output OOB authentication is used */
#define BTMW_RPC_TEST_MESH_PROV_START_AUTHEN_METHOD_INPUT_OOB                 (0x03)    /**< Input OOB authentication is used */
#define BTMW_RPC_TEST_MESH_PROV_CAPABILITY_OOB_STATIC_TYPE_SUPPORTED          (1<<0)    /**< Capabilities bit indicating that static OOB authentication is supported. */

#define BTMW_RPC_TEST_MESH_ADDR_UNASSIGNED_VALUE       0x0000
#define BTMW_RPC_TEST_MESH_ADDR_UNASSIGNED_MASK        0xFFFF
#define BTMW_RPC_TEST_MESH_ADDR_UNICAST_MASK           0x8000
#define BTMW_RPC_TEST_MESH_ADDR_UNICAST_VALUE          0x0000
#define BTMW_RPC_TEST_MESH_ADDR_VIRTUAL_MASK           0xc000
#define BTMW_RPC_TEST_MESH_ADDR_VIRTUAL_VALUE          0x8000
#define BTMW_RPC_TEST_MESH_ADDR_GROUP_MASK             0xc000
#define BTMW_RPC_TEST_MESH_ADDR_GROUP_VALUE            0xc000

/*Sig Group address type description */
#define BTMW_RPC_TEST_MESH_ADDR_GROUP_PROXIES_VALUE    0xFFFC  /**< All-proxies group address. */
#define BTMW_RPC_TEST_MESH_ADDR_GROUP_FRIENDS_VALUE    0xFFFD  /**< All-friends group address. */
#define BTMW_RPC_TEST_MESH_ADDR_GROUP_RELAYS_VALUE     0xFFFE  /**< All-relays group address. */
#define BTMW_RPC_TEST_MESH_ADDR_GROUP_NODES_VALUE      0xFFFF  /**< All-nodes group address. */

/*Primary network key */
#define BTMW_RPC_TEST_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX  (0x0)    /**< Primary network key index in mesh, can't be modified.*/

/* Location description */
#define BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_FIRST              0x0001
#define BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_SECOND             0x0002
#define BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_FRONT              0x0100
#define BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_BACK               0x0101
#define BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_TOP                0x0102
#define BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_BOTTOM             0x0103
#define BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_UPPER              0x0104
#define BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_LOWER              0x0105
#define BTMW_RPC_TEST_MESH_MODEL_ELEMENT_LOCATION_MAIN               0x0106

/*SIG Generic model ID */
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_CONFIGURATION_SERVER 0x0000
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_CONFIGURATION_CLIENT 0x0001
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_HEALTH_SERVER 0x0002
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_HEALTH_CLIENT 0x0003
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_ONOFF_SERVER    0x1000
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_ONOFF_CLIENT    0x1001
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_LEVEL_SERVER    0x1002
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_LEVEL_CLIENT    0x1003
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_DEFAULT_TRANSITION_TIME_SERVER    0x1004
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_DEFAULT_TRANSITION_TIME_CLIENT    0x1005
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SERVER    0x1006
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_SETUP_SERVER    0x1007
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_POWER_ONOFF_CLIENT    0x1008
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_POWER_LEVEL_SERVER    0x1009
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_POWER_LEVEL_SETUP_SERVER    0x100a
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_POWER_LEVEL_CLENT    0x100b
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_BATTERY_SERVER    0x100c
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_BATTERY_CLIENT    0c100d
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_LOCATION_SERVER    0x100e
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_LOCATION_SETUP_SERVER    0x100f
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_LOCATION_CLIENT    0x1010
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_ADMIN_PROPERTY_SERVER    0x1011
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_MANUFACTURER_PROPERTY_SERVER    0x1012
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_USER_PROPERTY_SERVER    0x1013
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_CLIENT_PROPERTY_SERVER    0x1014
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_GENERIC_PROPERTY_CLENT    0x1015

/*!
     @name Sensors model ID
     @{
 */
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_SENSOR_SERVER    0x1100    /**< The model ID of the sensor server. */
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_SENSOR_SETUP_SERVER    0x1101    /**< The model ID of the sensor setup server. */
#define BTMW_RPC_TEST_MESH_SIG_MODEL_ID_SENSOR_CLIENT    0x1102    /**< The model ID of the sensor client. */
/*!  @} */


/*Generic On Off Model Message Definition*/
#define BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_GET               0x8201
#define BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_SET               0x8202
#define BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_SET_UNRELIABLE    0x8203
#define BTMW_RPC_TEST_MESH_MSG_GENERIC_ONOFF_STATUS            0x8204

/*CTL Model Message Definition*/
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_GET                                     0x825D
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_SET                                     0x825E
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_SET_UNACKNOWLEDGED                      0x825F
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_STATUS                                  0x8260
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_GET                         0x8261
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_RANGE_GET                   0x8262
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_RANGE_STATUS                0x8263
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_SET                         0x8264
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_SET_UNACKNOWLEDGED          0x8265
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_STATUS                      0x8266
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_DEFAULT_GET                             0x8267
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_DEFAULT_STATUS                          0x8268
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_DEFAULT_SET                             0x8269
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_DEFAULT_SET_UNACKNOWLEDGED              0x826A
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_RANGE_SET                   0x826B
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_CTL_TEMPERATURE_RANGE_SET_UNACKNOWLEDGED    0x826C

/*HSL Model Message Definition*/
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_GET                                     0x826D
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_GET                                 0x826E
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_SET                                 0x826F
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_SET_UNACKNOWLEDGED                  0x8270
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_HUE_STATUS                              0x8271
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_GET                          0x8272
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_SET                          0x8273
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_SET_UNACKNOWLEDGED           0x8274
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SATURATION_STATUS                       0x8275
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SET                                     0x8276
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_SET_UNACKNOWLEDGED                      0x8277
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_STATUS                                  0x8278
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_TARGET_GET                              0x8279
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_TARGET_STATUS                           0x827A
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_GET                             0x827B
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_STATUS                          0x827C
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_GET                               0x827D
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_STATUS                            0x827E
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_SET                             0x827F
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_DEFAULT_SET_UNACKNOWLEDGED              0x8280
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_SET                               0x8281
#define BTMW_RPC_TEST_MESH_MSG_LIGHT_HSL_RANGE_SET_UNACKNOWLEDGED                0x8282

/*Health model message definition*/
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_CURRENT_STATUS 0x04
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_FAULT_STATUS   0x05
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_FAULT_CLEAR    0x802F
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_FAULT_CLEAR_UNACKNOWLEDGED 0x8030
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_FAULT_GET  0x8031
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_FAULT_TEST 0x8032
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_FAULT_TEST_UNACKNOWLEDGED  0x8033
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_PERIOD_GET 0x8034
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_PERIOD_SET 0x8035
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_PERIOD_SET_UNACKNOWLEDGED  0x8036
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_PERIOD_STATUS  0x8037
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_ATTENTION_GET  0x8004
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_ATTENTION_SET  0x8005
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_ATTENTION_SET_UNACKNOWLEDGED   0x8006
#define BTMW_RPC_TEST_MESH_MSG_HEALTH_ATTENTION_STATUS   0x8007

/*!
     @name Sensor model message opcode.
     @brief Opcode for sensor models.
     @{
 */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_DESCRIPTOR_GET                    0x8230 /**< opcode for Sensor Descriptor Get */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_DESCRIPTOR_STATUS                 0x51   /**< opcode for Sensor Descriptor Status */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_GET                               0x8231 /**< opcode for Sensor Get */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_STATUS                            0x52   /**< opcode for Sensor Status */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_COLUMN_GET                        0x8232 /**< opcode for Sensor Column Get */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_COLUMN_STATUS                     0x53   /**< opcode for Sensor Column Status */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_SERIES_GET                        0x8233 /**< opcode for Sensor Series Get */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_SERIES_STATUS                     0x54   /**< opcode for Sensor Series Status */
/*! @} */

/*!
     @name Sensor setup model message opcode.
     @brief Opcode for sensor models.
     @{
 */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_GET                       0x8234 /**< opcode for Sensor Cadence Get */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_SET                       0x55   /**< opcode for Sensor Cadence Set */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_SET_UNACKNOWLEDGED        0x56   /**< opcode for Sensor Cadence Set Unacknowledged */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_CADENCE_STATUS                    0x57   /**< opcode for Sensor Cadence Status */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTINGS_GET                      0x8235 /**< opcode for Sensor Settings Get */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTINGS_STATUS                   0x58   /**< opcode for Sensor Settings Status */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_GET                       0x8236 /**< opcode for Sensor Setting Get */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_SET                       0x59   /**< opcode for Sensor Setting Set */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_SET_UNACKNOWLEDGED        0x5A   /**< opcode for Sensor Setting Set Unacknowledged */
#define BTMW_RPC_TEST_MESH_MSG_SENSOR_SETTING_STATUS                    0x5B   /**< opcode for Sensor Setting Status */
/*! @} */

/*Vendor Models Definition */
#define BTMW_RPC_TEST_MESH_VENDOR_MODEL_ID(companyid, modelid) ((companyid << 16) | modelid)
#define BTMW_RPC_TEST_MESH_MODEL_COMPANY_ID_NONE 0xFFFF
#define BTMW_RPC_TEST_MESH_VENDOR_COMPANY_ID     0x000a
#define BTMW_RPC_TEST_MESH_VENDOR_MODEL_ID1      0x002A
#define BTMW_RPC_TEST_MESH_VENDOR_MODEL_ID2      0x002B
#define BTMW_RPC_TEST_MESH_VENDOR_OPCODE_1       0x00C1
#define BTMW_RPC_TEST_MESH_VENDOR_OPCODE_2       0x00C2
#define BTMW_RPC_TEST_MESH_VENDOR_OPCODE_3       0x0001
#define BTMW_RPC_TEST_MESH_VENDOR_OPCODE_4       0x0002

/*CTL State and bind*/
#define BTMW_RPC_TEST_MESH_MODEL_BINDING_PRESENT_VALUE   0x1
#define BTMW_RPC_TEST_MESH_MODEL_BINDING_TARGET_VALUE    0x2
#define BTMW_RPC_TEST_MESH_MODEL_BINDING_BOTH_VALUE      (BTMW_RPC_TEST_MESH_MODEL_BINDING_PRESENT_VALUE | BTMW_RPC_TEST_MESH_MODEL_BINDING_TARGET_VALUE)
#define BTMW_RPC_TEST_MESH_MODEL_BINDING_MASK            0x3

#define BTMW_RPC_TEST_MESH_MODEL_STATE_GENERIC_ON_OFF                  0x1
#define BTMW_RPC_TEST_MESH_MODEL_STATE_GENERIC_LEVEL                   0x2
#define BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_LIGHTNESS_ACTUAL       0x20
#define BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_LIGHTNESS_LINEAR       0x40
#define BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_LIGHTNESS_RANGE        0x80
#define BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_CTL_LIGHTNESS          0x100
#define BTMW_RPC_TEST_MESH_MODEL_STATE_LIGHTING_CTL_TEMPERATURE        0x200
#define BTMW_RPC_TEST_MESH_MODEL_STATE_MASK                            0x7FF


/** @brief Status codes for messages */
#define BTMW_RPC_TEST_MESH_ACCESS_MSG_STATUS_SUCCESS     0                        /**< Success */

#define BTMW_RPC_TEST_MESH_MSG_RESEND_COUNT  (0x03)
#define BTMW_RPC_TEST_MESH_MSG_RESEND_INTERVAL  (2)  //unit is second

#define BTMW_RPC_TEST_MESH_COMPOSITION_DATA_LEN  10

typedef enum {
    BTMW_RPC_TEST_MESH_CONFIG_STATE_IDLE,
    BTMW_RPC_TEST_MESH_CONFIG_STATE_PROVISIONING,
    BTMW_RPC_TEST_MESH_CONFIG_STATE_GET_COMPOSITION_DATA,
    BTMW_RPC_TEST_MESH_CONFIG_STATE_ADD_APPKEY,
    BTMW_RPC_TEST_MESH_CONFIG_STATE_MODEL_APP_BIND,
    BTMW_RPC_TEST_MESH_CONFIG_STATE_DONE,
}BTMW_RPC_TEST_MESH_NODE_CONFIG_STATE_T;

//Generic level server model
typedef struct
{
    INT16 present_level;
    INT16 target_level;
    INT16 original_present_level;
    INT16 original_target_level;
    UINT8 TID;
    UINT8 transition_time;
    UINT8 delay;
} BTMW_RPC_TEST_MESH_GENERIC_LEVEL_SERVER_MODEL_T;

//CTL Server  state
typedef struct
{
    UINT16 present_ctl_lightness;
    UINT16 present_ctl_temperature;
    UINT16 target_ctl_lightness;
    UINT16 target_ctl_temperature;
    UINT16 present_ctl_delta_uv;
    UINT16 target_ctl_delta_uv;
    UINT16 default_lightness;
    UINT16 default_temperature;
    UINT16 default_delta_uv;
    UINT16 range_min;
    UINT16 range_max;
    UINT8 TID;
    UINT8 transition_time;
    UINT8 delay;
    BTMW_RPC_TEST_MESH_GENERIC_LEVEL_SERVER_MODEL_T level_server;
    UINT16 element_index;
}BTMW_RPC_TEST_MESH_LIGHTING_CTL_SERVER_T;

//CTL temperature server state
typedef struct
{
    UINT16 present_ctl_temperature;
    UINT16 target_ctl_temperature;
    UINT16 present_ctl_delta_uv;
    UINT16 target_ctl_delta_uv;
    UINT8 TID;
    UINT8 transition_time;
    UINT8 delay;
    BTMW_RPC_TEST_MESH_GENERIC_LEVEL_SERVER_MODEL_T level_server;
    UINT16 element_index;
}BTMW_RPC_TEST_MESH_LIGHTING_CTL_TEMP_SERVER_T;

typedef struct
{
    UINT16 present_lightness;
    UINT16 target_lightness;
    UINT16 present_linear_lightness;
    UINT16 target_linear_lightness;
    UINT16 default_lightness;
    UINT16 last_lightness;
    UINT16 range_min;
    UINT16 range_max;
    UINT8 TID;
    UINT8 transition_time;
    UINT8 delay;
}BTMW_RPC_TEST_MESH_LIGHTING_LIGNTNESS_SERVER_T;

typedef struct
{
    UINT16 present_hsl_lightness;
    UINT16 present_hsl_hue;
    UINT16 present_hsl_saturation;
    UINT16 target_hsl_lightness;
    UINT16 target_hsl_hue;
    UINT16 target_hsl_saturation;
    UINT16 default_lightness;
    UINT16 default_hue;
    UINT16 default_saturation;
    UINT16 hue_range_min;
    UINT16 hue_range_max;
    UINT16 saturation_range_min;
    UINT16 saturation_range_max;
    UINT8 TID;
    UINT8 transition_time;
    UINT8 delay;
    UINT16 element_index;
}BTMW_RPC_TEST_MESH_LIGHTING_HSL_SERVER_T;

typedef struct
{
    UINT16 present_hsl_hue;
    UINT16 target_hsl_hue;
    UINT8 TID;
    UINT8 transition_time;
    UINT8 delay;
    BTMW_RPC_TEST_MESH_GENERIC_LEVEL_SERVER_MODEL_T Level_server;
    UINT16 element_index;
}BTMW_RPC_TEST_MESH_LIGHTING_HSL_HUE_SERVER_T;

typedef struct
{
    UINT16 present_hsl_saturation;
    UINT16 target_hsl_saturation;
    UINT8 TID;
    UINT8 transition_time;
    UINT8 delay;
    BTMW_RPC_TEST_MESH_GENERIC_LEVEL_SERVER_MODEL_T Level_server;
    UINT16 element_index;
}BTMW_RPC_TEST_MESH_LIGHTING_HSL_SAT_SERVER_T;

typedef struct {
    UINT32 element_index;
    UINT32 model_id;
} BTMW_RPC_TEST_MESH_BINDING_MODEL_T;

typedef struct
{
    UINT8 onOff;
    UINT8 TID;
    UINT8 transTime;
    UINT8 delay;
} BTMW_RPC_TEST_MESH_ONOFF_SERVER_MODEL_T;

typedef struct
{
    UINT32 msg_id;              //used to identify the resend msg
    UINT8 resend_cnt;           //the max count that the message wil be resend
    struct timespec send_time;  //the timing that the msg was sent out last time
    UINT16 opcode;              //msg opcode
    UINT16 src;                 //the src address of the msg
    UINT16 dst;                 //eh dst address of the msg
    VOID *buf;                  //the whole content of the msg
    UINT32 buf_len;             //the size of the msg
}BTMW_RPC_TEST_MESH_MSG_TO_RESEND_T;

typedef struct
{
    UINT16 addr;   //the unicast address of its primary element
    UINT8 uuid[BT_MESH_UUID_SIZE];
    UINT8 dev_key[BT_MESH_DEVKEY_SIZE];
    BTMW_RPC_TEST_MESH_NODE_CONFIG_STATE_T config_state;
    BOOL in_blacklist;    //If true, the node will be removed in next key refresh procedure.
}BTMW_RPC_TEST_MESH_NODE_ENTRY_T;

typedef struct {
    UINT16 keyidx;
    UINT8 state;
    UINT8 node_identity;
    UINT8 flag;
    UINT8 netkey[BT_MESH_KEY_SIZE]; //The currently in used
    UINT8 temp_netkey[BT_MESH_KEY_SIZE];    //new key value set by key update, after key refresh, it stores the old key
    VOID *appkey_list;
    VOID *temp_appkey_list;
    VOID *kr_node_list;    //the count of node which is not blacklisted to this netkey for key refresh
    VOID *kr_node_ack_list;     //the node which reply for Key refresh related messages.
} BTMW_RPC_TEST_MESH_NETKEY_ENTRY_T;

enum
{
    BTMW_RPC_TEST_MESH_KR_DISTRIBUTION_TIMER_ID = 0,
    BTMW_RPC_TEST_MESH_KR_PHASE_SET_TIMER_ID = 1,
    BTMW_RPC_TEST_MESH_CURR_PROV_NODE_CONFIG_TIMER_ID = 2,
}BTMW_RPC_TEST_MESH_TIMER_ID_T;

typedef struct {
    UINT8 uuid[BT_MESH_UUID_SIZE];
    CHAR *bt_addr;
    UINT8 auth_value[BT_MESH_AUTHENTICATION_SIZE];
} BTMW_RPC_TEST_MESH_DEVICE_DB_T;

int btmw_rpc_test_mesh_init();

#endif /* __BTMW_RPC_TEST_MESH_IF_H__ */

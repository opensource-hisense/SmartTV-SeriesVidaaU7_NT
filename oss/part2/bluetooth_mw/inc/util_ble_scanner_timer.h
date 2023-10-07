#ifndef __UTIL_BLE_SCANNER_TIMER_HH_
#define __UTIL_BLE_SCANNER_TIMER_HH_
#include <sys/queue.h>
#include <stdint.h>
#include "u_bt_mw_types.h"

typedef VOID (*UTIL_BLE_SCANNER_TIMER_HANDLER)(UINT32 timer_id, VOID *pv_tag);

typedef enum
{
    UTIL_BLE_SCANNER_TIMER_REPEAT_TYPE_ONCE = 0,
    UTIL_BLE_SCANNER_TIMER_REPEAT_TYPE_REPEAT,
}UTIL_BLE_SCANNER_TIMER_REPEAT_TYPE_T;

typedef struct
{
    INT32 id;   //timer id
    VOID *handle;   //Handle of this timer
    UINT32 delay_ms;    //timeout value
    UTIL_BLE_SCANNER_TIMER_REPEAT_TYPE_T repeat;
    UTIL_BLE_SCANNER_TIMER_HANDLER pf_handler; //timeout callback function
    VOID *pv_args;  //argument for pf_handler
}UTIL_BLE_SCANNER_TIMER_T;


EXPORT_SYMBOL INT32 util_ble_scanner_timer_start(UTIL_BLE_SCANNER_TIMER_T *pt_timer);
EXPORT_SYMBOL INT32 util_ble_scanner_timer_stop(VOID * h_timer);
EXPORT_SYMBOL INT32 util_ble_scanner_timer_delete(VOID * h_timer);
EXPORT_SYMBOL INT32 util_ble_scanner_timer_resume(VOID * h_timer);
EXPORT_SYMBOL INT32 util_ble_scanner_timer_init(VOID);


#endif

#ifndef __UTIL_DLIST_HH_
#define __UTIL_DLIST_HH_
#include <sys/queue.h>
#include <stdbool.h>
#include <stdint.h>
#include "u_bt_mw_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
     @defgroup PAGE_API_LIBRARY_DLIST util_dlist.h
     @{
        @page PAGE_API_LIBRARY_DLIST util_dlist.h
        util_dlist.h is a double linked list management API set.

*/

/*!
    @brief Allocate a double-linked list
*/
VOID *util_dlist_alloc(VOID);

/*!
    @brief Release a double-linked list
    @param list The double-linked list @ref util_dlist_alloc
*/
VOID util_dlist_free(VOID *list);

VOID util_dlist_empty(VOID *list, BOOL free_data);

/*!
    @brief double-linked list entry and data memory allocation
    @param size data memory size
*/
VOID *util_dlist_entry_alloc(UINT32 size);

/*!
    @brief Release a double-linked list enrty and data memory
    @param dat The double-linked list entry @ref util_dlist_entry_alloc
*/
VOID util_dlist_entry_free(VOID *dat);

VOID *util_dlist_remove_head(VOID *list);

VOID *util_dlist_remove(VOID *list, VOID *removed);

BOOL util_dlist_delete(VOID *list, VOID *deleted);

/*!
    @brief Insert a double-linked list enrty into a double-linked list
    @param list The double-linked list @ref util_queue_alloc
    @param inserted The double-linked list entry @ref util_dlist_entry_alloc want to be inserted
*/
VOID util_dlist_insert(VOID *list, VOID *inserted);

/*!
    @brief Insert a double-linked list enrty after a specified double-linked list entry
    @param list The double-linked list @ref util_dlist_alloc
    @param inlist the target double-linked list entry
    @param inserted The double-linked list entry @ref util_dlist_entry_alloc want to be inserted
*/
VOID util_dlist_insert_after(VOID *list, VOID *inlist, VOID *inserted);

/*!
    @brief Insert a double-linked list enrty before a specified double-linked list entry
    @param list The double-linked list @ref util_dlist_alloc
    @param inlist the target double-linked list entry
    @param inserted The double-linked list entry @ref util_dlist_entry_alloc want to be inserted
*/
VOID util_dlist_insert_before(VOID *list, VOID *inlist, VOID *inserted);

/*!
    @brief Query the first double-linked enrty from a double-linked
    @param queue The double-linked @ref util_dlist_alloc
*/
VOID *util_dlist_first(VOID *list);

/*!
    @brief Query the next double-linked list enrty from current double-linked list entry
    @param queue The double-linked list @ref util_dlist_alloc
    @param dat The double-linked list entry @ref util_dlist_entry_alloc
*/
VOID *util_dlist_next(VOID *list, VOID *data);

/*!
    @brief Query the double-linked list entry of queue
    @param list The double-linked list @ref util_dlist_alloc
    @return the elements count of double-linked list
*/
UINT32 util_dlist_count(VOID *list);

/*! @} */

#ifdef __cplusplus
}
#endif

#endif // __util_DLIST_HH_

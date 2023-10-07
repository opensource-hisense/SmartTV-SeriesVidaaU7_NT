#ifndef __UTIL_QUEUE_HH_
#define __UTIL_QUEUE_HH_
#include <sys/queue.h>
#include <stdint.h>
#include "u_bt_mw_types.h"


/*!
     @defgroup PAGE_API_LIBRARY_QUEUE util_queue.h
     @{
        @page PAGE_API_LIBRARY_QUEUE util_queue.h
        util_queue.h is a queue management API set.

*/

/*!
    @brief Allocate a queue
*/
VOID * util_queue_alloc(VOID);

/*!
    @brief Release a queue
    @param queue The queue @ref util_queue_alloc
*/
VOID util_queue_free(VOID *queue);

/*!
    @brief Query entry and data memory allocation
    @param size data memory size
*/
VOID * util_queue_entry_alloc(UINT32 size);

/*!
    @brief Release a queue enrty and data memory
    @param dat The queue @ref util_queue_entry_alloc
*/
VOID util_queue_entry_free(VOID *dat);

/*!
    @brief Push a queue enrty into a queue
    @param queue The queue @ref util_queue_alloc
    @param dat The queue entry @ref util_queue_entry_alloc
*/
VOID util_queue_push(VOID *queue, VOID *dat);

/*!
    @brief Push a queue enrty into a queue front
    @param queue The queue @ref util_queue_alloc
    @param dat The queue entry @ref util_queue_entry_alloc
*/
VOID util_queue_push_front(VOID *queue, VOID *dat);

/*!
    @brief Pop a queue enrty from a queue
    @param queue The queue @ref util_queue_alloc
*/
VOID * util_queue_pop(VOID *queue);

/*!
    @brief Query the first queue enrty from a queue
    @param queue The queue @ref util_queue_alloc
*/
VOID *util_queue_first(VOID *queue);

/*!
    @brief Query the next queue enrty from current queue entry
    @param queue The queue @ref util_queue_alloc
    @param dat The queue entry @ref util_queue_entry_alloc
*/
VOID *util_queue_next(VOID *queue, VOID* dat);

/*!
    @brief Insert a queue enrty in current queue entry
    @param queue The queue @ref util_queue_alloc
    @param inqueue The queue entry @ref util_queue_entry_alloc
    @param inserted The queue entry @ref util_queue_entry_alloc
*/
VOID util_queue_insert_after(VOID *queue, VOID* inqueue, VOID *inserted);

/*!
    @brief Remove a queue enrty
    @param queue The queue @ref util_queue_alloc
    @param removed The queue entry @ref util_queue_entry_alloc
*/
VOID util_queue_remove(VOID *queue, VOID* removed);

/*!
    @brief Query the queue entry of queue
    @param queue The queue @ref util_queue_alloc
*/
UINT32 util_queue_count(VOID *queue);

/*! @} */

#endif

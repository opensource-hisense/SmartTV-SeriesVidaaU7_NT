#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "util_queue.h"

////////////////////////////////////////////////////////////////////////////////
// Private Type Definition /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    STAILQ_HEAD(HEAD, entry) head;
    UINT32 tag;
    UINT32 count;
}UTIL_QUEUE;


typedef struct entry
{
    STAILQ_ENTRY(entry) entry;
    UINT32 tag;
    VOID *data;
}UTIL_QUEUE_ENTRY;

////////////////////////////////////////////////////////////////////////////////
// Public Functions ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
EXPORT_SYMBOL VOID * util_queue_alloc(VOID)
{
    UTIL_QUEUE *queue;

    queue = (UTIL_QUEUE *)malloc(sizeof(UTIL_QUEUE));

    if (queue)
    {
        queue->tag = 0x19760108;
        queue->count = 0;
        queue->head.stqh_first = NULL;
        queue->head.stqh_last = &(queue->head.stqh_first);
    }

    return queue;
}

EXPORT_SYMBOL VOID util_queue_free(VOID *queue)
{
    VOID *dat;
    while((dat = util_queue_pop(queue)))
    {
        util_queue_entry_free(dat);
    }

    free(queue);
}

EXPORT_SYMBOL VOID * util_queue_entry_alloc(UINT32 size)
{
    UTIL_QUEUE_ENTRY *new_entry;

    new_entry = (UTIL_QUEUE_ENTRY *) malloc(sizeof(UTIL_QUEUE_ENTRY) + size);

    if(new_entry)
    {
        new_entry->tag = 0x20111118;
        return (uint8_t*)new_entry + offsetof(UTIL_QUEUE_ENTRY, data);
    }

    return NULL;
}

EXPORT_SYMBOL VOID util_queue_entry_free(VOID *dat)
{
    UINT32 *ptag;
    UTIL_QUEUE_ENTRY *pentry;

    ptag = (UINT32*)((uint8_t*)dat  - offsetof(UTIL_QUEUE_ENTRY, data) + offsetof(UTIL_QUEUE_ENTRY, tag));
    pentry = (UTIL_QUEUE_ENTRY*)((uint8_t*)dat - offsetof(UTIL_QUEUE_ENTRY, data));

    if(*ptag == 0x20111118) //valid entry
    {
        free(pentry);
    }
}

EXPORT_SYMBOL VOID util_queue_push(VOID *queue, VOID *dat)
{
    UINT32 *ptag;

    UTIL_QUEUE_ENTRY *pentry;

    if(((UTIL_QUEUE*)queue)->tag == 0x19760108)
    {
        ptag = (UINT32*)((uint8_t*)dat  - offsetof(UTIL_QUEUE_ENTRY, data) + offsetof(UTIL_QUEUE_ENTRY, tag));
        pentry = (UTIL_QUEUE_ENTRY*)((uint8_t*)dat - offsetof(UTIL_QUEUE_ENTRY, data));

        if(*ptag == 0x20111118) //valid entry
        {
            STAILQ_INSERT_TAIL(&((UTIL_QUEUE*)queue)->head, pentry, entry);
            ((UTIL_QUEUE*)queue)->count++;
        }
    }
}

EXPORT_SYMBOL VOID util_queue_push_front(VOID *queue, VOID *dat)
{
    UINT32 *ptag;

    UTIL_QUEUE_ENTRY *pentry;

    if(((UTIL_QUEUE*)queue)->tag == 0x19760108)
    {
        ptag = (UINT32*)((uint8_t*)dat  - offsetof(UTIL_QUEUE_ENTRY, data) + offsetof(UTIL_QUEUE_ENTRY, tag));
        pentry = (UTIL_QUEUE_ENTRY*)((uint8_t*)dat - offsetof(UTIL_QUEUE_ENTRY, data));

        if(*ptag == 0x20111118) //valid entry
        {
            STAILQ_INSERT_HEAD(&((UTIL_QUEUE*)queue)->head, pentry, entry);
            ((UTIL_QUEUE*)queue)->count++;
        }
    }
}

EXPORT_SYMBOL VOID * util_queue_pop(VOID *queue)
{
    VOID *pdat = NULL;

    if(STAILQ_FIRST(&((UTIL_QUEUE*)queue)->head))
    {
        pdat = ((uint8_t*)STAILQ_FIRST(&((UTIL_QUEUE*)queue)->head) + offsetof(UTIL_QUEUE_ENTRY, data));
        STAILQ_REMOVE_HEAD(&((UTIL_QUEUE*)queue)->head, entry);
        ((UTIL_QUEUE*)queue)->count--;
    }

    return pdat;
}

EXPORT_SYMBOL VOID *util_queue_first(VOID *queue)
{
    VOID *pdat = NULL;

    if(STAILQ_FIRST(&((UTIL_QUEUE*)queue)->head))
    {
        pdat = ((uint8_t*)STAILQ_FIRST(&((UTIL_QUEUE*)queue)->head) + offsetof(UTIL_QUEUE_ENTRY, data));
    }

    return pdat;
}

EXPORT_SYMBOL VOID *util_queue_next(VOID *queue, VOID* dat)
{
    UINT32 *ptag;
    VOID *pdat = NULL;

    if (dat != NULL) {
        UTIL_QUEUE_ENTRY *pentry;

        ptag = (UINT32*)((uint8_t*)dat  - offsetof(UTIL_QUEUE_ENTRY, data) + offsetof(UTIL_QUEUE_ENTRY, tag));
        pentry = (UTIL_QUEUE_ENTRY*)((uint8_t*)dat - offsetof(UTIL_QUEUE_ENTRY, data));

        if(*ptag == 0x20111118) //valid entry
        {
            pdat = STAILQ_NEXT(pentry, entry);
            if (pdat != NULL) {
                pdat += offsetof(UTIL_QUEUE_ENTRY, data);
            }
        }
    }

    return pdat;
}

EXPORT_SYMBOL VOID util_queue_insert_after(VOID *queue, VOID* inqueue, VOID *inserted)
{
    UINT32 *tag_inqueue;
    UINT32 *tag_inserted;

    UTIL_QUEUE_ENTRY *entry_inqueue;
    UTIL_QUEUE_ENTRY *entry_inserted;

    tag_inqueue = (UINT32*)((uint8_t*)inqueue  - offsetof(UTIL_QUEUE_ENTRY, data) + offsetof(UTIL_QUEUE_ENTRY, tag));
    tag_inserted = (UINT32*)((uint8_t*)inserted  - offsetof(UTIL_QUEUE_ENTRY, data) + offsetof(UTIL_QUEUE_ENTRY, tag));

    if(*tag_inqueue == 0x20111118 && *tag_inserted == 0x20111118) //valid entry
    {
        entry_inqueue = (UTIL_QUEUE_ENTRY*)((uint8_t*)inqueue - offsetof(UTIL_QUEUE_ENTRY, data));
        entry_inserted = (UTIL_QUEUE_ENTRY*)((uint8_t*)inserted - offsetof(UTIL_QUEUE_ENTRY, data));
        STAILQ_INSERT_AFTER(&((UTIL_QUEUE*)queue)->head, entry_inqueue, entry_inserted, entry);
        ((UTIL_QUEUE*)queue)->count++;
    }

}

EXPORT_SYMBOL VOID util_queue_remove(VOID *queue, VOID* removed)
{
    UINT32 *tag_removed;

    UTIL_QUEUE_ENTRY *entry_removed;

    tag_removed = (UINT32*)((uint8_t*)removed  - offsetof(UTIL_QUEUE_ENTRY, data) + offsetof(UTIL_QUEUE_ENTRY, tag));

    if(*tag_removed == 0x20111118) //valid entry
    {
        entry_removed = (UTIL_QUEUE_ENTRY*)((uint8_t*)removed - offsetof(UTIL_QUEUE_ENTRY, data));
        STAILQ_REMOVE(&((UTIL_QUEUE*)queue)->head, entry_removed, entry, entry);
        ((UTIL_QUEUE*)queue)->count--;
    }
}

EXPORT_SYMBOL UINT32 util_queue_count(VOID *queue)
{
    if(((UTIL_QUEUE*)queue)->tag == 0x19760108)
        return ((UTIL_QUEUE*)queue)->count;

    return 0;
}



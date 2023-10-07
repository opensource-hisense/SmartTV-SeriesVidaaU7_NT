#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "util_dlist.h"

////////////////////////////////////////////////////////////////////////////////
// Private Type Definition /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    LIST_HEAD(HEAD, entry) head;
    UINT32 tag;
    UINT32 count;
}UTIL_DLIST;

typedef struct entry
{
    LIST_ENTRY(entry) entry;
    UINT32 tag;
    VOID *data;
}UTIL_DLIST_ENTRY;

////////////////////////////////////////////////////////////////////////////////
// Public Functions ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
EXPORT_SYMBOL VOID * util_dlist_alloc(VOID)
{
    UTIL_DLIST *list;

    list = (UTIL_DLIST *)malloc(sizeof(UTIL_DLIST));
    if (list != NULL)
    {
        list->tag = 0x19760108;
        list->count = 0;
        list->head.lh_first = NULL;
    }
    return list;
}

EXPORT_SYMBOL VOID util_dlist_free(VOID *list)
{
    VOID *dat;
    while (NULL != (dat = util_dlist_remove_head(list)))
    {
        util_dlist_entry_free(dat);
    }

    free(list);
}

EXPORT_SYMBOL VOID util_dlist_empty(VOID *list, BOOL free_data)
{
    VOID *dat;
    while (NULL != (dat = util_dlist_remove_head(list)))
    {
        if (free_data)
        {
            util_dlist_entry_free(dat);
        }
    }
}

EXPORT_SYMBOL VOID * util_dlist_entry_alloc(UINT32 size)
{
    UTIL_DLIST_ENTRY *new_entry;

    new_entry = (UTIL_DLIST_ENTRY *) malloc(sizeof(UTIL_DLIST_ENTRY) + size);

    if(new_entry)
    {
        new_entry->tag = 0x20111118;
        return (uint8_t*)new_entry + offsetof(UTIL_DLIST_ENTRY, data);
    }

    return NULL;
}

EXPORT_SYMBOL VOID util_dlist_entry_free(VOID *dat)
{
    UINT32 *ptag;
    UTIL_DLIST_ENTRY *pentry;

    if (dat == NULL) {
        return;
    }

    ptag = (UINT32*)((uint8_t*)dat  - offsetof(UTIL_DLIST_ENTRY, data) + offsetof(UTIL_DLIST_ENTRY, tag));
    pentry = (UTIL_DLIST_ENTRY*)((uint8_t*)dat - offsetof(UTIL_DLIST_ENTRY, data));

    if(*ptag == 0x20111118) //valid entry
    {
        free(pentry);
    }
}

EXPORT_SYMBOL VOID *util_dlist_remove_head(VOID *list)
{
    VOID *pdat = NULL;
    UTIL_DLIST_ENTRY *entry_removed;

    if(LIST_FIRST(&((UTIL_DLIST*)list)->head))
    {
        pdat = ((uint8_t*)LIST_FIRST(&((UTIL_DLIST*)list)->head) + offsetof(UTIL_DLIST_ENTRY, data));
        entry_removed = (UTIL_DLIST_ENTRY*)LIST_FIRST(&((UTIL_DLIST*)list)->head);

        LIST_REMOVE(entry_removed, entry);
        ((UTIL_DLIST*)list)->count--;
    }

    return pdat;
}

EXPORT_SYMBOL VOID *util_dlist_remove(VOID *list, VOID *removed)
{
    UINT32 *tag_removed;
    VOID *pdat = removed;

    UTIL_DLIST_ENTRY *entry_removed;

    if (pdat == NULL)
    {
        pdat = util_dlist_remove_head(list);
    }
    else
    {
        tag_removed = (UINT32*)((uint8_t*)removed  - offsetof(UTIL_DLIST_ENTRY, data) + offsetof(UTIL_DLIST_ENTRY, tag));

        if(*tag_removed == 0x20111118) //valid entry
        {
            entry_removed = (UTIL_DLIST_ENTRY*)((uint8_t*)removed - offsetof(UTIL_DLIST_ENTRY, data));
            LIST_REMOVE(entry_removed, entry);
            ((UTIL_DLIST*)list)->count--;
        }
    }

    return pdat;
}

EXPORT_SYMBOL BOOL util_dlist_delete(VOID *list, VOID *deleted)
{
    BOOL ret = FALSE;
    VOID *pdat = NULL;
    if (deleted == NULL)
    {
        UTIL_DLIST_ENTRY *head_entry = LIST_FIRST(&((UTIL_DLIST*)list)->head);

        if(head_entry)
        {
            LIST_REMOVE(head_entry, entry);
            ((UTIL_DLIST*)list)->count--;
            return TRUE;
        }
    }
    else
    {
        pdat = util_dlist_remove(list, deleted);

        if (pdat) {
            util_dlist_entry_free(deleted);
            ret = TRUE;
        }
    }

    return ret;

}

EXPORT_SYMBOL VOID util_dlist_insert(VOID *list, VOID *inserted)
{
    UINT32 *tag_inserted;
    UTIL_DLIST_ENTRY *entry_inserted;

    if(inserted == NULL)
        return;

    tag_inserted = (UINT32*)((uint8_t*)inserted  - offsetof(UTIL_DLIST_ENTRY, data) + offsetof(UTIL_DLIST_ENTRY, tag));

    if(*tag_inserted == 0x20111118) //valid entry
    {
        entry_inserted = (UTIL_DLIST_ENTRY*)((uint8_t*)inserted - offsetof(UTIL_DLIST_ENTRY, data));
        LIST_INSERT_HEAD(&((UTIL_DLIST*)list)->head, entry_inserted, entry);
        ((UTIL_DLIST*)list)->count++;
    }
}

EXPORT_SYMBOL VOID util_dlist_insert_after(VOID *list, VOID *inlist, VOID *inserted)
{
    UINT32 *tag_inlist;
    UINT32 *tag_inserted;

    UTIL_DLIST_ENTRY *entry_inlist;
    UTIL_DLIST_ENTRY *entry_inserted;

    if(inlist == NULL || inserted == NULL)
        return;

    tag_inlist = (UINT32*)((uint8_t*)inlist  - offsetof(UTIL_DLIST_ENTRY, data) + offsetof(UTIL_DLIST_ENTRY, tag));
    tag_inserted = (UINT32*)((uint8_t*)inserted  - offsetof(UTIL_DLIST_ENTRY, data) + offsetof(UTIL_DLIST_ENTRY, tag));

    if(*tag_inlist == 0x20111118 && *tag_inserted == 0x20111118) //valid entry
    {
        entry_inlist = (UTIL_DLIST_ENTRY*)((uint8_t*)inlist - offsetof(UTIL_DLIST_ENTRY, data));
        entry_inserted = (UTIL_DLIST_ENTRY*)((uint8_t*)inserted - offsetof(UTIL_DLIST_ENTRY, data));
        LIST_INSERT_AFTER(entry_inlist, entry_inserted, entry);
        ((UTIL_DLIST*)list)->count++;
    }
}

EXPORT_SYMBOL VOID util_dlist_insert_before(VOID *list, VOID *inlist, VOID *inserted)
{
    UINT32 *tag_inlist;
    UINT32 *tag_inserted;

    UTIL_DLIST_ENTRY *entry_inlist;
    UTIL_DLIST_ENTRY *entry_inserted;

    if(inlist == NULL || inserted == NULL)
        return;

    tag_inlist = (UINT32*)((uint8_t*)inlist  - offsetof(UTIL_DLIST_ENTRY, data) + offsetof(UTIL_DLIST_ENTRY, tag));
    tag_inserted = (UINT32*)((uint8_t*)inserted  - offsetof(UTIL_DLIST_ENTRY, data) + offsetof(UTIL_DLIST_ENTRY, tag));

    if(*tag_inlist == 0x20111118 && *tag_inserted == 0x20111118) //valid entry
    {
        entry_inlist = (UTIL_DLIST_ENTRY*)((uint8_t*)inlist - offsetof(UTIL_DLIST_ENTRY, data));
        entry_inserted = (UTIL_DLIST_ENTRY*)((uint8_t*)inserted - offsetof(UTIL_DLIST_ENTRY, data));
        LIST_INSERT_BEFORE(entry_inlist, entry_inserted, entry);
        ((UTIL_DLIST*)list)->count++;
    }
}

EXPORT_SYMBOL VOID *util_dlist_first(VOID *list)
{
    VOID *pdat = NULL;

    if(list != NULL && LIST_FIRST(&((UTIL_DLIST*)list)->head))
    {
        pdat = ((uint8_t*)LIST_FIRST(&((UTIL_DLIST*)list)->head) + offsetof(UTIL_DLIST_ENTRY, data));
    }

    return pdat;
}

EXPORT_SYMBOL VOID * util_dlist_next(VOID *list, VOID *dat)
{
    UINT32 *ptag;
    VOID *pdat = NULL;

    UTIL_DLIST_ENTRY *pentry;

    ptag = (UINT32*)((uint8_t*)dat  - offsetof(UTIL_DLIST_ENTRY, data) + offsetof(UTIL_DLIST_ENTRY, tag));
    pentry = (UTIL_DLIST_ENTRY*)((uint8_t*)dat - offsetof(UTIL_DLIST_ENTRY, data));

    if(*ptag == 0x20111118) //valid entry
    {
        pdat = LIST_NEXT(pentry, entry);
        if (pdat != NULL) {
            pdat += offsetof(UTIL_DLIST_ENTRY, data);
        }
    }

    return pdat;
}

EXPORT_SYMBOL UINT32 util_dlist_count(VOID *list)
{
    if(((UTIL_DLIST*)list)->tag == 0x19760108)
        return ((UTIL_DLIST*)list)->count;

    return 0;
}


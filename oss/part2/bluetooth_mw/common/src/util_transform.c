#include <string.h>
#include <stdio.h>
#include "util_transform.h"

EXPORT_SYMBOL BOOL Util_Transform_Str2HexNum(CHAR *str, UINT32 *num)
{
    CHAR *end;
    *num = strtoul(str, &end, 16);
    if(*end)//fail
    {
        return FALSE;
    }

    return TRUE;
}

EXPORT_SYMBOL BOOL Util_Transform_Str2u8Num(CHAR *str, UINT8 *num)
{
    UINT32 u32;
    BOOL ret;
    ret = Util_Transform_Str2Num(str, &u32);
    *num = (UINT8)u32;
    return ret;
}

EXPORT_SYMBOL BOOL Util_Transform_Str2u16Num(CHAR *str, UINT16 *num)
{
    UINT32 u32;
    BOOL ret;
    ret = Util_Transform_Str2Num(str, &u32);
    *num = (UINT16)u32;

    return ret;
}

EXPORT_SYMBOL BOOL Util_Transform_Str2u32Num(CHAR *str, UINT32 *num)
{
    UINT32 u32;
    BOOL ret;
    ret = Util_Transform_Str2Num(str, &u32);
    *num = u32;

    return ret;
}

EXPORT_SYMBOL BOOL Util_Transform_Str2Num(CHAR *str, UINT32 *num)
{
    CHAR *end;
    *num = strtoul(str, &end, 10);
    if(*end) //fail
    {
        *num = strtoul(str, &end, 16);
        if(*end)//fail
        {
            return FALSE;
        }
    }

    return TRUE;
}

#define C2N(x) (x > '9' ? x - 'A' + 10 : x - '0')

EXPORT_SYMBOL INT32 Util_Transform_Str2u8HexNumArray(CHAR *str, UINT8 numbs[])
{
    INT32 i, dig;
    INT32 str_len = strlen(str);

    for (i = 0; i < (str_len / 2); i++) {
        str[2*i] = toupper(str[2*i]);
        dig = C2N(str[2*i]);
        numbs[i] = dig << 4;
        str[2*i+1] = toupper(str[2*i+1]);
        dig = C2N(str[2*i + 1]);
        numbs[i] |= dig;
    }

    return i;
}

EXPORT_SYMBOL BOOL Util_Transform_StrArray2u8HexNumArray(INT32 argc, CHAR **argv, UINT8 nums[])
{
    UINT32 tmp;
    INT32 i;

    for(i = 0; i < argc; i++)
    {
        if(!Util_Transform_Str2HexNum(argv[i], &tmp))
            return FALSE;
         nums[i] = (UINT8)tmp;
    }

    return TRUE;
}

EXPORT_SYMBOL BOOL Util_Transform_StrArray2u8NumArray(INT32 argc, CHAR **argv, UINT8 nums[])
{
    UINT32 tmp;
    INT32 i;

    for(i = 0; i < argc; i++)
    {
        if(!Util_Transform_Str2Num(argv[i], &tmp))
            return FALSE;
         nums[i] = (UINT8)tmp;
    }

    return TRUE;
}

EXPORT_SYMBOL BOOL Util_Transform_StrArray2u16HexNumArray(INT32 argc, CHAR **argv, UINT16 nums[])
{
    UINT32 tmp;
    INT32 i;

    for(i = 0; i < argc; i++)
    {
        if(!Util_Transform_Str2HexNum(argv[i], &tmp))
            return FALSE;
         nums[i] = (UINT16)tmp;
    }

    return TRUE;
}

EXPORT_SYMBOL BOOL Util_Transform_StrArray2u16NumArray(INT32 argc, CHAR **argv, UINT16 nums[])
{
    UINT32 tmp;
    INT32 i;

    for(i = 0; i < argc; i++)
    {
        if(!Util_Transform_Str2Num(argv[i], &tmp))
            return FALSE;
         nums[i] = (UINT16)tmp;
    }

    return TRUE;
}

EXPORT_SYMBOL BOOL Util_Transform_StrArray2u32HexNumArray(INT32 argc, CHAR **argv, UINT32 nums[])
{
    UINT32 tmp;
    INT32 i;

    for(i = 0; i < argc; i++)
    {
        if(!Util_Transform_Str2HexNum(argv[i], &tmp))
            return FALSE;
         nums[i] = (UINT32)tmp;
    }

    return TRUE;
}

EXPORT_SYMBOL BOOL Util_Transform_StrArray2u32NumArray(INT32 argc, CHAR **argv, UINT32 nums[])
{
    UINT32 tmp;
    INT32 i;

    for(i = 0; i < argc; i++)
    {
        if(!Util_Transform_Str2Num(argv[i], &tmp))
            return FALSE;
         nums[i] = (UINT32)tmp;
    }

    return TRUE;
}


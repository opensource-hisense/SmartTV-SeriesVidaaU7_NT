#ifndef __UTIL_TRANSFORM_HH__
#define __UTIL_TRANSFORM_HH__
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "u_bt_mw_types.h"

/*!
     @defgroup PAGE_API_LIBRARY_UTILITY util_transform.h
     @{
        @page PAGE_API_LIBRARY_UTILITY util_transform.h
        util_transform.h provides some useful function.

*/


/*!
    @brief Convert the hex string to number
    @param str Input the number as ASCII array
    @param num Out the result as unit32 type
    @return True if success
*/
BOOL Util_Transform_Str2HexNum(CHAR *str, UINT32 *num);
/*!
    @brief Convert the string to number
    @param str Input the number as ASCII array
    @param num Out the result as unit32 type
    @return True if success
*/
BOOL Util_Transform_Str2Num(CHAR *str, UINT32 *num);

/*!
    @brief Convert the string to unsigned 16 bits number
    @param str Input the number as ASCII array
    @param num Out the result as unit16_t type
    @return True if success
*/
BOOL Util_Transform_Str2u16Num(CHAR *str, UINT16 *num);

BOOL Util_Transform_Str2u32Num(CHAR *str, UINT32 *num);

/*!
    @brief Convert the string to unsigned 8 bits number
    @param str Input the number as ASCII array
    @param num Out the result as unit8_t type
    @return True if success
*/
BOOL Util_Transform_Str2u8Num(CHAR *str, UINT8 *num);


/*!
    @brief Convert a string  to number array
    @param str The string
    @param nums Out the result as unit8 array
    @return Size of array
*/
INT32 Util_Transform_Str2u8HexNumArray(CHAR *str, UINT8 nums[]);

/*!
    @brief Convert the hex string set to number array
    @param argc The number of ASCII array
    @param argv Input the number as ASCII array
    @param nums Out the result as unit8 array
    @return True if success
*/
BOOL Util_Transform_StrArray2u8HexNumArray(INT32 argc, CHAR **argv, UINT8 nums[]);

/*!
    @brief Convert the string set to number array
    @param argc The number of ASCII array
    @param argv Input the number as ASCII array
    @param nums Out the result as unit8 array
    @return True if success
*/
BOOL Util_Transform_StrArray2u8NumArray(INT32 argc, CHAR **argv, UINT8 nums[]);

BOOL Util_Transform_StrArray2u16HexNumArray(INT32 argc, CHAR **argv, UINT16 nums[]);

BOOL Util_Transform_StrArray2u16NumArray(INT32 argc, CHAR **argv, UINT16 nums[]);

BOOL Util_Transform_StrArray2u32HexNumArray(INT32 argc, CHAR **argv, UINT32 nums[]);

/*!
    @brief Convert the string set to number array
    @param argc The number of ASCII array
    @param argv Input the number as ASCII array
    @param nums Out the result as unit8 array
    @return True if success
*/
BOOL Util_Transform_StrArray2u32NumArray(INT32 argc, CHAR **argv, UINT32 nums[]);

/*! @} */

#endif

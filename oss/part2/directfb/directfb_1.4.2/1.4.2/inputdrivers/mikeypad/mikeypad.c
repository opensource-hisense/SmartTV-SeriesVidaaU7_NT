// <MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
// <MStar Software>

#include <config.h>
#include <stdlib.h>
#include <directfb_keyboard.h>
#include <misc/conf.h>
#include <core/input_driver.h>
#include <directfb.h>
#include <linux/input.h>
#include <core/input.h>
//MI header
//#include "mi_common.h"
//#include "mi_sar.h"
//#include "mi_sys.h"
DFB_INPUT_DRIVER( mikeypad_input )


#define KEYCODE_CNT  DIKS_CUSTOM99 - DIKS_CUSTOM80 + 1
#define MAX_LINUX_INPUT_DEVICES 16

/// The path to read boot cmd from Mboot
#define CMD_LINE_PATH               "/proc/cmdline"
#define ENV_PROJECT_ID              "cusdata_projectid"
#define DATA_CONFIG_INI_PATH        "/cusdata/config/dataIndex/"
#define PROJECT_ID_LEN              (10+1)
#define MAX_BUFFER                  63
#define CMD_LINE_SIZE               1024
#define INI_PARSER_SIZE             256
//Keypad related definition
#define MAX_KEYPAD_BUFFER 63  //LoadKEYPADInfo
#define MaxKeycodeSelect 4
#define SupportLongPressOnce 0
#define KEY_MAX 0x2ff
#define DefaultTimeBound 2

#ifndef BITS_PER_LONG
#define BITS_PER_LONG        (sizeof(long) * 8)
#endif
#define NBITS(x)             ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)               ((x)%BITS_PER_LONG)
#define BIT(x)               (1UL<<OFF(x))
#define LONG(x)              ((x)/BITS_PER_LONG)
#undef test_bit
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

/*
 * declaration of private data
 */
typedef struct {
    CoreInputDevice         *device;
    bool                     dispatch_enable;
    int                      fd;
    DirectThread              *thread;
} keypadInputData;

typedef struct {
    const char *pKeyDefine;
    unsigned int UtopiaKeycode;
    DFBInputDeviceKeySymbol DfbKeycode;
} stKeycodeInfo;

typedef struct {
    bool bMultiFuncKeyEnabled; /* Support long/short press or not */
    u32 u32TimeBound; /* Greater than time bound means long press; otherwise, means short press */
    unsigned int UtopiaKeycode;
    unsigned int OriKeycode;
    unsigned int ExtKeycode;
    u8 u8MultiEventMode;  /* 0:long press event once, 1: long press event repeat */
} MultiFuncKeypadInfo;

typedef enum {
    EN_MUITIFUNCKEY_CTRL_NONE = 0,
    EN_MUITIFUNCKEY_CTRL_START,
    EN_MUITIFUNCKEY_CTRL_ORIGINAL_KEY,
    EN_MUITIFUNCKEY_CTRL_EXTENSION_KEY,
    EN_MUITIFUNCKEY_CTRL_COUNTING,
    EN_MUITIFUNCKEY_CTRL_END,
}EN_MUITIFUNCKEY_CTRL;

//============================================
//  Sysinfo Keycode,  Utopia Keycode,  DFB Keycode
//============================================
stKeycodeInfo KeycodeMappingTable[KEYCODE_CNT] = {
    {"KEYPAD_POWER", 0x74, 0x8f000},
    {"MVK_CHANNEL_PLUS", 0x192, 0x5f000},
    {"MVK_CHANNEL_MINUS", 0x193, 0x5f001},
    {"MVK_VOLUME_MINUS", 0x72, 0x6f001},
    {"MVK_VOLUME_PLUS", 0x73, 0x6f000},
    {"KEYPAD_MENU", 0x8B, 0x8f001},
    {"KEYPAD_SELECT", 0x1C, 0x4f000},
    {"KEYPAD_EXIT", 0xA7, DIKS_CUSTOM87},
    {"KEYPAD_SOURCE", 0x4F, 0x5f01f},
    {"KEYPAD_CUSTOMER89", 0xA9, DIKS_CUSTOM89},
    {"KEYPAD_CUSTOMER90", 0xAA, DIKS_CUSTOM90},
    {"KEYPAD_CUSTOMER91", 0xAB, DIKS_CUSTOM91},
    {"KEYPAD_CUSTOMER92", 0xAC, DIKS_CUSTOM92},
    {"KEYPAD_CUSTOMER93", 0xAD, DIKS_CUSTOM93},
    {"KEYPAD_CUSTOMER94", 0xAE, DIKS_CUSTOM94},
    {"KEYPAD_CUSTOMER95", 0xAF, DIKS_CUSTOM95},
    {"KEYPAD_CUSTOMER96", 0xB0, DIKS_CUSTOM96},
    {"KEYPAD_CUSTOMER97", 0xB1, DIKS_CUSTOM97},
    {"KEYPAD_CUSTOMER98", 0xB2, DIKS_CUSTOM98},
    {"KEYPAD_CUSTOMER99", 0xB3, DIKS_CUSTOM99},
};

/* global parameters for Multi fun keypad*/
bool  bMultiFuncKeyHandling = false;
u64 u64BeginTime = 0;
u64 u64ElapsedTime = 0;
u64 u64CurrentTime = 0;
int MultiFunKeyCount = 0;//recored how many times DFB dispatches the "long press" event

//Translates a keypad keycode into DirectFB key id.
static DFBInputDeviceKeyIdentifier keyboard_get_identifier( int code )
{
     return DIKI_UNKNOWN;
}


/* exported symbols */

/*
 * Return the number of available devices.
 * Called once during initialization of DirectFB.
 */
static int
driver_get_available( void )
{
    if (NULL==dfb_config)
    {
        D_WARN("[DFB] Warning!!!!%s (%d) dfb_config is NULL\n",__FUNCTION__, __LINE__);
        return 0;
    }

    return 1;
}

/*
 * Fill out general information about this driver.
 * Called once during initialization of DirectFB.
 */
static void
driver_get_info( InputDriverInfo *info )
{
    int ret = 0;

    if (NULL == info)
    {
        DBG_INPUT_MSG("[DFB] Warning!!!!%s (%d) mikeypad info is NULL\n",__FUNCTION__, __LINE__);
        return;
    }

     /* fill driver info structure */
     ret = snprintf ( info->name,
                DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "keypad Keypad Driver" );
     if(ret < 0)
     {
         D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
         return;
     }
     ret = snprintf ( info->vendor,
                DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "MStar Semiconductor." );
     if(ret < 0)
     {
         D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
         return;
     }

     info->version.major = 0;
     info->version.minor = 69;
}

static bool cmdLineParser(char *str, size_t str_len, const char* keyword_string)
{
    FILE *cmdLine;
    char cmdLineBuf[CMD_LINE_SIZE];
    char *buf;
    char* pTypeStart = 0;
    char* pTypeEnd = 0;
    int ret = 0;
    cmdLine=fopen(CMD_LINE_PATH, "r");
    if(cmdLine != NULL)
    {
        if (fgets(cmdLineBuf, CMD_LINE_SIZE, cmdLine) != NULL)
        {
            if(!fclose(cmdLine))
                D_ERROR("[DFB] %s, %d : close %s error!\n",__FUNCTION__,__LINE__,CMD_LINE_PATH);
            buf = strstr(cmdLineBuf, keyword_string);
            if (buf)
            {
                pTypeStart = strchr(buf,'=');
            }
            else
            {
                return false;
            }
            if (pTypeStart)
            {
                pTypeEnd = strchr(buf,' ');
            }
            else
            {
                return false;
            }
            if (pTypeEnd)
            {
                *pTypeEnd = 0;
            }
            else
            {
                return false;
            }
            ++pTypeStart;
            ret = snprintf(str, str_len, "%s", pTypeStart);
            if(ret < 0)
            {
                D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
                return false;
            }
            return true;
        }
        else
        {
            if (!fclose(cmdLine))   /* Close the stream. */
                D_ERROR("[DFB] %s, %d : close %s error!\n",__FUNCTION__,__LINE__,CMD_LINE_PATH);
            return false;
        }
    }
    return false;
}

/*
    Count the number of buttons need to support long/short press in keypadCode.ini
    If you want to know how to set the long/short press setting, please search /config/keypadCode.ini in console.
*/
static bool multiKeySettingCount(const char* filename, const char* section, int* ret_count)
{
    FILE* in;
    char  buf[INI_PARSER_SIZE]={0};
    char* ptr = NULL;
    bool ret = false;
    int index = 0;
    int count = 0;

    if(filename == NULL || section == NULL )
    {
        return ret;
    }

    in = fopen(filename, "r");
    if(NULL == in)
    {
        return ret;
    }

    while(fgets(buf, INI_PARSER_SIZE, in) != NULL)
    {
        ptr = buf;
        index = 0;
        while(isspace(*ptr) && *ptr) // skip white-space character.
        {
            index++;
            if (index > sizeof(buf))
            {
                break;
            }
            ptr = &buf[index];
        }
        if((*ptr == '[') && ((ptr=strstr(ptr,section)) != NULL))
            count ++; // found the specified section, count++

        memset(buf, 0 , INI_PARSER_SIZE);
    }

    if (!fclose(in))   /* Close the stream. */
        D_ERROR("[DFB] %s, %d : close %s error!\n",__FUNCTION__,__LINE__,filename);

    if (count > 0)
        ret = true;

    DBG_INPUT_MSG("[DFB] %s %d, there are %d multi-func keypad settings\n",__FUNCTION__, __LINE__, count);

    *ret_count =  count;

    return ret;

}


static bool strParser(const char* buf, const char* key, char* string)
{
    char* ptr = buf;
    bool ret = false;
    int index = 0;

    if( key == NULL || string == NULL)
    {
        return ret;
    }

    if((ptr=strstr(ptr, key)) != NULL)
    {
        index = 0;
        if((ptr=strstr(ptr, "=")) != NULL)
        {
            index++;
            while(*(ptr+index) != '\0')
            {
                if(isspace(*(ptr+index)) || *(ptr+index) == '"')
                {
                    index++;
                    continue;
                }
                else if(*(ptr+index) == ';' || *(ptr+index) == '#')
                {
                    break;
                }
                else
                {
                    *string = *(ptr+index);
                    string++;
                    ret = true;
                }
                index++;
            }
            *string = '\0';
        }
    }

    return ret;

}


/*
    Catch the contents of [MultiFuncKeypad_X], including MultiFuncKeypadSelect, MultiFuncTimebound, MultiKeyEventModeSelect,
    UtopiaKeycode, OriKeycode and ExtKeycode.
    If you want to know how to set the long/short press setting, please search /config/keypadCode.ini in console.
*/
static MultiFuncKeypadInfo multiKeySettingParser(const char* filename, const char* section )
{
    FILE* in;
    char  buf[INI_PARSER_SIZE]={0};
    char* ptr = NULL;
    int index = 0;
    char inistrbuff[MAX_BUFFER] = {0};
    char strUtopiaKeyCode[MAX_KEYPAD_BUFFER] = {0};
    char strOriKeycode[MAX_KEYPAD_BUFFER] = {0};
    char strExtKeycode[MAX_KEYPAD_BUFFER] = {0};
    bool bknowkeyselectnumber = false;
    int ret = 0;

    /* initialize value*/
    MultiFuncKeypadInfo multiFuncKeyInfo;
    multiFuncKeyInfo.bMultiFuncKeyEnabled = false;
    multiFuncKeyInfo.u32TimeBound = 1000000 * DefaultTimeBound;
    multiFuncKeyInfo.UtopiaKeycode = DIKS_NULL;
    multiFuncKeyInfo.OriKeycode = DIKS_NULL;
    multiFuncKeyInfo.ExtKeycode = DIKS_NULL;
    multiFuncKeyInfo.u8MultiEventMode = 0;
    int iKeycodeSelect =-1;
    int count = 0;

    if(filename == NULL || section == NULL )
    {
        printf("\33[0;33;44m[DFB] %s %d, the filename or section is NULL\33[0m, fail to parse setting\n",__FUNCTION__,__LINE__);
        return multiFuncKeyInfo;
    }

    in = fopen(filename, "r");
    if(NULL == in)
    {
        printf("\33[0;33;44m[DFB] %s %d, fail to open keypadCode.ini\33[0m\n",__FUNCTION__,__LINE__);
        return multiFuncKeyInfo;
    }

    while(fgets(buf, INI_PARSER_SIZE, in) != NULL)
    {
        ptr = buf;
        index = 0;
        while(isspace(*ptr) && *ptr) // skip white-space character.
        {
            index++;
            if (index > sizeof(buf))
            {
                break;
            }
            ptr = &buf[index];
        }
        if((*ptr == '[') && ((ptr=strstr(ptr,section)) != NULL)) break; // found the specified section

        memset(buf, 0 , INI_PARSER_SIZE);
    }

    while(fgets(buf, INI_PARSER_SIZE, in) != NULL)
    {
        ptr = buf;
        index = 0;
        while(isspace(*ptr) && *ptr) // skip white-space character.
        {
            index++;
            if (index > sizeof(buf))
            {
                break;
            }
            ptr = &buf[index];
        }

        if(*ptr == '#') // skip comment line
        {
            memset(buf, 0, INI_PARSER_SIZE);
            continue;
        }

        if(*ptr == '[') // reach next section
        {
            break;
        }


        if (strParser(ptr,"MultiFuncKeypadSelect", inistrbuff))
        {
            iKeycodeSelect = strtol(inistrbuff, NULL, 0);
            if(errno == ERANGE)
                D_ERROR("[DFB] %s, %d : overflow/underflow happened!\n",__FUNCTION__,__LINE__);
            if( iKeycodeSelect <= 0 || iKeycodeSelect > MaxKeycodeSelect )
            {
                printf("[DFB] %s %d, iKeycodeSelect is invalid, value=%d\n",__FUNCTION__,__LINE__, iKeycodeSelect);
                break;
            }

             /* We don't know which group of keypad settings the user may select until we get the MultiFuncKeypadSelect.
                  In this flow, it means we have known MultiFuncKeypadSelect value and we can start to parse the string which is relevant to keycode.
             */
            if (!bknowkeyselectnumber){
                ret = snprintf(strUtopiaKeyCode, MAX_KEYPAD_BUFFER, "UtopiaKeycode_%d", iKeycodeSelect);
                if(ret < 0)
                {
                    D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
                    return multiFuncKeyInfo;
                }
                ret = snprintf(strOriKeycode, MAX_KEYPAD_BUFFER, "OriKeycode_%d", iKeycodeSelect);
                if(ret < 0)
                {
                    D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
                    return multiFuncKeyInfo;
                }
                ret = snprintf(strExtKeycode, MAX_KEYPAD_BUFFER, "ExtKeycode_%d", iKeycodeSelect);
                if(ret < 0)
                {
                    D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
                    return multiFuncKeyInfo;
                }
                DBG_INPUT_MSG("[DFB] %s%d, strUtopiaKeyCode=%s, strOriKeycode=%s, strExtKeycode=%s\n",__FUNCTION__,__LINE__,strUtopiaKeyCode,strOriKeycode,strExtKeycode);
                bknowkeyselectnumber = true;
            }
        }
        else if (strParser(ptr,"MultiFuncTimebound", inistrbuff))
        {
            float timebound = atof(inistrbuff); // timebound unit is usec
            if (timebound < 1) // timebound setting can't less than 1 sec(1000000 usec)
            {
                multiFuncKeyInfo.u32TimeBound = 1000000 * DefaultTimeBound; //default 2 sec
                printf("\33[0;33;44m[DFB] %s,%d TimeBound setting is valid ( %f sec < 1 sec) \33[0m, use default time bound 2s\n",__FUNCTION__,__LINE__,timebound);
            }
            else
                multiFuncKeyInfo.u32TimeBound = 1000000 * timebound;
        }
        else if (strParser(ptr,"MultiKeyEventModeSelect", inistrbuff))
        {
            multiFuncKeyInfo.u8MultiEventMode = strtol(inistrbuff, NULL, 0);
            if(errno == ERANGE)
                D_ERROR("[DFB] %s, %d : overflow/underflow happened!\n",__FUNCTION__,__LINE__);
        }
        else if (bknowkeyselectnumber && strParser(ptr, strUtopiaKeyCode, inistrbuff))
        {
            multiFuncKeyInfo.UtopiaKeycode = strtol(inistrbuff, NULL, 0);
            if(errno == ERANGE)
                D_ERROR("[DFB] %s, %d : overflow/underflow happened!\n",__FUNCTION__,__LINE__);
        }
        else if (bknowkeyselectnumber && strParser(ptr, strOriKeycode, inistrbuff))
        {
            multiFuncKeyInfo.OriKeycode = strtol(inistrbuff, NULL, 0);
            if(errno == ERANGE)
                D_ERROR("[DFB] %s, %d : overflow/underflow happened!\n",__FUNCTION__,__LINE__);
        }
        else if (bknowkeyselectnumber && strParser(ptr, strExtKeycode, inistrbuff))
        {
            multiFuncKeyInfo.ExtKeycode = strtol(inistrbuff, NULL, 0);
            if(errno == ERANGE)
                D_ERROR("[DFB] %s, %d : overflow/underflow happened!\n",__FUNCTION__,__LINE__);
        }
        else
            DBG_INPUT_MSG("[DFB] No match string, buf=%s\n", ptr);

        memset(buf, 0, INI_PARSER_SIZE);
    }

    /* If we get keycode successfully, set bMultiFuncKeyEnabled TRUE which means this keycode setting is valid. */
    if (multiFuncKeyInfo.UtopiaKeycode != DIKS_NULL &&
         multiFuncKeyInfo.OriKeycode != DIKS_NULL &&
         multiFuncKeyInfo.ExtKeycode != DIKS_NULL )
    {
        multiFuncKeyInfo.bMultiFuncKeyEnabled = true;
    }

    if(!fclose(in))
        D_ERROR("[DFB] %s, %d : close %s failed!\n",__FUNCTION__,__LINE__,filename);

    return multiFuncKeyInfo;

}

static bool iniStrParser(const char* filename, const char* section, const char* key, char* string)
{
    FILE* in;
    char  buf[INI_PARSER_SIZE]={0};
    char* ptr = NULL;
    bool ret = false;
    int index = 0;

    if(filename == NULL || section == NULL || key == NULL || string == NULL)
    {
        return ret;
    }
    in = fopen(filename, "r");
    if(NULL == in)
    {
        return ret;
    }

    while(fgets(buf, INI_PARSER_SIZE, in) != NULL)
    {
        ptr = buf;
        index = 0;
        while(isspace(*ptr) && *ptr) // skip white-space character.
        {
            index++;
            if (index > sizeof(buf))
            {
                break;
            }
            ptr = &buf[index];
        }
        if((*ptr == '[') && ((ptr=strstr(ptr,section)) != NULL)) break; // found the specified section

        memset(buf, 0 , INI_PARSER_SIZE);
    }

    while(fgets(buf, INI_PARSER_SIZE, in) != NULL)
    {
        ptr = buf;
        index = 0;
        while(isspace(*ptr) && *ptr) // skip white-space character.
        {
            index++;
            if (index > sizeof(buf))
            {
                break;
            }
            ptr = &buf[index];
        }

        if(*ptr == '#') // skip comment line
        {
            memset(buf, 0, INI_PARSER_SIZE);
            continue;
        }

        if(*ptr == '[') // reach next section
        {
            break;
        }

        if((ptr=strstr(ptr, key)) != NULL)
        {
            index = 0;
            if((ptr=strstr(ptr, "=")) != NULL)
            {
                index++;
                while(*(ptr+index) != '\0')
                {
                    if(isspace(*(ptr+index)) || *(ptr+index) == '"')
                    {
                        index++;
                        continue;
                    }
                    else if(*(ptr+index) == ';' || *(ptr+index) == '#')
                    {
                        break;
                    }
                    else
                    {
                        *string = *(ptr+index);
                        string++;
                        ret = true;
                    }
                    index++;
                }
                *string = '\0';
            }
            break;
        }
        memset(buf, 0, INI_PARSER_SIZE);
    }

    if(!fclose(in))
        D_ERROR("[DFB] %s, %d : close %s failed\n",__FUNCTION__,__LINE__,filename);

    return ret;

}

static bool getKeypadCodeFilePath(char* pRetKeypadCodeFilePath, const unsigned short u16DataNameLen)
{
    char pDataIndexName[MAX_BUFFER] = {0};
    char keypadCodeFilePath[MAX_BUFFER] = {0};
    char pProjectId[PROJECT_ID_LEN] = {0};
    int ret = 0;

    if(cmdLineParser(pProjectId, sizeof(pProjectId), ENV_PROJECT_ID) == true)
    {
        if( strchr(pProjectId, '.') != NULL || strchr(pProjectId, '/') != NULL)
        {
            ret = snprintf(pDataIndexName, sizeof(pDataIndexName)-1, "/cusdata/config/dataIndex/dataIndex_1.ini");
            if(ret < 0)
            {
                D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
                return false;
            }
        }
        else
        {
            ret = snprintf(pDataIndexName, sizeof(pDataIndexName)-1, "/cusdata/config/dataIndex/dataIndex_%s.ini",pProjectId);
            if(ret < 0)
            {
                D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
                return false;
            }
        }

        if(iniStrParser(pDataIndexName, "Keypad", "m_pKeypad_File", keypadCodeFilePath) == true)
        {
            memset(pRetKeypadCodeFilePath ,0 ,u16DataNameLen);
            strncpy(pRetKeypadCodeFilePath, keypadCodeFilePath,strlen(keypadCodeFilePath));
            return true;
        }
    }
    return false;
}


static bool keypad_LoadMultiFuncKeyInfo(MultiFuncKeypadInfo **ret_multiFuncKeyInfos, int *ret_count)
{
    MultiFuncKeypadInfo multiFuncKeyInfo;
    multiFuncKeyInfo.bMultiFuncKeyEnabled = false;
    multiFuncKeyInfo.u32TimeBound = 0;
    multiFuncKeyInfo.UtopiaKeycode = DIKS_NULL;
    multiFuncKeyInfo.OriKeycode = DIKS_NULL;
    multiFuncKeyInfo.ExtKeycode = DIKS_NULL;
    multiFuncKeyInfo.u8MultiEventMode = 0;
    int ret = 0;

    char keypadCodeFile[MAX_BUFFER] = {0};
    if(getKeypadCodeFilePath(keypadCodeFile, MAX_BUFFER) == false)
    {
        printf("[DFB] %s,%d, Get KeypadCode file path fail\n",__FUNCTION__,__LINE__);
        return false;
    }

    int numberofsetting = 0;

    if (multiKeySettingCount(keypadCodeFile, "MultiFuncKeypad_", &numberofsetting) == false)
    {
        printf("[DFB] %s,%d, No one set MultiFuncKeypad setting in keypadCode.ini\n",__FUNCTION__,__LINE__);
        return false;
    }

    MultiFuncKeypadInfo *multiFuncKeyInfos;
    int i =0;
    char aIniKey[MAX_KEYPAD_BUFFER] = {0};
    multiFuncKeyInfos = D_CALLOC( numberofsetting, sizeof(MultiFuncKeypadInfo) );

    if (!multiFuncKeyInfos)
         return D_OOM();

    DBG_INPUT_MSG("[DFB] ********************MultiFun key setting start**********************, total %d settings\n",numberofsetting);
    for (i=0; i<numberofsetting; i++) {
        ret = snprintf(aIniKey, MAX_KEYPAD_BUFFER, "MultiFuncKeypad_%d", i);
        if(ret < 0)
        {
            D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
            return false;
        }

        multiFuncKeyInfos[i] = multiKeySettingParser(keypadCodeFile, aIniKey );

        DBG_INPUT_MSG("group[%d] enable=%d, short press keycode=%d, long press keycode=%d, evt mode=%d, time bound=%d\n ",
        i, multiFuncKeyInfos[i].bMultiFuncKeyEnabled, multiFuncKeyInfos[i].OriKeycode, multiFuncKeyInfos[i].ExtKeycode,  multiFuncKeyInfos[i].u8MultiEventMode,multiFuncKeyInfos[i].u32TimeBound );
    }
    DBG_INPUT_MSG("[DFB] ********************MultiFun key setting end**********************");

    *ret_multiFuncKeyInfos = multiFuncKeyInfos;
    *ret_count = numberofsetting;
    return true;

}

static bool findMultiFunKeyIndex(const MultiFuncKeypadInfo *multiFuncKeyInfos, const int keycode, const int count, int *index)
{
    if (multiFuncKeyInfos ==NULL || index == NULL)
    {
        printf("[DFB] %s %d, multiFuncKeyInfos or index is NULL\n",__FUNCTION__,__LINE__);
        return false;
    }
    int i = 0;
    bool ret = false;
    for(i=0; i<count;i++)
    {
        if (multiFuncKeyInfos[i].UtopiaKeycode == keycode){
            *index =i;
            ret = true;
            break;
        }
    }

    return ret;
}

DFBResult keypad_MultiFuncKey_Ctrl( EN_MUITIFUNCKEY_CTRL enMultiFuncKeyCtrl, MultiFuncKeypadInfo multiFuncKeypadInfo, unsigned int *Keycode)
{
     switch(enMultiFuncKeyCtrl)
     {
        case EN_MUITIFUNCKEY_CTRL_EXTENSION_KEY:
        {
            u64ElapsedTime =0;
            u64BeginTime =0;
            bMultiFuncKeyHandling = false;

            /*
              MultiFunKeyCount is used to recored how many times DFB dispatches the "long press" event.
              If u64ElapsedTime is layer than multiFuncKeyInfo.u32TimeBound, you would enter this flow and dispatch event to APP.
            */
            MultiFunKeyCount ++;

            /*
               1. If multiFuncKeypadInfo.u8MultiEventMode =0, only dispatch key event once every key press
               2. If multiFuncKeypadInfo.u8MultiEventMode =1, dispatch key event [Repeat] every u32TimeBound second
            */
            if ( multiFuncKeypadInfo.u8MultiEventMode == SupportLongPressOnce && MultiFunKeyCount > 1 )
                *Keycode = DIKS_NULL;
            else
                *Keycode = multiFuncKeypadInfo.ExtKeycode;

            return DFB_OK;
        }
        case EN_MUITIFUNCKEY_CTRL_COUNTING:
        {
            /*
               If u64ElapsedTime is not greater than multiFuncKeyInfo.u32TimeBound, keep counting and not dispatch the event to APP
               we will return DFB_INCOMPLETE which means the once press/relese event not over yet.
            */
            *Keycode = multiFuncKeypadInfo.UtopiaKeycode;
            return DFB_INCOMPLETE;
        }
        case EN_MUITIFUNCKEY_CTRL_START:
        {
             /*
                The once press/relese event, DFB will go to  EN_MUITIFUNCKEY_CTRL_START first, record u64BeginTime and set bMultiFuncKeyHandling TRUE
                to start probing for long/short key event.
             */
            u64BeginTime = u64CurrentTime;
            bMultiFuncKeyHandling = true;
            *Keycode = DIKS_NULL;
            return DFB_INCOMPLETE;
        }
        case EN_MUITIFUNCKEY_CTRL_END:
        {
            *Keycode = (MultiFunKeyCount > 0) ? multiFuncKeypadInfo.ExtKeycode : multiFuncKeypadInfo.OriKeycode;
            /*if it's release event, reset all the parameter and recalculate. */
            u64ElapsedTime =0;
            u64BeginTime =0;
            MultiFunKeyCount =0;
            bMultiFuncKeyHandling = false;
            return DFB_OK;
        }
        default:
        {
            *Keycode = DIKS_NULL;
            return DFB_FAILURE;
        }
     }
}

static void*
MstarKeypadEventThread( DirectThread *thread, void *driver_data )
{
    keypadInputData *data = (keypadInputData*) driver_data;
    struct input_event levt[64];
    fd_set   set;

    int  fdmax;
    fdmax = data->fd ;
    int  status,readlen;
    unsigned int  i;
    int index = -1;
    int Keycode =0;
    int count = 0;

    MultiFuncKeypadInfo *multiFuncKeyInfos; /* Record all of the multi fun key setting in keypadCode.ini */
    EN_MUITIFUNCKEY_CTRL enMultiFuncKey = EN_MUITIFUNCKEY_CTRL_NONE;
    DFBInputEventType  Evttype =DIET_UNKNOWN;
    bool isMultiFunKey = false;
    bool isRepeat =false;
    DFBResult ret = DFB_OK;

    keypad_LoadMultiFuncKeyInfo(&multiFuncKeyInfos, &count);
    while(1){

         DFBInputEvent devt = { .type = DIET_UNKNOWN };
         FD_ZERO( &set );
         FD_SET( data->fd, &set );
         struct timeval timeout={1,0};
         status = select( fdmax + 1, &set, NULL, NULL, &timeout );

         if (status < 0 && errno != EINTR)
         {
             int err= errno;
             DBG_INPUT_MSG("[DFB] Mstar Keypad EventThread  break line:%d, status=%d, err=%d \n", __LINE__, status, err);
             break;
         }

         if (status <= 0){
             //printf("[DFB] Mstar Keypad EventThread  break line:%d, status=%d, err=%d \n", __LINE__, status);
             continue;}

         direct_thread_testcancel( thread );

         readlen = read( data->fd, levt, sizeof(levt) );
         if (readlen < 0 && errno != EINTR)
         {
             int err= errno;
             DBG_INPUT_MSG("[DFB] Mstar Keypad   EventThread break line:%d, readlen=%d, err=%d\n", __LINE__, readlen, err);
             break;
         }

         direct_thread_testcancel( thread );
         if (readlen <= 0){
             DBG_INPUT_MSG("[DFB] Mstar Keypad   EventThread break line:%d, readlen=%d\n", __LINE__, readlen);
             continue;
         }

         for (i=0; i<readlen / sizeof(levt[0]); i++) {

            if( levt[i].type != EV_KEY)
                continue;

            /* query key code and repeat flag every key event, and do initialization, such as ret=DFB_OK */
            Keycode = levt[i].code;
            Evttype = levt[i].value ? DIET_KEYPRESS : DIET_KEYRELEASE;

            if (levt[i].value == 2)
                isRepeat  = true;
            else
                isRepeat  = false;

            ret = DFB_OK;

            isMultiFunKey = findMultiFunKeyIndex(multiFuncKeyInfos, Keycode, count, &index);

            /*
               If you use MultiFuncKeypad, please refer to the following information!!

               1. Only the spefic keycode which is as same as UtopiaKeycode can enter multi function flow, otherwise, other keycodes are not affected.
               2. If multiFuncKeypadInfo.u8MultiEventMode =0,  DFB will dispatch one press and one release event.
               3. If multiFuncKeypadInfo.u8MultiEventMode =1,  DFB will dispatch press event -> press [repeat] event ->... press [repeat] event ->release event

            */
            if(isMultiFunKey && multiFuncKeyInfos[index].bMultiFuncKeyEnabled && multiFuncKeyInfos[index].UtopiaKeycode == Keycode )
            {
                DBG_INPUT_MSG("[DFB] %s,%d, bMultiFuncKeyEnabled=%d, OriKeycode=%d, ExtKeycode=%d, u8MultiEventMode=%d, u32TimeBound =%d\n",
                __FUNCTION__,__LINE__, multiFuncKeyInfos[index].bMultiFuncKeyEnabled, multiFuncKeyInfos[index].OriKeycode, multiFuncKeyInfos[index].ExtKeycode,
                multiFuncKeyInfos[index].u8MultiEventMode, multiFuncKeyInfos[index].u32TimeBound);

                if (Evttype ==DIET_KEYPRESS)
                {
                    struct timeval tvTime;
                    struct timezone tz;
                    gettimeofday(&tvTime, &tz);
                    u64CurrentTime = (tvTime.tv_sec * 1000000  + tvTime.tv_usec);

                    if(bMultiFuncKeyHandling == true)
                    {
                        u64ElapsedTime = u64CurrentTime - u64BeginTime;

                        /* If u64ElapsedTime is greater than u32TimeBound means it's EXTENSION_KEY, else keeping counting*/
                        enMultiFuncKey = u64ElapsedTime >=multiFuncKeyInfos[index].u32TimeBound? EN_MUITIFUNCKEY_CTRL_EXTENSION_KEY : EN_MUITIFUNCKEY_CTRL_COUNTING;

                        ret = keypad_MultiFuncKey_Ctrl(enMultiFuncKey, multiFuncKeyInfos[index], &Keycode);
                        if (MultiFunKeyCount == 1)
                            isRepeat = false;
                    }
                    else
                        ret = keypad_MultiFuncKey_Ctrl(EN_MUITIFUNCKEY_CTRL_START, multiFuncKeyInfos[index], &Keycode);

                }
                else if (Evttype ==DIET_KEYRELEASE)
                {
                    /* If it's a short press, need to do an additionl keypress event before key release.*/
                    if (MultiFunKeyCount == 0)
                    {
                        DFBInputEvent evt = { .type = DIET_UNKNOWN };
                        evt.flags = DIEF_TIMESTAMP;
                        evt.timestamp = levt[i].time;
                        evt.type = DIET_KEYPRESS;
                        evt.key_code = multiFuncKeyInfos[index].OriKeycode;


                        evt.flags |= DIEF_KEYCODE;
                        evt.key_id    = DIKI_UNKNOWN;
                        evt.key_symbol = DIKS_NULL;
                        dfb_input_dispatch( data->device, &evt );
                    }
                    ret = keypad_MultiFuncKey_Ctrl(EN_MUITIFUNCKEY_CTRL_END, multiFuncKeyInfos[index], &Keycode);
                }
            }

            DBG_INPUT_MSG("[DFB] Keycode=%d, repeat=%d, ret=%s\n",Keycode, isRepeat, DirectResultString( ret ));

            /*
                1. ret =OK, means this is the complete key event control flow.
                2. ret =DFB_INCOMPLETE, means now it's EN_MUITIFUNCKEY_CTRL_COUNTING or EN_MUITIFUNCKEY_CTRL_START status.
            */
            if(ret != DFB_OK)
                continue;

            /* check the keycode is vaild? */
            if(Keycode == DIKS_NULL || Keycode > KEY_MAX)
                continue;


            devt.flags = DIEF_TIMESTAMP;
            devt.timestamp = levt[i].time;
            devt.type = levt[i].value ? DIET_KEYPRESS : DIET_KEYRELEASE;
            devt.key_code = Keycode;

            if (isRepeat)
               devt.flags |= DIEF_REPEAT;

            devt.flags |= DIEF_KEYCODE;
            devt.key_id    = DIKI_UNKNOWN;
            devt.key_symbol = DIKS_NULL;

            dfb_input_dispatch( data->device, &devt );
        }
    }

    if (status <= 0)
        printf("\33[0;33;44m[DFB] mstar keypad thread died\33[0m\n");

    return NULL;
}


/*
 * Open the device, fill out information about it,
 * allocate and fill private data, start input thread.
 * Called during initialization, resuming or taking over mastership.
 */
static DFBResult
driver_open_device( CoreInputDevice  *device,
                    unsigned int      number,
                    InputDeviceInfo  *info,
                    void            **driver_data )
{

    int fd = -1;
    int i;
    int repeat_time[2];
    int ret = 0;
    InputDeviceInfo device_info;
    keypadInputData *data = NULL;
    char name[DFB_INPUT_DEVICE_DESC_NAME_LENGTH]={0};
    data = D_CALLOC(1, sizeof(keypadInputData));
    bool bsearch_keypad = false;

    if (data == NULL){
        printf("[DFB]%s,%d, data is NULL\n",__FUNCTION__,__LINE__);
        return DFB_FAILURE;
    }
    if (info == NULL){
        printf("[DFB]%s,%d, info is NULL\n",__FUNCTION__,__LINE__);
        D_FREE(data);
        return DFB_FAILURE;
    }

    for (i=0; i<MAX_LINUX_INPUT_DEVICES; i++)
    {
        int fd = -1;
        char dev[32];
        ret = snprintf( dev, 32, "/dev/input/event%d", i );
        if(ret < 0)
        {
            D_ERROR("[DFB] %s, %d : snprintf failed!\n",__FUNCTION__,__LINE__);
            return DFB_FAILURE;
        }
        fd = open( dev, O_RDWR );

        if (fd >= 0) {
            /* get device name */
            char     *pos = NULL;
            ioctl( fd, EVIOCGNAME(DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1), name);
            DBG_INPUT_MSG("[DFB] %s,%d, keypad name=%s\n", __FUNCTION__, __LINE__, name);
            pos = strstr(name, "KEYPAD");
            if (pos == NULL)
            {
                close( fd );
                continue;
            }
            else
            {
                strncpy(info->desc.name, name, DFB_INPUT_DEVICE_DESC_NAME_LENGTH);
                info->desc.name[DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1] = 0;
                DBG_INPUT_MSG("[DFB] keypad name=%s, event name=%s\n",info->desc.name,dev);
                data->fd = fd;
                bsearch_keypad = true;
                break;
            }
        }
        else
            DBG_INPUT_MSG("[DFB] ERROR!!! Fail to open keypad Device (%s, %s), errno=%d\n", __FILE__, __FUNCTION__, errno);
    }


    if(!bsearch_keypad){
        printf("[DFB] ERROR!!! Fail to open keypad Device (%s, %s)\n", __FILE__, __FUNCTION__);
        D_FREE(data);
        return DFB_FAILURE;
    }

    /* claim to be the primary keyboard */
    info->prefered_id = DIDID_KEYPAD;

    /* classify as a keyboard able to produce key events */
    /* set type flags */
    info->desc.type   = DIDTF_KEYBOARD;

    /* set capabilities */
    info->desc.caps   = DICAPS_KEYS;

    /* FIXME: the keycode range must be re-defined*/
    info->desc.min_keycode = 0x0;
    info->desc.max_keycode = KEY_MAX;


    data->device = device;
    data->thread = direct_thread_create( DTT_INPUT, MstarKeypadEventThread, data, "MTK Keypad" );
    /* set private data pointer */
    *driver_data = data;

    return DFB_OK;
}

static DFBInputDeviceKeySymbol GetDFBKeycodeByUtopiaKeycode(int iInput)
{
    int i = 0;
    for(i = 0; i < KEYCODE_CNT; i++)
    {
        if (iInput == KeycodeMappingTable[i].UtopiaKeycode)
            return KeycodeMappingTable[i].DfbKeycode;

    }
    /* if the keycode doesn't match, return NULL. */
    return DIKS_NULL;
}

/*
 * Fetch one entry from the kernel keymap.
 */
static DFBResult
driver_get_keymap_entry( CoreInputDevice           *device,
                         void                      *driver_data,
                         DFBInputDeviceKeymapEntry *entry )
{
     if(entry == NULL)
        return DFB_INVARG;

     int code = entry->code;
     DFBInputDeviceKeyIdentifier identifier;

     entry->locks &= ~DILS_CAPS;
     entry->locks &= !DILS_NUM;

     /* get the identifier for basic mapping */
     entry->identifier = keyboard_get_identifier( code );
     /* write base level symbol to entry */
     entry->symbols[DIKSI_BASE] = GetDFBKeycodeByUtopiaKeycode( code );

     return DFB_OK;
}

/*
 * End thread, close device and free private data.
 */
static void
driver_close_device( void *driver_data )
{
     keypadInputData *data = (keypadInputData*) driver_data;

     direct_thread_cancel( data->thread );

     direct_thread_join( data->thread );

     direct_thread_destroy( data->thread );

     /* close socket */
     close( data->fd );


     /* free private data */
    D_FREE( data );
}

static DFBResult
driver_device_ioctl( CoreInputDevice              *device,
                      void                         *driver_data,
                      InputDeviceIoctlData *param)
{
     if (NULL == param)
         return DFB_INVARG;

     if(MDRV_DFB_IOC_MAGIC!= _IOC_TYPE(param->request))
         return DFB_INVARG;

     return DFB_OK;
}

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


#include <directfb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...)\
     {\
          DFBResult  err;\
          err = x;\
          if (err != DFB_OK) {\
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );\
               DirectFBErrorFatal( #x, err );\
          }\
     }

#define RESET_CMD(X) X=CMD_UNKNOWN

#define TIME_UNIT 1000

enum
{
    REC_LAST_KEY=0,
    REC_CURR_KEY,
    REC_SIZE
};

typedef enum
{
    CMD_UNKNOWN = 0,
    CMD_LOOP_COUNT,
    CMD_FILENAME,
    CMD_SPEEDUP,
}E_COMMAND_TYPE;


typedef struct KEY_RECORD
{
    unsigned int  symbol;
    unsigned int  type;
    unsigned int  code;
    unsigned int  key_id;
    unsigned int  flags;
    unsigned int  button;
    unsigned int  axis;
    int                axisabs;
    int                axisrel;
    DFBInputDeviceID device_id;
    long              time;
} KeyRecord;


static IDirectFB *dfb = NULL;
static IDirectFBInputDevice *loopbackdevice = NULL;
static IDirectFBEventBuffer  *keybuffer;

static char *rec_file = NULL;
static int repeat = -1;
static int speedup = 1;

static bool random_test = false;
// for auto gen and random key testing.
static bool rand_key_test = false;

//
// How to "Post Event"?
//

// Step 1: Add the device callback function in the source code of your application
DFBEnumerationResult input_device_callback(DFBInputDeviceID id, DFBInputDeviceDescription desc, void* data)
{
    if(strcmp(desc.name, "mstarloopback") == 0) // Identify the device
    {
        if(DFB_OK == dfb->GetInputDevice(dfb, id, &loopbackdevice))
        {
          printf("[dfbkeyrecord] found loopback device: %d\n", id);

          return DFENUM_CANCEL;  /* Cancel enumeration */
        }
    }
    return DFENUM_OK; /* Proceed with enumeration */
}

// Step 2: Post Event
void PostEvent(KeyRecord *keyRec)
{
    InputDeviceIoctlData sendkeyparam;
    DFBInputEvent  evt = {0};

    sendkeyparam.request = DFB_DEV_IOC_SEND_LOOPBACK_EVENT;

    //
    // NOTE: "key-press" & "key-release" must be sent in a pair of events
    //

    evt.flags      = keyRec->flags;
    evt.type       = keyRec->type;
    evt.key_symbol = keyRec->symbol; //0xF001;
    evt.key_code   = keyRec->code;   //26;
    evt.key_id = keyRec->key_id;
    evt.button = keyRec->button;
    evt.axis = keyRec->axis;
    evt.axisabs = keyRec->axisabs;
    evt.axisrel = keyRec->axisrel;
    evt.device_id= keyRec->device_id;
    memcpy(sendkeyparam.param, &evt, sizeof(DFBInputEvent));
    loopbackdevice->IOCtl(loopbackdevice, &sendkeyparam);
}

// irkey symbol:
// 0 ~ 9 : 0x30 ~ 0x39
// others : 0xF000 ~ 0xF082
#define NUM_KEY_START  0X30
#define NUM_KEY_END    0x39
#define NUM_KEY_ALL (NUM_KEY_END - NUM_KEY_START + 1)

#define SPEC_KEY_START 0xF000
#define SPEC_KEY_END   0xF082
#define SPEC_KEY_ALL (SPEC_KEY_END - SPEC_KEY_START + 1)

#define ALL_KEY (NUM_KEY_ALL + SPEC_KEY_ALL)

unsigned int allKey[ALL_KEY];

// auto generate key symbol.
void gen_all_key()
{
    int i = 0;
    unsigned int key = 0;

    for(key = NUM_KEY_START; key <= NUM_KEY_END; key++)
        allKey[i++] = key;

    for(key = SPEC_KEY_START; key <= SPEC_KEY_END; key++)
        allKey[i++] = key;
}

// use auto gen key symbol to do random key testing.
void key_random_test()
{
    int i;
    int key_cnt = 0;
    int loop_cnt = 0;
    KeyRecord keyRec = {0};
    DFBResult ret;

    gen_all_key();

    printf("\033[1;31m[dfbkeyrecord][key_test] start to run key random test!\033[0m\n");
    ret = dfb->EnumInputDevices( dfb, input_device_callback, NULL );
    if (ret)
        printf( "[dfbkeyrecord] IDirectFB::EnumInputDevices failed\n" );
    else
        printf("[dfbkeyrecord] IDirectFB::EnumInputDevices  successful\n");

    if (speedup < 1)
        speedup = 1;

    while(1)
    {
        for( i = 0; i < ALL_KEY; i++)
        {
            int num = rand() % ALL_KEY;

            if (allKey[num] == DIKS_POWER)
                continue;

            keyRec.symbol = allKey[num];
            keyRec.type = 1; // press
            keyRec.code = 0;
            keyRec.key_id = 0;
            keyRec.flags = 761;
            keyRec.button = 716997440;
            keyRec.axis = keyRec.axisabs = keyRec.axisrel = 0;
            keyRec.time = 200; // speedup;
            keyRec.device_id = 0;

            PostEvent(&keyRec);
            usleep(keyRec.time * TIME_UNIT);
            printf("\033[1;31m[dfbkeyrecord][key_test] key_item = %d, key_symbol=0x%X, key_type=%d, key_code=%d, delay time=%ld\033[0m\n",
                num, keyRec.symbol, keyRec.type, keyRec.code, keyRec.time);

            keyRec.symbol = allKey[num];
            keyRec.type = 2; // release
            keyRec.code = 0;
            keyRec.key_id = 0;
            keyRec.flags = 761;
            keyRec.button = 716997440;
            keyRec.axis = keyRec.axisabs = keyRec.axisrel = 0;
            keyRec.time = 200; // speedup;
            keyRec.device_id = 0;

            PostEvent(&keyRec);
            usleep(keyRec.time * TIME_UNIT);
            printf("\033[1;31m[dfbkeyrecord][key_test] key_item = %d, key_symbol=0x%X, key_type=%d, key_code=%d, delay time=%ld\033[0m\n",
                num, keyRec.symbol, keyRec.type, keyRec.code, keyRec.time);
        }

        loop_cnt++;
        printf("\033[1;31m[dfbkeyrecord][key_test] loop count = %d!\033[0m\n", loop_cnt);

        sleep(2);
        if (repeat > 0) {
            repeat--;
            if (repeat == 0)
                break;
        }

    }
}

void key_test(FILE *pf)
{
    char buf[100];
    int i;
    int key_cnt = 0;
    int loop_cnt = 0;
    KeyRecord *keyRec = NULL;
    DFBResult ret;

    printf("\033[1;31m[dfbkeyrecord][key_test] Now playing the file \"%s\"!\033[0m\n", rec_file);

    /* find out the total key record count*/
    while( fgets(buf, 100, pf) ) {
        key_cnt++;
    }

    printf("[dfbkeyrecord] %d key record read from file!\n", key_cnt);

    keyRec = (KeyRecord *)malloc( sizeof(KeyRecord) * key_cnt);

    fseek(pf, 0, SEEK_SET);

    for(i = 0; i < key_cnt; i++) {
        if ( fscanf(pf, "0x%X, %d, %d, %d, %d, %d, %d, %d, %d, %d, %ld\n",
            &keyRec[i].symbol,
            &keyRec[i].type,
            &keyRec[i].code,
            &keyRec[i].key_id,
            &keyRec[i].flags,
            &keyRec[i].button,
            &keyRec[i].axis,
            &keyRec[i].axisabs,
            &keyRec[i].axisrel,
            &keyRec[i].device_id,
            &keyRec[i].time) == EOF )
            break;
    }

    printf("\033[1;31m[dfbkeyrecord][key_test] start to run key test %s!\033[0m\n", (random_test)? "random" : "sequence");
    ret = dfb->EnumInputDevices( dfb, input_device_callback, NULL );
    if (ret)
        printf( "[dfbkeyrecord] IDirectFB::EnumInputDevices failed\n" );
    else
        printf("[dfbkeyrecord] IDirectFB::EnumInputDevices  successful\n");

    if (speedup < 1)
        speedup = 1;

    while(1)
    {
        for( i = 0; i < key_cnt; i++)
        {
            int num = i;

            if (random_test) { // random key testing.
                num = rand() % (key_cnt / 2);
                //if (keyRec[num].type == 2)
                    //num--;

                // send press event.
                num = num * 2;
                PostEvent(&keyRec[num]);
                usleep(keyRec[num].time * TIME_UNIT / speedup);
                printf("\033[1;31m[dfbkeyrecord][key_test] key_item = %d, key_symbol=0x%X, key_type=%d, key_code=%d, delay time=%ld\033[0m\n",
                    num, keyRec[num].symbol, keyRec[num].type, keyRec[num].code, (keyRec[num].time / speedup) );

                // send release event.
                num++;
                PostEvent(&keyRec[num]);
                usleep(keyRec[num].time * TIME_UNIT / speedup);
                printf("\033[1;31m[dfbkeyrecord][key_test] key_item = %d, key_symbol=0x%X, key_type=%d, key_code=%d, delay time=%ld\033[0m\n",
                    num, keyRec[num].symbol, keyRec[num].type, keyRec[num].code, (keyRec[num].time / speedup) );
            }
            else { // in order key testing.
                PostEvent(&keyRec[num]);
                usleep(keyRec[num].time * TIME_UNIT / speedup);
                printf("\033[1;31m[dfbkeyrecord][key_test] key_item = %d, key_symbol=0x%X, key_type=%d, key_code=%d, delay time=%ld\033[0m\n",
                    num, keyRec[num].symbol, keyRec[num].type, keyRec[num].code, (keyRec[num].time / speedup) );
            }
        }

        loop_cnt++;
        printf("\033[1;31m[dfbkeyrecord][key_test] loop count = %d!\033[0m\n", loop_cnt);

        sleep(2);
        if (repeat > 0) {
            repeat--;
            if (repeat == 0)
                break;
        }

    }
    printf("\033[1;31m[dfbkeyrecord][key_test] key test done!\033[0m\n");

    free(keyRec);
}

void key_record(char *rec_file)
{
    DFBInputEvent evt;
    DFBInputDeviceKeySymbol  last_symbol = DIKS_NULL;

    FILE *pf = NULL;
    KeyRecord keyRec[REC_SIZE] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0}};
    struct timeval tv = {0,0}, start = {0,0};
    int i = REC_LAST_KEY;

    printf("\033[1;31m[dfbkeyrecord][key_record] star to recode key operation!\033[0m\n");

    dfb->EnumInputDevices( dfb, input_device_callback, NULL );
    DFBCHECK(dfb->CreateInputEventBuffer( dfb, DICAPS_ALL, DFB_TRUE, &keybuffer ));

    while(1)
    {
        while (keybuffer->GetEvent( keybuffer, DFB_EVENT(&evt)) == DFB_OK)
        {
            gettimeofday( &tv, NULL );
            keyRec[i].symbol = evt.key_symbol;
            keyRec[i].type = evt.type;
            keyRec[i].code = evt.key_code;
            keyRec[i].key_id = evt.key_id;
            keyRec[i].flags = evt.flags;
            keyRec[i].button = evt.button;
            keyRec[i].axis = evt.axis;
            keyRec[i].axisabs = evt.axisabs;
            keyRec[i].axisrel = evt.axisrel;
            keyRec[i].device_id = evt.device_id;

            if (i == REC_CURR_KEY)
            {
                keyRec[i].time = (tv.tv_sec - start.tv_sec)*TIME_UNIT + (tv.tv_usec - start.tv_usec)/TIME_UNIT + 1;

                pf = fopen(rec_file, "a");
                fprintf(pf, "0x%X, %d, %d, %d, %d, %d, %d, %d, %d, %d, %ld\n",
                    keyRec[REC_LAST_KEY].symbol,
                    keyRec[REC_LAST_KEY].type,
                    keyRec[REC_LAST_KEY].code,
                    keyRec[REC_LAST_KEY].key_id,
                    keyRec[REC_LAST_KEY].flags,
                    keyRec[REC_LAST_KEY].button,
                    keyRec[REC_LAST_KEY].axis,
                    keyRec[REC_LAST_KEY].axisabs,
                    keyRec[REC_LAST_KEY].axisrel,
                    keyRec[REC_LAST_KEY].device_id,
                    keyRec[i].time);
                printf("\033[1;31m[dfbkeyrecord][key_record] Record key_symbol=0x%X, key_type=%d, key_code=%d, delay_time=%ld\033[0m\n", keyRec[REC_LAST_KEY].symbol, keyRec[REC_LAST_KEY].type, keyRec[REC_LAST_KEY].code, keyRec[i].time);
                fclose(pf);

                keyRec[REC_LAST_KEY].symbol = keyRec[REC_CURR_KEY].symbol;
                keyRec[REC_LAST_KEY].type     = keyRec[REC_CURR_KEY].type;
                keyRec[REC_LAST_KEY].code    = keyRec[REC_CURR_KEY].code;
                keyRec[REC_LAST_KEY].key_id    = keyRec[REC_CURR_KEY].key_id;
                keyRec[REC_LAST_KEY].flags    = keyRec[REC_CURR_KEY].flags;
                keyRec[REC_LAST_KEY].button    = keyRec[REC_CURR_KEY].button;
                keyRec[REC_LAST_KEY].axis    = keyRec[REC_CURR_KEY].axis;
                keyRec[REC_LAST_KEY].axisabs    = keyRec[REC_CURR_KEY].axisabs;
                keyRec[REC_LAST_KEY].axisrel    = keyRec[REC_CURR_KEY].axisrel;
                keyRec[REC_LAST_KEY].device_id = keyRec[REC_CURR_KEY].device_id;
            }
            else
                i = REC_CURR_KEY;

            start.tv_sec   = tv.tv_sec;
            start.tv_usec = tv.tv_usec;
        }

        if (evt.type == DIET_KEYRELEASE) {
            if (last_symbol == DIKS_RED && keyRec[1].symbol == DIKS_BLUE)
            {
                printf("[dfbkeyrecord][key_record]  %s, %d\n", __FUNCTION__, __LINE__);
                break;
            }
            last_symbol = evt.key_symbol;
        }

        keybuffer->WaitForEvent(keybuffer);
    }

    keybuffer->Release( keybuffer );

    printf("\033[1;31m[dfbkeyrecord][key_record]  key record done!\033[0m\n");
}


static void
print_usage (const char *prg_name)
{
     fprintf (stderr, "Usage: %s [options] loop count/filename/...\n\n", prg_name);
     fprintf (stderr, "If not exist the file \"keyrecord.ini\", record any input key event into \"keyrecord.ini\ which is created after program running. \n");
     fprintf (stderr, "If exists the file \"keyrecord.ini\", plays that file repeatedly or loops with \"n\" times .\n");
     fprintf (stderr, "\nPress \"RED\" button plus \"BLUE\" button to stop recording.\n");
     fprintf (stderr, "Options:\n");
     fprintf (stderr, "   -h, --help                                          Show this help message.\n");
     fprintf (stderr, "   -n loop_cnt,                                        Loop play with the \"loop_cnt\" times.\n");
     fprintf (stderr, "   -f filename, --file filename                        Specify the recorded file which you want to play.\n");
     fprintf (stderr, "   -r, --random                                        random to send key record from file.\n");
     fprintf (stderr, "   --gen_rand_key_test                                    generate and random send key.\n");
     fprintf (stderr, "   -s x, --speedup x                                   speed up key testing by 'x' time\n");
     fprintf (stderr, "\n");
}


bool parse_command_line( int argc, char *argv[] )
{

    if(argc >1)
    {
        int n = 0;
        E_COMMAND_TYPE cmd;

        RESET_CMD(cmd);

         for (n = 1; n < argc; n++) {
                    char *arg = argv[n];

                    if (strcmp (arg, "-f") == 0 || strcmp (arg, "--file") == 0) {

                       cmd = CMD_FILENAME;
                       continue;
                    }

                    if (strcmp (arg, "-n") == 0) {

                       cmd = CMD_LOOP_COUNT;
                       continue;
                    }

                    if (strcmp (arg, "-h") == 0 || strcmp (arg, "--help") == 0) {
                       print_usage (argv[0]);
                       return false;
                    }

                    if (strcmp (arg, "-r") == 0 || strcmp (arg, "--random") == 0) {
                        random_test = true;
                        continue;
                    }

                    if (strcmp (arg, "--gen_rand_key_test") == 0) {
                        rand_key_test = true;
                        continue;
                    }

                    if (strcmp (arg, "-s") == 0 || strcmp (arg, "--speedup") == 0) {
                        cmd = CMD_SPEEDUP;
                        continue;
                    }

                    switch(cmd)
                    {
                        case CMD_FILENAME:
                        if(rec_file){
                            free(rec_file);
                        }
                        rec_file = strdup(arg);
                        RESET_CMD(cmd);
                        break;

                        case CMD_LOOP_COUNT:
                        repeat = strtol(arg, NULL, 10);
                        RESET_CMD(cmd);
                        break;

                        case CMD_SPEEDUP:
                        speedup = strtol(arg, NULL, 10);
                        break;

                        case CMD_UNKNOWN:
                        default:
                          break;
                    }

            }

    }

    return true;


}

int
main( int argc, char *argv[] )
{
    FILE * pf = NULL;

    rec_file = strdup ("keyrecord.ini");

    if(!parse_command_line( argc, argv ))
        return 0;

    DirectFBInit( &argc, &argv );
    DirectFBCreate( &dfb );

    if (rand_key_test) {
        key_random_test();
    }
    else {
        pf = fopen(rec_file, "r");

        if (pf) { // file exist.
            printf("\033[1;31m[dfbkeyrecord][key_record] file : %s exist!\033[0m\n", rec_file);
            key_test(pf);
            fclose(pf);
        }
        else { // file not exist, start record mode.
            printf("\033[1;31m[dfbkeyrecord][key_record] file : %s is not exist!\033[0m\n", rec_file);
            key_record(rec_file);
        }
    }

    dfb->Release( dfb );
    return 0;
}

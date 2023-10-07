/*----------------------------------------------------------------------------*
 * Copyright Statement:                                                       *
 *                                                                            *
 *   This software/firmware and related documentation ("MediaTek Software")   *
 * are protected under international and related jurisdictions'copyright laws *
 * as unpublished works. The information contained herein is confidential and *
 * proprietary to MediaTek Inc. Without the prior written permission of       *
 * MediaTek Inc., any reproduction, modification, use or disclosure of        *
 * MediaTek Software, and information contained herein, in whole or in part,  *
 * shall be strictly prohibited.                                              *
 * MediaTek Inc. Copyright (C) 2010. All rights reserved.                     *
 *                                                                            *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND     *
 * AGREES TO THE FOLLOWING:                                                   *
 *                                                                            *
 *   1)Any and all intellectual property rights (including without            *
 * limitation, patent, copyright, and trade secrets) in and to this           *
 * Software/firmware and related documentation ("MediaTek Software") shall    *
 * remain the exclusive property of MediaTek Inc. Any and all intellectual    *
 * property rights (including without limitation, patent, copyright, and      *
 * trade secrets) in and to any modifications and derivatives to MediaTek     *
 * Software, whoever made, shall also remain the exclusive property of        *
 * MediaTek Inc.  Nothing herein shall be construed as any transfer of any    *
 * title to any intellectual property right in MediaTek Software to Receiver. *
 *                                                                            *
 *   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its *
 * representatives is provided to Receiver on an "AS IS" basis only.          *
 * MediaTek Inc. expressly disclaims all warranties, expressed or implied,    *
 * including but not limited to any implied warranties of merchantability,    *
 * non-infringement and fitness for a particular purpose and any warranties   *
 * arising out of course of performance, course of dealing or usage of trade. *
 * MediaTek Inc. does not provide any warranty whatsoever with respect to the *
 * software of any third party which may be used by, incorporated in, or      *
 * supplied with the MediaTek Software, and Receiver agrees to look only to   *
 * such third parties for any warranty claim relating thereto.  Receiver      *
 * expressly acknowledges that it is Receiver's sole responsibility to obtain *
 * from any third party all proper licenses contained in or delivered with    *
 * MediaTek Software.  MediaTek is not responsible for any MediaTek Software  *
 * releases made to Receiver's specifications or to conform to a particular   *
 * standard or open forum.                                                    *
 *                                                                            *
 *   3)Receiver further acknowledge that Receiver may, either presently       *
 * and/or in the future, instruct MediaTek Inc. to assist it in the           *
 * development and the implementation, in accordance with Receiver's designs, *
 * of certain softwares relating to Receiver's product(s) (the "Services").   *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MediaTek Inc. with respect  *
 * to the Services provided, and the Services are provided on an "AS IS"      *
 * basis. Receiver further acknowledges that the Services may contain errors  *
 * that testing is important and it is solely responsible for fully testing   *
 * the Services and/or derivatives thereof before they are used, sublicensed  *
 * or distributed. Should there be any third party action brought against     *
 * MediaTek Inc. arising out of or relating to the Services, Receiver agree   *
 * to fully indemnify and hold MediaTek Inc. harmless.  If the parties        *
 * mutually agree to enter into or continue a business relationship or other  *
 * arrangement, the terms and conditions set forth herein shall remain        *
 * effective and, unless explicitly stated otherwise, shall prevail in the    *
 * event of a conflict in the terms in any agreements entered into between    *
 * the parties.                                                               *
 *                                                                            *
 *   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and    *
 * cumulative liability with respect to MediaTek Software released hereunder  *
 * will be, at MediaTek Inc.'s sole discretion, to replace or revise the      *
 * MediaTek Software at issue.                                                *
 *                                                                            *
 *   5)The transaction contemplated hereunder shall be construed in           *
 * accordance with the laws of Singapore, excluding its conflict of laws      *
 * principles.  Any disputes, controversies or claims arising thereof and     *
 * related thereto shall be settled via arbitration in Singapore, under the   *
 * then current rules of the International Chamber of Commerce (ICC).  The    *
 * arbitration shall be conducted in English. The awards of the arbitration   *
 * shall be final and binding upon both parties and shall be entered and      *
 * enforceable in any court of competent jurisdiction.                        *
 *---------------------------------------------------------------------------*/
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#include <direct/util.h>

#include <divine.h>


typedef struct
{
    const char              *name;
    DFBInputDeviceKeySymbol  symbol;
} ConConSymbol;

static const ConConSymbol symbols[] =
{   {"ESCAPE",       DIKS_ESCAPE},       {"EXIT",         DIKS_EXIT},
    {"POWER",        DIKS_POWER},
    {"OK",           DIKS_OK},           {"ENTER",        DIKS_ENTER},

    {"UP",           DIKS_CURSOR_UP},    {"DOWN",         DIKS_CURSOR_DOWN},
    {"LEFT",         DIKS_CURSOR_LEFT},  {"RIGHT",        DIKS_CURSOR_RIGHT},

    {"CHANNEL_UP",   DIKS_CHANNEL_UP},   {"CHANNEL_DOWN", DIKS_CHANNEL_DOWN},

    {"VOLUME_UP",    DIKS_VOLUME_UP},    {"VOLUME_DOWN",  DIKS_VOLUME_DOWN},
    {"MUTE",         DIKS_MUTE},

    {"PRINT",        DIKS_PRINT},        {"SCREENSHOT",   DIKS_PRINT},

    {"PAGE_UP",      DIKS_PAGE_UP},      {"PAGE_DOWN",    DIKS_PAGE_DOWN},

    {"BACK",         DIKS_BACK},         {"MENU",         DIKS_MENU},

    {"TV",           DIKS_TV},           {"F5",           DIKS_F5},

    {"RADIO",        DIKS_RADIO},        {"F6",           DIKS_F6},

    {"TEXT",         DIKS_TEXT},         {"F7",           DIKS_F7},

    {"MHP",          DIKS_MHP},          {"F8",           DIKS_F8},

    {"EPG",          DIKS_EPG},          {"F9",           DIKS_F9},

    {"CHANNEL",      DIKS_CHANNEL},      {"LIST",         DIKS_LIST},
    {"F10",          DIKS_F10},

    {"HELP",         DIKS_HELP},         {"INFO",         DIKS_INFO},
    {"F11",          DIKS_F11},

    {"SUBTITLE",     DIKS_SUBTITLE},     {"F12",          DIKS_F12},

    {"SETUP",        DIKS_SETUP},        {"LANGUAGE",     DIKS_LANGUAGE},

    {"PLAYER",       DIKS_PLAYER},       {"TUNER",        DIKS_TUNER},
    {"SCREEN",       DIKS_SCREEN},       {"INTERNET",     DIKS_INTERNET},
    {"MAIL",         DIKS_MAIL},

    {"PLAYPAUSE",    DIKS_PLAYPAUSE},    {"PLAY",         DIKS_PLAY},
    {"PAUSE",        DIKS_PAUSE},        {"STOP",         DIKS_STOP},
    {"REWIND",       DIKS_REWIND},       {"FASTFORWARD",  DIKS_FASTFORWARD},
    {"NEXT",         DIKS_NEXT},         {"PREVIOUS",     DIKS_PREVIOUS},
    {"RECORD",       DIKS_RECORD},       {"EJECT",        DIKS_EJECT},

    {"RED",          DIKS_RED},          {"F1",           DIKS_F1},
    {"GREEN",        DIKS_GREEN},        {"F2",           DIKS_F2},
    {"YELLOW",       DIKS_YELLOW},       {"F3",           DIKS_F3},
    {"BLUE",         DIKS_BLUE},         {"F4",           DIKS_F4},

    {"CUSTOM0",      DIKS_CUSTOM0},      {"CUSTOM1",      DIKS_CUSTOM1},
    {"CUSTOM2",      DIKS_CUSTOM2},      {"CUSTOM3",      DIKS_CUSTOM3},
    {"CUSTOM4",      DIKS_CUSTOM4},      {"CUSTOM5",      DIKS_CUSTOM5},
    {"CUSTOM6",      DIKS_CUSTOM6},      {"CUSTOM7",      DIKS_CUSTOM7},
    {"CUSTOM8",      DIKS_CUSTOM8},      {"CUSTOM9",      DIKS_CUSTOM9},

};

static void
print_help()
{
    fprintf( stderr, "Console Control\n" );
    fprintf( stderr, " Usage: concon [-lsc | key-event]\n" );
    fprintf( stderr, "Options:\n");
    fprintf( stderr, "        key-event        normal or special character\n");
    fprintf( stderr, "        -l               list special characters\n");
    fprintf( stderr, "        -s               enter strings and special char.\n");
    fprintf( stderr, "        -c               enter characters and escape seq.\n");
}

static DFBInputDeviceKeySymbol
map_inputkey( const char *name )
{
    int i;

    for (i = 0; i < D_ARRAY_SIZE(symbols); i++)
    {
        if (strcmp( symbols[i].name, name ) == 0)
            return symbols[i].symbol;
    }

    return DIKS_NULL;
}

static int
term_set_raw( int fd )
{
    struct termios termios_p;

    if (tcgetattr (fd, &termios_p))
        return(-1);

    termios_p.c_cc[VMIN]  =  1;
    termios_p.c_cc[VTIME] =  0;
    termios_p.c_lflag &=  ~(ECHO | ICANON | ISIG | ECHOE| ECHOK  | ECHONL);

    return(tcsetattr (fd, TCSADRAIN, &termios_p));
}

static int
term_set_normal( int fd )
{
    struct termios termios_p;

    if (tcgetattr (fd, &termios_p))
        return(-1);

    termios_p.c_lflag |= (ECHO | ICANON | ISIG | ECHOE| ECHOK | ECHONL);

    return(tcsetattr (fd, TCSADRAIN, &termios_p));
}

static int
getchar_timeout()
{
    int            fd;
    fd_set         fds;
    struct timeval tv;

    fd = fileno (stdin);

    FD_ZERO (&fds);
    FD_SET (fd, &fds);

    tv.tv_sec  = 0;
    tv.tv_usec = 100000;

    switch (select (fd + 1, &fds, NULL, NULL, &tv))
    {
        case -1: /* error */
            perror ("select()");
            return 0;

        case 0: /* timeout */
            return 0;
    }

    return getchar();
}

static void
concon_run_fc( DiVine *divine )
{
    unsigned int ch = 0;
    char         line[6];

    term_set_raw (0);

    setbuf (stdin, NULL);

    fprintf (stdout, "\n -= Console Control =- \n");
    fprintf (stdout, " press CTRL-D for quit \n");

    while (ch != 4)
    {
        int size;

        ch = getchar ();

        if (ch == 27)
        {
            int ch2, ch3, ch4;

            ch2 = getchar_timeout ();

            if (ch2 == '[' || ch2 == 'O')
            {
                ch3 = getchar ();

                if (ch3 == '2' || ch3 == '1')
                {
                    ch4 = getchar ();

                    if (ch4 != '~')
                        getchar ();

                    sprintf (line, "%c%c%c%c", ch, ch2, ch3, ch4);
                    size = 4;
                }
                else
                {
                    sprintf (line, "%c%c%c", ch, ch2, ch3);
                    size = 3;

                    if (ch3 >= '3' && ch3 <= '8')
                        getchar ();

                    if (ch3 >= 'O' || ch3 == 'F') size = 2;
                }
            }
            else if (ch2)
            {
                sprintf (line, "%c%c", ch, ch2);
                size = 2;
            }
            else
            {
                sprintf (line, "%c", ch);
                size = 1;
            }

            fprintf (stdout, "- send key: %s\n", line + 1);
            divine_send_vt102 (divine, size, line);
        }
        else
        {
            sprintf (line, "%c", ch);
            fprintf (stdout, "- send key: %s\n", line);

            if (ch != 4)
                divine_send_vt102 (divine, 1, line);
        }

        if (ch == 4)
            fprintf (stdout, "* logout \n\n");
    }

    term_set_normal (0);
}

static void
concon_run_fs( DiVine *divine )
{
    DFBInputDeviceKeySymbol keysymbol;
    char                    inputstring[20];
    int                     i;

    fprintf (stdout, "\n -= Console Controle =-\n");
    fprintf (stdout, " enter 'list' for special char. list\n");
    fprintf (stdout, " enter 'quit' for quit\n");

    while (true)
    {
        inputstring[0] = '\0';
        fgets (inputstring, 20, stdin);

        inputstring [strlen(inputstring) -1] = '\0';

        if (strcmp (inputstring, "quit") == 0)
        {
            fprintf (stdout, "* logout\n");
            return;
        }
        if (strcmp (inputstring, "list") == 0)
        {
            for (i = 0; i < D_ARRAY_SIZE(symbols); i++)
                fprintf (stdout, "%s\n", symbols[i].name);
        }
        else
        {
            keysymbol = map_inputkey (inputstring);
            divine_send_symbol (divine, keysymbol);
            fprintf (stdout, "- key send\n");
        }
    }
}


int
main( int argc, char *argv[] )
{
    int     i;
    DiVine *divine;

    /* open the connection to the input driver */
    divine = divine_open ("/tmp/divine");
    if (!divine)
        return EXIT_FAILURE;

    if (argc == 2)
    {
        if (argv[1][0] == '-' && strlen (argv[1]) >= 2)
        {
            switch (argv[1][1])
            {
                case 'l':
                    fprintf (stdout, "available special characters:\n");
                    for (i = 0; i < D_ARRAY_SIZE(symbols); i++)
                        fprintf (stdout,"%s\n", symbols[i].name);
                    break;
                case 's':
                    concon_run_fs (divine);
                    break;
                case 'h':
                    print_help ();
                    break;
                case 'c':
                    concon_run_fc (divine);
                    break;
                default:
                    print_help ();
                    break;
            }
        }
        else
        {
            divine_send_symbol (divine, map_inputkey( argv[1]));
            fprintf (stdout, "concon: key send.\n");
        }
    }
    else
    {
        concon_run_fc (divine);
    }

    divine_close (divine);

    return EXIT_SUCCESS;
}


/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <divine.h>

#include <direct/debug.h>
#include <direct/interface.h>
#include <direct/messages.h>

#include <idivine.h>

#include <voodoo/interface.h>
#include <voodoo/manager.h>
#include <voodoo/message.h>

#include "idivine_dispatcher.h"

static DFBResult Probe();
static DFBResult Construct( IDiVine          *thiz,
                            VoodooManager    *manager,
                            VoodooInstanceID *ret_instance );

#include <direct/interface_implementation.h>

DIRECT_INTERFACE_IMPLEMENTATION( IDiVine, Dispatcher )


/**************************************************************************************************/

/*
 * private data struct of IDiVine_Dispatcher
 */
typedef struct {
     int                    ref;          /* reference counter */

     IDiVine               *real;

     VoodooInstanceID       self;         /* The instance of this dispatcher itself. */
} IDiVine_Dispatcher_data;

/**************************************************************************************************/

static void
IDiVine_Dispatcher_Destruct( IDiVine *thiz )
{
     D_DEBUG( "%s (%p)\n", __FUNCTION__, thiz );

     DIRECT_DEALLOCATE_INTERFACE( thiz );
}

/**************************************************************************************************/

static DirectResult
IDiVine_Dispatcher_AddRef( IDiVine *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDiVine_Dispatcher)

     data->ref++;

     return DFB_OK;
}

static DirectResult
IDiVine_Dispatcher_Release( IDiVine *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDiVine_Dispatcher)

     if (--data->ref == 0)
          IDiVine_Dispatcher_Destruct( thiz );

     return DFB_OK;
}

static DFBResult
IDiVine_Dispatcher_SendEvent( IDiVine             *thiz,
                              const DFBInputEvent *event )
{
     DIRECT_INTERFACE_GET_DATA(IDiVine_Dispatcher)

     return DFB_UNIMPLEMENTED;
}

/**************************************************************************************************/

// hardware keycode for virtual remote is 0x3000000

#define VIRTUAL_DIGIT(digit) (0x3000000 | digit)

static DFBInputDeviceKeySymbol
translate_key_symbol(DFBInputDeviceKeySymbol input_symbol, int input_code)
{
     switch(input_symbol) {
		  case DIKS_PERIOD        : return DFB_BTN_DIGIT_DOT; // Dot key		
		  case DIKS_0             : return (input_code == VIRTUAL_DIGIT(0)) ?  DFB_BTN_DIGIT_0: DIKS_0;
          case DIKS_1             : return (input_code == VIRTUAL_DIGIT(1)) ?  DFB_BTN_DIGIT_1: DIKS_1;
          case DIKS_2             : return (input_code == VIRTUAL_DIGIT(2)) ?  DFB_BTN_DIGIT_2: DIKS_2;
          case DIKS_3             : return (input_code == VIRTUAL_DIGIT(3)) ?  DFB_BTN_DIGIT_3: DIKS_3;
          case DIKS_4             : return (input_code == VIRTUAL_DIGIT(4)) ?  DFB_BTN_DIGIT_4: DIKS_4;
          case DIKS_5             : return (input_code == VIRTUAL_DIGIT(5)) ?  DFB_BTN_DIGIT_5: DIKS_5;
          case DIKS_6             : return (input_code == VIRTUAL_DIGIT(6)) ?  DFB_BTN_DIGIT_6: DIKS_6;
          case DIKS_7             : return (input_code == VIRTUAL_DIGIT(7)) ?  DFB_BTN_DIGIT_7: DIKS_7;
          case DIKS_8             : return (input_code == VIRTUAL_DIGIT(8)) ?  DFB_BTN_DIGIT_8: DIKS_8;
          case DIKS_9             : return (input_code == VIRTUAL_DIGIT(9)) ?  DFB_BTN_DIGIT_9: DIKS_9;
          case DIKS_CURSOR_LEFT         : return DFB_BTN_CURSOR_LEFT;     
          case DIKS_CURSOR_RIGHT        : return DFB_BTN_CURSOR_RIGHT;
          case DIKS_CURSOR_UP           : return DFB_BTN_CURSOR_UP;
          case DIKS_CURSOR_DOWN         : return DFB_BTN_CURSOR_DOWN;
//cl>		  
		  case DIKS_PAGE_DOWN		: return DFB_BTN_NEXT;
		  case DIKS_PAGE_UP			: return DFB_BTN_PREV;
//cl<		  
          case DIKS_OK                  : 
		  case DIKS_RETURN              :					//cl-131205: Change for keyboard autoshow
          case DIKS_SELECT              : return DFB_BTN_SELECT;
          case DIKS_EXIT                : return DFB_BTN_EXIT;
          case DIKS_CLEAR               : return DFB_BTN_CLEAR;
//cl-131205          case DIKS_RETURN              : return DFB_BTN_RETURN;
          case DIKS_CHANNEL_UP          : return DFB_BTN_PRG_UP;     
          case DIKS_CHANNEL_DOWN        : return DFB_BTN_PRG_DOWN;
          case DIKS_ZOOM                : return DFB_BTN_ASPECT;
		  case DIKS_MODE				: return DFB_BTN_PIP_POP;	//multi view
          case DIKS_INFO                : return DFB_BTN_PRG_INFO;         
          case DIKS_TUNER               :
          case DIKS_TV                  : return DFB_BTN_TV;
          case DIKS_DVD                 : return DFB_BTN_DVD;
          case DIKS_VOLUME_UP           : return DFB_BTN_VOL_UP;
          case DIKS_VOLUME_DOWN         : return DFB_BTN_VOL_DOWN;
          case DIKS_MUTE                : return DFB_BTN_MUTE;
          case DIKS_AUDIO               : return DFB_BTN_AUDIO;
          case DIKS_POWER               : return DFB_BTN_POWER;
          case DIKS_MENU                : return DFB_BTN_MENU;
          case DIKS_OPTION              : return DFB_BTN_CUSTOM_6;  // Option Menu
          case DIKS_EPG                 : return DFB_BTN_EPG;
		  case DIKS_PLAYPAUSE			: if (VIRTUAL_DIGIT(0x2C) == input_code) return DFB_BTN_PLAY;
                                          else if(VIRTUAL_DIGIT(0x30) == input_code) return DFB_BTN_PAUASE;
                                          else return DFB_BTN_PLAY_PAUSE;
          case DIKS_PLAY                : return DFB_BTN_PLAY;
          case DIKS_PAUSE               : return DFB_BTN_PAUASE;
          case DIKS_STOP                : return DFB_BTN_STOP;
		  case DIKS_REWIND              : return DFB_BTN_FR;
          case DIKS_FASTFORWARD         : return DFB_BTN_FF;
          case DIKS_RECORD              : return DFB_BTN_RECORD;
          case DIKS_NEXT                : return DFB_BTN_INPUT_SRC; //DFB_BTN_NEXT;
          case DIKS_PREVIOUS            : return DFB_BTN_RETURN;
          case DIKS_EJECT               : return DFB_BTN_EJECT;
          case DIKS_TITLE               : return DFB_BTN_TITLE_MENU;
		  case DIKS_GOTO                : return DFB_BTN_GOTO;
          case DIKS_PROGRAM             : return DFB_BTN_PROGRAM;
          case DIKS_SUBTITLE            : return DFB_BTN_SUB_TITLE;
          case DIKS_ANGLE               : return DFB_BTN_ANGLE;
          case DIKS_RED                 : return DFB_BTN_RED;
          case DIKS_GREEN               : return DFB_BTN_GREEN;
          case DIKS_YELLOW              : return DFB_BTN_YELLOW;
          case DIKS_BLUE                : return DFB_BTN_BLUE; 
		  case DIKS_TEXT				: return DFB_BTN_TTX; 
		  case DIKS_LIST				: return DFB_BTN_CUSTOM_32; // Channel List
          case DIKS_F1				: return DFB_BTN_HDMI_1; //return DFB_BTN_FUNCTION_1;         
          case DIKS_F2				: return DFB_BTN_HDMI_2;         
          case DIKS_F3				: return DFB_BTN_HDMI_3;         
          case DIKS_F4				: return DFB_BTN_HDMI;  
          case DIKS_F5				: return DFB_BTN_COMPONENT;         
          case DIKS_F6				: return DFB_BTN_COMPOSITE;
          case DIKS_F7				: return DFB_BTN_VGA;         
          case DIKS_F8              : return DFB_BTN_INP_SRC_1; // Browse USB      
          case DIKS_F9              : return DFB_BTN_FUNCTION_9;         
          case DIKS_F10             : return DFB_BTN_FUNCTION_10;        
          case DIKS_F11             : return BTN_3D; // 3D mode
          case DIKS_F12             : return DFB_BTN_INP_SRC_2; // Browse PC
          case DIKS_CUSTOM1               : return DFB_BTN_CUSTOM_1;           
          case DIKS_CUSTOM2               : return DFB_BTN_CUSTOM_2;           
          case DIKS_CUSTOM3               : return DFB_BTN_CUSTOM_3;           
          case DIKS_CUSTOM4               : return DFB_BTN_CUSTOM_4;           
          case DIKS_CUSTOM5               : return DFB_BTN_CUSTOM_5;           
          case DIKS_CUSTOM6               : return DFB_BTN_CUSTOM_6;           
          case DIKS_CUSTOM7               : return DFB_BTN_CUSTOM_7;           
          case DIKS_CUSTOM8               : return DFB_BTN_CUSTOM_8;           
          case DIKS_CUSTOM9               : return DFB_BTN_CUSTOM_9;           
          case DIKS_CUSTOM10              : return DFB_BTN_CUSTOM_10;          
          case DIKS_CUSTOM11              : return DFB_BTN_CUSTOM_11;          
          case DIKS_CUSTOM12              : return DFB_BTN_CUSTOM_12;          
          case DIKS_CUSTOM13              : return DFB_BTN_CUSTOM_13;          
          case DIKS_CUSTOM14              : return DFB_BTN_CUSTOM_14;          
          case DIKS_CUSTOM15              : return DFB_BTN_CUSTOM_15;          
          case DIKS_CUSTOM16              : return DFB_BTN_CUSTOM_16;          
          case DIKS_CUSTOM17              : return DFB_BTN_CUSTOM_17;          
          case DIKS_CUSTOM18              : return DFB_BTN_CUSTOM_18;          
          case DIKS_CUSTOM19              : return DFB_BTN_CUSTOM_81; // Smart TV      
          case DIKS_CUSTOM20              : return DFB_BTN_CUSTOM_20;          
          case DIKS_CUSTOM21              : return DFB_BTN_CUSTOM_21;          
          case DIKS_CUSTOM22              : return DFB_BTN_CUSTOM_22;          
          case DIKS_CUSTOM23              : return DFB_BTN_CUSTOM_23;          
          case DIKS_CUSTOM24              : return DFB_BTN_CUSTOM_24;          
          case DIKS_CUSTOM25              : return DFB_BTN_CUSTOM_25;          
          case DIKS_CUSTOM26              : return DFB_BTN_CUSTOM_26;          
          case DIKS_CUSTOM27              : return DFB_BTN_CUSTOM_27;          
          case DIKS_CUSTOM28              : return DFB_BTN_CUSTOM_28;          
          case DIKS_CUSTOM29              : return DFB_BTN_CUSTOM_29;          
          case DIKS_CUSTOM30              : return DFB_BTN_CUSTOM_30;          
          case DIKS_CUSTOM31              : return DFB_BTN_SUB_TITLE;   // Subtitle      
          case DIKS_CUSTOM32              : return DFB_BTN_CUSTOM_32;      
          case DIKS_CUSTOM33              : return DFB_BTN_CUSTOM_8;    // Ambilight      
          case DIKS_CUSTOM34              : return DFB_BTN_PAUASE; // Pause        
          case DIKS_CUSTOM35              : return DFB_BTN_PREV;  // Prev
          case DIKS_CUSTOM36              : return DFB_BTN_NEXT;  // Next   
          case DIKS_CUSTOM37              : return DFB_BTN_CUSTOM_37;          
          case DIKS_CUSTOM38              : return DFB_BTN_CUSTOM_38;          
          case DIKS_CUSTOM39              : return DFB_BTN_CUSTOM_39;          
          case DIKS_CUSTOM40              : return DFB_BTN_CUSTOM_40;         
          case DIKS_CUSTOM41              : return DFB_BTN_CUSTOM_41;          
          case DIKS_CUSTOM42              : return DFB_BTN_CUSTOM_42;          
          case DIKS_CUSTOM43              : return DFB_BTN_CUSTOM_43;          
          case DIKS_CUSTOM44              : return DFB_BTN_CUSTOM_44;          
          case DIKS_CUSTOM45              : return DFB_BTN_CUSTOM_45;          
          case DIKS_CUSTOM46              : return DFB_BTN_CUSTOM_46;         
          case DIKS_CUSTOM47              : return DFB_BTN_CUSTOM_47;          
          case DIKS_CUSTOM48              : return DFB_BTN_CUSTOM_48;          
          case DIKS_CUSTOM49              : return DFB_BTN_CUSTOM_49;          
          case DIKS_CUSTOM50              : return DFB_BTN_CUSTOM_50;          
          case DIKS_CUSTOM51              : return DFB_BTN_CUSTOM_51;          
          case DIKS_CUSTOM52              : return DFB_BTN_CUSTOM_52;          
          case DIKS_CUSTOM53              : return DFB_BTN_CUSTOM_53;          
          case DIKS_CUSTOM54              : return DFB_BTN_CUSTOM_54;          
          case DIKS_CUSTOM55              : return DFB_BTN_CUSTOM_55;          
          case DIKS_CUSTOM56              : return DFB_BTN_CUSTOM_56;          
          case DIKS_CUSTOM57              : return DFB_BTN_CUSTOM_57;          
          case DIKS_CUSTOM58              : return DFB_BTN_CUSTOM_58;          
          case DIKS_CUSTOM59              : return DFB_BTN_CUSTOM_59;          
          case DIKS_CUSTOM60              : return DFB_BTN_CUSTOM_60;          
          case DIKS_CUSTOM61              : return DFB_BTN_CUSTOM_61;          
          case DIKS_CUSTOM62              : return DFB_BTN_CUSTOM_62;          
          case DIKS_CUSTOM63              : return DFB_BTN_CUSTOM_63;          
          case DIKS_CUSTOM64              : return DFB_BTN_CUSTOM_64;          
          case DIKS_CUSTOM65              : return DFB_BTN_CUSTOM_65;          
          case DIKS_CUSTOM66              : return DFB_BTN_CUSTOM_66;          
          case DIKS_CUSTOM67              : return DFB_BTN_CUSTOM_67;          
          case DIKS_CUSTOM68              : return DFB_BTN_CUSTOM_68;          
          case DIKS_CUSTOM69              : return DFB_BTN_CUSTOM_69;          
          case DIKS_CUSTOM70              : return DFB_BTN_CUSTOM_70;          
          case DIKS_CUSTOM71              : return DFB_BTN_CUSTOM_71;          
          case DIKS_CUSTOM72              : return DFB_BTN_CUSTOM_72;          
          case DIKS_CUSTOM73              : return DFB_BTN_CUSTOM_73;          
          case DIKS_CUSTOM74              : return DFB_BTN_CUSTOM_74;          
          case DIKS_CUSTOM75              : return DFB_BTN_CUSTOM_75;          
          case DIKS_CUSTOM76              : return DFB_BTN_CUSTOM_76;          
          case DIKS_CUSTOM77              : return DFB_BTN_CUSTOM_77;          
          case DIKS_CUSTOM78              : return DFB_BTN_CUSTOM_78;          
          case DIKS_CUSTOM79              : return DFB_BTN_CUSTOM_79;         
          case DIKS_CUSTOM80              : return DFB_BTN_CUSTOM_80;          
          case DIKS_CUSTOM81              : return DFB_BTN_CUSTOM_81;          
          case DIKS_CUSTOM82              : return DFB_BTN_CUSTOM_82;          
          case DIKS_CUSTOM83              : return DFB_BTN_CUSTOM_83;          
          case DIKS_CUSTOM84              : return DFB_BTN_CUSTOM_84;          
          case DIKS_CUSTOM85              : return DFB_BTN_CUSTOM_85;          
          case DIKS_CUSTOM86              : return DFB_BTN_CUSTOM_86;          
          case DIKS_CUSTOM87              : return DFB_BTN_CUSTOM_87;          
          case DIKS_CUSTOM88              : return DFB_BTN_CUSTOM_88;          
          case DIKS_CUSTOM89              : return DFB_BTN_CUSTOM_89;          
          case DIKS_CUSTOM90              : return DFB_BTN_CUSTOM_90;          
          case DIKS_CUSTOM91              : return DFB_BTN_CUSTOM_91;         
          case DIKS_CUSTOM92              : return DFB_BTN_CUSTOM_92;          
          case DIKS_CUSTOM93              : return DFB_BTN_CUSTOM_93;          
          case DIKS_CUSTOM94              : return DFB_BTN_CUSTOM_94;          
          case DIKS_CUSTOM95              : return DFB_BTN_CUSTOM_95;         
          case DIKS_CUSTOM96              : return DFB_BTN_CUSTOM_96;          
          case DIKS_CUSTOM97              : return DFB_BTN_CUSTOM_97;          
          case DIKS_CUSTOM98              : return DFB_BTN_CUSTOM_98;          
          case DIKS_CUSTOM99              : return DFB_BTN_CUSTOM_99;          
          case DIKS_CUSTOM100             : return DFB_BTN_CUSTOM_100;         
          case DIKS_CUSTOM101             : return DFB_BTN_CUSTOM_101;         
          case DIKS_CUSTOM102             : return DFB_BTN_CUSTOM_102;        
          case DIKS_CUSTOM103             : return DFB_BTN_CUSTOM_103;         
          case DIKS_CUSTOM104             : return DFB_BTN_CUSTOM_104;         
          case DIKS_CUSTOM105             : return DFB_BTN_CUSTOM_105;         
          case DIKS_CUSTOM106             : return DFB_BTN_CUSTOM_106;         
          case DIKS_CUSTOM107             : return DFB_BTN_CUSTOM_107;         
          case DIKS_CUSTOM108             : return DFB_BTN_CUSTOM_108;         
          case DIKS_CUSTOM109             : return DFB_BTN_CUSTOM_109;         
          case DIKS_CUSTOM110             : return BTN_3D; // 3D mode
          case DIKS_CUSTOM111             : return DFB_BTN_CUSTOM_111;         
          case DIKS_CUSTOM112             : return DFB_BTN_CUSTOM_112;         
          case DIKS_CUSTOM113             : return DFB_BTN_CUSTOM_113;         
          case DIKS_CUSTOM114             : return DFB_BTN_CUSTOM_114;        
          case DIKS_CUSTOM115             : return DFB_BTN_CUSTOM_115;        
          case DIKS_CUSTOM116             : return DFB_BTN_CUSTOM_116;         
          case DIKS_CUSTOM117             : return DFB_BTN_CUSTOM_117;         
          case DIKS_CUSTOM118             : return DFB_BTN_CUSTOM_118;         
          case DIKS_CUSTOM119             : return DFB_BTN_CUSTOM_119;         
          case DIKS_CUSTOM120             : return DFB_BTN_CUSTOM_120;         
          case DIKS_CUSTOM121             : return DFB_BTN_CUSTOM_121;         
          case DIKS_CUSTOM122             : return DFB_BTN_CUSTOM_122;         
          case DIKS_CUSTOM123             : return DFB_BTN_CUSTOM_123;         
          case DIKS_CUSTOM124             : return DFB_BTN_CUSTOM_124;         
          case DIKS_CUSTOM125             : return DFB_BTN_CUSTOM_125;         
          case DIKS_CUSTOM126             : return DFB_BTN_CUSTOM_126;         
          case DIKS_CUSTOM127             : return DFB_BTN_CUSTOM_127;         
          case DIKS_CUSTOM128             : return BTN_CC;         
          case DIKS_CUSTOM129             : return DFB_BTN_CUSTOM_129;         
          case DIKS_CUSTOM130             : return DFB_BTN_CUSTOM_130;         
          case DIKS_CUSTOM131             : return DFB_BTN_CUSTOM_131;         
          case DIKS_CUSTOM132             : return DFB_BTN_CUSTOM_132;         
          case DIKS_CUSTOM133             : return DFB_BTN_CUSTOM_133;         
          case DIKS_CUSTOM134             : return DFB_BTN_CUSTOM_134;         
          case DIKS_CUSTOM135             : return DFB_BTN_CUSTOM_135;         
          case DIKS_CUSTOM136             : return DFB_BTN_CUSTOM_136;         
          case DIKS_CUSTOM137             : return DFB_BTN_CUSTOM_137;         
          case DIKS_CUSTOM138             : return DFB_BTN_CUSTOM_138;         
          case DIKS_CUSTOM139             : return DFB_BTN_CUSTOM_139;         
          case DIKS_CUSTOM140             : return DFB_BTN_CUSTOM_140;         
          case DIKS_CUSTOM141             : return DFB_BTN_CUSTOM_141;         
          case DIKS_CUSTOM142             : return DFB_BTN_CUSTOM_142;         
          case DIKS_CUSTOM143             : return DFB_BTN_CUSTOM_143;        
          case DIKS_CUSTOM144             : return DFB_BTN_CUSTOM_144;         
          case DIKS_CUSTOM145             : return DFB_BTN_CUSTOM_145;         
          case DIKS_CUSTOM146             : return DFB_BTN_CUSTOM_146;         
          case DIKS_CUSTOM147             : return DFB_BTN_CUSTOM_147;         
          case DIKS_CUSTOM148             : return DFB_BTN_CUSTOM_148;         
          case DIKS_CUSTOM149             : return DFB_BTN_CUSTOM_149;         
          case DIKS_CUSTOM150             : return DFB_BTN_CUSTOM_150;
          case DIKS_NULL              : return DFB_BTN_KB_NULL;
          case DIKS_CAPS_LOCK         : return DFB_BTN_KB_CAPS_LOCK;
          case DIKS_NUM_LOCK          : return DFB_BTN_KB_NUM_LOCK;
          case DIKS_SCROLL_LOCK       : return DFB_BTN_KB_SCROLL_LOCK;
          case DIKS_SHIFT             : return DFB_BTN_KB_SHIFT;
          case DIKS_CONTROL           : return DFB_BTN_KB_CONTROL;
          case DIKS_ALT               : return DFB_BTN_KB_ALT;
          case DIKS_ALTGR             : return DFB_BTN_KB_ALTGR;
          case DIKS_META              : return DFB_BTN_KB_META;
          case DIKS_SUPER             : return DFB_BTN_KB_SUPER;
          case DIKS_HYPER             : return DFB_BTN_KB_HYPER;
          default:
               return input_symbol;
     }
}

static DirectResult
Dispatch_SendEvent( IDiVine *thiz, IDiVine *real,
                    VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult         ret;
     const DFBInputEvent *event;
     VoodooMessageParser  parser;

     DIRECT_INTERFACE_GET_DATA(IDiVine_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, event );
     VOODOO_PARSER_END( parser );

     {
          /* Fix MediaTek Platform specific key mapping. */
          DFBInputEvent tmp_event;
          tmp_event = *event;
          tmp_event.key_symbol = translate_key_symbol(event->key_symbol, event->key_code);

          ret = real->SendEvent( real, &tmp_event );

		  D_INFO("Recieved - key_symbol=0x%X, key_id=0x%X, key_code=0x%X\n", event->key_symbol, event->key_id, event->key_code);
		  D_INFO("Translated - key_symbol=0x%X, key_id=0x%X, key_code=0x%X\n", tmp_event.key_symbol, tmp_event.key_id, tmp_event.key_code);
     }

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_SendSymbol( IDiVine *thiz, IDiVine *real,
                     VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult            ret;
     DFBInputDeviceKeySymbol symbol;
     VoodooMessageParser     parser;

     DIRECT_INTERFACE_GET_DATA(IDiVine_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_UINT( parser, symbol );
     VOODOO_PARSER_END( parser );

     ret = real->SendSymbol( real, symbol );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch( void *dispatcher, void *real, VoodooManager *manager, VoodooRequestMessage *msg )
{
     D_DEBUG( "IDiVine/Dispatcher: "
              "Handling request for instance %u with method %u...\n", msg->instance, msg->method );

     switch (msg->method) {
          case IDIVINE_METHOD_ID_SendEvent:
               return Dispatch_SendEvent( dispatcher, real, manager, msg );

          case IDIVINE_METHOD_ID_SendSymbol:
               return Dispatch_SendSymbol( dispatcher, real, manager, msg );
     }

     return DFB_NOSUCHMETHOD;
}

/**************************************************************************************************/

static DFBResult
Probe()
{
     /* This implementation has to be loaded explicitly. */
     return DFB_UNSUPPORTED;
}

/*
 * Constructor
 *
 * Fills in function pointers and intializes data structure.
 */
static DFBResult
Construct( IDiVine *thiz, VoodooManager *manager, VoodooInstanceID *ret_instance )
{
     DFBResult         ret;
     IDiVine          *real;
     VoodooInstanceID  instance;

     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDiVine_Dispatcher)

     ret = DiVineCreate( &real );
     if (ret) {
          DIRECT_DEALLOCATE_INTERFACE( thiz );
          return ret;
     }

     ret = voodoo_manager_register_local( manager, true, thiz, real, Dispatch, &instance );
     if (ret) {
          real->Release( real );
          DIRECT_DEALLOCATE_INTERFACE( thiz );
          return ret;
     }

     *ret_instance = instance;

     data->ref  = 1;
     data->real = real;
     data->self = instance;

     thiz->AddRef        = IDiVine_Dispatcher_AddRef;
     thiz->Release       = IDiVine_Dispatcher_Release;
     thiz->SendEvent     = IDiVine_Dispatcher_SendEvent;

     return DFB_OK;
}


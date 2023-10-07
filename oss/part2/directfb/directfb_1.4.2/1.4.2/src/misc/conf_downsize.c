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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <assert.h>
#include <search.h>


#include <directfb.h>
#include <directfb_util.h>

#include <direct/conf.h>
#include <direct/log.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <fusion/conf.h>
#include <fusion/vector.h>

#include <core/coretypes.h>
#include <core/surface.h>
#include <core/layers.h>

#include <gfx/convert.h>

#include <misc/conf.h>

D_DEBUG_DOMAIN( DirectFB_Config, "DirectFB/Config", "Runtime configuration options for DirectFB" );

static void registerSigalstack(void);

DFBConfig *dfb_config = NULL;


/**********************************************************************************************************************/

/* serial mouse device names */
#define DEV_NAME     "/dev/mouse"
#define DEV_NAME_GPM "/dev/gpmdata"

/**********************************************************************************************************************/

////////////////////////////////////////
// start of hashtable
#if USE_HASH_TABLE_SREACH

typedef struct data_struct_s
{
    const char *key_string;
    void (*FuncPtr)(char *value);

} data_struct_t;

data_struct_t* tableData;
struct hsearch_data hash;


void FUN___system(char *value);
void FUN___keypad_callback(char *value);
void FUN___keypad_internal(char *value);
void FUN___primary_layer(char *value);
void FUN___mst_gfx_width(char *value);
void FUN___mst_gfx_height(char *value);
void FUN___mst_lcd_width(char *value);
void FUN___mst_lcd_height(char *value);
void FUN___mst_gfx_h_offset(char *value);
void FUN___mst_gfx_v_offset(char *value);
void FUN___mst_enable_GOP_HMirror(char *value);
void FUN___mst_disable_GOP_VMirror(char *value);
void FUN___mst_disable_GOP_HMirror(char *value);
void FUN___mst_enable_GOP_VMirror(char *value);
void FUN___mst_enable_GOP_HMirror(char *value);
void FUN___mst_enable_GOP_VMirror(char *value);
void FUN___mst_enable_GOP_HMirror(char *value);
void FUN___mst_enable_GOP_VMirror(char *value);
void FUN___mst_disable_GOP_HMirror(char *value);
void FUN___mst_disable_GOP_VMirror(char *value);
void FUN___mst_ge_vq_phys_addr(char *value);
void FUN___mst_ge_vq_phys_len(char *value);
void FUN___mst_gop_regdma_phys_addr(char *value);
void FUN___mst_gop_regdma_phys_len(char *value);
void FUN___mst_gfx_gop_index(char *value);
void FUN___mst_miu0_hal_offset(char *value);
void FUN___mst_miu1_hal_offset(char *value);
void FUN___mst_miu0_cpu_offset(char *value);
void FUN___mst_miu0_hal_length(char *value);
void FUN___mst_miu1_cpu_offset(char *value);
void FUN___mst_miu1_hal_length(char *value);
void FUN___pixelformat(char *value);
void FUN___video_phys(char *value);
void FUN___video_length(char *value);
void FUN___mst_gop_miu_setting(char *value);
void FUN___video_phys_secondary(char *value);
void FUN___video_length_secondary(char *value);
void FUN___mst_gop_counts(char *value);
void FUN___mst_gop_available0(char *value);
void FUN___mst_gop_available1(char *value);
void FUN___mst_gop_available2(char *value);
void FUN___mst_gop_available3(char *value);
void FUN___mst_gop_dstPlane0(char *value);
void FUN___mst_gop_dstPlane1(char *value);
void FUN___mst_gop_dstPlane2(char *value);
void FUN___mst_gop_dstPlane3(char *value);
void FUN___muxCounts(char *value);
void FUN___mux0_gopIndex(char *value);
void FUN___mux1_gopIndex(char *value);
void FUN___mux2_gopIndex(char *value);
void FUN___mux3_gopIndex(char *value);
void FUN___mst_GOP_Set_YUV(char *value);
void FUN___mst_jpeg_readbuff_addr(char *value);
void FUN___mst_jpeg_readbuff_length(char *value);
void FUN___mst_jpeg_interbuff_addr(char *value);
void FUN___mst_jpeg_interbuff_length(char *value);
void FUN___mst_jpeg_outbuff_addr(char *value);
void FUN___mst_jpeg_outbuff_length(char *value);
void FUN___mst_jpeg_hwdecode(char *value);
void FUN___miu_protect(char *value); 
void FUN___mst_mem_peak_usage(char *value);
void FUN___surface_memory_type(char *value);
void FUN___window_double_buffer(char *value);
void FUN___default_layer_opacity ( char *value);
void FUN___bg_color ( char *value);
void FUN___no_cursor ( char *value);

data_struct_t ConfigTable[] =
{
    { "system", FUN___system },
    { "keypad_callback", FUN___keypad_callback },
    { "keypad_internal", FUN___keypad_internal },
    { "primary-layer", FUN___primary_layer },
    { "mst_gfx_width", FUN___mst_gfx_width },
    { "mst_gfx_height", FUN___mst_gfx_height },
    { "mst_lcd_width", FUN___mst_lcd_width },
    { "mst_lcd_height", FUN___mst_lcd_height },
    { "mst_gfx_h_offset", FUN___mst_gfx_h_offset },
    { "mst_gfx_v_offset", FUN___mst_gfx_v_offset },        
    { "mst_enable_GOP_HMirror", FUN___mst_enable_GOP_HMirror },
    { "mst_disable_GOP_VMirror", FUN___mst_disable_GOP_VMirror },
    { "mst_disable_GOP_HMirror", FUN___mst_disable_GOP_HMirror },
    { "mst_enable_GOP_VMirror", FUN___mst_enable_GOP_VMirror },
    { "mst_enable_GOP_HMirror", FUN___mst_enable_GOP_HMirror },
    { "mst_enable_GOP_VMirror", FUN___mst_enable_GOP_VMirror },
    { "mst_enable_GOP_HMirror", FUN___mst_enable_GOP_HMirror },
    { "mst_enable_GOP_VMirror", FUN___mst_enable_GOP_VMirror },
    { "mst_disable_GOP_HMirror", FUN___mst_disable_GOP_HMirror },
    { "mst_disable_GOP_VMirror", FUN___mst_disable_GOP_VMirror },
    { "mst_ge_vq_phys_addr", FUN___mst_ge_vq_phys_addr },
    { "mst_ge_vq_phys_len", FUN___mst_ge_vq_phys_len },
    { "mst_gop_regdma_phys_addr", FUN___mst_gop_regdma_phys_addr },
    { "mst_gop_regdma_phys_len", FUN___mst_gop_regdma_phys_len },
    { "mst_gfx_gop_index", FUN___mst_gfx_gop_index },
    { "mst_miu0_hal_offset", FUN___mst_miu0_hal_offset },
    { "mst_miu1_hal_offset", FUN___mst_miu1_hal_offset },
    { "mst_miu0_cpu_offset", FUN___mst_miu0_cpu_offset },
    { "mst_miu0_hal_length", FUN___mst_miu0_hal_length },
    { "mst_miu1_cpu_offset", FUN___mst_miu1_cpu_offset },
    { "mst_miu1_hal_length", FUN___mst_miu1_hal_length },
    { "pixelformat", FUN___pixelformat },
    { "video-phys", FUN___video_phys },
    { "video-length", FUN___video_length },
    { "mst_gop_miu_setting", FUN___mst_gop_miu_setting },
    { "video-phys-secondary", FUN___video_phys_secondary },
    { "video-length-secondary", FUN___video_length_secondary },
    { "mst_gop_counts", FUN___mst_gop_counts },
    { "mst_gop_available[0]", FUN___mst_gop_available0 },
    { "mst_gop_available[1]", FUN___mst_gop_available1 },
    { "mst_gop_available[2]", FUN___mst_gop_available2 },
    { "mst_gop_available[3]", FUN___mst_gop_available3 },
    { "mst_gop_dstPlane[0]", FUN___mst_gop_dstPlane0 },
    { "mst_gop_dstPlane[1]", FUN___mst_gop_dstPlane1 },
    { "mst_gop_dstPlane[2]", FUN___mst_gop_dstPlane2 },
    { "mst_gop_dstPlane[3]", FUN___mst_gop_dstPlane3 },
    { "muxCounts", FUN___muxCounts },
    { "mux0_gopIndex", FUN___mux0_gopIndex },
    { "mux1_gopIndex", FUN___mux1_gopIndex },
    { "mux2_gopIndex", FUN___mux2_gopIndex },
    { "mux3_gopIndex", FUN___mux3_gopIndex },
    { "mst_GOP_Set_YUV", FUN___mst_GOP_Set_YUV },
    { "mst_jpeg_readbuff_addr", FUN___mst_jpeg_readbuff_addr },
    { "mst_jpeg_readbuff_length", FUN___mst_jpeg_readbuff_length },
    { "mst_jpeg_interbuff_addr", FUN___mst_jpeg_interbuff_addr },
    { "mst_jpeg_interbuff_length", FUN___mst_jpeg_interbuff_length },
    { "mst_jpeg_outbuff_addr", FUN___mst_jpeg_outbuff_addr },
    { "mst_jpeg_outbuff_length", FUN___mst_jpeg_outbuff_length },
    { "mst_jpeg_hwdecode", FUN___mst_jpeg_hwdecode },
    { "miu_protect", FUN___miu_protect },        
    { "mst_mem_peak_usage", FUN___mst_mem_peak_usage },
    { "surface_memory_type", FUN___surface_memory_type },
    { "window_double_buffer", FUN___window_double_buffer },
    { "default_layer_opacity", FUN___default_layer_opacity },
    { "bg-color", FUN___bg_color },
    { "no-cursor", FUN___no_cursor },
};


void CreateConfigHashTable()
{
    ENTRY e, *ep;
    int i, size, ret;
    size = sizeof(ConfigTable)/sizeof(data_struct_t);

    //hcreate(size);

    memset( &hash, 0, sizeof(hash) );

    ret = hcreate_r(size, &hash);
    if(!ret) {
        if (errno == ENOMEM)
            printf("DFB hashtable NOMEM, %s, %d\n", __FUNCTION__, __LINE__);
        
        printf("DFB hashtable ERROR, %s, %d\n", __FUNCTION__, __LINE__);
    }


    for (i = 0; i < size; i++) 
    {
        e.key = ConfigTable[i].key_string;
        /* data is just an integer, instead of a
          pointer to something */
        e.data = (void*)ConfigTable[i].FuncPtr;

        //ep = hsearch(e, ENTER);

        ret = hsearch_r(e, ENTER, &ep, &hash);
        if(ret == 0) {
        printf("DFB Hashtable is full %s, %d\n", __FUNCTION__, __LINE__);
        }


        /* there should be no failures */
        if (ep == NULL) {
            printf("ERROR %s, %d\n", __FUNCTION__, __LINE__);
            
           fprintf(stderr, "entry failed\n");
           exit(EXIT_FAILURE);
        }
    }
}

bool SearchConfigHashTable( const char* name, const char* value)
{
    ENTRY e, *ep;
    /* print two entries from the table, and
    show that two are not in the table */
    e.key = name;
    //ep = hsearch(e, FIND);
    hsearch_r( e, FIND, &ep, &hash );

    //D_INFO("misc  %9.9s -> %9.9s:%d\n", e.key, ep ? ep->key : "NULL", ep ? (int)(ep->data) : 0);

    if (ep == NULL)
        return false;

    //data_struct_t *tableData;


    void (*FuncPtr)(char *value);
    
    FuncPtr = (void*)ep->data;
    FuncPtr(value);

    return true;

}

#endif
//end of hashtable
//////////////////////////////////////////

DFBSurfacePixelFormat
dfb_config_parse_pixelformat( const char *format )
{
     int    i;
     size_t length = strlen(format);

     for (i=0; dfb_pixelformat_names[i].format != DSPF_UNKNOWN; i++) {
          if (!strcasecmp( format, dfb_pixelformat_names[i].name ))
               return dfb_pixelformat_names[i].format;
     }

     for (i=0; dfb_pixelformat_names[i].format != DSPF_UNKNOWN; i++) {
          if (!strncasecmp( format, dfb_pixelformat_names[i].name, length ))
               return dfb_pixelformat_names[i].format;
     }

     return DSPF_UNKNOWN;
}

/**********************************************************************************************************************/

static void config_values_parse( FusionVector *vector, const char *arg )
{
     char *values = D_STRDUP( arg );
     char *s      = values;
     char *r, *p  = NULL;

     if (!values) {
          D_OOM();
          return;
     }

     while ((r = strtok_r( s, ",", &p ))) {
          direct_trim( &r );

          r = D_STRDUP( r );
          if (!r)
               D_OOM();
          else
               fusion_vector_add( vector, r );

          s = NULL;
     }

     D_FREE( values );
}

static void config_values_free( FusionVector *vector )
{
     char *value;
     int   i;

     fusion_vector_foreach (value, i, *vector)
          D_FREE( value );

     fusion_vector_destroy( vector );
     fusion_vector_init( vector, 2, NULL );
}

static int config_read_cmdline( char *cmdbuf, int size, FILE *f )
{
     int ret = 0;
     int len = 0;

     ret = fread( cmdbuf, 1, 1, f );

     /* empty dividing 0 */
     if( ret==1 && *cmdbuf==0 ) {
          ret = fread( cmdbuf, 1, 1, f );
     }

     while(ret==1 && len<(size-1)) {
          len++;
          ret = fread( ++cmdbuf, 1, 1, f );
          if( *cmdbuf == 0 )
               break;
     }

     if( len ) {
          cmdbuf[len]=0;
     }

     return  len != 0;
}


/*
 * allocates config and fills it with defaults
 */
static void config_allocate( void )
{
         int i;

     if (dfb_config)
          return;

     dfb_config = (DFBConfig*) calloc( 1, sizeof(DFBConfig) );

     for (i=0; i<D_ARRAY_SIZE(dfb_config->layers); i++) {
          dfb_config->layers[i].src_key_index          = -1;

          dfb_config->layers[i].background.color.a     = 0;
          dfb_config->layers[i].background.color.r     = 0;
          dfb_config->layers[i].background.color.g     = 0;
          dfb_config->layers[i].background.color.b     = 0;
          dfb_config->layers[i].background.color_index = -1;
          dfb_config->layers[i].background.mode        = DLBM_COLOR;
     }

     dfb_config->layers[0].init               = true;
     dfb_config->layers[0].background.color.a = 0x0;
     dfb_config->layers[0].background.color.r = 0x0;
     dfb_config->layers[0].background.color.g = 0x0;
     dfb_config->layers[0].background.color.b = 0x0;
     dfb_config->layers[0].stacking           = (1 << DWSC_UPPER)  |
                                                (1 << DWSC_MIDDLE) |
                                                (1 << DWSC_LOWER);


     dfb_config->pci.bus                  = 1;
     dfb_config->pci.dev                  = 0;
     dfb_config->pci.func                 = 0;

     dfb_config->banner                   = true;
     dfb_config->deinit_check             = true;
     dfb_config->mmx                      = true;
     dfb_config->vt                       = true;
     dfb_config->vt_switch                = true;
     dfb_config->vt_num                   = -1;
     dfb_config->vt_switching             = true;
     dfb_config->kd_graphics              = true;
     dfb_config->translucent_windows      = true;
     dfb_config->font_premult             = true;
     dfb_config->mouse_motion_compression = true;
     dfb_config->mouse_gpm_source         = false;
     dfb_config->mouse_source             = D_STRDUP( DEV_NAME );
     dfb_config->linux_input_grab         = false;
     dfb_config->window_policy            = -1;
     dfb_config->buffer_mode              = -1;
     dfb_config->wm                       = D_STRDUP( "default" );
     dfb_config->decorations              = true;
     dfb_config->unichrome_revision       = -1;
     dfb_config->dma                      = false;
     dfb_config->agp                      = 0;
     dfb_config->matrox_tv_std            = DSETV_PAL;
     dfb_config->i8xx_overlay_pipe_b      = false;
     dfb_config->surface_shmpool_size     = 64 * 1024 * 1024;
     dfb_config->keep_accumulators        = 1024;
     dfb_config->font_format              = DSPF_A8;





///////////////////////////////////////////////////////////////////
    /* default to devmem */
    dfb_config->system = D_STRDUP( "DEVMEM" );

    dfb_config->mst_gfx_width           = 1280;
    dfb_config->mst_gfx_height          = 720;
    dfb_config->mst_gfx_h_offset        = 0;
    dfb_config->mst_gfx_v_offset        = 0;
    dfb_config->mst_gfx_gop_index       = 0;

    dfb_config->mst_miu0_cpu_offset     = 0x00000000;
    dfb_config->mst_miu1_cpu_offset     = 0x60000000;

    //FUN___pixelformat("ARGB4444");

    dfb_config->mst_gop_counts          = 2;
    dfb_config->mst_gop_available[0]    = 0;//GOP0
    dfb_config->mst_gop_available[1]    = 1;
    dfb_config->mst_gop_available[2]    = 1;

    dfb_config->mst_gop_dstPlane[0]     = 2;//E_GOP_DST_OP0
    dfb_config->mst_gop_dstPlane[1]     = 0;     
    dfb_config->mst_gop_dstPlane[2]     = 0;

    /*mux-gop settings*/
    dfb_config->mst_mux_counts          = 2;
    dfb_config->mst_gop_mux[0]          = 0;
    dfb_config->mst_gop_mux[1]          = 1;
    dfb_config->mst_gop_mux[2]          = 3;
    dfb_config->mst_gop_mux[3]          = 2;


//////////////////////////////////////////////////////////////////

    dfb_config->mst_gop_mux[4]               = 0;
    dfb_config->mst_gop_mux[5]               = 0;

    dfb_config->mst_gop_available[3] = 3;
    dfb_config->mst_gop_available[4] = 4;
    dfb_config->mst_gop_available[5] = 5;

    dfb_config->mst_gop_available_r[0] = 0x0f;//GOP0
    dfb_config->mst_gop_available_r[1] = 0x0f;
    dfb_config->mst_gop_available_r[2] = 0x0f;
    dfb_config->mst_gop_available_r[3] = 0x0f;
    dfb_config->mst_gop_available_r[4] = 0x0f;
    dfb_config->mst_gop_available_r[5] = 0x0f;

    dfb_config->mst_gop_dstPlane[3] = 2;
    dfb_config->mst_gop_dstPlane[4] = 2;
    dfb_config->mst_gop_dstPlane[5] = 2;


     dfb_config->default_layer_opacity=255;
     dfb_config->mst_lcd_width = 0;
     dfb_config->mst_lcd_height = 0;
     dfb_config->mst_ge_vq_phys_addr=0;
     dfb_config->mst_ge_vq_phys_len=0;
     dfb_config->mst_gop_regdmaphys_addr = 0;
     dfb_config->mst_gop_regdmaphys_len = 0;



     /* hw decode jpeg related buf */
     dfb_config->mst_jpeg_readbuff_addr       = 0x0;
     dfb_config->mst_jpeg_readbuff_length     = 0x0;
     dfb_config->mst_jpeg_interbuff_addr      = 0x0;
     dfb_config->mst_jpeg_interbuff_length    = 0x0;
     dfb_config->mst_jpeg_outbuff_addr        = 0x0;
     dfb_config->mst_jpeg_outbuff_length      = 0x0;
     dfb_config->mst_jpeg_hwdecode              = false;

     /* sw decode jpeg related info */
     dfb_config->mst_jpeg_hwdecode_option  = true;

     /* mstar hw dither configurations */
     dfb_config->mst_dither_enable  = false;



     dfb_config->ion_heapmask_by_layer[0] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_layer[1] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_layer[2] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_layer[3] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_layer[4] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_layer[5] = CONF_ION_HEAP_MIU0_MASK;
     dfb_config->ion_heapmask_by_surface = CONF_ION_HEAP_MIU0_MASK;
 
     /*gop layer settings*/
     dfb_config->mst_goplayer_counts            = 0;
     dfb_config->mst_goplayer[0]                = 0;
     dfb_config->mst_goplayer[1]                = 0;
     dfb_config->mst_goplayer[2]                = 0;
     dfb_config->mst_goplayer[3]                = 0;
     dfb_config->mst_goplayer[4]                = 0;
     dfb_config->mst_goplayer[5]                = 0;



     /*miu settings*/
     dfb_config->mst_miu0_hal_offset            = 0;
     dfb_config->mst_miu0_cpu_offset            = 0;
     dfb_config->mst_miu0_hal_length            = 0x8000000;
     dfb_config->mst_miu1_hal_length            = 0x8000000;

     dfb_config->mst_miu2_hal_offset    = 0;
     dfb_config->mst_miu2_cpu_offset    = 0;
     dfb_config->mst_miu2_hal_length    = 0;

     dfb_config->mst_disable_hwclip = false;
     dfb_config->mst_dfb_register_app_manager = false;


     dfb_config->video_length_secondary       = 0;
     dfb_config->video_phys_secondary_cpu         = 0;
     dfb_config->video_phys_secondary_hal         = 0;
     dfb_config->mst_gop_miu_setting          = 0;
     dfb_config->mst_GOP_HMirror = -1;
     dfb_config->mst_GOP_VMirror = -1;
     dfb_config->mst_layer_bTiled[0]          = false;
     dfb_config->mst_layer_bTiled[1]          = false;
     dfb_config->mst_layer_bTiled[2]          = false;
     dfb_config->mst_layer_bTiled[3]          = false;
     dfb_config->mst_osd_to_ve                  = false;
     dfb_config->mst_xc_to_ve_mux               = 0;
     dfb_config->mst_GOP_Set_YUV           = false;
     dfb_config->do_stretchdown_patch = false;
     dfb_config->line_stretchblit_patch = false;
     dfb_config->static_stretchdown_buf = false;
     dfb_config->mst_png_hwdecode = true;
     dfb_config->mst_forcewrite_DFB = 0x0;
     dfb_config->tvos_mode = false;
     dfb_config->enable_devmem_dump = false;
     dfb_config->do_yuvtorgb_sw_patch = false;
     dfb_config->mst_ir_max_keycode = 0x7FFF;
     dfb_config->bUsingHWTLB = false;
     dfb_config->bGOPUsingHWTLB = false;
     dfb_config->TLBAlignmentSize = 0x8000; //32k
     dfb_config->bPrealloc_map_tlb = false;
     dfb_config->mbootGOPIndex = 0;
     dfb_config->mst_enable_gevq = false;
     dfb_config->mst_disable_layer_init = -1;
     dfb_config->mst_debug_layer = false;
     dfb_config->mst_debug_surface= false;
     dfb_config->mst_debug_input= DFB_DBG_LEVEL_DISABLE;
     dfb_config->mst_debug_ion= false;
     dfb_config->mst_layer_default_width=1280;
     dfb_config->mst_layer_default_height=720;
     dfb_config->full_update_numerator = 9;
     dfb_config->full_update_denominator = 10;
     dfb_config->mst_surface_memory_type = -1;
     dfb_config->mst_disable_decode_small_jpeg_by_sw = false;
     dfb_config->mst_enable_GOP_Vmirror_Patch = false;     
     dfb_config->do_GE_Vmirror = false;     
     dfb_config->stretchdown_patch_ratio = 1;
     dfb_config->null_driver = false;
     dfb_config->sw_render = SWRF_NONE;
     dfb_config->mst_enable_jpeg_quality = false;
     dfb_config->mst_enable_dip = false;
     dfb_config->mst_dip_mload_addr = 0x0;
     dfb_config->mst_dip_mload_length = 0x0;
     dfb_config->mst_dip_select = 0;
     dfb_config->mst_disable_master_pri_high = false;
     dfb_config->small_font_patch = false;
     dfb_config->freeze_image_by_dfb = false;
     dfb_config->do_i8toargb4444_sw_patch = false;     
     dfb_config->mst_enable_ve_init = false;
     dfb_config->mst_register_gpd = true;
     dfb_config->window_double_buffer = false;
     dfb_config->do_hw_jpg_limit_patch = false;     
     dfb_config->mst_GE_performanceTest = false;
     dfb_config->mst_MIU_protect = false;
     dfb_config->mst_MIU_protect_BlockID = 3;
     //dfb_config->mst_disable_msos = false;
     dfb_config->mst_disable_dfbinfo = false;
     //dfb_config->mst_framebuffer_cma = false;
     dfb_config->mst_cma_heap_id = 26;  //set heap id to 26 default
     dfb_config->mst_sec_cma_heap_id = 26;  //set heap id to 26 default
     dfb_config->mst_mapmem_mode = 1;   // default msos,  0: devmem  1:msos_mpool  2: cma_pool
     dfb_config->src_color_key_index_patch = 1;
     dfb_config->mst_ir_repeat_time = 250;
     dfb_config->mst_handle_wm_key = false;
     dfb_config->disable_cjk_string_break = false;
     dfb_config->disable_quick_press = false;
     dfb_config->mst_ir_first_time_out = 0;
     dfb_config->mst_disable_window_scale_patch = false;
     dfb_config->mst_force_flip_wait_tagid = false;
     dfb_config->enable_cursor_mouse_optimize = false;
     dfb_config->layer_support_palette = false;
     dfb_config->stretchblit_with_rotate = true;
     dfb_config->mst_layer_gwin_level = 1;
     dfb_config->mst_enable_gop_gwin_partial_update = false;
     dfb_config->mst_new_alphamode = -1;
     dfb_config->mst_newalphamode_on_layer = -1; //all layer
     /* layer full update  settings*/
     dfb_config->mst_layer_bfullupdate[0]                = false;
     dfb_config->mst_layer_bfullupdate[1]                = false;
     dfb_config->mst_layer_bfullupdate[2]                = false;
     dfb_config->mst_layer_bfullupdate[3]                = false;
     dfb_config->mst_layer_bfullupdate[4]                = false;
     dfb_config->mst_layer_bfullupdate[5]                = false;
     dfb_config->mst_gop_miu2_setting_extend = 0;
     dfb_config->mst_AFBC_layer_enable = 0; // Disable layer 0 ~ layer 5
     dfb_config->mst_AFBC_mode = 0x101; // E_GOP_FB_AFBC_NONSPLT_YUVTRS_ARGB8888=0x101

     dfb_config->mst_enhance_stretchblit_precision = false;

     dfb_config->mst_enable_GLES2 = false;
     dfb_config->mst_mem_peak_usage = 0;
     dfb_config->mst_enable_gwin_multialpha = false;
     dfb_config->mst_measure_png_performance = false;
     dfb_config->mst_rgb2yuv_mode = 0; //default : BT601
     dfb_config->mst_debug_cma= false;

     dfb_config->mst_margine_left = 0;
     dfb_config->mst_margine_wright = 0;
     dfb_config->mst_margine_top = 0;
     dfb_config->mst_margine_bottom = 0;

     dfb_config->mst_CTV_linux_input_patch = false;
     /* default to no-vt-switch if we don't have root privileges */
     if (geteuid())
          dfb_config->vt_switch = false;

     fusion_vector_init( &dfb_config->linux_input_devices, 2, NULL );
     fusion_vector_init( &dfb_config->tslib_devices, 2, NULL );

}



int query_miu(const unsigned long addr, unsigned long *offset)
{
    unsigned long miu_offset;
    int miu_select;

    if ( dfb_config->mst_miu2_hal_offset != 0 && addr >= dfb_config->mst_miu2_hal_offset) 
    {
        miu_offset = dfb_config->mst_miu2_hal_offset;
        miu_select = 2;
    }
    else if ( dfb_config->mst_miu1_hal_offset != 0 && addr >= dfb_config->mst_miu1_hal_offset) 
    {
        miu_offset = dfb_config->mst_miu1_hal_offset;
        miu_select = 1;
    }    
    else {
        miu_offset = dfb_config->mst_miu0_hal_offset;
        miu_select = 0;
    }
                
    *offset = (addr - miu_offset);   

    return miu_select;
}

u32 _mstarGFXAddr(u32 cpuPhysicalAddr)
{
    if (dfb_config->mst_miu2_cpu_offset != 0 && cpuPhysicalAddr >= dfb_config->mst_miu2_cpu_offset )
        return (cpuPhysicalAddr - dfb_config->mst_miu2_cpu_offset + dfb_config->mst_miu2_hal_offset);
    else if (cpuPhysicalAddr >= dfb_config->mst_miu1_cpu_offset )
        return (cpuPhysicalAddr - dfb_config->mst_miu1_cpu_offset + dfb_config->mst_miu1_hal_offset);
    else
        return (cpuPhysicalAddr - dfb_config->mst_miu0_cpu_offset + dfb_config->mst_miu0_hal_offset);
}

u32 _mstarCPUPhyAddr(u32 halPhysicalAddr)
{
    if (dfb_config->mst_miu2_hal_offset != 0 && halPhysicalAddr >= dfb_config->mst_miu2_hal_offset)
        return (dfb_config->mst_miu2_cpu_offset + halPhysicalAddr - dfb_config->mst_miu2_hal_offset);
    else if (halPhysicalAddr >= dfb_config->mst_miu1_hal_offset)
        return (dfb_config->mst_miu1_cpu_offset + halPhysicalAddr - dfb_config->mst_miu1_hal_offset);
    else
        return (dfb_config->mst_miu0_cpu_offset + halPhysicalAddr - dfb_config->mst_miu0_hal_offset);
}

static void update_videomem_addr()
{
     unsigned long  miu1_cpu_hal_diff = dfb_config->mst_miu1_cpu_offset - dfb_config->mst_miu1_hal_offset;
     unsigned long  miu2_cpu_hal_diff = dfb_config->mst_miu2_cpu_offset - dfb_config->mst_miu2_hal_offset;

     if (dfb_config->mst_miu2_hal_offset != 0 && dfb_config->video_phys_hal >= dfb_config->mst_miu2_hal_offset)
     {
         dfb_config->video_phys_cpu = dfb_config->video_phys_hal + miu2_cpu_hal_diff;
     }
     else if (dfb_config->video_phys_hal >= dfb_config->mst_miu1_hal_offset)
     {
         dfb_config->video_phys_cpu = dfb_config->video_phys_hal + miu1_cpu_hal_diff;
     }
     else
     {
         dfb_config->video_phys_cpu = dfb_config->video_phys_hal + dfb_config->mst_miu0_cpu_offset;
     }

     if (dfb_config->mst_miu2_hal_offset != 0 &&  dfb_config->video_phys_secondary_hal >= dfb_config->mst_miu2_hal_offset)
         dfb_config->video_phys_secondary_cpu = dfb_config->video_phys_secondary_hal + miu2_cpu_hal_diff;
     else if ( dfb_config->video_phys_secondary_hal >= dfb_config->mst_miu1_hal_offset)
         dfb_config->video_phys_secondary_cpu = dfb_config->video_phys_secondary_hal + miu1_cpu_hal_diff;
     else
         dfb_config->video_phys_secondary_cpu = dfb_config->video_phys_secondary_hal + dfb_config->mst_miu0_cpu_offset;

}

#if USE_HASH_TABLE_SREACH
DFBResult dfb_config_set( const char *name, const char *value )
{    
    bool bCheck = true;
    
    bCheck = SearchConfigHashTable(name, value);

    if (bCheck == false)
    {
        if (fusion_config_set( name, value ))
            return DFB_UNSUPPORTED;
    }
    
    return DFB_OK;

}

#else

#endif

DFBResult dfb_config_init( int *argc, char *(*argv[]) )
{
     DFBResult ret;
     int i;
     char *home = getenv( "HOME" );
     char *dfbhome = getenv( "DFBHOME" );
     char *config_path = getenv("CONFIG_PATH");
     char *prog = NULL;
     char *session;
     char *dfbargs;
     char  cmdbuf[1024];

#if USE_HASH_TABLE_SREACH
    CreateConfigHashTable();
#endif

     if (dfb_config)
          return DFB_OK;

     config_allocate();

     /* Read system settings. */
     ret = dfb_config_read( SYSCONFDIR"/directfbrc" );
     if (ret  &&  ret != DFB_IO)
          return ret;

      /* Read new path settings. */
     if (config_path) {
          int  len = strlen(config_path) + strlen("/.directfbrc") + 1;
          char buf[len];

          snprintf( buf, len, "%s/.directfbrc", config_path );

          ret = dfb_config_read( buf );
          if (ret  &&  ret != DFB_IO)
               return ret;
     }

     /* Read user settings. */
     if (home) {
          int  len = strlen(home) + strlen("/.directfbrc") + 1;
          char buf[len];

          snprintf( buf, len, "%s/.directfbrc", home );

          ret = dfb_config_read( buf );
          if (ret  &&  ret != DFB_IO)
               return ret;
     }

     if (dfbhome) {
          int  len = strlen(dfbhome) + strlen("/.directfbrc") + 1;
          char buf[len];

          snprintf( buf, len, "%s/.directfbrc", dfbhome );

          ret = dfb_config_read( buf );
          if (ret  &&  ret != DFB_IO)
               return ret;
     }

     if (!dfb_config->vt_switch)
          dfb_config->kd_graphics = true;


     return DFB_OK;
}

DFBResult dfb_config_read( const char *filename )
{
     DFBResult ret = DFB_OK;
     char line[400];
     FILE *f = NULL; //Fix converity END

     char *slash = 0;
     char *cwd   = 0;

     config_allocate();

     dfb_config->config_layer = &dfb_config->layers[0];

     f = fopen( filename, "r" );
     if (!f) {
          D_DEBUG_AT( DirectFB_Config, "Unable to open config file `%s'!\n", filename );
          return DFB_IO;
     } else {
          D_DEBUG_AT( DirectFB_Config, "Parsing config file '%s'.\n", filename );
     }

     /* store/restore the cwd (needed for the "include" command */
     slash = strrchr( filename, '/' );
     if( slash ) {
          cwd = getcwd(0,0);
          if( !cwd )
          {
               fclose( f );
               return D_OOM();
          }

          /* must copy filename for path, due to const'ness */
          char nwd[strlen(filename)+1];  //Coverity-03132013
          strcpy( nwd, filename );
          nwd[slash-filename] = 0;
          chdir( nwd );

          D_DEBUG_AT( DirectFB_Config, "changing configuration lookup directory to '%s'.\n", nwd );
     }

     while (fgets( line, 400, f )) {
          char *name = line;
          char *comment = strchr( line, '#');
          char *value;

          if (comment) {
               *comment = 0;
          }

          value = strchr( line, '=' );

          if (value) {
               *value++ = 0;
               direct_trim( &value );
          }

          direct_trim( &name );

          if (!*name  ||  *name == '#')
               continue;

          ret = dfb_config_set( name, value );
          if (ret) {
               if (ret == DFB_UNSUPPORTED) {
                    D_ERROR( "DirectFB/Config: *********** In config file `%s': "
                             "Invalid option `%s'! ***********\n", filename, name );
                    ret = DFB_OK;
                    continue;
               }
               break;
          }
     }

     fclose( f );

     /* restore original cwd */
     if( cwd ) {
          chdir( cwd );
          free( cwd );
     }

     return ret;
}

#if !USE_SIZE_OPTIMIZATION
#ifndef DFB_AARCH64
static void sigalHandler(int sig, siginfo_t* sigInfo, void* context)
{
    void* trace[256];
    int n = backtrace(trace, sizeof(trace) / sizeof(trace[0]));
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("!!!!!!!!!!!!!!! [DFB] SIGNAL = %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", sig);
    printf("!!!!!!!!!!!!!!! [DFB] backtrace returned %d addresses!!!!!!!!!!!!!!!\n", n);
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    backtrace_symbols_fd(trace, n, fileno(stderr));
    exit(EXIT_FAILURE);
}

static void registerSigalstack()
{
    stack_t newStack;
    struct sigaction newSigaction;

    newStack.ss_sp = malloc(SIGSTKSZ); // 8192 byte
    if (!newStack.ss_sp)
        printf("newStack.ss_sp is NULL, malloc fail\n");

    newStack.ss_size = SIGSTKSZ; // 8192 byte
    newStack.ss_flags = 0;

    if (sigaltstack(&newStack, 0) == -1)
        printf("sigaltstack == -1, that is fail\n");
        
    newSigaction.sa_sigaction = sigalHandler;
    newSigaction.sa_flags = 0;
    if (sigemptyset(&newSigaction.sa_mask) < 0)
        printf("set new action mask error\n");

    if (sigaction(SIGSEGV, &newSigaction, 0) < 0)
        printf("sigaction SIGSEGV error\n");

    if (sigaction(SIGFPE, &newSigaction, 0) < 0)
        printf("sigaction SIGSEGV error\n");

    if (sigaction(SIGBUS, &newSigaction, 0) < 0)
        printf("sigaction SIGSEGV error\n");       

}
#endif
#endif

#if USE_HASH_TABLE_SREACH

void FUN___system(char *value) {        
    if (value) {        
           if (dfb_config->system)        
                D_FREE( dfb_config->system );        
    dfb_config->system = D_STRDUP( value );        
    }        
    else {        
           printf("DirectFB/Config 'system': No system specified!\n");            
    }    
}        

void FUN___primary_layer(char *value) {        
    if (value) {        
           int id;        
        
           if (sscanf( value, "%d", &id ) < 1){        
                printf("DirectFB/Config 'primary-layer': Could not parse id!\n");        
                        
         }        
        
    dfb_config->primary_layer = id;        
    }        
    else {        
           printf("DirectFB/Config 'primary-layer': No id specified!\n");        
                   
    }        
}

void FUN___no_cursor(char *value) {        
    dfb_config->no_cursor = true;        
}        

void FUN___pixelformat(char *value) {        
    DFBConfigLayer *conf = dfb_config->config_layer;        
        
    if (value) {        
           DFBSurfacePixelFormat format;        
        
           format = dfb_config_parse_pixelformat( value );        
           if (format == DSPF_UNKNOWN){        
                printf("DirectFB/Config '%s': Could not parse format!\n", __FUNCTION__);        
                        
         }        
        
           if (conf == &dfb_config->layers[0])        
       dfb_config->mode.format = format;        
        
           conf->config.pixelformat = format;        
           conf->config.flags      |= DLCONF_PIXELFORMAT;        
    }        
    else {        
           printf("DirectFB/Config '%s': No format specified!\n", __FUNCTION__);        
                   
    }        
}        

void FUN___video_phys(char *value) {        
    if (value) {        
           char *error;        
           ulong phys;        
        
           phys = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in hex value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->video_phys_hal = phys;        
           update_videomem_addr();        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___video_length(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->video_length = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___video_phys_secondary(char *value) {        
    if (value) {        
           char *error;        
           ulong phys;        
        
           phys = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in hex value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->video_phys_secondary_hal= phys;        
           update_videomem_addr();        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___video_length_secondary(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->video_length_secondary = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
    
void FUN___mst_gfx_width(char *value) {        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gfx_width = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___mst_gfx_height(char *value) {        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gfx_height = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
         
void FUN___mst_lcd_width(char *value) {        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_lcd_width = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_lcd_height(char *value) {        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_lcd_height = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_ge_vq_phys_addr(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_ge_vq_phys_addr = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_ge_vq_phys_len(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_ge_vq_phys_len = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_gop_regdma_phys_addr(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gop_regdmaphys_addr = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_gop_regdma_phys_len(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gop_regdmaphys_len = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu0_hal_offset(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu0_hal_offset = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu0_hal_length(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu0_hal_length = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu0_cpu_offset(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu0_cpu_offset = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu1_hal_offset(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu1_hal_offset = val;        
           update_videomem_addr();        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu1_hal_length(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_miu1_hal_length = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_miu1_cpu_offset(char *value) {        
        
    if (value) {        
           char *error;        
           unsigned long val;        
           val = strtoul( value, &error, 16 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
    dfb_config->mst_miu1_cpu_offset = val;        
           update_videomem_addr();        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        

void FUN___mst_gfx_h_offset(char *value) {        
        
    if (value) {        
           char *error;        
          unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gfx_h_offset = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_gfx_v_offset(char *value) {        
    if (value) {        
           char *error;        
          unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gfx_v_offset = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
}        
        
void FUN___mst_gfx_gop_index(char *value) {        
    if (value) {        
           char *error;        
          unsigned long val;        
           val = strtoul( value, &error, 10 );        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->mst_gfx_gop_index = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
                   
    }        
        
}        
  
void FUN___mst_gop_counts(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                     
      }        
      dfb_config->mst_gop_counts = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available0(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available[0] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available1(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available[1] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available2(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available[2] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_available3(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_available[3] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}

void FUN___mst_gop_dstPlane0(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_dstPlane[0] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_dstPlane1(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_dstPlane[1] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_dstPlane2(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_dstPlane[2] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mst_gop_dstPlane3(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_dstPlane[3] = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        

void FUN___mst_jpeg_readbuff_addr(char *value) {        
    if (value) {        
           char *error;        
           ulong val;        
        
           val = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_readbuff_addr = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
         
void FUN___mst_jpeg_readbuff_length(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_readbuff_length = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
         
void FUN___mst_jpeg_interbuff_addr(char *value) {        
    if (value) {        
           char *error;        
           ulong val;        
        
           val = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_interbuff_addr= val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
         
void FUN___mst_jpeg_interbuff_length(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_interbuff_length = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
         
void FUN___mst_jpeg_outbuff_addr(char *value) {        
    if (value) {        
           char *error;        
           ulong val;        
        
           val = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_outbuff_addr = val;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}         
        
void FUN___mst_jpeg_outbuff_length(char *value) {        
    if (value) {        
           char *error;        
           ulong length;        
        
           length = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->mst_jpeg_outbuff_length = length;        
    }        
    else {        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    }        
}        
        
void FUN___mst_jpeg_hwdecode(char *value) {        
    dfb_config->mst_jpeg_hwdecode = true;        
}         
        
void FUN___muxCounts(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_mux_counts= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mux0_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_mux[0]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mux1_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_mux[1]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mux2_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_mux[2]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        
        
void FUN___mux3_gopIndex(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 10 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_mux[3]= val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
} 

void FUN___mst_gop_miu_setting(char *value) {        
     if (value){        
         char *error;        
         unsigned long val;        
         val = strtoul( value, &error, 16 );        
        
        if (*error){        
             printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
      }        
      dfb_config->mst_gop_miu_setting = val;        
   }        
     else{        
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
   }        
}        

void FUN___mst_enable_GOP_HMirror(char *value) {        
    dfb_config->mst_GOP_HMirror = 1;        
}        
void FUN___mst_disable_GOP_HMirror(char *value) {        
    dfb_config->mst_GOP_HMirror = 0;        
}

void FUN___mst_enable_GOP_VMirror(char *value) {        
    dfb_config->mst_GOP_VMirror = 1;        
}    

void FUN___mst_disable_GOP_VMirror(char *value) {        
    dfb_config->mst_GOP_VMirror = 0;        
}

void FUN___keypad_callback(char *value) {        
    if (value)        
   {        
           char *error;        
           ulong addr;        
        
           addr = strtoul( value, &error, 16);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
                        
         }        
        
    dfb_config->keypadCallback = (keypad_func)addr;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
  }        
}        

void FUN___keypad_internal(char *value) {        
    if (value)        
   {        
           char *error;        
           ulong internal;        
        
           internal = strtoul( value, &error, 10);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
         }        
        
    dfb_config->keyinternal= internal;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
  }        
}        
        
void FUN___mst_GOP_Set_YUV(char *value) {        
     dfb_config->mst_GOP_Set_YUV = true;        
}        

void FUN___window_double_buffer(char *value) {         
    dfb_config->window_double_buffer= true;        
}        
       
void FUN___surface_memory_type(char *value) {         
    if (value)        
   {        
           char *error;        
           u8 type;        
        
           type = strtoul( value, &error, 10);        
        
           if (*error){        
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );        
        
         }        
        
    dfb_config->mst_surface_memory_type = type;        
  }        
    else        
   {        
        printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);        
    
  }        
}              
  
void FUN___miu_protect(char *value) {        
    dfb_config->mst_MIU_protect = true;        
        
    if (value) {        
           int id;        
        
           if (sscanf( value, "%d", &id ) < 0){        
                printf("DirectFB/Config '%s': Could not parse id!\n", __FUNCTION__);        
    
         }        
        
           if (id < 0 || id > 3){        
                printf("DirectFB/Config '%s': ID %d out of bounds!\n", __FUNCTION__, id);        
        
         }        
        
    dfb_config->mst_MIU_protect_BlockID = id;        
        
    }        
        
}          

void FUN___mst_mem_peak_usage(char *value) {

    if (value) {
        char *error;
        int val;
        
        val = strtoul( value, &error, 10 );

        if (*error) {
            D_ERROR( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );
            return DFB_INVARG;
        }

        dfb_config->mst_mem_peak_usage = val;
    }
    else {
       D_ERROR( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__ );
       return DFB_INVARG;
    }
}

void FUN___default_layer_opacity(char *value) {
    if (value) {
           char *error;
          unsigned long val;
           val = strtoul( value, &error, 10 );

           if (*error){
                printf( "DirectFB/Config '%s': Error in value '%s'!\n", __FUNCTION__, error );

         }

    dfb_config->default_layer_opacity = val;
    }
    else {
           printf( "DirectFB/Config '%s': No value specified!\n", __FUNCTION__);

    }
}

void FUN___bg_color(char *value) {
    DFBConfigLayer *conf = dfb_config->config_layer;

    if (value) {
           char *error;
           u32   argb;

           argb = strtoul( value, &error, 16 );

           if (*error){
                printf( "DirectFB/Config '%s': Error in color '%s'!\n", __FUNCTION__, error );

         }

           conf->background.color.b = argb & 0xFF;
           argb >>= 8;
           conf->background.color.g = argb & 0xFF;
           argb >>= 8;
           conf->background.color.r = argb & 0xFF;
           argb >>= 8;
           conf->background.color.a = argb & 0xFF;

           conf->background.color_index = -1;
           conf->background.mode        = DLBM_COLOR;
    }
    else {
           printf( "DirectFB/Config '%s': No color specified!\n", __FUNCTION__ );

    }
}

#endif

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

//#ifdef MSTAR_DEBUG_LAYER
#define DIRECT_ENABLE_DEBUG
//#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <directfb.h>
#include <direct/thread.h>
#include <misc/conf.h>
#include <fusion/property.h>
#include <config.h>

#define GL_GLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include "mstar_natives.h"

#include "mstar_gles2.h"

#include "mstar_gles2_priv.h"


#define SHARE_CONTEXT 1


//#define  USE_BINARY_SHADER_PROGRAM

#ifdef USE_BINARY_SHADER_PROGRAM
#define BINARY_SHADER_PROGRAM "gfxdrivers/sdr2hdr.bin"
#else

//#define USE_BINARY_VERTEX_SHADER

#ifdef USE_BINARY_VERTEX_SHADER
#define BINARY_VERTEX_SHADER "gfxdrivers/sdr2hdr.vsb"
#endif

//#define USE_BINARY_FRAGMENT_SHADER

#ifdef USE_BINARY_FRAGMENT_SHADER
#define BINARY_FRAGMENT_SHADER "gfxdrivers/sdr2hdr.fsb"
#endif

#endif //USE_BINARY_SHADER_PROGRAM


///////////////////////////////////////////////////////////////////////////////

typedef enum {
     GLES2VA_POSITIONS      = 0,
     GLES2VA_TEXCOORDS      = 1,
} GLES2VertexAttribs;

/*
 * A GLES shader program object and locations of various uniform variables.
 */

typedef enum {
     GLES2_BLIT   =  0,
     GLES2_SDR2HDR,

     GLES2_NUM_PROGRAMS
} GLES2ProgramIndex;



///////////////////////////////////////////////////////


/* global variables */

//static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = 0;
static PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = 0;
static PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = 0;
static PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = 0;

static EGLDisplay eglDisplay = EGL_NO_DISPLAY;
static EGLConfig  eglConfig  = 0;
static EGLSurface eglSurface = EGL_NO_SURFACE;
static EGLContext eglContext = EGL_NO_CONTEXT;

static EGLDisplay eglDisplayPrev = EGL_NO_DISPLAY;
static EGLSurface eglDrawSurfacePrev = EGL_NO_SURFACE;
static EGLSurface eglReadSurfacePrev = EGL_NO_SURFACE;
static EGLContext eglContextPrev  = EGL_NO_CONTEXT;

#if SHARE_CONTEXT
static EGLContext eglShareContext = EGL_NO_CONTEXT;
#endif

EGLImageKHR eglImage_src = 0;
EGLImageKHR eglImage_dst = 0;

mst_native_buffer_t  *dst_mst_buffer = NULL;
mst_native_buffer_t  *src_mst_buffer = NULL;

static GLuint src_texture       = 0;
static GLuint dest_texture      = 0;
static GLuint fbo               = 0;
static GLuint vertex_buffer     = 0;

static GLuint gles2_progs[GLES2_NUM_PROGRAMS];

EGLint contextAttrs[] =
{
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
};

static void (*blit_func[GLES2_NUM_PROGRAMS])( GLESBlitInfo *info) = {0};


static bool EGL_Init(void);


////////////////////////////////////////////////////////
#ifdef USE_BINARY_SHADER_PROGRAM
static bool saveShaderProgramBinary(GLuint prog_obj)
{
    /* get the program binary length */
    int len = 0;
    char *buffer = NULL;
    FILE *fp = NULL;

    glGetProgramiv(prog_obj, GL_PROGRAM_BINARY_LENGTH_OES, &len);

    /* fill the first 4 bytes of the buffer with the program binary type and
       the rest with the program binary */
    buffer = D_MALLOC(sizeof(char)* (len + 4));
    glGetProgramBinaryOES(prog_obj, len, &len, (GLenum*)buffer, buffer + 4);

    /* save to file */
    fp = fopen(BINARY_SHADER_PROGRAM, "w");

    if (fp == NULL)    {
        printf("failed to save file shader_bin/program.bin");
        D_FREE(buffer);
        return false;
    }

    fwrite(buffer, 1, len + 4, fp);

    fclose(fp);
    D_FREE(buffer);

    return true;
}

static bool loadShaderProgramBinary(GLuint prog_obj)
{
    int len = 0;
    char *buffer = NULL;
    char filepath[64];
    FILE *fp = NULL;

    sprintf(filepath, "%s/%s",  direct_config->module_dir, BINARY_SHADER_PROGRAM);
    /* read the file, the first 4 bytes are the program binary type, the rest are the program binary */
    fp = fopen(filepath, "r");

    if (fp == NULL)    {
        printf("failed to load file shader_bin/program.bin");
        return false;
    }

    DBG_GLES2_MSG("loadShaderProgramBinary, %s\n", filepath);

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = D_MALLOC(sizeof(char) * (len+1));
    len = fread(buffer, 1, len, fp);
    buffer[len] = 0;
    fclose(fp);

    /* set the program binary */
    glProgramBinaryOES(prog_obj, ((GLenum*)buffer)[0], buffer + 4, len - 4);

    D_FREE( buffer );

    DBG_GLES2_MSG("loadShaderProgramBinary, done!\n");
    return true;
}

#else

#if defined( USE_BINARY_FRAGMENT_SHADER ) || defined( USE_BINARY_VERTEX_SHADER )
static bool loadShaderBinary(GLuint prog_obj, const char *shader_binary, GLenum type)
{
    int size = 0;
    char *buffer = NULL;
    char filepath[64];
    FILE *fp = NULL;
    GLuint shader;

    sprintf(filepath, "%s/%s",  direct_config->module_dir, shader_binary);
    /* read the file, the first 4 bytes are the program binary type, the rest are the program binary */
    fp = fopen(filepath, "r");

    if (fp == NULL)    {
        printf("failed to open %s\n", filepath);
        return false;
    }

    DBG_GLES2_MSG("[DFB] mstar_gles2, loadShaderBinary, %s\n", filepath);

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = D_MALLOC( size );

    if (buffer == NULL) {
        printf("can not alloc mem for binary shader\n");
        fclose(fp);
        return false;
    }

    fread(buffer, sizeof(char), size, fp);
    fclose(fp);

    shader = glCreateShader(type);
    glShaderBinary(1, &shader, GL_MALI_SHADER_BINARY_ARM, buffer, size);
    glAttachShader(prog_obj, shader);
    glDeleteShader(shader);

    D_FREE( buffer );

    DBG_GLES2_MSG("loadShaderProgramBinary, done!\n");
    return true;
}
#endif
#endif

static void init_buffers(void)
{
    const GLfloat vertex_data[] =
    {
        /* x, y, u, v */
        -1.0f, -1.0f, 0.0f, 0.0f, /* bottom left    */
        -1.0f,  1.0f, 0.0f, 1.0f, /* top left       */
         1.0f, -1.0f, 1.0f, 0.0f, /* bottom right   */
         1.0f,  1.0f, 1.0f, 1.0f, /* top right      */
    };

    //DBG_GLES2_MSG("init_buffers start!\n");

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(GLES2VA_POSITIONS);
    glEnableVertexAttribArray(GLES2VA_TEXCOORDS);
    glVertexAttribPointer(GLES2VA_POSITIONS, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);
    glVertexAttribPointer(GLES2VA_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, (void*)(sizeof(GLfloat)*2));
    //DBG_GLES2_MSG("init_buffers end!\n");

}


static void init_buffers_by_value(const float tex_u, const float tex_v)
{
    const GLfloat vertex_data[] =
    {
        /* x, y, u, v */
        -1.0f, -1.0f, 0.0f, 0.0f, /* bottom left    */
        -1.0f,  1.0f, 0.0f, tex_v, /* top left       */
         1.0f, -1.0f, tex_u, 0.0f, /* bottom right   */
         1.0f,  1.0f, tex_u, tex_v, /* top right      */
    };

    //DBG_GLES2_MSG("init_buffers start!\n");

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(GLES2VA_POSITIONS);
    glEnableVertexAttribArray(GLES2VA_TEXCOORDS);
    glVertexAttribPointer(GLES2VA_POSITIONS, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);
    glVertexAttribPointer(GLES2VA_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, (void*)(sizeof(GLfloat)*2));
    //DBG_GLES2_MSG("init_buffers end!\n");

}

static void init_buffers_by_calculate(const float rx1, const float ry1, const float rx2, const float ry2,
                                      const float tx1, const float ty1, const float tx2, const float ty2)
{
    const GLfloat vertex_data[] =
    {
        /* x, y, u, v */
        rx1, ry1, tx1, ty1, /* bottom left    */
        rx1, ry2, tx1, ty2, /* top left       */
        rx2, ry1, tx2, ty1, /* bottom right   */
        rx2, ry2, tx2, ty2, /* top right      */
    };

    //DBG_GLES2_MSG("init_buffers start!\n");

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(GLES2VA_POSITIONS);
    glEnableVertexAttribArray(GLES2VA_TEXCOORDS);
    glVertexAttribPointer(GLES2VA_POSITIONS, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);
    glVertexAttribPointer(GLES2VA_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, (void*)(sizeof(GLfloat)*2));
    //DBG_GLES2_MSG("init_buffers end!\n");

}


static void init_textures(void)
{

    DBG_GLES2_MSG("SDR2HDR_init_textures first init!\n");

    glGenFramebuffers(1, &fbo);

    /* texture of destination buffer for render-to-texture */
    glGenTextures(1, &dest_texture);
    glBindTexture(GL_TEXTURE_2D, dest_texture);
    if (dfb_config->mst_layer_up_scaling || dfb_config->mst_GPU_window_compose) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* texture of source buffer */
    glGenTextures(1, &src_texture);
    glBindTexture(GL_TEXTURE_2D, src_texture);
    if (dfb_config->mst_layer_up_scaling || dfb_config->mst_GPU_window_compose) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifdef SDR2HDR
    DBG_GLES2_MSG("%s, %d\n", __FUNCTION__, __LINE__);

    glGenTextures(3, tex_lut);
    /* texture which stores de-gamma output */
    {
        const unsigned int* tex_data_degamma = (const unsigned int*)degamma_data;
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_lut[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, NUM_DEGAMMA_STEPS, 1, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, (void*)tex_data_degamma);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    /* texture which stores PQ output */
    {
        unsigned short tex_data_pq[PQ_TABLE_HEIGHT][PQ_TABLE_WIDTH];
        int row = 0;
        int step;
        memset(tex_data_pq, 0, sizeof(tex_data_pq));
        while (step < NUM_PQ_STEPS)        {
            int column = 0;
            step = (PQ_TABLE_WIDTH - 1) * row;
            /* the first step of each row is the last step of the previous row */
            while ((step < NUM_PQ_STEPS) && (column < PQ_TABLE_WIDTH))            {
                tex_data_pq[row][column] = pq_data[step];
                column++;
                step++;
            }
            row++;
        }
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tex_lut[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, PQ_TABLE_WIDTH, PQ_TABLE_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, (void*)tex_data_pq);
        /* use texture load to do the interpolation */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
#ifdef USE_TMO
    /* texture which stores TMO output */
    {
        const unsigned short* tex_data_tmo = (const unsigned short*)tmo_data;
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, tex_lut[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TMO_TABLE_WIDTH, TMO_TABLE_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, (void*)tex_data_tmo);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        /* use texture load to do the interpolation */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
#endif
#endif

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

}


/*
 * Create program shader objects for sharing across all EGL contexts.
 */

static bool init_shader(GLuint prog_obj,
                        const char *prog_src,
                        GLenum type )
{
     char  *log;
     GLuint shader;
     GLint  status, log_length, char_count;
     GLint  sourceLen;
     bool ret = false;

     sourceLen = strlen( prog_src );

     shader = glCreateShader(type);
     glShaderSource(shader, 1, (const char**)&prog_src, &sourceLen);
     glCompileShader(shader);

     glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
     if (status) {
          glAttachShader(prog_obj, shader);
          ret = true;
     }
     else {
          glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
          log = D_MALLOC(log_length);

          glGetShaderInfoLog(shader, log_length, &char_count, log);
          printf("\033[1;31mGLES2/Driver: shader (type = 0x%x, len = %d)  compilation failure:\n%s \033[0m\n", type, sourceLen, log);
          D_FREE(log);

          glDeleteShader(shader);
     }

     return ret;
}

static bool init_program(GLuint prog_obj,
                         char *vert_prog_name,
                         const char *vert_prog_src,
                         char *frag_prog_name,
                         const char *frag_prog_src )
{
     GLint status;
     bool ret = false;

     if (!init_shader(prog_obj, vert_prog_src, GL_VERTEX_SHADER)) {
          printf("\033[1;31mGLES2/Driver: %s failed to compile!\033[0m\n", vert_prog_name);
          return ret;
     }

     if (!init_shader(prog_obj, frag_prog_src, GL_FRAGMENT_SHADER)) {
          printf("\033[1;31mGLES2/Driver: %s failed to compile!\033[0m\n", frag_prog_name);
          return ret;
     }

     /* Link the program object and check for errors. */
     glLinkProgram(prog_obj);
     glValidateProgram(prog_obj);
     glGetProgramiv(prog_obj, GL_LINK_STATUS, &status);

     if (status) {
          /* Don't need the shader objects anymore. */
          GLuint  shaders[2];
          GLsizei shader_count;

          glGetAttachedShaders(prog_obj, 2, &shader_count, shaders);

          glDetachShader(prog_obj, shaders[0]);
          glDetachShader(prog_obj, shaders[1]);
          glDeleteShader(shaders[0]);
          glDeleteShader(shaders[1]);

          ret = true;
     }
     else {
          char *log;
          GLint  log_length, char_count;
          /* Report errors.  Shader objects detached when program is deleted. */
          glGetProgramiv(prog_obj, GL_INFO_LOG_LENGTH, &log_length);
          log = D_MALLOC(log_length);

          glGetProgramInfoLog(prog_obj, log_length, &char_count, log);
          printf("\033[1;31mGLES2/Driver: shader program link failure:\n%s \033[0m\n", log);
          D_FREE(log);
     }

     return ret;
}

static bool init_program_binary(GLuint prog_obj,
                                char *vert_prog_name,
                                const char *vert_prog_src,
                                char *frag_prog_name,
                                const char *frag_prog_src )
{
    GLint status;
    bool ret = false;

#ifdef USE_BINARY_VERTEX_SHADER
    if (!loadShaderBinary(prog_obj, vert_prog_src, GL_VERTEX_SHADER)) {
        printf("\033[1;31mGLES2/Driver: failed to load binary shader : %s!\033[0m\n", vert_prog_name);
        return ret;
    }
#else
    if (!init_shader(prog_obj, vert_prog_src, GL_VERTEX_SHADER)) {
        printf("\033[1;31mGLES2/Driver: %s failed to compile!\033[0m\n", vert_prog_name);
        return ret;
    }
#endif

#ifdef USE_BINARY_FRAGMENT_SHADER
    if (!loadShaderBinary(prog_obj, frag_prog_src, GL_FRAGMENT_SHADER)) {
        printf("\033[1;31mGLES2/Driver: failed to load binary shader : %s!\033[0m\n", frag_prog_name);
        return ret;
    }
#else
    if (!init_shader(prog_obj, frag_prog_src, GL_FRAGMENT_SHADER)) {
        printf("\033[1;31mGLES2/Driver: %s failed to compile!\033[0m\n", frag_prog_name);
        return ret;
    }
#endif
    /* Link the program object and check for errors. */
    glLinkProgram(prog_obj);
    glValidateProgram(prog_obj);
    glGetProgramiv(prog_obj, GL_LINK_STATUS, &status);

    if (status) {
         /* Don't need the shader objects anymore. */
         GLuint  shaders[2];
         GLsizei shader_count;

         glGetAttachedShaders(prog_obj, 2, &shader_count, shaders);

         glDetachShader(prog_obj, shaders[0]);
         glDetachShader(prog_obj, shaders[1]);

         ret = true;
    }
    else {
         char *log;
         GLint log_length, char_count;
         /* Report errors.  Shader objects detached when program is deleted. */
         glGetProgramiv(prog_obj, GL_INFO_LOG_LENGTH, &log_length);
         log = D_MALLOC(log_length);

         glGetProgramInfoLog(prog_obj, log_length, &char_count, log);
         printf("\033[1;31mGLES2/Driver: shader program link failure:\n%s \033[0m\n", log);
         D_FREE(log);
    }

    return ret;
}

static bool init_shader_programs()
{
     int i;
     GLuint prog;
     bool status;

     DBG_GLES2_MSG("[%s]  start!\n", __FUNCTION__);

     /* for blit */
     prog = glCreateProgram();
     status = init_program(prog, "blit_vert", vert_src_blit,
                                 "blit_frag", frag_src_blit);
     if (status) {

          gles2_progs[GLES2_BLIT] = prog;

          DBG_GLES2_MSG("[%s] -> created blit program OK!\n", __FUNCTION__);
     }
     else {
          printf("\033[1;31m[%s] ->  blit shader program failed!\033[0m\n", __FUNCTION__);
          goto FAILED;
     }

#ifdef SDR2HDR
     /* for sdr to hdr */
     prog = glCreateProgram();
#ifdef USE_BINARY_SHADER_PROGRAM
     /* load from binary shader program file. */
     status = loadShaderProgramBinary(prog);
#else
     status = init_program_binary(prog, "sdr_to_hdr_vex",
#ifdef USE_BINARY_VERTEX_SHADER
                                        BINARY_VERTEX_SHADER,
#else
                                        vert_src_blit,
#endif /* end of USE_BINARY_VERTEX_SHADER */
                                        "sdr_to_hdr_frag",
#ifdef USE_BINARY_FRAGMENT_SHADER
                                        BINARY_FRAGMENT_SHADER );
#else
                                        frag_src_sdr2hdr);
#endif /* end of USE_BINARY_FRAGMENT_SHADER */

#endif /* end of USE_BINARY_SHADER_PROGRAM */

     if (status) {

          gles2_progs[GLES2_SDR2HDR] = prog;

          DBG_GLES2_MSG("[%s] -> created SDR to HDR program OK!\n", __FUNCTION__);
     }
     else {
          printf("\033[1;31m[%s] -> SDR to HDR shader program failed!\033[0m\n", __FUNCTION__);
          goto FAILED;
     }
#endif
     DBG_GLES2_MSG("[%s] end!\n", __FUNCTION__);

     return true;

FAILED:

     /* Delete all program objects.  glDeleteProgram() will ignore object id 0. */
     //for (i = 0; i < GLES2_NUM_PROGRAMS; i++)
         //glDeleteProgram(gles2_progs[i]);

     return false;
}

static void EGL_Destroy()
{
    DBG_GLES2_MSG("[%s][pid=%d][tid=%d]\n", __FUNCTION__, getpid(), syscall(SYS_gettid) );

    
    if (0 != src_texture)
    {
        glDeleteTextures(1, &src_texture);
        src_texture = 0;
    }
    if (0 != dest_texture)
    {
        glDeleteTextures(1, &dest_texture);
        dest_texture = 0;
    }
    if (0 != fbo)
    {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }

    if (0 != gles2_progs[GLES2_BLIT])
    {
        glDeleteProgram(gles2_progs[GLES2_BLIT]);
        gles2_progs[GLES2_BLIT] = 0;    
    }

    if (EGL_NO_DISPLAY != eglDisplay)
    {
        DBG_GLES2_MSG("[%s][pid=%d][tid=%d]\n", __FUNCTION__, getpid(), syscall(SYS_gettid) );

        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (EGL_NO_CONTEXT != eglContext)
        {
            eglDestroyContext(eglDisplay, eglContext);
            eglContext = EGL_NO_CONTEXT;
        }
#if SHARE_CONTEXT
        if (EGL_NO_CONTEXT != eglShareContext)
        {
            eglDestroyContext(eglDisplay, eglShareContext);
            eglShareContext = EGL_NO_CONTEXT;
        }
#endif
        if (EGL_NO_SURFACE != eglSurface)
        {
            eglDestroySurface(eglDisplay, eglSurface);
            eglSurface = EGL_NO_SURFACE;
        }

        eglTerminate(eglDisplay);
        eglConfig = NULL;
        eglDisplay = EGL_NO_DISPLAY;
    }
	
}

static mst_buffer_format_t GL_format(unsigned int dfbFormat)
{
    mst_buffer_format_t gl_format;

    switch(dfbFormat)
    {
    case DSPF_ARGB:
        gl_format = MST_BUFFER_FORMAT_ARGB8888;
        break;
    case DSPF_ABGR:
        gl_format = MST_BUFFER_FORMAT_ABGR8888;
        break;
    case DSPF_ARGB4444:
        gl_format =  MST_BUFFER_FORMAT_ARGB4444;
        break;
    default:
        printf("[DFB] GL_format not found!\n");
        gl_format =  MST_BUFFER_FORMAT_ARGB8888;
        break;
    }

    return gl_format;
}

#define FD_SIZE 32

typedef struct {
    u64 phys;
    int fd;
}IOVA_FD;

static IOVA_FD iova_fd[FD_SIZE] = {0}; // fd for iova.

#define SHIFT28 28

static int getfd( u64 phys, unsigned int offset)
{
    u32 i = 0;
    int fd =0;
    bool fd_exist = false;
    /* fbdev needs delete offset to get buffer start addr. */
#if USE_MTK_STI
    u64 halPhys = ((phys << SHIFT28) |offset);
#else
    u64 halPhys = _BusAddrToHalAddr( (phys - offset) );
#endif
    DBG_GLES2_MSG("[DFB] %s, buf phys = %llx, buf offset = %x, iommu halPhys = %llx\n", __FUNCTION__, phys, offset, halPhys);

    for(i=0; i<FD_SIZE; i++) {
        if (iova_fd[i].phys != 0) {
            if (iova_fd[i].phys == halPhys && iova_fd[i].fd != 0) {
                fd_exist = true;
                break;
            }
            else
                continue;
        }
        else {
                break;
        }
    }

    if (i>= FD_SIZE ) {
        i=0;
        if(iova_fd[i].fd > 0) {
            DBG_GLES2_MSG("[DFB] %s, iova_fd is full, close iova_fd[%d] =%d\n", __FUNCTION__, i, iova_fd[i].fd);
            close(iova_fd[i].fd);

            iova_fd[i].phys = 0;
            iova_fd[i].fd = 0;
        }
    }

    if (fd_exist == false) {
        /* Get dma-buf fd by phy */
        if( !dfb_MMA_Get_Buf_Fd( halPhys, &fd)) {
            printf("[DFB] %s, Iova address can't get dmabuf-fd (pid = %d)\n", __FUNCTION__, getpid());
            return -1;
        }
        iova_fd[i].phys = halPhys;
        iova_fd[i].fd = fd;
    }

    return i;
}

static bool bindEGL(EGLImageInfo *info)
{

    DBG_GLES2_MSG("[%s][pid=%d][tid=%d]\n", __FUNCTION__, getpid(), syscall(SYS_gettid) );


    DBG_GLES2_MSG("[%s] w=%d, h=%d, src_pitch: %d, src_phys: 0x%llx, src_format: 0x%x, dst_pitch: %d, dst_phys: 0x%llx, dst_format: 0x%x\n", __FUNCTION__,
                   info->src.width, info->src.height, info->src.pitch, info->src.phys, info->src.format, info->dst.pitch, info->dst.phys, info->dst.format);

    src_mst_buffer = D_MALLOC(sizeof(*src_mst_buffer));
    src_mst_buffer->format = (info->enableAFBC)? MST_BUFFER_FORMAT_ABGR8888 : GL_format(info->src.format);
    src_mst_buffer->width  = info->src.width;
    src_mst_buffer->height = info->src.height;
    src_mst_buffer->pitch  = info->src.pitch;

    if (dfb_MMA_IsIOVA_Address(info->src.phys) == false) {
        src_mst_buffer->flags  = MST_BUFFER_USE_BUS_ADDRESS;
        src_mst_buffer->phys   = info->src.phys;
    }
    else {
        int i = getfd( info->src.phys, info->src.offset);
        if (i < 0)
            return false;

        DBG_GLES2_MSG("[DFB] %s, src using iommu, get fd = %d, src offset = 0x%x\n", __FUNCTION__, iova_fd[i].fd, info->src.offset);
        src_mst_buffer->flags  = MST_BUFFER_USE_FILE_DESCRIPTOR;
        src_mst_buffer->fd[0] = iova_fd[i].fd;
#if USE_MTK_STI
        src_mst_buffer->offset[0] = 0;
#else
        src_mst_buffer->offset[0] = info->src.offset;
#endif
    }
    /* for new mst_native_buffer_type, it needs to give version number. */
    src_mst_buffer->version = MSTAR_NATIVES_VERSION;

    eglImage_src = eglCreateImageKHR(eglDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_MST, (EGLClientBuffer)src_mst_buffer, 0);
    if(EGL_NO_IMAGE_KHR == eglImage_src)
    {
        printf("\33[0;33;44m[%s][pid=%d]\33[0m: eglCreateImageKHR Failed for eglImage_src  ! eglGetError = 0x%x\n", __FUNCTION__, getpid(), eglGetError());
        D_FREE(src_mst_buffer);
        return false;
    }
    DBG_GLES2_MSG("[%s][%d] glGetError = %x\n", __FUNCTION__, __LINE__, glGetError());
    dst_mst_buffer = D_MALLOC(sizeof(*dst_mst_buffer));
    dst_mst_buffer->format = (info->enableAFBC)? MST_BUFFER_FORMAT_ABGR8888 : GL_format(info->dst.format);
    dst_mst_buffer->width  = info->dst.width;
    dst_mst_buffer->height = info->dst.height;
    dst_mst_buffer->pitch  = info->dst.pitch;

    if (dfb_MMA_IsIOVA_Address(info->dst.phys) == false) {
        dst_mst_buffer->flags  = (info->enableAFBC)? (MST_BUFFER_USE_BUS_ADDRESS | MST_BUFFER_FORMAT_AFBC) : MST_BUFFER_USE_BUS_ADDRESS;
        dst_mst_buffer->phys   = info->dst.phys;
    }
    else {
        int i = getfd( info->dst.phys, info->dst.offset );
        if (i < 0)
            return false;

        DBG_GLES2_MSG("[DFB] %s, dst using iommu, get fd = %d, dst offset = 0x%x, AFBC=%d\n", __FUNCTION__, iova_fd[i].fd, info->dst.offset, info->enableAFBC);
        dst_mst_buffer->flags  = (info->enableAFBC)? (MST_BUFFER_USE_FILE_DESCRIPTOR | MST_BUFFER_FORMAT_AFBC) : MST_BUFFER_USE_FILE_DESCRIPTOR;
        dst_mst_buffer->fd[0] = iova_fd[i].fd;
#if USE_MTK_STI
        dst_mst_buffer->offset[0] = 0;
#else
        dst_mst_buffer->offset[0] = info->dst.offset;
#endif
    }
    /* for new mst_native_buffer_type, it needs to give version number. */
    dst_mst_buffer->version = MSTAR_NATIVES_VERSION;

    eglImage_dst = eglCreateImageKHR(eglDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_MST, (EGLClientBuffer)dst_mst_buffer, 0);
    if(EGL_NO_IMAGE_KHR == eglImage_dst)
    {
        printf("\33[0;33;44m[%s][pid=%d]\33[0m: eglCreateImageKHR Failed for eglImage_dst  ! eglGetError = 0x%x\n", __FUNCTION__, getpid(), eglGetError());
        D_FREE(dst_mst_buffer);
        return false;
    }
    DBG_GLES2_MSG("[%s][%d] glGetError = %x\n", __FUNCTION__, __LINE__, glGetError());
    DBG_GLES2_MSG("[%s] eglImage_src = 0x%08x, eglImage_dst = 0x%08x\n", __FUNCTION__, eglImage_src, eglImage_dst);
    DBG_GLES2_MSG("[%s] done !\n", __FUNCTION__);

    /* bind EGL images */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dest_texture);
    DBG_GLES2_MSG("[%s][%d] glGetError = %x\n", __FUNCTION__, __LINE__, glGetError());
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage_dst);
    DBG_GLES2_MSG("[%s][%d] glGetError = %x\n", __FUNCTION__, __LINE__, glGetError());
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    DBG_GLES2_MSG("[%s][%d] glGetError = %x\n", __FUNCTION__, __LINE__, glGetError());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dest_texture, 0);
    DBG_GLES2_MSG("[%s][%d] glGetError = %x\n", __FUNCTION__, __LINE__, glGetError());

    glBindTexture(GL_TEXTURE_2D, src_texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage_src);
    DBG_GLES2_MSG("[%s][%d] glGetError = %x\n", __FUNCTION__, __LINE__, glGetError());

    return glGetError()? false : true;
}

static void unbindEGL()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (EGL_NO_IMAGE_KHR != eglImage_src) {
        //DBG_GLES2_MSG("[%s] destroy eglImage_src !\n", __FUNCTION__);
        eglDestroyImageKHR(eglDisplay, eglImage_src);
        eglImage_src = EGL_NO_IMAGE_KHR;
    }

    if (EGL_NO_IMAGE_KHR != eglImage_dst) {
        //DBG_GLES2_MSG("[%s] destroy eglImage_dst !\n", __FUNCTION__);
        eglDestroyImageKHR(eglDisplay, eglImage_dst);
        eglImage_dst = EGL_NO_IMAGE_KHR;
    }

    if (src_mst_buffer)
        D_FREE(src_mst_buffer);

    if (dst_mst_buffer)
        D_FREE(dst_mst_buffer);

    if (0 != vertex_buffer)
    {
        glDeleteBuffers(1, &vertex_buffer);
        vertex_buffer = 0;
    }
}

static inline void GL_Blend_Func(DFBSurfaceBlendFunction src_blend, DFBSurfaceBlendFunction dst_blend)
{
    GLenum src = GL_ZERO, dst = GL_ZERO;
    switch (src_blend) {
        case DSBF_ZERO:
             break;

        case DSBF_ONE:
             src = GL_ONE;
             break;

        case DSBF_SRCCOLOR:
             src = GL_SRC_COLOR;
             break;

        case DSBF_INVSRCCOLOR:
             src = GL_ONE_MINUS_SRC_COLOR;
             break;

        case DSBF_SRCALPHA:
             src = GL_SRC_ALPHA;
             break;

        case DSBF_INVSRCALPHA:
             src = GL_ONE_MINUS_SRC_ALPHA;
             break;

        case DSBF_DESTALPHA:
             src = GL_DST_ALPHA;
             break;

        case DSBF_INVDESTALPHA:
             src = GL_ONE_MINUS_DST_ALPHA;
             break;

        case DSBF_DESTCOLOR:
             src = GL_DST_COLOR;
             break;

        case DSBF_INVDESTCOLOR:
             src = GL_ONE_MINUS_DST_COLOR;
             break;

        case DSBF_SRCALPHASAT:
             src = GL_SRC_ALPHA_SATURATE;
             break;

        default:
             D_BUG("unexpected src blend function %d", src_blend);
    }

    switch (dst_blend) {
        case DSBF_ZERO:
             break;

        case DSBF_ONE:
             dst = GL_ONE;
             break;

        case DSBF_SRCCOLOR:
             dst = GL_SRC_COLOR;
             break;

        case DSBF_INVSRCCOLOR:
             dst = GL_ONE_MINUS_SRC_COLOR;
             break;

        case DSBF_SRCALPHA:
             dst = GL_SRC_ALPHA;
             break;

        case DSBF_INVSRCALPHA:
             dst = GL_ONE_MINUS_SRC_ALPHA;
             break;

        case DSBF_DESTALPHA:
             dst = GL_DST_ALPHA;
             break;

        case DSBF_INVDESTALPHA:
             dst = GL_ONE_MINUS_DST_ALPHA;
             break;

        case DSBF_DESTCOLOR:
             dst = GL_DST_COLOR;
             break;

        case DSBF_INVDESTCOLOR:
             dst = GL_ONE_MINUS_DST_COLOR;
             break;

        case DSBF_SRCALPHASAT:
             dst = GL_SRC_ALPHA_SATURATE;
             break;

        default:
             D_BUG("unexpected dst blend function %d", dst_blend);
     }

     glBlendFunc(src, dst);
}

static void Blit_GLES2(GLESBlitInfo *info)
{
    DBG_GLES2_MSG("[%s] start!\n", __FUNCTION__);

#if 0
#if FULL_FRAME_UPDATE

    GLfloat  vVertices[] =
    {
        -1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
    };
    GLfloat vTexCoords[] =
    {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

#else

    float pScale_x = 2.0f/info->eglImageInfo.dst.width;
    float pScale_y = 2.0f/info->eglImageInfo.dst.height;

    float tScale_x = 1.0f/info->eglImageInfo.src.width;
    float tScale_y = 1.0f/info->eglImageInfo.src.height;

    float x1 = info->dst_x * pScale_x - 1.0f;
    float y1 = info->dst_y * pScale_y - 1.0f;
    float x2 = (info->rect.w + info->dst_x) * pScale_x - 1.0f;
    float y2 = (info->rect.h + info->dst_y) * pScale_y - 1.0f;

    float tx1 = info->rect.x * tScale_x;
    float ty1 = info->rect.y * tScale_y;
    float tx2 = (info->rect.w + info->rect.x) * tScale_x;
    float ty2 = (info->rect.h + info->rect.y) * tScale_y;


    GLfloat  vVertices[] = {
        x1, y1,
        x2, y1,
        x2, y2,
        x1, y2,
    };
    GLfloat vTexCoords[] =
    {
        tx1, ty1,
        tx2, ty1,
        tx2, ty2,
        tx1, ty2
    };

    DBG_GLES2_MSG("[%s] TexCoords(%f, %f, %fx%f)\n", __FUNCTION__, tx1, ty1, tx2, ty2);

#endif
#endif

    // region scaling up, modify tex coord.
    if (info->eglImageInfo.dst.width != info->drect.w || info->eglImageInfo.dst.height != info->drect.h) {
        if (info->drect.x == 0 && info->drect.y == 0) {
            float tex_u = 1.0f, tex_v = 1.0f;

            tex_u = (float)info->eglImageInfo.dst.width / info->drect.w;
            tex_v = (float)info->eglImageInfo.dst.height / info->drect.h;

            DBG_GLES2_MSG("[DFB] %s, refine tex coord u = %f, v = %f\n", __FUNCTION__, tex_u, tex_v);

            init_buffers_by_value(tex_u, tex_v);
        }
        else if (info->drect.x != 0 || info->drect.y != 0) {
            const float oth_2x = 2.0f;

            float pScale_x = oth_2x / info->eglImageInfo.dst.width;
            float pScale_y = oth_2x / info->eglImageInfo.dst.height;

            float rx1 = info->drect.x * pScale_x - 1.0f;
            float ry1 = info->drect.y * pScale_y - 1.0f;
            float rx2 = (info->drect.w + info->drect.x) * pScale_x - 1.0f;
            float ry2 = (info->drect.h + info->drect.y) * pScale_y - 1.0f;

            float tScale_x = 1.0f / info->eglImageInfo.src.width;
            float tScale_y = 1.0f / info->eglImageInfo.src.height;

            float tx1 = info->srect.x * tScale_x;
            float ty1 = info->srect.y * tScale_y;
            float tx2 = (info->srect.w + info->srect.x) * tScale_x;
            float ty2 = (info->srect.h + info->srect.y) * tScale_y;

            DBG_GLES2_MSG("[DFB] %s, refine coord (%f, %f)-(%f, %f), tex coord (%f, %f)-(%f, %f) \n", __FUNCTION__,
                           rx1, ry1, rx2, ry2, tx1, ty1, tx2, ty2);
            init_buffers_by_calculate(rx1, ry1, rx2, ry2, tx1, ty1, tx2, ty2);
        }
    }
    else // dst.width = drect.w && dst.height = drect.h
        init_buffers();


    /* use blit Program*/
    glUseProgram(gles2_progs[GLES2_BLIT]);

    glUniform1i(glGetUniformLocation(gles2_progs[GLES2_BLIT], "sampler"), 0);

    DBG_GLES2_MSG("[%s] viewport : (0, 0, %dx%d)\n", __FUNCTION__, info->eglImageInfo.dst.width, info->eglImageInfo.dst.height);
    glViewport(0, 0, info->eglImageInfo.dst.width, info->eglImageInfo.dst.height);
//#if FULL_FRAME_UPDATE
    //glScissor(0, 0, info->eglImageInfo.dst.width, info->eglImageInfo.dst.height);
//#else
    //glScissor(info->dst_x, info->dst_y, info->rect.w, info->rect.h);
//#endif
    //glEnable(GL_SCISSOR_TEST);

    if (info->dfb_src_blend > 0 && info->dfb_dst_blend > 0) {
        /* GL Blend */
        glEnable(GL_BLEND);

        glBlendEquation(GL_FUNC_ADD);
    
        GL_Blend_Func(info->dfb_src_blend, info->dfb_dst_blend);
        DBG_GLES2_MSG("[%s], src_blend=%d, dst_blend=%d\n", __FUNCTION__, info->dfb_src_blend, info->dfb_dst_blend);
    }
    else
        glDisable(GL_BLEND);

    // Clear the color buffer
    //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glClear(GL_COLOR_BUFFER_BIT);
    //DBG_GLES2_MSG("[%s][%d] glClear, glGetError = %x\n", __FUNCTION__, __LINE__, glGetError());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    DBG_GLES2_MSG("[%s][%d] glDrawArrays, glGetError = %x\n", __FUNCTION__, __LINE__, glGetError());

    DBG_GLES2_MSG("[%s] done!\n", __FUNCTION__);

}

#ifdef SDR2HDR
/*
    blit with SDR to HDR
*/
static void Blit_SDR2HDR(GLESBlitInfo *info)
{
    float nits = info->coeff.nits;
#ifdef USE_TMO
    static const int predefined_nits[NUM_TMO_TABLES] = { 50, 100, 150, 200, 300, 400, 500, 600, 700, 800, 900, 1000 };
    int tmo_index;

    for (tmo_index = 0; tmo_index < NUM_TMO_TABLES; tmo_index++) {
        //if ( info->coeff.nits == predefined_nits[tmo_index]) {
        if ( info->coeff.nits < predefined_nits[tmo_index] ) {
            tmo_index --;
            break;
        }
    }

    if (tmo_index >= NUM_TMO_TABLES || tmo_index < 0)        {
        /* default to table 1 (nits 100) if no matching nits */
        tmo_index = 1;
        nits = predefined_nits[tmo_index];
    }

    DBG_GLES2_MSG("tmo_index=%d\n", tmo_index);
#endif
    DBG_GLES2_MSG("[%s][pid=%d] start!\n", __FUNCTION__, getpid());

    init_buffers();

    if (nits == 0)
        nits = 600;

    DBG_GLES2_MSG("[%s][pid=%d]  nits=%f, dst.width=%d, dst.height=%d\n", __FUNCTION__, getpid(), nits, info->eglImageInfo.dst.width, info->eglImageInfo.dst.height);


    /* bind EGL images */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dest_texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)eglImage_dst);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dest_texture, 0);

    glBindTexture(GL_TEXTURE_2D, src_texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)eglImage_src);

    /* use Program */
    glUseProgram(gles2_progs[GLES2_SDR2HDR]);

    /* set render option */
    glUniform1f(glGetUniformLocation(gles2_progs[GLES2_SDR2HDR], "nits"),  nits/10000.0f);

    glUniform1i(glGetUniformLocation(gles2_progs[GLES2_SDR2HDR], "sampler_degamma"), 1);
    glUniform1i(glGetUniformLocation(gles2_progs[GLES2_SDR2HDR], "sampler_pq"), 2);

#ifdef USE_TMO
    glUniform1f(glGetUniformLocation(gles2_progs[GLES2_SDR2HDR], "tmo_index"),  (float)tmo_index);
    glUniform1i(glGetUniformLocation(gles2_progs[GLES2_SDR2HDR], "sampler_tmo"), 3);
#endif

    glViewport(0, 0, info->eglImageInfo.dst.width, info->eglImageInfo.dst.height);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glFinish();

    DBG_GLES2_MSG("[%s] done!\n", __FUNCTION__);
}
#endif

////////////////////// DEFINE PUBLIC INTERFACE ///////////////////////////

//#ifdef VISIBILITY_HIDDEN
//#pragma GCC visibility push(default)
//#endif  //VISIBILITY_HIDDEN

static pthread_mutex_t  gles_lock = PTHREAD_MUTEX_INITIALIZER;

static bool EGL_Init(void)
{
    EGLint iMajorVersion, iMinorVersion;
    unsigned int uiRet = 0;
    bool ret = false;

    EGLint configAttribs[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_NONE
    };

    EGLint iConfigs = 0;

    EGLint attributes[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        //EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
        //EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
        EGL_NONE
    };

   
    /* check eglDisplay is init or not. */

    DBG_GLES2_MSG("[%s][pid=%d] start!\n", __FUNCTION__, getpid());

    /* if not support eglGetPlatformDisplayEXT, then use eglGetDisplay */
    eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
    if( eglGetPlatformDisplayEXT )
    {
#ifdef MTK_DTV_DEFAULT_DISPLAY_ID_NULL
        eglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_NULL_MST, MTK_DTV_DEFAULT_DISPLAY_ID_NULL, NULL);
#else
        eglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_NULL_MST, EGL_DEFAULT_DISPLAY, NULL);
#endif
    }
    else
    {
        eglDisplay = eglGetDisplay((EGLNativeDisplayType) EGL_DEFAULT_DISPLAY);
    }

    if(eglDisplay == EGL_NO_DISPLAY)
    {
        printf("\033[1;31m[%s][pid=%d]: eglDisplay ERROR = 0x%x ! \033[0m\n", __FUNCTION__, getpid(), eglGetError());
        return false;
    }

    uiRet = eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion);
    if(uiRet != EGL_TRUE)
    {
        printf("\033[1;31m[%s][pid=%d]: eglInitialize ERROR = 0x%x ! \033[0m\n", __FUNCTION__, getpid(), eglGetError());
        return false;
    }

    eglBindAPI(EGL_OPENGL_ES_API);

    uiRet = eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &iConfigs);
    if(uiRet != EGL_TRUE)
    {
        printf("\033[1;31m[%s][pid=%d]: eglChooseConfig ERROR = 0x%x !  \033[0m\n", __FUNCTION__, getpid(), eglGetError());
        return false;
    }

    eglSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, attributes);
    if(eglSurface == EGL_NO_SURFACE)
    {
        printf("\033[1;31m[%s][pid=%d]: eglCreatePbufferSurface ERROR = 0x%x! \033[0m\n", __FUNCTION__, getpid(), eglGetError());
        return false;
    }

    eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, contextAttrs);
    if(eglContext == EGL_NO_CONTEXT)
    {
        printf("\033[1;31m[%s][pid=%d]: eglCreateContext ERROR = 0x%x!  \033[0m\n", __FUNCTION__, getpid(), eglGetError());
        return false;
    }

    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

    if(!eglCreateImageKHR)
    {
        eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
        assert(eglCreateImageKHR);
    }

    if(!eglDestroyImageKHR)
    {
        eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
        assert(eglDestroyImageKHR);
    }

    DBG_GLES2_MSG("[%s][pid=%d][tid=%d] eglDisplay: 0x%08x, eglSurface: 0x%08x, eglContext: 0x%08x\n", 
                  __FUNCTION__, getpid(), syscall(SYS_gettid), eglDisplay, eglSurface, eglContext );

    DBG_GLES2_MSG("[%s] done! \n", __FUNCTION__);

    /* init all shader program */
    ret = init_shader_programs();
    if (ret == false) {
         printf("[%s] GLES2: Could not create shader program objects!\n", __FUNCTION__);
         return false;
    }

    /* init texture attr setting */
    init_textures();

    /* setting blit func */
    //blit_func[GLES2_BLIT]     =  Blit_GLES2;
    //blit_func[GLES2_SDR2HDR]  = Blit_SDR2HDR;

    /* using for multi-thread, need to release context for the other's used. */
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    return true;
}

static bool GL_blit(GLESBlitInfo *info)
{
    bool ret = true;

    DBG_GLES2_MSG("[%s]  eglImageInfo src(%d, %d), dest(%d, %d), srect(%d, %d, %d x %d), drect(%d, %d, %d x %d), SDR2HDR enable : %d\n",
        __FUNCTION__,
        info->eglImageInfo.src.width, info->eglImageInfo.src.height,
        info->eglImageInfo.dst.width, info->eglImageInfo.dst.height,
        info->srect.x, info->srect.y, info->srect.w, info->srect.h,
        info->drect.x, info->drect.y, info->drect.w, info->drect.h,        
        info->coeff.en_sdr2hdr);

    /* enable sdr2hdr. back to back copy.
    if (info->coeff.en_sdr2hdr) {
        info->eglImageInfo.dst.width = info->eglImageInfo.src.width;
        info->eglImageInfo.dst.height = info->eglImageInfo.src.height;
        info->eglImageInfo.dst.phys = info->eglImageInfo.src.phys;
        info->eglImageInfo.dst.pitch = info->eglImageInfo.src.pitch;
    }*/

    if (bindEGL(&info->eglImageInfo)) {
        //blit_func[info->coeff.en_sdr2hdr](info);
        Blit_GLES2(info);
    }
    else {
        printf("[DFB] GL_blit, bindEGL failed!\n");
        ret = false;
    }

    unbindEGL();

    return ret;
}

static void GL_MakeCurrent(void **shareContext)
{
    int result = EGL_SUCCESS;

    pthread_mutex_lock( &gles_lock );

    eglDisplayPrev = eglGetCurrentDisplay();
    eglDrawSurfacePrev = eglGetCurrentSurface(EGL_DRAW);
    eglReadSurfacePrev = eglGetCurrentSurface(EGL_READ);
    eglContextPrev = eglGetCurrentContext();
    if (eglDisplayPrev != NULL) {
        DBG_GLES2_MSG("[%s][pid=%d][tid=%d] eglDisplayPrev: %p, eglDrawSurfacePrev: %p, eglReadSurfacePrev: %p, eglContextPrev: %p\n",
            __FUNCTION__, getpid(), syscall(SYS_gettid), eglDisplayPrev, eglDrawSurfacePrev, eglReadSurfacePrev, eglContextPrev);
    }

    if (eglDisplay == EGL_NO_DISPLAY || eglSurface == EGL_NO_SURFACE || eglContext == EGL_NO_CONTEXT) {
        printf("\033[1;31m[%s][pid=%d][tid=%d]: eglDisplay is NO DISPLAY! call EGL_Init! \033[0m\n", __FUNCTION__, getpid(), syscall(SYS_gettid));

        if (!EGL_Init()) {
            printf("\033[1;31m[%s][pid=%d][tid=%d]: EGL_Init() failed! \033[0m\n", __FUNCTION__, getpid(), syscall(SYS_gettid));
        }

    }

#if SHARE_CONTEXT

    eglShareContext = eglCreateContext(eglDisplay, eglConfig, eglContext, contextAttrs);

    DBG_GLES2_MSG("[%s][pid=%d][tid=%d] eglDisplay: %p, eglSurface: %p, eglContext: %p, shareContext: %p\n",
            __FUNCTION__, getpid(), syscall(SYS_gettid), eglDisplay, eglSurface, eglContext, eglShareContext);

    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglShareContext);
    result = eglGetError();
    if (result != EGL_SUCCESS) {
        printf("\033[1;31m[%s][%d][pid=%d][tid=%d] eglMakeCurrent, getError = 0x%x\033[0m\n", __FUNCTION__, __LINE__, getpid(), syscall(SYS_gettid), result);
    }

    *shareContext = eglShareContext;

#else

    DBG_GLES2_MSG("[%s][pid=%d][tid=%d] eglDisplay: %p, eglSurface: %p, eglContext: %p\n",
                  __FUNCTION__, getpid(), syscall(SYS_gettid), eglDisplay, eglSurface, eglContext);

    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    result = eglGetError();
    if (result != EGL_SUCCESS) {
        printf("\33[0;33;44m[%s][%d][pid=%d][tid=%d] eglMakeCurrent, getError = 0x%x\33[0m\n", __FUNCTION__, __LINE__, getpid(), syscall(SYS_gettid), result);
        EGL_Destroy();

        EGL_Init();
    }

#endif

}

static void GL_finish(void *shareContext)
{
    glFinish();
    DBG_GLES2_MSG("[%s][%d] glFinish, glGetError = %x\n", __FUNCTION__, __LINE__, glGetError());

    /* using for multi-thread, need to release context for the other's used. */
    if (eglDisplayPrev != NULL) {
        DBG_GLES2_MSG("[DFB][%s %d][pid=%d][tid=%d]\n", __FUNCTION__, __LINE__, getpid(), syscall(SYS_gettid));
        eglMakeCurrent(eglDisplayPrev, eglDrawSurfacePrev, eglReadSurfacePrev, eglContextPrev);
        eglDisplayPrev = EGL_NO_DISPLAY;
        eglDrawSurfacePrev = EGL_NO_SURFACE;
        eglReadSurfacePrev = EGL_NO_SURFACE;
        eglContextPrev  = EGL_NO_CONTEXT;
    } else {
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }

#if SHARE_CONTEXT
    DBG_GLES2_MSG("[%s][pid=%d][tid=%d] shareContext: %p to destroy, the global share context: %p\n",
                  __FUNCTION__, getpid(), syscall(SYS_gettid), shareContext, eglShareContext);
    if (EGL_NO_CONTEXT != eglShareContext)
    {
        eglDestroyContext(eglDisplay, eglShareContext);
        eglShareContext = EGL_NO_CONTEXT;
    }
#endif

    DBG_GLES2_MSG("[%s][pid=%d][tid=%d] done!\n", __FUNCTION__, getpid(), syscall(SYS_gettid));

    for(int i=0; i< FD_SIZE; i++) {
        if(iova_fd[i].fd > 0) {
            DBG_GLES2_MSG("[DFB] %s, close iova_fd[%d] =%d\n", __FUNCTION__, i, iova_fd[i].fd);
            close(iova_fd[i].fd);

            iova_fd[i].phys = 0;
            iova_fd[i].fd = 0;
        }
    }

    pthread_mutex_unlock( &gles_lock );
}

static SDR2HDRParameter *shareData = NULL;

/*
    for CFDMonitor to assign SDR2HDRCoeff.
*/
void  WriteToShareData(SDR2HDRCoeff *coeff)
{
    MutexLock *lock_sdr2hdr = (MutexLock *)shareData->lock;

    printf("\033[1;31m[%s], share Data addr: %x, en_gles2 = %d, en_sdr2hdr = %d, nits = %f\033[0m\n", __FUNCTION__, shareData, coeff->en_gles2, coeff->en_sdr2hdr, coeff->nits);

    SDR2HDR_MUTEX_LOCK_MASTER(lock_sdr2hdr);
    memcpy(&shareData->coeff, coeff, sizeof(SDR2HDRCoeff));
    *(shareData->ptr_en_gles2) = shareData->coeff.en_gles2;
    SDR2HDR_MUTEX_UNLOCK(lock_sdr2hdr);

}

/*
    for share mem to save SDR2HDRParameter.
*/
static void initSDR2HDRParameter(void *Data)
{
    DBG_GLES2_MSG("%s, share Data addr : %x\n", __FUNCTION__, Data);
    shareData = (SDR2HDRParameter *)Data;
    /* if default to enable SDR2HDR, needs to enable shareData->ptr_en_gles2. */
    //*(shareData->ptr_en_gles2) = true;
}

bool GLES_Func_init(GLESFuncs *funcs)
{
    bool ret = true;

    /* Init EGL in first Make Current. */
    funcs->GLES2Init = NULL; //EGL_Init;

    funcs->GLES2Blit = GL_blit;

    funcs->InitSDR2HDRParameter = initSDR2HDRParameter;

    funcs->GLES2Finish = GL_finish;

    funcs->GLES2Destroy = EGL_Destroy;

    funcs->GLES2MakeCurrent = GL_MakeCurrent;

    return ret;
}

//#ifdef VISIBILITY_HIDDEN
//#pragma GCC visibility pop
//#endif  //VISIBILITY_HIDDEN

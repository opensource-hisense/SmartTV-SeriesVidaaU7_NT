/*
   (c) Copyright 2000-2002  convergence integrated media GmbH.
   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de> and
              Sven Neumann <neo@directfb.org>.

   This file is subject to the terms and conditions of the MIT License:

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <directfb.h>
#include <stdio.h>

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...)                                                    \
     {                                                                    \
          err = x;                                                        \
          if (err != DFB_OK) {                                            \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );     \
               DirectFBErrorFatal( #x, err );                             \
          }                                                               \
     }


/* define structure */
typedef struct _Node {
     struct Node * next;
     IDirectFBSurface * data; 
} Node;


/* define global variable */
static IDirectFB * dfb;
static unsigned int surface_cnt = 0;
static Node* pHeadNode = NULL;
static Node* pCurNode = NULL;


/*
 * push data to link-list
 */
static void push(IDirectFBSurface * data)
{
    if (pHeadNode == NULL)
    {
         pHeadNode = (Node*)malloc(sizeof(Node));
         
         pHeadNode->next = NULL;
         pHeadNode->data = data;
         
         pCurNode = pHeadNode;
    }
    else
    {
        pCurNode->next = (Node*)malloc(sizeof(Node));
        pCurNode = pCurNode->next;
        
        pCurNode->next = NULL;
        pCurNode->data = data;
    }
}

/*
 * release data in link-list
 */
static void release_all()
{
    Node * cur = NULL;
    Node * tmp = NULL;
    
    cur = pHeadNode;
    IDirectFBSurface * sur = NULL;
    
    while(cur)
    {
        sur = cur->data;
        if (sur) {            
            sur->Release(sur);
        }
        
        tmp = cur;        
        cur = cur->next;
        free(tmp);
    }
    
    pHeadNode = NULL;
}

/*
 * callback function to handle some resource release while OOM occurred
 */
static void CallBack(void * pUserData)
{
    release_all();
    
    if ( *(int*)pUserData != surface_cnt )
        printf("Error\n");
}

/*
 * set up DirectFB 
 */
static void init_resources( int argc, char *argv[] )
{
   DFBResult err;

   DFBCHECK(DirectFBInit( &argc, &argv ));

   /* create the super interface */
   DFBCHECK(DirectFBCreate( &dfb ));
   
   /* step1: register a callback function */
   DirectFBSetOption("notify_callback", (const char*)CallBack);
   
   /* step2: set a threshold of memory usage rate (%) to trigger OOM callback function */
   DirectFBSetOption("usage_threshold", (const char*)80);
   
   /* step3: set a user data to bypass to callback function (optional) */
   DirectFBSetOption("user_data", (const char*)&surface_cnt);
}

/*
 * deinitializes resources and DirectFB
 */
static void deinit_resources()
{
   dfb->Release( dfb );
}

/*
 * allocate a surface buffer with width x height
 */
static void create_surface(int width, int height)
{
    IDirectFBSurface *surface;
    DFBSurfaceDescription dsc;    
    void * ret_ptr;
    int pitch;
    
    dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS ;
    dsc.width = width;
    dsc.height = height;
    dsc.caps = DSCAPS_VIDEOONLY;
    dfb->CreateSurface( dfb, &dsc, &surface );    
    
    surface->Lock(surface, DSLF_WRITE, &ret_ptr, &pitch);
    push(surface);
    surface->Unlock(surface);
    
    surface_cnt++;
}

int main( int argc, char *argv[] )
{
    init_resources( argc, argv );
    
    int w, h;
    w = 1920;
    h = 1080;
    
    while(1) {
        create_surface(w, h);             
    }
            
    deinit_resources();
    return 0;
}

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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <limits>
#include <string>

#include <directfb.h>
#include <directfb_util.h>

using namespace std;
typedef unsigned long U32;
typedef unsigned short U16;



/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                \
          DFBResult err = x;                                                    \
          if (err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, err );                         \
          }                                                           \
     }

/* macro for a safe release to DFB handles */
#define SAFE_RELEASE(PTR) do{ if(PTR) {PTR->Release(PTR);} PTR = 0;}while(0)

/* macro for packing information to ROPTestItem constructor */
#define MAKE_PAIR(X) X, #X

/* macro for doing ROP operation by CPU */
#define ROP2_OP_ZERO(PS, PD) 0
#define ROP2_NOT_PS_OR_PD(PS, PD) (~(PS | PD))
#define ROP2_NS_AND_PD(PS, PD) ((~PS) & PD)
#define ROP2_NS(PS, PD) (~(PS))
#define ROP2_PS_AND_ND(PS, PD) (PS & (~PD))
#define ROP2_ND(PS, PD) (~(PD))
#define ROP2_PS_XOR_PD(PS, PD) ( PS ^ PD)
#define ROP2_NOT_PS_AND_PD(PS, PD) (~(PS & PD))
#define ROP2_PS_AND_PD(PS, PD) (PS & PD)
#define ROP2_NOT_PS_XOR_PD(PS, PD) (~(PS ^ PD))
#define ROP2_PD(PS, PD) PD
#define ROP2_NS_OR_PD(PS, PD) ((~PS) | PD)
#define ROP2_PS(PS, PD) PS
#define ROP2_PS_OR_ND(PS, PD) (PS | (~PD))
#define ROP2_PD_OR_PS(PS, PD) (PD | PS)
#define ROP2_ONE(PS, PD) ROPTestItem<FORMAT, T>::MAX_VALUE


class ROPTestBase
{
public:
    virtual bool DoTest() = 0;
    virtual const char* GetName() const = 0;
    virtual const char* GetPixelFormatName() = 0;
    virtual IDirectFBSurface* GetDestSurface() = 0;
};


template<DFBSurfacePixelFormat FORMAT, typename T>
class ROPTestItem: public ROPTestBase
{
    static const int WIDTH = 20;
    static const int HEIGHT = 20;

    static const int Rsrc = 0x70;
    static const int Gsrc = 0x56;
    static const int Bsrc = 0x17;
    static const int Asrc = 0xFF;

    static const int Rdst = 0x20;
    static const int Gdst = 0x20;
    static const int Bdst = 0x80;
    static const int Adst = 0xFF;

public:

    ROPTestItem(IDirectFB *dfb, DFB_GFX_ROP2_OP rop, const char* name):m_dfb(dfb), m_Rop(rop), m_Name(name)
    {
        assert(dfb != 0);

        DFBSurfaceDescription   desc;
        desc.flags  = (DFBSurfaceDescriptionFlags)( DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS |  DSDESC_PIXELFORMAT );
        desc.width  = WIDTH;
        desc.height = HEIGHT;
        desc.caps   = DSCAPS_VIDEOONLY;
        desc.pixelformat = FORMAT;
        DFBCHECK(dfb->CreateSurface( dfb, &desc, &m_pSurSrc ) );
        DFBCHECK(dfb->CreateSurface( dfb, &desc, &m_pSurDst ) );
    }

    virtual ~ROPTestItem()
    {
        SAFE_RELEASE(m_pSurSrc);
        SAFE_RELEASE(m_pSurDst);
    }

    bool DoTest()
    {
        int pitch = 0;
        T * addr = 0;
        T color_src = 0;
        T color_dst = 0;
        T color_output = 0;

        DFBCHECK(m_pSurSrc->Clear( m_pSurSrc , Rsrc, Gsrc , Bsrc, Asrc));
        DFBCHECK(m_pSurDst->Clear( m_pSurDst , Rdst, Gdst, Bdst, Adst));


        /* Get color of Source Surface */
        DFBCHECK(m_pSurSrc->Lock(m_pSurSrc, DSLF_READ, (void**)&addr, &pitch));
        color_src = *addr;
        DFBCHECK(m_pSurSrc->Unlock(m_pSurSrc));

        /* Get color of Dest Surface */
        DFBCHECK(m_pSurDst->Lock(m_pSurDst, DSLF_READ, (void**)&addr, &pitch));
        color_dst = *addr;
        DFBCHECK(m_pSurDst->Unlock(m_pSurDst));

        /* Apply ROP setting and execute */
        DFBCHECK(m_pSurDst->SetROPFlags(m_pSurDst, m_Rop ));
        DFBCHECK(m_pSurDst->Blit( m_pSurDst, m_pSurSrc, 0, 0, 0 ));

        /* Get color of Dest Surface after an operation */
        DFBCHECK(m_pSurDst->Lock(m_pSurDst, DSLF_READ, (void**)&addr, &pitch));
        color_output = *addr;
        DFBCHECK(m_pSurDst->Unlock(m_pSurDst));

        /* Return the test result: true/false means PASS/FAIL */
        return _ResultCheck(color_src, color_dst, color_output);

     }

    const char* GetName() const { return m_Name; }
    const char* GetPixelFormatName() {  return dfb_pixelformat_name(FORMAT);  }
    IDirectFBSurface* GetDestSurface() { return m_pSurDst; }


protected:
    virtual bool _ResultCheck(T clr_src, T clr_dst, T clr_output) = 0;

    static T MAX_VALUE;

private:
    IDirectFB *m_dfb;
    IDirectFBSurface *m_pSurSrc;
    IDirectFBSurface* m_pSurDst;
    DFB_GFX_ROP2_OP m_Rop;
    const char* m_Name;
};

template<DFBSurfacePixelFormat FORMAT, typename T>
T ROPTestItem<FORMAT, T>::MAX_VALUE = std::numeric_limits<T>::max();


#define DECAL_ROP_TEST_CLASS(ROP)\
template<DFBSurfacePixelFormat FORMAT, typename T>\
class TEST_##ROP: public ROPTestItem<FORMAT, T>\
{\
public:\
    explicit TEST_##ROP(IDirectFB * dfb): ROPTestItem<FORMAT, T>(dfb, MAKE_PAIR(DFB_##ROP)){}\
protected:\
    virtual bool _ResultCheck(T clr_src, T clr_dst, T clr_output)\
    {\
        return clr_output == ROP(clr_src, clr_dst);\
    }\
};

DECAL_ROP_TEST_CLASS(ROP2_OP_ZERO)
DECAL_ROP_TEST_CLASS(ROP2_NOT_PS_OR_PD)
DECAL_ROP_TEST_CLASS(ROP2_NS_AND_PD)
DECAL_ROP_TEST_CLASS(ROP2_NS)
DECAL_ROP_TEST_CLASS(ROP2_PS_AND_ND)
DECAL_ROP_TEST_CLASS(ROP2_ND)
DECAL_ROP_TEST_CLASS(ROP2_PS_XOR_PD)
DECAL_ROP_TEST_CLASS(ROP2_NOT_PS_AND_PD)
DECAL_ROP_TEST_CLASS(ROP2_PS_AND_PD)
DECAL_ROP_TEST_CLASS(ROP2_NOT_PS_XOR_PD)
DECAL_ROP_TEST_CLASS(ROP2_PD)
DECAL_ROP_TEST_CLASS(ROP2_NS_OR_PD)
DECAL_ROP_TEST_CLASS(ROP2_PS)
DECAL_ROP_TEST_CLASS(ROP2_PS_OR_ND)
DECAL_ROP_TEST_CLASS(ROP2_PD_OR_PS)
DECAL_ROP_TEST_CLASS(ROP2_ONE)



template<DFBSurfacePixelFormat FORMAT, typename T>
class ROPTestCaseFactory
{
public:

    static void Create(IDirectFB *dfb, vector<ROPTestBase* >& vec)
    {
        /* Register ROP test case */
        vec.push_back(new TEST_ROP2_OP_ZERO<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_NOT_PS_OR_PD<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_NS_AND_PD<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_NS<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_PS_AND_ND<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_ND<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_PS_XOR_PD<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_NOT_PS_AND_PD<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_PS_AND_PD<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_NOT_PS_XOR_PD<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_PD<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_NS_OR_PD<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_PS<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_PS_OR_ND<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_PD_OR_PS<FORMAT, T>(dfb));
        vec.push_back(new TEST_ROP2_ONE<FORMAT, T>(dfb));
    }
};


class TestBench
{
public:

    TestBench(): m_font(0), m_dfb(0), m_layer(0)
     {
        _Init();

        /* Create test cases by different pixel format */
        ROPTestCaseFactory<DSPF_ARGB, U32>::Create(m_dfb, m_vector);
        ROPTestCaseFactory<DSPF_ARGB4444, U16>::Create(m_dfb, m_vector);
        ROPTestCaseFactory<DSPF_YUY2, U16>::Create(m_dfb, m_vector);


        /* Run test case */
        int nums = m_vector.size();

        _Init_layout(nums);

        for(int i=0; i < nums; ++i)
        {
            bool is_passed = m_vector[i]->DoTest();
            const char * pixel_format_name = m_vector[i]->GetPixelFormatName();
            const char * rop_name = m_vector[i]->GetName();

            printf("[%s]  %s (%s)\n", pixel_format_name,  rop_name, is_passed ? "PASS" : "FAIL");

            _Do_layout(i, m_vector[i]->GetDestSurface(), pixel_format_name, rop_name, is_passed ? "PASS" : "FAIL");
        }

        _Show();

     }

    ~TestBench()
     {
        for(int i=0; i< m_vector.size(); i++)
        {
            delete m_vector[i];
            m_vector[i] = 0;
        }

        SAFE_RELEASE(m_window_surface);
        SAFE_RELEASE(m_window);
        SAFE_RELEASE(m_layer);
        SAFE_RELEASE(m_font);
        SAFE_RELEASE(m_dfb);
     }

private:

    static const int WIN_WIDTH = 1280;
    static const int WIN_HEIGHT = 720;

    static const int LAYOUT_COLUMN_NUM = 8;
    static const int LAYOUT_GAP_X = 5;
    static const int LAYOUT_GAP_Y = 5;

    static const int LAYOUT_FONT_WIDTH = 15;
    static const int LAYOUT_FONT_HEIGHT = 15;

    void _Init()
    {
        DFBCHECK(DirectFBCreate( &m_dfb ));
        DFBCHECK(m_dfb->GetDisplayLayer( m_dfb, DLID_PRIMARY, &m_layer ));


        DFBWindowDescription wdesc;
        wdesc.flags  = (DFBWindowDescriptionFlags)( DWDESC_POSX | DWDESC_POSY | DWDESC_SURFACE_CAPS |
                      DWDESC_WIDTH | DWDESC_HEIGHT );
        wdesc.posx   = 0;
        wdesc.posy   = 0;
        wdesc.width  = WIN_WIDTH;
        wdesc.height = WIN_HEIGHT;
        wdesc.surface_caps = DSCAPS_VIDEOONLY;
        DFBCHECK(m_layer->CreateWindow( m_layer, &wdesc, &m_window ) );
        DFBCHECK(m_window->GetSurface( m_window, &m_window_surface ));
        DFBCHECK(m_window_surface->Clear(m_window_surface, 0xff, 0xff, 0xff, 0xff));

    }

    void _Init_layout(int nums)
    {
        if(nums == 0)
            return;

        m_layout_grid_w = WIN_WIDTH / LAYOUT_COLUMN_NUM;
        m_layout_grid_h = WIN_HEIGHT / ( (nums /LAYOUT_COLUMN_NUM) + 1);

        DFBFontDescription desc;
        desc.flags  = (DFBFontDescriptionFlags)( DFDESC_HEIGHT | DFDESC_WIDTH);
        desc.height = LAYOUT_FONT_WIDTH;
        desc.width  = LAYOUT_FONT_HEIGHT;
        DFBCHECK(m_dfb->CreateFont(m_dfb, NULL, &desc, &m_font ));
        DFBCHECK(m_font->GetHeight( m_font, &m_fontheight ));

        DFBCHECK(m_window_surface->SetColor( m_window_surface, 0x70, 0xD0, 0xA0, 0xFF ));
        DFBCHECK(m_window_surface->SetFont( m_window_surface, m_font ) );

    }

    void _Do_layout(int id, IDirectFBSurface *surface, const char* pixelformat, const char *rop_name, const char *result)
    {
        string name = pixelformat;
        name += ":";
        name += rop_name;

        DFBRectangle dst_rect;
        dst_rect.x = (id % LAYOUT_COLUMN_NUM) * m_layout_grid_w;
        dst_rect.y = (id / LAYOUT_COLUMN_NUM) * m_layout_grid_h;
        dst_rect.w = m_layout_grid_w - LAYOUT_GAP_X;
        dst_rect.h = m_layout_grid_h - LAYOUT_GAP_Y;

        DFBCHECK(m_window_surface->StretchBlit(m_window_surface, surface, 0, &dst_rect));
        DFBCHECK(m_window_surface->DrawString(m_window_surface, name.c_str(), -1, dst_rect.x, dst_rect.y, (DFBSurfaceTextFlags)(DSTF_LEFT | DSTF_TOP) ));
        DFBCHECK(m_window_surface->DrawString(m_window_surface, result, -1, dst_rect.x, dst_rect.y + m_fontheight, (DFBSurfaceTextFlags)(DSTF_LEFT | DSTF_TOP) ));
    }

    void _Show()
    {
        DFBCHECK(m_window_surface->Flip(m_window_surface, 0, (DFBSurfaceFlipFlags)0));
        DFBCHECK(m_window->SetOpacity(m_window, 0xFF));
    }

private:
    IDirectFB *m_dfb;
    IDirectFBDisplayLayer *m_layer;
    IDirectFBWindow *m_window;
    IDirectFBSurface *m_window_surface;

    vector<ROPTestBase* > m_vector;

    IDirectFBFont *m_font;
    int m_fontheight;
    int m_layout_grid_w;
    int m_layout_grid_h;

};

int main( int argc, char *argv[] )
{
    DFBCHECK(DirectFBInit( &argc, &argv ));

    TestBench tests;

    pause();

    return 0;
}

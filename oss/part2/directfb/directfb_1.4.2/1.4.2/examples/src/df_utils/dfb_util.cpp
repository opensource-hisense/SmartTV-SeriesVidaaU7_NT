//  [8/29/2013 andy.hsu]

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <stdarg.h>
#include <directfb.h>
#include "dfb_util.h"


/// ********************************************************************************************* 
void IFSurfaceCommon::Clear (unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->Clear(surface, r, g, b, a));
}

void IFSurfaceCommon::Blit (IFSurfaceCommon & source, DFBRectangle * source_rect, int x, int y)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->Blit(surface, source.GetSurface(), source_rect, x, y));
}

void IFSurfaceCommon::Blit (IDirectFBSurface * source, DFBRectangle * source_rect, int x, int y)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->Blit(surface, source, source_rect, x, y));
}

void IFSurfaceCommon::StretchBlit (IFSurfaceCommon & source, DFBRectangle * source_rect, DFBRectangle * destination_rect)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->StretchBlit(surface, source.GetSurface(), source_rect, destination_rect));
}

void IFSurfaceCommon::StretchBlit (IDirectFBSurface * source, DFBRectangle * source_rect, DFBRectangle * destination_rect)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->StretchBlit(surface, source, source_rect, destination_rect));
}

void IFSurfaceCommon::Flip (const DFBRegion * region, DFBSurfaceFlipFlags flags)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->Flip(surface, region, flags));
}

void IFSurfaceCommon::SetColor (unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->SetColor(surface, r, g, b, a));
}

void IFSurfaceCommon::SetColorIndex(unsigned int index)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->SetColorIndex(surface, index));
}

void IFSurfaceCommon::SetPorterDuff(DFBSurfacePorterDuffRule rule)
{
    IDirectFBSurface * surface = GetSurface();
    DFBCHECK(surface->SetPorterDuff(surface, rule));
}

void IFSurfaceCommon::FillRectangle (int x, int y, int w, int h)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->FillRectangle(surface, x, y, w, h));
}

void IFSurfaceCommon::FillRectangles (DFBRectangle * rects, unsigned int num)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->FillRectangles(surface, rects, num));
}

void IFSurfaceCommon::DrawRectangle (int x, int y, int w, int h)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->DrawRectangle(surface, x, y, w, h));
}

void IFSurfaceCommon::DrawString (const char * text, int bytes, int x, int y, DFBSurfaceTextFlags flags)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->DrawString(surface, text, bytes, x, y, flags));
}

void IFSurfaceCommon::DrawGlyph (unsigned int character, int x, int y, DFBSurfaceTextFlags flags)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->DrawGlyph(surface, character, x, y, flags));
}

void IFSurfaceCommon::SetBlittingFlags (DFBSurfaceBlittingFlags flags)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->SetBlittingFlags(surface, flags));
}

void IFSurfaceCommon::SetSrcBlendFunction (DFBSurfaceBlendFunction function)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->SetSrcBlendFunction(surface, function));
}

void IFSurfaceCommon::SetDstBlendFunction (DFBSurfaceBlendFunction function)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->SetDstBlendFunction(surface, function));
}

void IFSurfaceCommon::SetFont (DFBFont& font)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->SetFont(surface, font.GetInstance()));
}

void IFSurfaceCommon::SetAlphaCmpMode (DFB_ACmpOp cmpmode)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->SetAlphaCmpMode(surface, cmpmode));
}

void IFSurfaceCommon::SetBlitNearestMode (bool enable)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->SetBlitNearestMode(surface, enable));
}

void IFSurfaceCommon::SetBlitMirror (bool xmirror_enable, bool ymirror_enable)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->SetBlitMirror(surface, xmirror_enable, ymirror_enable));
}

void IFSurfaceCommon::SetRenderOptions (DFBSurfaceRenderOptions	options)
{
    IDirectFBSurface * surface = GetSurface ();
    DFBCHECK(surface->SetRenderOptions (surface, options));
}

void IFSurfaceCommon::GetCapabilities (DFBSurfaceCapabilities   *ret_caps	)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetCapabilities (surface, ret_caps));
}

void  IFSurfaceCommon::GetPosition (int *ret_x, int *ret_y)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetPosition (surface, ret_x, ret_y));
}

void IFSurfaceCommon::GetSize (int *ret_width, int *ret_height)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetSize (surface, ret_width, ret_height));
}

void IFSurfaceCommon::GetVisibleRectangle (DFBRectangle *ret_rect)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetVisibleRectangle (surface, ret_rect));
}

void  IFSurfaceCommon::GetAccelerationMask (IDirectFBSurface *source, DFBAccelerationMask *ret_mask)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetAccelerationMask (surface, source, ret_mask));
}

void IFSurfaceCommon::GetPalette (IDirectFBPalette **ret_interface)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetPalette (surface, ret_interface));
}

void IFSurfaceCommon::SetPalette (IDirectFBPalette *palette)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetPalette (surface, palette));
}

void IFSurfaceCommon::SetAlphaRamp (u8 a0, u8 a1, u8 a2, u8 a3)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetAlphaRamp (surface, a0, a1, a2, a3));
}

void IFSurfaceCommon::Lock (DFBSurfaceLockFlags flags, void **ret_ptr, int *ret_pitch)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->Lock (surface, flags, ret_ptr, ret_pitch));
}

void IFSurfaceCommon::GetFramebufferOffset (int *offset)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetFramebufferOffset (surface, offset));
}

void IFSurfaceCommon::Unlock ()
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->Unlock (surface));
}

void IFSurfaceCommon::GetClip (DFBRegion *ret_clip)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetClip (surface, ret_clip));
}			  

void IFSurfaceCommon::SetSrcColorKey (u8 r, u8 g, u8 b)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetSrcColorKey (surface, r, g, b));
}

void IFSurfaceCommon::SetSrcColorKeyIndex (unsigned int index)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetSrcColorKeyIndex (surface, index));
}		  

void IFSurfaceCommon::SetDstColorKey (u8 r, u8 g, u8 b)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetDstColorKey (surface, r, g, b));
}

void IFSurfaceCommon::SetDstColorKeyIndex (unsigned int index)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetDstColorKeyIndex (surface, index));
}

void IFSurfaceCommon::TileBlit (IDirectFBSurface *source, const DFBRectangle *source_rect, int x, int y)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->TileBlit (surface, source, source_rect, x, y));
}

void IFSurfaceCommon::BatchBlit (IDirectFBSurface *source, const DFBRectangle *source_rects, const DFBPoint *dest_points, int num)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->BatchBlit (surface, source, source_rects, dest_points, num));
}

void IFSurfaceCommon::TextureTriangles (IDirectFBSurface *texture, const DFBVertex *vertices, const int *indices, int num, DFBTriangleFormation formation)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->TextureTriangles (surface, texture, vertices, indices, num, formation));
}

void IFSurfaceCommon::SetDrawingFlags (DFBSurfaceDrawingFlags    flags)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetDrawingFlags (surface, flags));
}

void IFSurfaceCommon::DrawLine (int x1, int y1, int x2, int y2)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->DrawLine (surface, x1, y1, x2, y2));
}

void IFSurfaceCommon::DrawLines (const DFBRegion *lines, unsigned int num_lines)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->DrawLines (surface, lines, num_lines));
}

void IFSurfaceCommon::FillTriangle (int x1, int y1, int x2, int y2, int x3, int y3)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->FillTriangle (surface, x1, y1, x2, y2, x3, y3));
}

void IFSurfaceCommon::FillSpans (int y, const DFBSpan *spans, unsigned int num)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->FillSpans (surface, y, spans, num));
}

void IFSurfaceCommon::FillTriangles (const DFBTriangle *tris, unsigned int num)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->FillTriangles (surface, tris, num));
}

void IFSurfaceCommon::SetEncoding (DFBTextEncodingID encoding)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetEncoding (surface, encoding));
}

void IFSurfaceCommon::GetSubSurface (const DFBRectangle *rect, IDirectFBSurface **ret_interface)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetSubSurface (surface, rect, ret_interface));
}

void IFSurfaceCommon::GetGL (IDirectFBGL **ret_interface)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetGL (surface, ret_interface));
}

void IFSurfaceCommon::Dump (const char *directory, const char *prefix)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->Dump (surface, directory, prefix));
}		   

void IFSurfaceCommon::DisableAcceleration (DFBAccelerationMask mask)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->DisableAcceleration (surface, mask));
}						  

void IFSurfaceCommon::ReleaseSource (	)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->ReleaseSource (surface));
}

void IFSurfaceCommon::SetIndexTranslation (
						  const int                *indices,
						  int                       num_indices
						  )
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetIndexTranslation (surface, indices, num_indices));
}

void IFSurfaceCommon::SetMatrix (
				const s32                *matrix
				)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetMatrix (surface, matrix));
}

void IFSurfaceCommon::SetSourceMask (
					IDirectFBSurface         *mask,
					int                       x,
					int                       y,
					DFBSurfaceMaskFlags       flags
					)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetSourceMask (surface, mask, x, y, flags));
}

void IFSurfaceCommon::MakeSubSurface (IDirectFBSurface *from, const DFBRectangle *rect)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->MakeSubSurface (surface, from, rect));
}

void IFSurfaceCommon::Write (const DFBRectangle *rect, const void *ptr, int pitch)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->Write (surface, rect, ptr, pitch));
}

void IFSurfaceCommon::Read (const DFBRectangle *rect, void *ptr, int pitch)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->Read (surface, rect, ptr, pitch));
}

void IFSurfaceCommon::DrawString2 (const char *text, int bytes, int x, int y, DFBSurfaceTextFlags flags, DFBMatrix matrix)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->DrawString2 (surface, text, bytes, x, y, flags, matrix));
}

void IFSurfaceCommon::GetAlphaCmpMode ( DFB_ACmpOp * pcmpmode)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetAlphaCmpMode (surface, pcmpmode));
}

void IFSurfaceCommon::TrapezoidBlit (IDirectFBSurface   *source, const DFBRectangle *source_rect, const DFBTrapezoid *trapezoid)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->TrapezoidBlit (surface, source, source_rect, trapezoid));
}

void IFSurfaceCommon::FillTrapezoid ( const DFBTrapezoid *trapezoid)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->FillTrapezoid (surface, trapezoid));
}

void IFSurfaceCommon::DrawOvals (const DFBOval *ovals, unsigned int  num)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->DrawOvals (surface, ovals, num));
}


void IFSurfaceCommon::DrawOval (
			   int                       x,
			   int                       y,
			   int                       w,
			   int                       h,
			   int                       line_width,
			   DFBColor                  line_color
			   )
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->DrawOval (surface,x, y, w, h, line_width, line_color));
}

void IFSurfaceCommon::DrawGlyphs (
				 const unsigned int *character,
				 const int *x,
				 const int *y,
				 DFBSurfaceTextFlags      flags,
				 unsigned int             num
				 )
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->DrawGlyphs (surface, character, x, y, flags, num));
}

void IFSurfaceCommon::Lock2 (
			DFBSurfaceLockFlags       flags,
			void                    **ret_addr,
			void                    **ret_phys,
			int                      *ret_pitch
			)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->Lock2 (surface, flags, ret_addr, ret_phys, ret_pitch));
}

void IFSurfaceCommon::StretchBlitEx (
					IDirectFBSurface         *source,
					const DFBRectangle       *source_rect,
					const DFBRectangle       *destination_rect,
					const DFBGFX_ScaleInfo   *scale_info
					)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->StretchBlitEx (surface, source, source_rect, destination_rect, scale_info));
}

void IFSurfaceCommon::Sync(DFBSurfaceSyncFlags flags)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->Sync (surface, flags));
}

void IFSurfaceCommon::SetGradientColor2(u8 r, u8 g, u8 b, u8 a )
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->SetGradientColor2 (surface, r, g, b, a));
}

void IFSurfaceCommon::GetOnScreenRect (DFBRectangle *pRect)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->GetOnScreenRect (surface, pRect));
}

void IFSurfaceCommon::CanAccelerate (IDirectFBSurface  *source, DFBAccelerationMask  accel)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->CanAccelerate (surface, source, accel));
}

void IFSurfaceCommon::DiscardContents (	)
{
	IDirectFBSurface * surface = GetSurface ();
	DFBCHECK(surface->DiscardContents (surface));
}

/// ********************************************************************************************* 
DFBEnvBase::DFBEnvBase()
:m_dfb(0)
{
    DFBResult ret = DirectFBInit( NULL, NULL );
    if (ret) {
        DirectFBError( "DirectFBInit() failed", ret );
        exit(0);
    }

    /* create the super interface. */
    ret = DirectFBCreate( &m_dfb );
    if (ret) {
        DirectFBError( "DirectFBCreate() failed", ret );
        exit(0);
    }
}

DFBEnvBase::~DFBEnvBase()
{
    SAFE_RELEASE(m_dfb);
}

IDirectFB * DFBEnvBase::GetDFB () 
{ 
    static DFBEnvBase dfb;
    return dfb.m_dfb;
}

void DFBEnvBase::GetDeviceDescription (DFBGraphicsDeviceDescription * ret_desc)
{
    GetDFB()->GetDeviceDescription(GetDFB(), ret_desc);
}

/// ********************************************************************************************* 
class DFBLayerPrivate
{
public:
    IDirectFBDisplayLayer * m_layer;
    IDirectFBSurface *      m_surface;
    DFBDisplayLayerConfig   m_config;
    DFBDisplayLayerConfig   m_config_bak;
    bool                    m_isAutResCfg;
};

DFBLayer::DFBLayer (unsigned int id)
{
    D_P(DFBLayer);

    p->m_layer = 0;
    p->m_isAutResCfg = false;

    assert ( id <= 1 );

    DFBCHECK(DFBEnvBase::GetDFB()->GetDisplayLayer(DFBEnvBase::GetDFB(), id, &p->m_layer));

    DFBCHECK(p->m_layer->SetCooperativeLevel(p->m_layer,DLSCL_ADMINISTRATIVE));    
    DFBCHECK(p->m_layer->GetConfiguration(p->m_layer, &p->m_config));
    DFBCHECK(p->m_layer->GetConfiguration(p->m_layer, &p->m_config_bak));
}

DFBLayer::~DFBLayer()
{
    if (p->m_isAutResCfg)
    {
        RestoreConfig();
    }

    SAFE_RELEASE(p->m_layer);
}

IDirectFBSurface * DFBLayer::GetSurface ()
{
    DFBCHECK(p->m_layer->GetSurface(p->m_layer, &p->m_surface));
    return p->m_surface;
}

IDirectFBDisplayLayer * DFBLayer::GetInstance () const    
{
    return p->m_layer;
}

void DFBLayer::SetSize(unsigned int width, unsigned int height)
{
    DFBCHECK(p->m_layer->SetCooperativeLevel(p->m_layer, DLSCL_ADMINISTRATIVE));
    p->m_config.flags = (DFBDisplayLayerConfigFlags)( DLCONF_HEIGHT | DLCONF_WIDTH);
    p->m_config.height = height;                
    p->m_config.width  = width;
    DFBCHECK(p->m_layer->SetConfiguration(p->m_layer, &p->m_config));
}

void DFBLayer::SetBufferMode(int mode)
{
    DFBCHECK(p->m_layer->SetCooperativeLevel(p->m_layer, DLSCL_ADMINISTRATIVE));
    p->m_config.flags = (DFBDisplayLayerConfigFlags)( DLCONF_BUFFERMODE );

    switch(mode)
    {
    case BUF_MOD_SINGLE:
        p->m_config.buffermode = DLBM_FRONTONLY;
        break;
    case BUF_MOD_DOUBLE:
        p->m_config.buffermode = DLBM_BACKVIDEO ;
        break;
    case BUF_MOD_TRIPLE:
        p->m_config.buffermode = DLBM_TRIPLE;
        break;
    }
    DFBCHECK(p->m_layer->SetConfiguration(p->m_layer, &p->m_config));
}

void DFBLayer::RestoreConfig()
{
    DFBCHECK(p->m_layer->SetCooperativeLevel(p->m_layer, DLSCL_ADMINISTRATIVE));
    DFBCHECK(p->m_layer->SetConfiguration(p->m_layer, &p->m_config_bak));
}

void DFBLayer::SetAutoRestoreConfig(bool en)
{
    p->m_isAutResCfg = en;
}

void DFBLayer::SetLevel (int level)
{
    DFBCHECK(p->m_layer->SetLevel(p->m_layer, level));
}

void DFBLayer::GetConfiguration (DFBDisplayLayerConfig * ret_config)
{
    DFBCHECK(p->m_layer->SetCooperativeLevel(p->m_layer, DLSCL_ADMINISTRATIVE));
    DFBCHECK(p->m_layer->GetConfiguration(p->m_layer, ret_config));
}

void DFBLayer::SetCooperativeLevel (DFBDisplayLayerCooperativeLevel level)
{
    DFBCHECK(p->m_layer->SetCooperativeLevel(p->m_layer, level));
}

void DFBLayer::SetConfiguration (const DFBDisplayLayerConfig * config)
{
    DFBCHECK(p->m_layer->SetCooperativeLevel(p->m_layer, DLSCL_ADMINISTRATIVE));
    DFBCHECK(p->m_layer->SetConfiguration(p->m_layer, config));
}

void DFBLayer::EnableCursor(int enable)
{
    DFBCHECK(p->m_layer->EnableCursor(p->m_layer, enable));
}

void DFBLayer::ForceFullUpdateWindowStack(bool enable)
{
    DFBCHECK(p->m_layer->ForceFullUpdateWindowStack(p->m_layer, enable));
}

void DFBLayer::SetHVMirrorEnable (bool HEnable, bool VEnable)
{
    DFBCHECK(p->m_layer->SetHVMirrorEnable(p->m_layer, HEnable, VEnable));
}

void DFBLayer::BindShadowLayer (IDirectFBDisplayLayer *other)
{
    DFBCHECK(p->m_layer->BindShadowLayer(p->m_layer, other));
}

void DFBLayer::UnbindShadowLayer (IDirectFBDisplayLayer *other)
{
    DFBCHECK(p->m_layer->UnbindShadowLayer(p->m_layer, other));
}

void DFBLayer::SetLBCoupleEnable (bool LBCoupleEnable)
{
    DFBCHECK(p->m_layer->SetLBCoupleEnable(p->m_layer, LBCoupleEnable));
}

void DFBLayer::SetGOPDstByPassEnable (bool ByPassEnable)
{
    DFBCHECK(p->m_layer->SetGOPDstByPassEnable(p->m_layer, ByPassEnable));
}

void DFBLayer::SetHVScale (int HScale, int VScale)
{
    DFBCHECK(p->m_layer->SetHVScale(p->m_layer, HScale, VScale));
}

void DFBLayer::GetScreen (IDirectFBScreen **ret_interface)
{
    DFBCHECK(p->m_layer->GetScreen(p->m_layer, ret_interface));
}

void DFBLayer::SetScreenLocation (float x, float y, float width, float height)
{
    DFBCHECK(p->m_layer->SetScreenLocation(p->m_layer, x, y, width, height));
}

void DFBLayer::SetScreenPosition (int x, int y)
{
    DFBCHECK(p->m_layer->SetScreenPosition(p->m_layer, x, y));
}

void DFBLayer::SetScreenRectangle (int x, int y, int width, int height)
{
    DFBCHECK(p->m_layer->SetScreenRectangle(p->m_layer, x, y, width, height));
}

void DFBLayer::GetCursorPosition (int *ret_x, int  *ret_y)
{
    DFBCHECK(p->m_layer->GetCursorPosition(p->m_layer, ret_x, ret_y));
}

void DFBLayer::WarpCursor (int x, int y)
{
    DFBCHECK(p->m_layer->WarpCursor(p->m_layer, x, y));
}

void DFBLayer::SetCursorAcceleration (int numerator, int denominator, int threshold)
{
    DFBCHECK(p->m_layer->SetCursorAcceleration(p->m_layer, numerator, denominator, threshold));
}

void DFBLayer::SetCursorShape (IDirectFBSurface *shape, int hot_x, int hot_y)
{
    DFBCHECK(p->m_layer->SetCursorShape(p->m_layer, shape, hot_x, hot_y));
}

void DFBLayer::SetCursorOpacity (u8 opacity)
{
    DFBCHECK(p->m_layer->SetCursorOpacity(p->m_layer, opacity));
}

void DFBLayer::WaitForSync ()
{
    DFBCHECK(p->m_layer->WaitForSync(p->m_layer));
}

void DFBLayer::SwitchContext (DFBBoolean exclusive)
{
    DFBCHECK(p->m_layer->SwitchContext(p->m_layer, exclusive));
}
                    
void DFBLayer::SetRotation (int rotation)
{
    DFBCHECK(p->m_layer->SetRotation(p->m_layer, rotation));
}

void DFBLayer::GetRotation (int *ret_rotation)
{
    DFBCHECK(p->m_layer->GetRotation(p->m_layer, ret_rotation));
}
                  
void DFBLayer::SetAlphaMode (DFBDisplayLayerOptions flag)
{
    DFBCHECK(p->m_layer->SetAlphaMode(p->m_layer, flag));
}   
                   
void DFBLayer::SetStretchMode (DFBDisplayLayerStretchSetting stretch_mode)
{
    DFBCHECK(p->m_layer->SetStretchMode(p->m_layer, stretch_mode));
}
                     
void DFBLayer::SetDeskDisplayMode (DFBDisplayLayerDeskTopDisplayMode disktop_mode)
{
    DFBCHECK(p->m_layer->SetDeskDisplayMode(p->m_layer, disktop_mode));
}
                         
void DFBLayer::MoveCursorTo (int x, int y)
{
    DFBCHECK(p->m_layer->MoveCursorTo(p->m_layer, x, y));
}

void DFBLayer::GetActiveWindowNum (int *ret_num)
{
    DFBCHECK(p->m_layer->GetActiveWindowNum(p->m_layer, ret_num));
}

void DFBLayer::ReactiveLayer ()
{
    DFBCHECK(p->m_layer->ReactiveLayer(p->m_layer));
}

void DFBLayer::GetID (DFBDisplayLayerID *ret_layer_id)
{
    DFBCHECK(p->m_layer->GetID(p->m_layer, ret_layer_id));
}

void DFBLayer::GetDescription (DFBDisplayLayerDescription *ret_desc)
{
    DFBCHECK(p->m_layer->GetDescription(p->m_layer, ret_desc));
}

void DFBLayer::GetSourceDescriptions (DFBDisplayLayerSourceDescription *ret_descriptions)
{
    DFBCHECK(p->m_layer->GetSourceDescriptions(p->m_layer, ret_descriptions));
}

void DFBLayer::GetCurrentOutputField (int *ret_field)
{
    DFBCHECK(p->m_layer->GetCurrentOutputField(p->m_layer, ret_field));
}

void DFBLayer::GetColorAdjustment (DFBColorAdjustment *ret_adj)
{
    DFBCHECK(p->m_layer->GetColorAdjustment(p->m_layer, ret_adj));
}

void DFBLayer::SetColorAdjustment (const DFBColorAdjustment *adj)
{
    DFBCHECK(p->m_layer->SetColorAdjustment(p->m_layer, adj));
}

/// ********************************************************************************************* 
class DFBWindowPrivate
{
public:
    DFBLayer  *          m_parent;
    IDirectFBWindow *    m_window;
    DFBWindowDescription m_windesc;
    IDirectFBSurface *   m_winsurface;
};

DFBWindow::DFBWindow(DFBLayer & parent)
{
    D_P(DFBWindow);

    p->m_parent = &parent;
    p->m_window = 0;
    p->m_winsurface = 0;

    p->m_windesc.flags  = (DFBWindowDescriptionFlags)(
        DWDESC_POSX | DWDESC_POSY |
        DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS);
    p->m_windesc.posx   = 0;
    p->m_windesc.posy   = 0;
    p->m_windesc.width  = 640;
    p->m_windesc.height = 480;
    p->m_windesc.caps   = DWCAPS_ALPHACHANNEL;
}

DFBWindow::~DFBWindow()
{
    SAFE_RELEASE(p->m_winsurface);
    SAFE_RELEASE(p->m_window);
}


IDirectFBWindow * DFBWindow::GetInstance ()
{
    if (p->m_window)
        return p->m_window;

    IDirectFBDisplayLayer * layer = p->m_parent->GetInstance();
    DFBCHECK(layer->CreateWindow(layer, &p->m_windesc, &p->m_window));
    return p->m_window;
}

IDirectFBSurface * DFBWindow::GetSurface ()
{
    if (p->m_winsurface)
        return p->m_winsurface;

    GetInstance();

    if (p->m_window)
    {
        DFBCHECK(p->m_window->GetSurface(p->m_window, &p->m_winsurface));
    }

    return p->m_winsurface;
}

void DFBWindow::Resize (int width, int height)
{
    IDirectFBWindow * window = GetInstance();
    DFBCHECK(window->Resize(window, width, height));

    if (window)
        DFBCHECK(window->GetSurface(window, &p->m_winsurface));
}

void DFBWindow::SetOpacity (unsigned char opacity)
{
    IDirectFBWindow * window = GetInstance();
    DFBCHECK(window->SetOpacity(window, opacity));
}

void DFBWindow::SetStackingClass (DFBWindowStackingClass stacking_class)
{
    IDirectFBWindow * window = GetInstance();
    DFBCHECK(window->SetStackingClass(window, stacking_class));
}

void DFBWindow::SetWindowDescription (DFBWindowDescription desc)
{
    if (memcmp(&p->m_windesc, &desc, sizeof(desc)) == 0)
        return;

    if (desc.flags & DWDESC_CAPS)
    {
        p->m_windesc.caps = desc.caps;
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_CAPS);
    }

    if (desc.flags & DWDESC_WIDTH)
    {
        p->m_windesc.width = desc.width;
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_WIDTH);
    }

    if (desc.flags & DWDESC_HEIGHT)
    {
        p->m_windesc.height = desc.height;
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_HEIGHT);
    }

    if (desc.flags & DWDESC_PIXELFORMAT)
    {
        p->m_windesc.pixelformat = desc.pixelformat;
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_PIXELFORMAT);
    }

    if (desc.flags & DWDESC_POSX)
    {
        p->m_windesc.posx = desc.posx;
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_POSX);
    }

    if (desc.flags & DWDESC_POSY)
    {
        p->m_windesc.posy = desc.posy;
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_POSY);
    }

    if (desc.flags & DWDESC_SURFACE_CAPS)
    {
        p->m_windesc.surface_caps = desc.surface_caps;
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_SURFACE_CAPS);
    }

    if (desc.flags & DWDESC_PARENT)
    {
        p->m_windesc.parent_id = desc.parent_id;
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_PARENT);
    }

    if (desc.flags & DWDESC_OPTIONS)
    {
        p->m_windesc.options = desc.options;
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_OPTIONS);
    }

    if (desc.flags & DWDESC_STACKING)
    {
        p->m_windesc.stacking = desc.stacking;
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_STACKING);
    }

    if (desc.flags & DWDESC_TOPLEVEL_ID)
    {
        p->m_windesc.toplevel_id = desc.toplevel_id;
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_TOPLEVEL_ID);
    }

    if (desc.flags & DWDESC_RESOURCE_ID)
    {
        p->m_windesc.resource_id = desc.resource_id;    
        p->m_windesc.flags = (DFBWindowDescriptionFlags) (p->m_windesc.flags | DWDESC_RESOURCE_ID);
    }

    SAFE_RELEASE(p->m_winsurface);
    SAFE_RELEASE(p->m_window);
}

void DFBWindow::SetOptions (DFBWindowOptions options)
{
    IDirectFBWindow * window = GetInstance();
    DFBCHECK(window->SetOptions(window, options));
}

void DFBWindow::Move (int dx, int dy)
{
    IDirectFBWindow * window = GetInstance();
    DFBCHECK(window->Move(window, dx, dy));
}

void DFBWindow::MoveTo (int x, int y)
{
    IDirectFBWindow * window = GetInstance();
    DFBCHECK(window->MoveTo(window, x, y));
}

void DFBWindow::SetColor (unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    IDirectFBWindow * window = GetInstance();
    DFBCHECK(window->SetColor(window, r, g, b, a));
}

void DFBWindow::GetID (DFBWindowID *ret_window_id)
{
    IDirectFBWindow * window = GetInstance();
    DFBCHECK(window->GetID(window, ret_window_id));
}

void DFBWindow::GetPosition (int *ret_x, int *ret_y)
{
    IDirectFBWindow * window = GetInstance();
    DFBCHECK(window->GetPosition(window, ret_x, ret_y));
}

void DFBWindow::GetSize (int *ret_width, int *ret_height)
{
    IDirectFBWindow * window = GetInstance();
    DFBCHECK(window->GetSize(window, ret_width, ret_height));
}

void DFBWindow::ResizeSurface (int width, int height)
{
    IDirectFBWindow * window = GetInstance();
    DFBCHECK(window->ResizeSurface(window, width, height));
}

void DFBWindow::CreateEventBuffer (IDirectFBEventBuffer **ret_buffer)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->CreateEventBuffer(window, ret_buffer));
}

void DFBWindow::AttachEventBuffer (IDirectFBEventBuffer *buffer)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->AttachEventBuffer(window, buffer));
}

void DFBWindow::DetachEventBuffer (IDirectFBEventBuffer *buffer)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->DetachEventBuffer(window, buffer));
}

void DFBWindow::EnableEvents (DFBWindowEventType mask)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->EnableEvents(window, mask));
}


void DFBWindow::DisableEvents (DFBWindowEventType mask)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->DisableEvents(window, mask));
}

void DFBWindow::GetOptions (DFBWindowOptions *ret_options)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->GetOptions(window, ret_options));
}

void DFBWindow::SetColorKey (u8 r, u8 g, u8 b)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->SetColorKey(window, r, g, b));
}

void DFBWindow::SetColorKeyIndex (unsigned int index)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->SetColorKeyIndex(window, index));
}

void DFBWindow::SetOpaqueRegion (int x1, int y1, int x2, int y2)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->SetOpaqueRegion(window, x1, y1, x2, y2));
}

void DFBWindow::GetOpacity (u8 *ret_opacity)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->GetOpacity(window, ret_opacity));
}

void DFBWindow::SetCursorShape(IDirectFBSurface  *shape, int hot_x, int  hot_y)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->SetCursorShape(window, shape, hot_x, hot_y));
}

void DFBWindow::SetBounds (int  x, int  y, int  width, int height)
{
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->SetBounds(window, x, y, width, height));
}


 void  DFBWindow::Raise ()
 {
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->Raise(window));
 }

 void DFBWindow::Lower ()
 {
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->Lower(window));
 }

 void DFBWindow::RaiseToTop ()
 {
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->RaiseToTop(window)); 
 }

 void DFBWindow::LowerToBottom ()
 {
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->LowerToBottom(window));  
 }

 void DFBWindow::PutAtop (IDirectFBWindow  *lower)
 {
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->PutAtop(window, lower));  
 }

 void DFBWindow::PutBelow (IDirectFBWindow  *upper)
 {
	IDirectFBWindow * window = GetInstance();
	DFBCHECK(window->PutBelow(window, upper));  
 }

 void DFBWindow::Bind (IDirectFBWindow *window, int x, int y)
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->Bind(win, window, x, y));  
 }

 void DFBWindow::Unbind (IDirectFBWindow *window)
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->Unbind(win, window));  
 }

 void DFBWindow::RequestFocus ()
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->RequestFocus(win));  
 }

 void DFBWindow::GrabKeyboard ()
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->GrabKeyboard(win));   
 }

 void DFBWindow::UngrabKeyboard ()
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->UngrabKeyboard(win));   
 }

 void DFBWindow::GrabPointer ()
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->GrabPointer(win));   
 }

 void DFBWindow::UngrabPointer ()
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->UngrabPointer(win)); 
 }

 void DFBWindow::GrabKey (DFBInputDeviceKeySymbol symbol, DFBInputDeviceModifierMask modifiers)
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->GrabKey(win, symbol, modifiers)); 
 }

 void DFBWindow::UngrabKey (DFBInputDeviceKeySymbol symbol, DFBInputDeviceModifierMask modifiers)
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->UngrabKey(win, symbol, modifiers)); 
 }

 void DFBWindow::SetKeySelection (
	 DFBWindowKeySelection          selection,
	 const DFBInputDeviceKeySymbol *keys,
	 unsigned int                   num_keys
	 )
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->SetKeySelection(win, selection, keys, num_keys)); 
 }

 void DFBWindow::GrabUnselectedKeys ()
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->GrabUnselectedKeys(win)); 
 }

 void DFBWindow::UngrabUnselectedKeys ()
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->UngrabUnselectedKeys(win)); 
 }

 void DFBWindow::SetSrcGeometry (const DFBWindowGeometry       *geometry)
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->SetSrcGeometry(win, geometry)); 
 }

 void DFBWindow::SetDstGeometry (const DFBWindowGeometry       *geometry)
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->SetDstGeometry(win, geometry)); 
 }

 void DFBWindow::SetProperty (const char *key, void *value, void **ret_old_value)
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->SetProperty(win, key, value, ret_old_value)); 
 }
	 
 void DFBWindow::GetProperty (const char *key, void **ret_value)
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->GetProperty(win, key, ret_value)); 
 }

 void DFBWindow::RemoveProperty (const char *key, void **ret_value)
 {
	 IDirectFBWindow * win = GetInstance();
	 DFBCHECK(win->RemoveProperty(win, key, ret_value)); 
 }

void DFBWindow::SetRotation (int rotation)
{
	IDirectFBWindow * win = GetInstance();
	DFBCHECK(win->SetRotation(win, rotation)); 
}

void DFBWindow::SetAssociation (DFBWindowID window_id)
{
	IDirectFBWindow * win = GetInstance();
	DFBCHECK(win->SetAssociation(win, window_id)); 
}					 

void DFBWindow::SetApplicationID (unsigned long application_id)
{
	IDirectFBWindow * win = GetInstance();
	DFBCHECK(win->SetApplicationID(win, application_id)); 
}

void DFBWindow::GetApplicationID (unsigned long *ret_application_id)
{
	IDirectFBWindow * win = GetInstance();
	DFBCHECK(win->GetApplicationID(win, ret_application_id)); 
}

/// ********************************************************************************************* 
class DFBSurfacePrivate
{
public:
    IDirectFBSurface * m_surface;
    DFBSurfaceDescription m_surface_desc;
    bool  m_is_assigned;
};

DFBSurface::DFBSurface(IDirectFBSurface * surface)
{
    D_P(DFBSurface);

    p->m_surface = surface;
    p->m_is_assigned = surface != NULL;

    if (!surface) 
    {
        p->m_surface_desc.flags = (DFBSurfaceDescriptionFlags) ( DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT );
        p->m_surface_desc.caps = DSCAPS_VIDEOONLY;
        p->m_surface_desc.width = 640;
        p->m_surface_desc.height = 480;
    }
}

DFBSurface::DFBSurface ()
{
    D_P(DFBSurface);

    p->m_surface = 0;
    p->m_is_assigned = false;

    p->m_surface_desc.flags = (DFBSurfaceDescriptionFlags) ( DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT );
    p->m_surface_desc.caps = DSCAPS_VIDEOONLY;
    p->m_surface_desc.width = 640;
    p->m_surface_desc.height = 480;
}

DFBSurface::~DFBSurface()
{
    if (!p->m_is_assigned)
        SAFE_RELEASE(p->m_surface);
}

IDirectFBSurface * DFBSurface::GetSurface ()
{
    if (p->m_surface)
        return p->m_surface;

    DFBEnvBase::GetDFB()->CreateSurface(DFBEnvBase::GetDFB(), &p->m_surface_desc, &p->m_surface);
    return p->m_surface;
}

void DFBSurface::SetSurfaceDescription (DFBSurfaceDescription desc)
{
    if (memcmp(&p->m_surface_desc, &desc, sizeof(desc)) == 0)
        return;

    if (desc.flags & DSDESC_CAPS)
    {
        p->m_surface_desc.flags = (DFBSurfaceDescriptionFlags)(p->m_surface_desc.flags | DSDESC_CAPS);
        p->m_surface_desc.caps = desc.caps;
    }

    if (desc.flags & DSDESC_WIDTH)
    {
        p->m_surface_desc.flags = (DFBSurfaceDescriptionFlags)(p->m_surface_desc.flags | DSDESC_WIDTH);
        p->m_surface_desc.width = desc.width;
    }

    if (desc.flags & DSDESC_HEIGHT)
    {
        p->m_surface_desc.flags = (DFBSurfaceDescriptionFlags)(p->m_surface_desc.flags | DSDESC_HEIGHT);
        p->m_surface_desc.height = desc.height;
    }

    if (desc.flags & DSDESC_PIXELFORMAT)
    {
        p->m_surface_desc.flags = (DFBSurfaceDescriptionFlags)(p->m_surface_desc.flags | DSDESC_PIXELFORMAT);
        p->m_surface_desc.pixelformat = desc.pixelformat;
    }

    if (desc.flags & DSDESC_PREALLOCATED)
    {
        p->m_surface_desc.flags = (DFBSurfaceDescriptionFlags)(p->m_surface_desc.flags | DSDESC_PREALLOCATED);
        memcpy(p->m_surface_desc.preallocated, desc.preallocated, sizeof(desc.preallocated));
    }

    if (desc.flags & DSDESC_PALETTE)
    {
        p->m_surface_desc.flags = (DFBSurfaceDescriptionFlags)(p->m_surface_desc.flags | DSDESC_PALETTE);
        p->m_surface_desc.palette = desc.palette;
    }

    if (desc.flags & DSDESC_RESOURCE_ID)
    {
        p->m_surface_desc.flags = (DFBSurfaceDescriptionFlags)(p->m_surface_desc.flags | DSDESC_RESOURCE_ID);
        p->m_surface_desc.resource_id = desc.resource_id;
    }

    if (desc.flags & DSDESC_HINTS)
    {
        p->m_surface_desc.flags = (DFBSurfaceDescriptionFlags)(p->m_surface_desc.flags | DSDESC_HINTS);
        p->m_surface_desc.hints = desc.hints;
    }

    SAFE_RELEASE(p->m_surface);
}

/// ********************************************************************************************* 
DFBImageProvider::DFBImageProvider (const char * filename)
:m_provider(0)
{
    DFBCHECK(DFBEnvBase::GetDFB()->CreateImageProvider(DFBEnvBase::GetDFB(), filename, &m_provider));
}

DFBImageProvider::~DFBImageProvider()
{
    SAFE_RELEASE(m_provider);
}

IDirectFBImageProvider * DFBImageProvider::GetInstance ()
{
    return m_provider;
}

void DFBImageProvider::GetSurfaceDescription (DFBSurfaceDescription * ret_dsc)
{
    if (m_provider)
        DFBCHECK(m_provider->GetSurfaceDescription(m_provider, ret_dsc));
}

void DFBImageProvider::GetImageDescription (DFBImageDescription * ret_dsc)
{
    if (m_provider)
        DFBCHECK(m_provider->GetImageDescription(m_provider, ret_dsc));
}

void DFBImageProvider::RenderTo (IFSurfaceCommon & destination, const DFBRectangle * destination_rect)
{
    if (m_provider)
        DFBCHECK(m_provider->RenderTo(m_provider, destination.GetSurface(), destination_rect));
}

void DFBImageProvider::RenderTo (IDirectFBSurface * destination, const DFBRectangle * destination_rect)
{
    if (m_provider)
        DFBCHECK(m_provider->RenderTo(m_provider, destination, destination_rect));
}

void DFBImageProvider::SetRenderCallback (DIRenderCallback callback, void *callback_data)
{
	if (m_provider)
		DFBCHECK(m_provider->SetRenderCallback(m_provider, callback, callback_data));
}

void DFBImageProvider::WriteBack (IDirectFBSurface *surface,	const DFBRectangle *src_rect, const char *filename)
{
	if (m_provider)
		DFBCHECK(m_provider->WriteBack(m_provider, surface, src_rect, filename));
}

void DFBImageProvider::SetHWDecoderParameter(void *hw_decoder_setting)
{
	if (m_provider)
		DFBCHECK(m_provider->SetHWDecoderParameter(m_provider, hw_decoder_setting));
}


/// ********************************************************************************************* 
DFBVideoProvider::DFBVideoProvider (const char * filename)
{
    DFBCHECK(DFBEnvBase::GetDFB()->CreateVideoProvider(DFBEnvBase::GetDFB(), filename, &m_provider));
}

DFBVideoProvider::~DFBVideoProvider()
{
    SAFE_RELEASE(m_provider);
}

IDirectFBVideoProvider * DFBVideoProvider::GetInstance ()
{
    return m_provider;
}

void DFBVideoProvider::GetSurfaceDescription (DFBSurfaceDescription * ret_dsc)
{
    if (m_provider)
        DFBCHECK(m_provider->GetSurfaceDescription (m_provider, ret_dsc));
}

void DFBVideoProvider::PlayTo (IDirectFBSurface * destination, const DFBRectangle * destination_rect, DVFrameCallback callback, void * ctx)
{
    if (m_provider)
        DFBCHECK(m_provider->PlayTo (m_provider, destination, destination_rect, callback, ctx));
}

void DFBVideoProvider::PlayTo (IFSurfaceCommon & destination, const DFBRectangle * destination_rect, DVFrameCallback callback, void * ctx)
{
    if (m_provider)
        DFBCHECK(m_provider->PlayTo (m_provider, destination.GetSurface(), destination_rect, callback, ctx));
}

void DFBVideoProvider::GetStreamDescription (DFBStreamDescription *ret_dsc	)
{
	if (m_provider)
		DFBCHECK(m_provider->GetStreamDescription (m_provider, ret_dsc));
}

void DFBVideoProvider::Stop ()
{
	if (m_provider)
		DFBCHECK(m_provider->Stop (m_provider));
}

void DFBVideoProvider::GetStatus (DFBVideoProviderStatus   *ret_status)
{
	if (m_provider)
		DFBCHECK(m_provider->GetStatus (m_provider, ret_status));
}


/// ********************************************************************************************* 
DFBFont::DFBFont(const char * filename, const  DFBFontDescription * desc)
{
    DFBCHECK(DFBEnvBase::GetDFB()->CreateFont(DFBEnvBase::GetDFB(), filename, desc, &m_font));
}

DFBFont::~DFBFont()
{
    SAFE_RELEASE(m_font);
}

IDirectFBFont * DFBFont::GetInstance ()
{
    return m_font;
}

void DFBFont::GetHeight (int * ret_height)
{
    if (m_font)
        DFBCHECK(m_font->GetHeight (m_font, ret_height));
}

void DFBFont::GetAscender (	int *ret_ascender)
{
	if (m_font)
		DFBCHECK(m_font->GetAscender (m_font, ret_ascender));
}

void DFBFont::GetDescender (int *ret_descender)
{
	if (m_font)
		DFBCHECK(m_font->GetDescender (m_font, ret_descender));
}

void DFBFont::GetMaxAdvance (int *ret_maxadvance)
{
	if (m_font)
		DFBCHECK(m_font->GetMaxAdvance (m_font, ret_maxadvance));
}

void DFBFont::GetKerning (unsigned int  prev, unsigned int current, int *ret_kern_x, int *ret_kern_y)
{
	if (m_font)
		DFBCHECK(m_font->GetKerning (m_font, prev, current, ret_kern_x, ret_kern_y));
}

void DFBFont::GetStringWidth (const char *text, int bytes, int *ret_width)
{
	if (m_font)
		DFBCHECK(m_font->GetStringWidth (m_font, text, bytes, ret_width));
}

void DFBFont::GetStringExtents (const char *text, int bytes, DFBRectangle *ret_logical_rect, DFBRectangle *ret_ink_rect)
{
	if (m_font)
		DFBCHECK(m_font->GetStringExtents (m_font, text, bytes, ret_logical_rect, ret_ink_rect));
}

void DFBFont::GetGlyphExtents (unsigned int character, DFBRectangle *ret_rect, int *ret_advance)
{
	if (m_font)
		DFBCHECK(m_font->GetGlyphExtents (m_font, character, ret_rect, ret_advance));
}

void DFBFont::GetStringBreak (const char *text, int bytes, int max_width, int *ret_width, int *ret_str_length, const char **ret_next_line)
{
	if (m_font)
		DFBCHECK(m_font->GetStringBreak (m_font, text, bytes, max_width, ret_width, ret_str_length, ret_next_line));
}

void DFBFont::SetEncoding (DFBTextEncodingID encoding)
{
	if (m_font)
		DFBCHECK(m_font->SetEncoding (m_font, encoding));				  
}

void DFBFont::EnumEncodings (DFBTextEncodingCallback  callback, void *context)
{
	if (m_font)
		DFBCHECK(m_font->EnumEncodings (m_font, callback, context));
}

void DFBFont::FindEncoding (const char *name, DFBTextEncodingID *ret_encoding)
{
	if (m_font)
		DFBCHECK(m_font->FindEncoding (m_font, name, ret_encoding));
}

/// ********************************************************************************************* 
DFBDataBuffer::DFBDataBuffer(DFBDataBufferDescription ddsc)
{
	DFBCHECK(DFBEnvBase::GetDFB()->CreateDataBuffer(DFBEnvBase::GetDFB(), &ddsc, &m_databuffer));
}

DFBDataBuffer::~DFBDataBuffer()
{
	if(m_databuffer)
		m_databuffer->Release(m_databuffer);
}

void DFBDataBuffer::Flush()
{
	if(m_databuffer)
		DFBCHECK(m_databuffer->Flush(m_databuffer));
}

void DFBDataBuffer::Finish ()
{
	if(m_databuffer)
		DFBCHECK(m_databuffer->Finish(m_databuffer));
}

void DFBDataBuffer::SeekTo (unsigned int offset)
{
	if(m_databuffer)
		DFBCHECK(m_databuffer->SeekTo(m_databuffer, offset));
}

void DFBDataBuffer::GetPosition (unsigned int *ret_offset)
{
	if(m_databuffer)
		DFBCHECK(m_databuffer->GetPosition(m_databuffer, ret_offset));
}

void DFBDataBuffer::GetLength (unsigned int *ret_length)
{
	if(m_databuffer)
		DFBCHECK(m_databuffer->GetLength(m_databuffer, ret_length));
}

void DFBDataBuffer::WaitForData (unsigned int length)
{
	if(m_databuffer)
		DFBCHECK(m_databuffer->WaitForData(m_databuffer, length));
}

void DFBDataBuffer::WaitForDataWithTimeout (unsigned int length, unsigned int seconds, unsigned int milli_seconds)
{
	if(m_databuffer)
		DFBCHECK(m_databuffer->WaitForDataWithTimeout(m_databuffer, length, seconds, milli_seconds));
}
							 

void DFBDataBuffer::GetData (unsigned int length, void *ret_data, unsigned int *ret_read)
{
	if(m_databuffer)
		DFBCHECK(m_databuffer->GetData(m_databuffer, length, ret_data, ret_read));
}

void DFBDataBuffer::PeekData (unsigned int length, int offset, void *ret_data, unsigned int *ret_read)
{
	if(m_databuffer)
		DFBCHECK(m_databuffer->PeekData(m_databuffer, length, offset, ret_data, ret_read));
}

/// ********************************************************************************************* 

struct PProbePrivate
{
    std::string m_String;

    timeval m_StartTime, m_EndTime;

    unsigned int m_RepeatInterval;
    unsigned int m_Counter;
    char m_FPS_string [32];
};

PProbe::PProbe(const char * loginfo, ...)
{
    D_P(PProbe);

    p->m_Counter = 
        p->m_RepeatInterval = 1;

    p->m_FPS_string[0] = '\0';

    if (loginfo)
    {
        char buffer[MAX_PARAMETER_SIZE];
        va_list arg_ptr;
        va_start(arg_ptr, loginfo);
        vsprintf(buffer, loginfo, arg_ptr);
        va_end(arg_ptr);

        p->m_String = buffer;

        start();
    }
}

PProbe::~PProbe()
{
    if (!p->m_String.empty())
    {
        stop ();
        printf("[FPS = %f (duration = %f sec)] %s\n", getFPS(), getDuration(), p->m_String.c_str());
    }
}

void PProbe::start ()
{
    if (p->m_RepeatInterval == 1)
    {
        memset(&p->m_EndTime, 0 ,sizeof(timeval));
        gettimeofday(&p->m_StartTime, NULL);
    }
    else
    {
        // In Loop Counter
        if (p->m_Counter == 1)
        {
            memset(&p->m_EndTime, 0 ,sizeof(timeval));
            gettimeofday(&p->m_StartTime, NULL);
        }
    }
}

void PProbe::stop ()
{
    if (p->m_RepeatInterval == 1)
    {
        gettimeofday(&p->m_EndTime, NULL);
    }
    else
    {
        // In Loop Counter

        if (p->m_Counter == p->m_RepeatInterval)
        {
            gettimeofday(&p->m_EndTime, NULL);

            // Reset Counter
            p->m_Counter = 1;
        }
        else
            p->m_Counter++;
    }
}

void PProbe::setRepeatInterval (unsigned int interval)
{
    p->m_RepeatInterval = interval >= 1 ? interval : 1;
}

float PProbe::getDuration ()
{
    return p->m_EndTime.tv_sec == 0 && p->m_EndTime.tv_usec == 0 ? 
        0.f : 
    (p->m_EndTime.tv_sec - p->m_StartTime.tv_sec) + (float)(p->m_EndTime.tv_usec - p->m_StartTime.tv_usec) * 0.000001f;
}

float PProbe::getFPS ()
{
    return getDuration () != 0.f ? p->m_RepeatInterval / getDuration () : 0.f;    
}

const char* PProbe::getStringFPS ()
{
    if (getFPS() != 0.f)
        snprintf( p->m_FPS_string, sizeof(p->m_FPS_string), "%.1f", getFPS() );
    return p->m_FPS_string;
}
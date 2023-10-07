#ifndef __DFB_UTIL_H__
#define __DFB_UTIL_H__

#include <directfb.h>
#include "dfb_util_common.h"


class DFBFont;

//////////////////////////////////////////////////////////////////////////

// -------------------------------
//      Common Interface
// -------------------------------

class IFSurfaceCommon
{
public:

/*
Retrieving information     
GetCapabilities         Return the capabilities of this surface.
GetPosition                Get the surface's position in pixels.
GetSize                    Get the surface's width and height in pixels.
GetVisibleRectangle        Created sub surfaces might be clipped by their parents, this function returns the resulting rectangle relative to this surface.
GetPixelFormat            Get the current pixel format.
GetAccelerationMask        Get a mask of drawing functions that are hardware accelerated with the current settings.

Palette & Alpha Ramp     
GetPalette                Get access to the surface's palette.
SetPalette                Change the surface's palette.
SetAlphaRamp            Set the alpha ramp for formats with one or two alpha bits.

Buffer operations     
Lock                    Lock the surface for the access type specified.
GetFramebufferOffset    Obsolete. Returns DFB_FAILURE always.
Unlock                    Unlock the surface after direct access.
Flip                    Flip/Update surface buffers.
SetField                Set the active field.
Clear                    Clear the surface and its depth buffer if existent.

Drawing/blitting control     
SetClip                    Set the clipping region used to limit the area for drawing, blitting and text functions.
GetClip                    Get the clipping region used to limit the area for drawing, blitting and text functions.
[done] SetColor                Set the color used for drawing/text functions or alpha/color modulation (blitting functions).
[done] SetColorIndex            Set the color like with SetColor() but using an index to the color/alpha lookup table.
[done] SetSrcBlendFunction        Set the blend function that applies to the source.
[done] SetDstBlendFunction        Set the blend function that applies to the destination.
SetPorterDuff            Set the source and destination blend function by specifying a Porter/Duff rule.
SetSrcColorKey            Set the source color key, i.e. the color that is excluded when blitting FROM this surface TO another that has source color keying enabled.
SetSrcColorKeyIndex        Set the source color key like with SetSrcColorKey() but using an index to the color/alpha lookup table.
SetDstColorKey            Set the destination color key, i.e. the only color that gets overwritten by drawing and blitting to this surface when destination color keying is enabled.
SetDstColorKeyIndex        Set the destination color key like with SetDstColorKey() but using an index to the color/alpha lookup table.

Blitting functions     
SetBlittingFlags        Set the flags for all subsequent blitting commands.
Blit                    Blit an area from the source to this surface.
TileBlit                Blit an area from the source tiled to this surface.
BatchBlit                Blit a bunch of areas at once.
StretchBlit                Blit an area scaled from the source to the destination rectangle.
TextureTriangles        Preliminary texture mapping support.

Drawing functions     
SetDrawingFlags            Set the flags for all subsequent drawing commands.
FillRectangle            Fill the specified rectangle with the given color following the specified flags.
DrawRectangle            Draw an outline of the specified rectangle with the given color following the specified flags.
DrawLine                Draw a line from one point to the other with the given color following the drawing flags.
DrawLines                Draw 'num_lines' lines with the given color following the drawing flags. Each line specified by a DFBRegion.
FillTriangle            Fill a non-textured triangle.
FillRectangles            Fill a bunch of rectangles with a single call.
FillSpans                Fill spans specified by x and width.
FillTriangles            Fill a bunch of triangles with a single call.

Text functions     
SetFont                    Set the font used by DrawString() and DrawGlyph(). You can pass NULL here to unset the font.
GetFont                    Get the font associated with a surface.
DrawString                Draw a string at the specified position with the given color following the specified flags.
DrawGlyph                Draw a single glyph specified by its character code at the specified position with the given color following the specified flags.
SetEncoding                Change the encoding used for text rendering.

Lightweight helpers     
GetSubSurface            Get an interface to a sub area of this surface.

OpenGL     
GetGL                    Get a unique OpenGL context for this surface.

Debug     
Dump                    Dump the contents of the surface to one or two files.
DisableAcceleration        Disable hardware acceleration.

Resources     
ReleaseSource            Release possible reference to source surface.

Blitting control     
SetIndexTranslation        Set index translation table.

Rendering     
SetRenderOptions        Set options affecting the output of drawing and blitting operations.

Drawing/blitting control     
SetMatrix                Set the transformation matrix.
SetSourceMask            Set the surface to be used as a mask for blitting.

Lightweight helpers     
MakeSubSurface            Make this a sub surface or adjust the rectangle of this sub surface.

Direct Write/Read     
Write                    Write to the surface without the need for (Un)Lock.
Read                    Read from the surface without the need for (Un)Lock.

Drawing/blitting control     
SetColors                Sets color values used for drawing/text functions or alpha/color modulation (blitting functions).

Blitting functions     
BatchBlit2                Blit a bunch of areas at once using secondary source for reading instead of destination.

Buffer operations     
GetPhysicalAddress        Returns the physical address of a locked surface.

Drawing functions     
FillTrapezoids            Fill a bunch of trapezoids with a single call.
*/
    virtual ~IFSurfaceCommon () {}

    virtual IDirectFBSurface * GetSurface () = 0;


    void Clear (unsigned char r, unsigned char g, unsigned char b, unsigned char a);

    void Blit (IFSurfaceCommon & source, DFBRectangle * source_rect, int x, int y);

    void Blit (IDirectFBSurface * source, DFBRectangle * source_rect, int x, int y);

    void StretchBlit (IFSurfaceCommon & source, DFBRectangle * source_rect, DFBRectangle * destination_rect);

    void StretchBlit (IDirectFBSurface * source, DFBRectangle * source_rect, DFBRectangle * destination_rect);


    void Flip (const DFBRegion * region, DFBSurfaceFlipFlags flags);

    void SetColor (unsigned char r, unsigned char g, unsigned char b, unsigned char a);

    //  Set the color like with SetColor() but using an index to the color/alpha lookup table.
    void SetColorIndex(unsigned int index);
    //  Set the source and destination blend function by specifying a Porter/Duff rule.
    void SetPorterDuff(DFBSurfacePorterDuffRule rule);
    void FillRectangle (int x, int y, int w, int h);

    void FillRectangles (DFBRectangle * rects, unsigned int num);

    void DrawRectangle (int x, int y, int w, int h);

    void DrawString (const char * text, int bytes, int x, int y, DFBSurfaceTextFlags flags);

    void DrawGlyph (unsigned int character, int x, int y, DFBSurfaceTextFlags flags);


    void SetBlittingFlags (DFBSurfaceBlittingFlags flags);

    void SetSrcBlendFunction (DFBSurfaceBlendFunction function);

    void SetDstBlendFunction (DFBSurfaceBlendFunction function);

    void SetFont (DFBFont& font);

    void SetAlphaCmpMode (DFB_ACmpOp cmpmode);

    void SetBlitNearestMode (bool enable);

    void SetBlitMirror (bool xmirror_enable, bool ymirror_enable);

    void SetRenderOptions (DFBSurfaceRenderOptions    options);


    /** Retrieving information **/

    /*
    * Return the capabilities of this surface.
    */
    void GetCapabilities (DFBSurfaceCapabilities   *ret_caps    );

    /*
    * Get the surface's position in pixels.
    */
    void  GetPosition (int *ret_x, int *ret_y);

    /*
    * Get the surface's width and height in pixels.
    */
    void GetSize (int *ret_width, int *ret_height);

    /*
    * Created sub surfaces might be clipped by their parents,
    * this function returns the resulting rectangle relative
    * to this surface.
    *
    * For non sub surfaces this function returns
    * { 0, 0, width, height }.
    */
    void GetVisibleRectangle (DFBRectangle *ret_rect);


    /*
    * Get a mask of drawing functions that are hardware
    * accelerated with the current settings.
    *
    * If a source surface is specified the mask will also
    * contain accelerated blitting functions.  Note that there
    * is no guarantee that these will actually be accelerated
    * since the surface storage (video/system) is examined only
    * when something actually gets drawn or blitted.
    */
    void  GetAccelerationMask (IDirectFBSurface *source, DFBAccelerationMask *ret_mask);


    /** Palette & Alpha Ramp **/

    /*
    * Get access to the surface's palette.
    *
    * Returns an interface that can be used to gain
    * read and/or write access to the surface's palette.
    */
    void GetPalette (IDirectFBPalette **ret_interface);

    /*
    * Change the surface's palette.
    */
    void SetPalette (IDirectFBPalette *palette);
        
    /*
    * Set the alpha ramp for formats with one or two alpha bits.
    *
    * Either all four values or the first and the
    * last one are used, depending on the format.
    * Default values are: 0x00, 0x55, 0xaa, 0xff.
    */
    void SetAlphaRamp (u8 a0, u8 a1, u8 a2, u8 a3);
        
        
    /** Buffer operations **/

    /*
    * Lock the surface for the access type specified.
    *
    * Returns a data pointer and the line pitch of it.<br>
    * <br>
    * <b>Note:</b> If the surface is double/triple buffered and
    * the DSLF_WRITE flag is specified, the pointer is to the back
    * buffer.  In all other cases, the pointer is to the front
    * buffer.
    */
    void Lock (
        DFBSurfaceLockFlags       flags,
        void                    **ret_ptr,
        int                      *ret_pitch
        );

    /*
    * Returns the framebuffer offset of a locked surface.
    *
    * The surface must exist in video memory.
    */
    void GetFramebufferOffset (
        int              *offset
        );

    /*
    * Unlock the surface after direct access.
    */
    void Unlock ();


    /** Drawing/blitting control **/


    /*
    * Get the clipping region used to limit the area for
    * drawing, blitting and text functions.
    */
    void GetClip (
        DFBRegion                *ret_clip
        );

    /*
    * Set the source color key, i.e. the color that is excluded
    * when blitting FROM this surface TO another that has
    * source color keying enabled.
    */
    void SetSrcColorKey (
        u8                        r,
        u8                        g,
        u8                        b
        );

    /*
    * Set the source color key like with SetSrcColorKey() but using
    * an index to the color/alpha lookup table.
    *
    * This method is only supported by surfaces with an
    * indexed pixelformat, e.g. DSPF_LUT8. For these formats
    * this method should be used instead of SetSrcColorKey().
    */
    void SetSrcColorKeyIndex (
        unsigned int              index
        );




    /*
    * Set the destination color key, i.e. the only color that
    * gets overwritten by drawing and blitting to this surface
    * when destination color keying is enabled.
    */
    void SetDstColorKey (
        u8                        r,
        u8                        g,
        u8                        b
        );


    /*
    * Set the destination color key like with SetDstColorKey() but using
    * an index to the color/alpha lookup table.
    *
    * This method is only supported by surfaces with an
    * indexed pixelformat, e.g. DSPF_LUT8. For these formats
    * this method should be used instead of SetDstColorKey().
    */
    void SetDstColorKeyIndex (
        unsigned int              index
        );


    /*
    * Blit an area from the source tiled to this surface.
    *
    * Pass a NULL rectangle to use the whole source surface.
    * Source may be the same surface.
    */
    void TileBlit (
        IDirectFBSurface         *source,
        const DFBRectangle       *source_rect,
        int                       x,
        int                       y
        );


    /*
    * Blit a bunch of areas at once.
    *
    * Source may be the same surface.
    */
    void BatchBlit (
        IDirectFBSurface         *source,
        const DFBRectangle       *source_rects,
        const DFBPoint           *dest_points,
        int                       num
        );



    /*
    * Preliminary texture mapping support.
    *
    * Maps a <b>texture</b> onto triangles being built
    * from <b>vertices</b> according to the chosen <b>formation</b>.
    *
    * Optional <b>indices</b> can be used to avoid rearrangement of vertex lists,
    * otherwise the vertex list is processed consecutively, i.e. as if <b>indices</b>
    * are ascending numbers starting at zero.
    *
    * Either the number of <b>indices</b> (if non NULL) or the number of <b>vertices</b> is
    * specified by <b>num</b> and has to be three at least. If the chosen <b>formation</b>
    * is DTTF_LIST it also has to be a multiple of three.
    */
    void TextureTriangles (
        IDirectFBSurface         *texture,
        const DFBVertex          *vertices,
        const int                *indices,
        int                       num,
        DFBTriangleFormation      formation
        );


    /*
    * Set the flags for all subsequent drawing commands.
    */
    void SetDrawingFlags (
        DFBSurfaceDrawingFlags    flags
        );

    /*
    * Draw a line from one point to the other with the given color
    * following the drawing flags.
    */
    void DrawLine (
        int                       x1,
        int                       y1,
        int                       x2,
        int                       y2
        );

    /*
    * Draw 'num_lines' lines with the given color following the
    * drawing flags. Each line specified by a DFBRegion.
    */
    void DrawLines (
        const DFBRegion          *lines,
        unsigned int              num_lines
        );



    /*
    * Fill a non-textured triangle.
    */
    void FillTriangle (
        int                       x1,
        int                       y1,
        int                       x2,
        int                       y2,
        int                       x3,
        int                       y3
        );


    /*
    * Fill spans specified by x and width.
    *
    * Fill <b>num</b> spans with the given color following the
    * drawing flags. Each span is specified by a DFBSpan.
    */
    void FillSpans (
        int                       y,
        const DFBSpan            *spans,
        unsigned int              num
        );


    /*
    * Fill a bunch of triangles with a single call.
    *
    * Fill <b>num</b> triangles with the current color following the
    * drawing flags. Each triangle specified by a DFBTriangle.
    */
    void FillTriangles (
        const DFBTriangle        *tris,
        unsigned int              num
        );

    /*
    * Change the encoding used for text rendering.
    */
    void SetEncoding (
        DFBTextEncodingID         encoding
        );



    /*
    * Get an interface to a sub area of this surface.
    *
    * No image data is duplicated, this is a clipped graphics
    * within the original surface. This is very helpful for
    * lightweight components in a GUI toolkit.  The new
    * surface's state (color, drawingflags, etc.) is
    * independent from this one. So it's a handy graphics
    * context.  If no rectangle is specified, the whole surface
    * (or a part if this surface is a subsurface itself) is
    * represented by the new one.
    */
    void GetSubSurface (
        const DFBRectangle       *rect,
        IDirectFBSurface        **ret_interface
        );

    /*
    * Get a unique OpenGL context for this surface.
    */
    void GetGL (
        IDirectFBGL             **ret_interface
        );


    /*
    * Dump the contents of the surface to one or two files.
    *
    * Creates a PPM file containing the RGB data and a PGM file with
    * the alpha data if present.
    *
    * The complete filenames will be
    * <b>directory</b>/<b>prefix</b>_<i>####</i>.ppm for RGB and
    * <b>directory</b>/<b>prefix</b>_<i>####</i>.pgm for the alpha channel
    * if present. Example: "/directory/prefix_0000.ppm". No existing files
    * will be overwritten.
    */
    void Dump (
        const char               *directory,
        const char               *prefix
        );

    /*
    * Disable hardware acceleration.
    *
    * If any function in <b>mask</b> is set, acceleration will not be used for it.<br/>
    * Default is DFXL_NONE.
    */
    void DisableAcceleration (
        DFBAccelerationMask       mask
        );


    /** Resources **/

    /*
    * Release possible reference to source surface.
    *
    * For performance reasons the last surface that has been used for Blit() and others stays
    * attached to the state of the destination surface to save the overhead of reprogramming
    * the same values each time.
    *
    * That leads to the last source being still around regardless of it being released
    * via its own interface. The worst case is generation of thumbnails using StretchBlit()
    * from a huge surface to a small one. The small thumbnail surface keeps the big one alive,
    * because no other blitting will be done to the small surface afterwards.
    *
    * To solve this, here's the method applications should use in such a case.
    */
    void ReleaseSource (    );


    /** Blitting control **/

    /*
    * Set index translation table.
    *
    * Set the translation table used for fast indexed to indexed
    * pixel format conversion.
    *
    * A negative index means that the pixel will not be written.
    *
    * Undefined indices will be treated like negative ones.
    */
    void SetIndexTranslation (
        const int                *indices,
        int                       num_indices
        );


    /** Drawing/blitting control **/

    /*
    * Set the transformation matrix.
    *
    * Enable usage of this matrix by setting DSRO_MATRIX via IDirectFBSurface::SetRenderOptions().
    *
    * The matrix consists of 3x3 fixed point 16.16 values.
    * The order in the array is from left to right and from top to bottom.
    *
    * All drawing and blitting will be transformed:
    *
    * <pre>
    *        X' = (X * v0 + Y * v1 + v2) / (X * v6 + Y * v7 + v8)
    *        Y' = (X * v3 + Y * v4 + v5) / (X * v6 + Y * v7 + v8)
    * </pre>
    */
    void SetMatrix (
        const s32                *matrix
        );


    void SetSourceMask (
        IDirectFBSurface         *mask,
        int                       x,
        int                       y,
        DFBSurfaceMaskFlags       flags
        );

    /*
    * Make this a sub surface or adjust the rectangle of this sub surface.
    */
    void MakeSubSurface (
        IDirectFBSurface         *from,
        const DFBRectangle       *rect
        );

    /** Direct Write/Read **/

    /*
    * Write to the surface without the need for (Un)Lock.
    *
    * <b>rect</b> defines the area inside the surface.
    * <br><b>ptr</b> and <b>pitch</b> specify the source.
    * <br>The format of the surface and the source data must be the same.
    */
    void Write (
        const DFBRectangle       *rect,
        const void               *ptr,
        int                       pitch
        );

    /*
    * Read from the surface without the need for (Un)Lock.
    *
    * <b>rect</b> defines the area inside the surface to be read.
    * <br><b>ptr</b> and <b>pitch</b> specify the destination.
    * <br>The destination data will have the same format as the surface.
    */
    void Read (
        const DFBRectangle       *rect,
        void                     *ptr,
        int                       pitch
        );


    void DrawString2 (
        const char               *text,
        int                       bytes,
        int                       x,
        int                       y,
        DFBSurfaceTextFlags       flags,
        DFBMatrix                 matrix
        );


        void GetAlphaCmpMode (
        DFB_ACmpOp *          pcmpmode
        );

    /*
    * Blit  from the source to the destination  trapezoid area
    *
    * Pass a NULL destination_trapezoid to use the whole source surface.
    */
    void TrapezoidBlit (
        IDirectFBSurface   *source,
        const DFBRectangle *source_rect,
        const DFBTrapezoid *trapezoid
        );

    /*
    * Fill the specified trapezoid with the given color
    * following the specified flags.
    */
    void FillTrapezoid ( 
        const DFBTrapezoid *trapezoid
        );

    /*
    * Fill a bunch of ovals with a single call.
    *
    * Fill <b>num</b> ovals with the current color following the
    * drawing flags. Each oval specified by a DFBOval.
    */
    void DrawOvals (
        const DFBOval *ovals,
        unsigned int  num
        );


    /*
    * Fill the specified oval with the given color
    * following the specified flags.
    */
    void DrawOval (
        int                       x,
        int                       y,
        int                       w,
        int                       h,
        int                       line_width,
        DFBColor                  line_color
        );

    /*
    * Draw an array of glyph specified by its character codes at the
    * specified positions with the given color following the
    * specified flags.
    *
    * If font was loaded with the DFFA_NOCHARMAP flag, index specifies
    * the raw glyph index in the font.
    *
    * You need to set a font using the SetFont() method before
    * calling this function.
    */
    void DrawGlyphs (
        const unsigned int *character,
        const int *x,
        const int *y,
        DFBSurfaceTextFlags      flags,
        unsigned int             num
        );

    /*
    * Lock the surface for the access type specified.
    *
    * Returns a virtual address  and physical address the line pitch of it.<br>
    * <br>
    * <b>Note:</b> If the surface is double/triple buffered and
    * the DSLF_WRITE flag is specified, the pointer is to the back
    * buffer.  In all other cases, the pointer is to the front
    * buffer.
    */
    void Lock2 (
        DFBSurfaceLockFlags       flags,
        void                    **ret_addr,
        void                    **ret_phys,
        int                      *ret_pitch
        );

    /*
    * Blit an area scaled from the source to the destination
    * rectangle with scaleinfo.
    *
    * Pass a NULL rectangle to use the whole source surface.
    */
    void StretchBlitEx (
        IDirectFBSurface         *source,
        const DFBRectangle       *source_rect,
        const DFBRectangle       *destination_rect,
        const DFBGFX_ScaleInfo   *scale_info
        );


    /*
    * force sync an area fo thsi surface.
    *
    * Pass a NULL sync_rect to sync the whole source surface.
    * flags indicate the sync type
    */
    void Sync(
        DFBSurfaceSyncFlags flags
        );

    /*
    *set gradient drawing end color
    *
    */
    void SetGradientColor2(
        u8 r, u8 g, u8 b, u8 a );

    /*Get Onscreen Rectangle*/
    void GetOnScreenRect (DFBRectangle *pRect);

    /*
    * Check if drawing functions can been hardware
    * accelerated with the current settings.
    *
    * Note that there
    * is no guarantee that these will actually be accelerated
    * since the surface storage (video/system) is examined only
    * when something actually gets drawn or blitted.
    */
    void CanAccelerate (
        IDirectFBSurface         *source,
        DFBAccelerationMask      accel
        );


    /*
    * release surface buffer if possible to save memory usage
    *required by MUF
    */
    void DiscardContents (    );

};

//////////////////////////////////////////////////////////////////////////

// -------------------------------
//      IDirectFB
// -------------------------------

class DFBEnvBase
{
public:

    ~DFBEnvBase ();

    static IDirectFB * GetDFB ();

    static void GetDeviceDescription (DFBGraphicsDeviceDescription * ret_desc);

private:
    DFBEnvBase();

private:
    IDirectFB *m_dfb;
};

//////////////////////////////////////////////////////////////////////////

// -------------------------------
//      IDirectFBDisplayLayer
// -------------------------------
class DFBLayerPrivate;

class DFBLayer : public IFSurfaceCommon
{
    FORBIDDEN_COPY(DFBLayer);

    DECL_PRIVATE(DFBLayer);

public:

    enum
    {
        BUF_MOD_SINGLE = 0,
        BUF_MOD_DOUBLE,
        BUF_MOD_TRIPLE
    };

public:

    explicit DFBLayer (unsigned int id);

    ~DFBLayer();

    IDirectFBDisplayLayer * GetInstance () const;

    IDirectFBSurface * GetSurface ();

    void SetSize (unsigned int width, unsigned int height);

    void SetBufferMode (int mode);

    void RestoreConfig ();

    void SetAutoRestoreConfig (bool en);

    void SetLevel (int level);

    void GetConfiguration (DFBDisplayLayerConfig * ret_config);

    void SetCooperativeLevel (DFBDisplayLayerCooperativeLevel level);

    void SetConfiguration (const DFBDisplayLayerConfig * config);

    void EnableCursor(int enable);


    /*
    * Get the unique layer ID.
    */
    void GetID (DFBDisplayLayerID *ret_layer_id);

    /*
    * Get a description of this display layer, i.e. the capabilities.
    */
    void GetDescription (DFBDisplayLayerDescription *ret_desc);

    /*
    * Get a description of available sources.
    *
    * All descriptions are written to the array pointed to by
    * <b>ret_descriptions</b>. The number of sources is returned by
    * IDirectFBDisplayLayer::GetDescription().
    */
    void GetSourceDescriptions (DFBDisplayLayerSourceDescription *ret_descriptions);
        
    /*
    * For an interlaced display, this returns the currently inactive
    * field: 0 for the top field, and 1 for the bottom field.
    *
    * The inactive field is the one you should draw to next to avoid
    * tearing, the active field is the one currently being displayed.
    *
    * For a progressive output, this should always return 0.  We should
    * also have some other call to indicate whether the display layer
    * is interlaced or progressive, but this is a start.
    */
    void GetCurrentOutputField (int *ret_field);


    // API enable force  update window stack to foce hw flip
    void ForceFullUpdateWindowStack (bool enable);

    // API to bind the Shadowlayer
    void BindShadowLayer (IDirectFBDisplayLayer *other);

    // API to unbind Shadowlayer
    void UnbindShadowLayer (IDirectFBDisplayLayer *other);

    // API  dynamic enable GOP Mirro mode
    void SetHVMirrorEnable (bool HEnable, bool VEnable);

    // API enable GOP LBCouple mode
    void SetLBCoupleEnable (bool LBCoupleEnable);

    // API enable GOP Dst ByPass mode
    void SetGOPDstByPassEnable (bool ByPassEnable);

    // API dynamic set GOP scale 
    void SetHVScale (int HScale, int VScale);


    /*
    * Get an interface to the screen to which the layer belongs.
    */
    void GetScreen (IDirectFBScreen **ret_interface);


    /*
    * Set location on screen as normalized values.
    *
    * So the whole screen is 0.0, 0.0, 1.0, 1.0.
    */
    void SetScreenLocation (
        float                               x,
        float                               y,
        float                               width,
        float                               height
        );

    /*
    * Set location on screen in pixels.
    */
    void SetScreenPosition (
        int                                 x,
        int                                 y
        );

    /*
    * Set location on screen in pixels.
    */
    void SetScreenRectangle (
        int                                 x,
        int                                 y,
        int                                 width,
        int                                 height
        );


    /*
    * Returns the x/y coordinates of the layer's mouse cursor.
    */
    void GetCursorPosition (
        int                                *ret_x,
        int                                *ret_y
        );

    /*
    * Move cursor to specified position.
    *
    * Handles movement like a real one, i.e. generates events.
    */
    void WarpCursor (
        int                                 x,
        int                                 y
        );

    /*
    * Set cursor acceleration.
    *
    * Sets the acceleration of cursor movements. The amount
    * beyond the 'threshold' will be multiplied with the
    * acceleration factor. The acceleration factor is
    * 'numerator/denominator'.
    */
    void SetCursorAcceleration (
        int                                 numerator,
        int                                 denominator,
        int                                 threshold
        );

    /*
    * Set the cursor shape and the hotspot.
    */
    void SetCursorShape (
        IDirectFBSurface                   *shape,
        int                                 hot_x,
        int                                 hot_y
        );






    /*
    * Set the cursor opacity.
    *
    * This function is especially useful if you want
    * to hide the cursor but still want windows on this
    * display layer to receive motion events. In this
    * case, simply set the cursor opacity to zero.
    */
    void SetCursorOpacity (
        u8                                  opacity
        );


    /** Synchronization **/

    /*
    * Wait for next vertical retrace.
    */
    void WaitForSync ();


    /** Contexts **/

    /*
    * Switch the layer context.
    *
    * Switches to the shared context unless <b>exclusive</b> is DFB_TRUE
    * and the cooperative level of this interface is DLSCL_EXCLUSIVE.
    */
    void SwitchContext (
        DFBBoolean                          exclusive
        );


    /** Rotation **/

    /*
    * Set the rotation of data within the layer.
    *
    * Only available in exclusive or administrative mode.
    *
    * Any <b>rotation</b> other than 0, 90, 180 or 270 is not supported.
    *
    * No layer hardware feature usage, only rotated blitting is used.
    */
    void SetRotation (
        int                                 rotation
        );

    /*
    * Get the rotation of data within the layer.
    */
    void GetRotation (
        int                                *ret_rotation
        );


    /*
    * Set the alpha mode of displayer
    * two type can be choose DLOP_OPACITY ,DLOP_ALPHACHANNEL
    */

    void SetAlphaMode (
        DFBDisplayLayerOptions             flag
        );

    /*
    *Set the stretch mode of the displayer
    *choose GOP H stretchmode, V stretch mode, color transparent stretch mode
    *
    */
    void SetStretchMode (
        DFBDisplayLayerStretchSetting      stretch_mode
        );

    /*
    *Set the desk top display mode for supporting stereo 3D UI
    *
    */
    void SetDeskDisplayMode (
        DFBDisplayLayerDeskTopDisplayMode  disktop_mode
        );



    /*
    *API to set layer cursor position
    *
    */
    void MoveCursorTo (int x, int y);

    /*
    *API to set get active window num
    *
    */
    void GetActiveWindowNum (int *ret_num);
        
    /*
    *API to set force update HW status
    *
    */
    void ReactiveLayer ();


    /** Color adjustment **/

    /*
    * Get the layers color adjustment.
    */
    void GetColorAdjustment (DFBColorAdjustment *ret_adj);

    /*
    * Set the layers color adjustment.
    *
    * Only available in exclusive or administrative mode.
    *
    * This function only has an effect if the underlying
    * hardware supports this operation. Check the layers
    * capabilities to find out if this is the case.
    */
    void SetColorAdjustment (const DFBColorAdjustment *adj);

};

//////////////////////////////////////////////////////////////////////////

// -------------------------------
//      IDirectFBWindow
// -------------------------------
class DFBWindowPrivate;

class DFBWindow : public IFSurfaceCommon
{
    FORBIDDEN_COPY(DFBWindow);

    DECL_PRIVATE(DFBWindow);

public:

    explicit DFBWindow (DFBLayer & parent);

    ~DFBWindow();

    IDirectFBWindow * GetInstance ();

    IDirectFBSurface * GetSurface ();

    void SetWindowDescription (DFBWindowDescription desc);

    void Resize (int width, int height);

    void SetOpacity (unsigned char opacity);

    void SetStackingClass (DFBWindowStackingClass stacking_class);

    

    void Move (int dx, int dy);

    void MoveTo (int x, int y);

    void SetColor (unsigned char r, unsigned char g, unsigned char b, unsigned char a);


    /** Retrieving information **/

    /*
    * Get the unique window ID.
    */
    void GetID (DFBWindowID *ret_window_id);

    /*
    * Get the current position of this window.
    */
    void GetPosition (int *ret_x, int *ret_y);

    /*
    * Get the size of the window in pixels.
    */
    void GetSize (int *ret_width, int *ret_height);

    /*
    * Resize the surface of a scalable window.
    *
    * This requires the option DWOP_SCALE.
    * See IDirectFBWindow::SetOptions().
    */
    void ResizeSurface (int width, int height);


    /** Events **/

    /*
    * Create an event buffer for this window and attach it.
    */
    void CreateEventBuffer (IDirectFBEventBuffer **ret_buffer);

    /*
    * Attach an existing event buffer to this window.
    *
    * NOTE: Attaching multiple times generates multiple events.
    *
    */
    void AttachEventBuffer (IDirectFBEventBuffer *buffer);

    /*
    * Detach an event buffer from this window.
    */
    void DetachEventBuffer (IDirectFBEventBuffer *buffer);

    /*
    * Enable specific events to be sent to the window.
    *
    * The argument is a mask of events that will be set in the
    * window's event mask. The default event mask is DWET_ALL.
    */
    void EnableEvents (DFBWindowEventType mask);




    /*
    * Disable specific events from being sent to the window.
    *
    * The argument is a mask of events that will be cleared in
    * the window's event mask. The default event mask is DWET_ALL.
    */
    void DisableEvents (DFBWindowEventType mask);


    /** Options **/

    /*
    * Set options controlling appearance and behaviour of the window.
    */
    void SetOptions (DFBWindowOptions options);

    /*
    * Get options controlling appearance and behaviour of the window.
    */
    void GetOptions (DFBWindowOptions *ret_options);
        

    /*
    * Set the window color key.
    *
    * If a pixel of the window matches this color the
    * underlying window or the background is visible at this
    * point.
    */
    void SetColorKey (u8 r, u8 g, u8 a);


    /*
    * Set the window color key (indexed).
    *
    * If a pixel (indexed format) of the window matches this
    * color index the underlying window or the background is
    * visible at this point.
    */
    void SetColorKeyIndex (unsigned int index);
        

    /*
    * Disable alpha channel blending for one region of the window.
    *
    * If DWOP_ALPHACHANNEL and DWOP_OPAQUE_REGION are set but not DWOP_COLORKEYING
    * and the opacity of the window is 255 the window gets rendered
    * without alpha blending within the specified region.
    *
    * This is extremely useful for alpha blended window decorations while
    * the main content stays opaque and gets rendered faster.
    */
    void SetOpaqueRegion (int x1, int y1, int x2, int y2);


    /*
    * Get the current opacity factor of this window.
    */
    void GetOpacity (u8 *ret_opacity);

    /*
    * Bind a cursor shape to this window.
    *
    * This method will set a per-window cursor shape. Everytime
    * the cursor enters this window, the specified shape is set.
    *
    * Passing NULL will unbind a set shape and release its surface.
    */
    void SetCursorShape(IDirectFBSurface  *shape, int hot_x, int  hot_y);

    /*
    * Set position and size in one step.
    */
    void SetBounds (int  x, int  y, int  width, int height);


     /*
      * Raise the window by one within the window stack.
      */
     void  Raise ();

     /*
      * Lower the window by one within the window stack.
      */
     void Lower ();

     /*
      * Put the window on the top of the window stack.
      */
     void RaiseToTop ();

     /*
      * Send a window to the bottom of the window stack.
      */
     void LowerToBottom ();

     /*
      * Put a window on top of another window.
      */
     void PutAtop (IDirectFBWindow  *lower);

     /*
      * Put a window below another window.
      */
     void PutBelow (IDirectFBWindow  *upper);


   /** Binding **/

     /*
      * Bind a window at the specified position of this window.
      *
      * After binding, bound window will be automatically moved
      * when this window moves to a new position.<br>
      * Binding the same window to multiple windows is not supported.
      * Subsequent call to Bind() automatically unbounds the bound window
      * before binding it again.<br>
      * To move the bound window to a new position call Bind() again
      * with the new coordinates.
      */
     void Bind (
          IDirectFBWindow               *window,
          int                            x,
          int                            y
     );

     /*
      * Unbind a window from this window.
      */
     void Unbind (
          IDirectFBWindow               *window
     );


   /** Focus handling **/

     /*
      * Pass the focus to this window.
      */
     void RequestFocus ();

     /*
      * Grab the keyboard, i.e. all following keyboard events are
      * sent to this window ignoring the focus.
      */
     void GrabKeyboard ();

     /*
      * Ungrab the keyboard, i.e. switch to standard key event
      * dispatching.
      */
     void UngrabKeyboard ();

     /*
      * Grab the pointer, i.e. all following mouse events are
      * sent to this window ignoring the focus.
      */
     void GrabPointer ();

     /*
      * Ungrab the pointer, i.e. switch to standard mouse event
      * dispatching.
      */
     void UngrabPointer ();

     /*
      * Grab a specific key, i.e. all following events of this key are
      * sent to this window ignoring the focus.
      */
     void GrabKey (
          DFBInputDeviceKeySymbol        symbol,
          DFBInputDeviceModifierMask     modifiers
     );

     /*
      * Ungrab a specific key, i.e. switch to standard key event
      * dispatching.
      */
     void UngrabKey (
          DFBInputDeviceKeySymbol        symbol,
          DFBInputDeviceModifierMask     modifiers
     );


     /** Key selection **/

     /*
     * Selects a mode for filtering keys while being focused.
     *
     * The <b>selection</b> defines whether all, none or a specific set (list) of keys is selected.
     * In case of a specific set, the <b>keys</b> array with <b>num_keys</b> has to be provided.
     *
     * Multiple calls to this method are possible. Each overrides all settings from the previous call.
     */
     void SetKeySelection (
         DFBWindowKeySelection          selection,
         const DFBInputDeviceKeySymbol *keys,
         unsigned int                   num_keys
         );

     /*
     * Grab all unselected (filtered out) keys.
     *
     * Unselected keys are those not selected by the focused window. These keys won't be sent
     * to that window. Instead one window in the stack can collect them.
     *
     * See also IDirectFBWindow::UngrabUnselectedKeys() and IDirectFBWindow::SetKeySelection().
     */
     void GrabUnselectedKeys ();

     /*
     * Release the grab of unselected (filtered out) keys.
     *
     * See also IDirectFBWindow::GrabUnselectedKeys() and IDirectFBWindow::SetKeySelection().
     */
     void UngrabUnselectedKeys ();


     /** Advanced Geometry **/

     /*
     * Set area of surface to be shown in window.
     *
     * Default and maximum is to show whole surface.
     */
     void SetSrcGeometry (const DFBWindowGeometry       *geometry);         


     /*
     * Set destination location of window within its bounds.
     *
     * Default and maximum is to fill whole bounds.
     */
     void SetDstGeometry (const DFBWindowGeometry       *geometry);


     /** Properties **/

     /*
     * Set property controlling appearance and behaviour of the window.
     */
     void SetProperty (
         const char                    *key,
         void                          *value,
         void                         **ret_old_value
         );

     /*
     * Get property controlling appearance and behaviour of the window.
     */
     void GetProperty (
         const char                    *key,
         void                         **ret_value
         );

     /*
     * Remove property controlling appearance and behaviour of the window.
     */
     void RemoveProperty (
         const char                    *key,
         void                         **ret_value
         );

     /*
     * Set window rotation.
     */
     void SetRotation (
         int                            rotation
         );

     /** Association **/

     /*
     * Change the window association.
     *
     * If <b>window_id</b> is 0, the window will be dissociated.
     */
     void SetAssociation (
         DFBWindowID                    window_id
         );

     /** Application ID **/

     /*
     * Set application ID.
     *
     * The usage of the application ID is not imposed by DirectFB
     * and can be used at will by the application. Any change will
     * be notified, and as such, an application manager using SaWMan
     * can be used to act on any change.
     */
     void SetApplicationID (
         unsigned long                  application_id
         );

     /*
     * Get current application ID.
     */
     void GetApplicationID (
         unsigned long                 *ret_application_id
         );
};

//////////////////////////////////////////////////////////////////////////

// -------------------------------
//      IDirectFBSurface
// -------------------------------
class DFBSurfacePrivate;

class DFBSurface : public IFSurfaceCommon
{
    FORBIDDEN_COPY(DFBSurface);

    DECL_PRIVATE(DFBSurface);

public:

    explicit DFBSurface (IDirectFBSurface * surface);

    DFBSurface ();

    ~DFBSurface();

    IDirectFBSurface * GetSurface ();

    void SetSurfaceDescription (DFBSurfaceDescription desc);

};

//////////////////////////////////////////////////////////////////////////

// -------------------------------
//      IDirectFBImageProvider
// -------------------------------

class DFBImageProvider
{
    FORBIDDEN_COPY(DFBImageProvider);

public:

    explicit DFBImageProvider (const char * filename);

    ~DFBImageProvider();

    IDirectFBImageProvider * GetInstance ();

    void GetSurfaceDescription (DFBSurfaceDescription * ret_dsc);

    void GetImageDescription (DFBImageDescription * ret_dsc);

    void RenderTo (IFSurfaceCommon & destination, const DFBRectangle * destination_rect);

    void RenderTo (IDirectFBSurface * destination, const DFBRectangle * destination_rect);

    void SetRenderCallback (
        DIRenderCallback          callback,
        void                     *callback_data
        );

    /** Encoding **/

    /*
    * Encode a portion of a surface.
    */
    void WriteBack (IDirectFBSurface *surface,    const DFBRectangle *src_rect, const char *filename);

    /*
    * Set info for the ImageProvider
    */
    void SetHWDecoderParameter(void *hw_decoder_setting);
        

private:
    IDirectFBImageProvider * m_provider;
};

//////////////////////////////////////////////////////////////////////////

// -------------------------------
//      IDirectFBVideoProvider
// -------------------------------

class DFBVideoProvider
{
    FORBIDDEN_COPY(DFBVideoProvider);

public:

    explicit DFBVideoProvider (const char * filename);

    ~DFBVideoProvider();

    IDirectFBVideoProvider * GetInstance ();

    void GetSurfaceDescription (DFBSurfaceDescription * ret_dsc);

    void PlayTo (IDirectFBSurface * destination, const DFBRectangle * destination_rect, DVFrameCallback callback, void * ctx);

    void PlayTo (IFSurfaceCommon & destination, const DFBRectangle * destination_rect, DVFrameCallback callback, void * ctx);

    /*
    * Get a description of the video stream.
    */
    void GetStreamDescription (DFBStreamDescription     *ret_dsc    );

    /*
    * Stop rendering into the destination surface.
    */
    void Stop ();

    /*
    * Get the status of the playback.
    */
    void GetStatus (DFBVideoProviderStatus   *ret_status);


private:
    IDirectFBVideoProvider * m_provider;

};

//////////////////////////////////////////////////////////////////////////

// -------------------------------
//      IDirectFBFont
// -------------------------------

class DFBFont
{
    FORBIDDEN_COPY(DFBFont);

public:
    DFBFont(const char * filename, const  DFBFontDescription * desc);

    ~DFBFont();

    IDirectFBFont * GetInstance ();

    void GetHeight (int * ret_height);

    /** Retrieving information **/

    /*
    * Get the distance from the baseline to the top of the
    * logical extents of this font.
    */
    void GetAscender (    int *ret_ascender);

    /*
    * Get the distance from the baseline to the bottom of
    * the logical extents of this font.
    *
    * This is a negative value!
    */
    void GetDescender (int *ret_descender);


    /*
    * Get the maximum character width.
    *
    * This is a somewhat dubious value. Not all fonts
    * specify it correcly. It can give you an idea of
    * the maximum expected width of a rendered string.
    */
    void GetMaxAdvance (int *ret_maxadvance);


    /*
    * Get the kerning to apply between two glyphs specified by
    * their character codes.
    */
    void GetKerning (unsigned int  prev, unsigned int current, int *ret_kern_x, int *ret_kern_y);


    /** Measurements **/

    /*
    * Get the logical width of the specified string
    * as if it were drawn with this font.
    *
    * Bytes specifies the number of bytes to take from the
    * string or -1 for the complete NULL-terminated string.
    *
    * The returned width may be different than the actual drawn
    * width of the text since this function returns the logical
    * width that should be used to layout the text. A negative
    * width indicates right-to-left rendering.
    */
    void GetStringWidth (
        const char               *text,
        int                       bytes,
        int                      *ret_width
        );

    /*
    * Get the logical and real extents of the specified
    * string as if it were drawn with this font.
    *
    * Bytes specifies the number of bytes to take from the
    * string or -1 for the complete NULL-terminated string.
    *
    * The logical rectangle describes the typographic extents
    * and should be used to layout text. The ink rectangle
    * describes the smallest rectangle containing all pixels
    * that are touched when drawing the string. If you only
    * need one of the rectangles, pass NULL for the other one.
    *
    * The ink rectangle is guaranteed to be a valid rectangle
    * with positive width and height, while the logical
    * rectangle may have negative width indicating right-to-left
    * layout.
    *
    * The rectangle offsets are reported relative to the
    * baseline and refer to the text being drawn using
    * DSTF_LEFT.
    */
    void GetStringExtents (
        const char               *text,
        int                       bytes,
        DFBRectangle             *ret_logical_rect,
        DFBRectangle             *ret_ink_rect
        );


    /*
    * Get the extents of a glyph specified by its character code.
    *
    * The rectangle describes the the smallest rectangle
    * containing all pixels that are touched when drawing the
    * glyph. It is reported relative to the baseline. If you
    * only need the advance, pass NULL for the rectangle.
    *
    * The advance describes the horizontal offset to the next
    * glyph (without kerning applied). It may be a negative
    * value indicating left-to-right rendering. If you don't
    * need this value, pass NULL for advance.
    */
    void GetGlyphExtents (
        unsigned int              character,
        DFBRectangle             *ret_rect,
        int                      *ret_advance
        );


    /*
    * Get the next explicit or automatic break within a string
    * along with the logical width of the text, the string length,
    * and a pointer to the next text line.
    *
    * The bytes specifies the maximum number of bytes to take from the
    * string or -1 for complete NULL-terminated string.
    *
    * The max_width specifies logical width of column onto which
    * the text will be drawn. Then the logical width of fitted
    * text is returned in ret_width. The returned width may overlap
    * the max width specified if there's only one character
    * that fits.
    *
    * The number of characters that fit into this column is
    * returned by the ret_str_length. This value can be used as
    * the number of bytes to take when using DrawString().
    *
    * In ret_next_line a pointer to the next line of text is returned. This
    * will point to NULL or the end of the string if there's no more break.
    */
    void GetStringBreak (
        const char               *text,
        int                       bytes,
        int                       max_width,
        int                      *ret_width,
        int                      *ret_str_length,
        const char              **ret_next_line
        );

    /** Encodings **/

    /*
    * Change the default encoding used when the font is set at a surface.
    *
    * It's also the encoding used for the measurement functions
    * of this interface, e.g. IDirectFBFont::GetStringExtents().
    */
    void SetEncoding (
        DFBTextEncodingID         encoding
        );

    /*
    * Enumerate all provided text encodings.
    */
    void EnumEncodings (
        DFBTextEncodingCallback   callback,
        void                     *context
        );


    /*
    * Find an encoding by its name.
    */
    void FindEncoding (
        const char               *name,
        DFBTextEncodingID        *ret_encoding
        );


private:
    IDirectFBFont * m_font;
};

//////////////////////////////////////////////////////////////////////////

// -------------------------------
//      IDirectFBDataBuffer
// -------------------------------
class DFBDataBuffer
{
    FORBIDDEN_COPY(DFBDataBuffer);

public:

    explicit DFBDataBuffer(DFBDataBufferDescription ddsc);
    ~DFBDataBuffer();

    /*
    * Flushes all data in this buffer.
    *
    * This method only applies to streaming buffers.
    */
    void Flush ();

    /*
    * Finish writing into a streaming buffer.
    *
    * Subsequent calls to PutData will fail,
    * while attempts to fetch data from the buffer will return EOF
    * unless there is still data available.
    */
    void Finish ();

    /*
    * Seeks to a given byte position.
    *
    * This method only applies to static buffers.
    */
    void SeekTo (unsigned int offset);

    /*
    * Get the current byte position within a static buffer.
    *
    * This method only applies to static buffers.
    */
    void GetPosition (unsigned int *ret_offset);


    /*
    * Get the length of a static or streaming buffer in bytes.
    *
    * The length of a static buffer is its static size.
    * A streaming buffer has a variable length reflecting
    * the amount of buffered data.
    */
    void GetLength (
        unsigned int             *ret_length
        );

    /** Waiting for data **/

    /*
    * Wait for data to be available.
    * Thread is idle in the meantime.
    *
    * This method blocks until at least the specified number of bytes
    * is available.
    */
    void WaitForData (
        unsigned int              length
        );

    /*
    * Wait for data to be available within an amount of time.
    * Thread is idle in the meantime.
    *
    * This method blocks until at least the specified number of bytes
    * is available or the timeout is reached.
    */
    void WaitForDataWithTimeout (
        unsigned int              length,
        unsigned int              seconds,
        unsigned int              milli_seconds
        );


    /** Retrieving data **/

    /*
    * Fetch data from a streaming or static buffer.
    *
    * Static buffers will increase the data pointer.
    * Streaming buffers will flush the data portion.
    *
    * The maximum number of bytes to fetch is specified by "length",
    * the actual number of bytes fetched is returned via "read".
    */
    void GetData (
        unsigned int              length,
        void                     *ret_data,
        unsigned int             *ret_read
        );

    /*
    * Peek data from a streaming or static buffer.
    *
    * Unlike GetData() this method won't increase the data
    * pointer or flush any portions of the data held.
    *
    * Additionally an offset relative to the current data pointer
    * or beginning of the streaming buffer can be specified.
    *
    * The maximum number of bytes to peek is specified by "length",
    * the actual number of bytes peeked is returned via "read".
    */
    void PeekData (
        unsigned int              length,
        int                       offset,
        void                     *ret_data,
        unsigned int             *ret_read
        );


private:

    IDirectFBDataBuffer *m_databuffer;
};


//////////////////////////////////////////////////////////////////////////

// -------------------------------
//      Count FPS
// -------------------------------

class PProbePrivate;

class PProbe
{
    DECL_PRIVATE(PProbe);
    FORBIDDEN_COPY(PProbe);

    static const int MAX_PARAMETER_SIZE = 128; // in chars

public:

    explicit PProbe(const char * loginfo = 0, ...);
    ~PProbe();

    void start ();
    void stop ();
    void setRepeatInterval (unsigned int interval = 1);

    float getDuration (); // in second
    float getFPS ();

    const char* getStringFPS ();
};

#endif
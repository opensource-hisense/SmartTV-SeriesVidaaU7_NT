## Makefile.am for DirectFB/gfxdrivers

if GFX_GL
GL_DIR = gl
else
GL_DIR =
endif

if GFX_MSTAR
MSTAR_DIR = mstar
else
MSTAR_DIR =
endif

if GFX_MSTAR_G2
MSTAR_G2_DIR = mstar_g2
else
MSTAR_G2_DIR = 
endif

if GFX_MSTAR_GLES2
MSTAR_GLES2_DIR = mstar_gles2
else
MSTAR_GLES2_DIR =
endif

if GFX_MSTAR_MI
MSTAR_MI_DIR = mstar_mi
else
MSTAR_MI_DIR =
endif

if GFX_MSTAR_STI
MSTAR_STI_DIR = mstar_sti
else
MSTAR_STI_DIR =
endif

SUBDIRS = \
	$(GL_DIR)	\
	$(MSTAR_DIR) \
	$(MSTAR_G2_DIR) \
	$(MSTAR_GLES2_DIR) \
	$(MSTAR_MI_DIR) \
	$(MSTAR_STI_DIR)

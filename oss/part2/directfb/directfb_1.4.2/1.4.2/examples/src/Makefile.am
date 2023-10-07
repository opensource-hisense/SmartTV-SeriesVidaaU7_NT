## Makefile.am for DirectFB-examples/src

SUBDIRS = df_knuckles df_neo spacedream df_gopc df_vec

fontsdatadir = @DATADIR@/fonts

AM_CFLAGS = @DIRECTFB_CFLAGS@ -D_GNU_SOURCE

AM_CXXFLAGS = $(CPPFLAGS) @DIRECTFB_CFLAGS@ -D_GNU_SOURCE

LIBADDS = \
	@DIRECTFB_LIBS@

AM_CPPFLAGS = \
	-DDATADIR=\"@DATADIR@\" \
	-DFONT=\"$(fontsdatadir)/decker.ttf\"  \
	-DFONT_MVF=\"$(fontsdatadir)/special.mvf\"

noinst_PROGRAMS = ve_test

ve_test_SOURCES = ve_test.c ve.c ve.h
ve_test_LDADD   = $(LIBADDS) -lm

if WATER_RUNS
WATER_PROGS = df_matrix_water
else
WATER_PROGS =
endif

bin_PROGRAMS = \
  df_imageprovider \
	df_andi \
	df_bltload \
	df_drivertest \
	df_cpuload \
	df_databuffer \
	df_dok \
	df_fire \
	df_flip \
	df_fonts \
    df_imageprovider_MT_Decode \
	df_input \
	df_joystick \
	df_layer \
	df_line_stretchblit \
	df_matrix \
	df_netload \
	df_dioload \
	df_palette \
	df_palette_colorkeyindex \
	df_palette_i4_i2_i1 \
	df_particle \
	df_porter \
	df_video \
	df_video_particle \
	df_stress \
	df_texture \
	df_window \
	df_window_ForceFullUpdate \
	df_multi_window \
	df_source_rectangle\
	df_layer_palette\
	df_font_stringbreak\
	df_font_style\
	pss \
	$(WATER_PROGS)

df_andi_SOURCES = df_andi.c
df_andi_LDADD  = $(LIBADDS)

df_drivertest_SOURCES = df_drivertest.c
df_drivertest_LDADD  = $(LIBADDS)

df_bltload_SOURCES = df_bltload.c
df_bltload_LDADD  = $(LIBADDS)

df_cpuload_SOURCES = df_cpuload.c
df_cpuload_LDADD  = $(LIBADDS)

df_databuffer_SOURCES = df_databuffer.c
df_databuffer_LDADD  = $(LIBADDS)

df_dok_SOURCES = df_dok.c pngtest3.h
df_dok_LDADD  = $(LIBADDS)

df_fire_SOURCES = df_fire.c
df_fire_LDADD = $(LIBADDS)

df_flip_SOURCES = df_flip.c
df_flip_LDADD = $(LIBADDS)

df_fonts_SOURCES = df_fonts.c
df_fonts_LDADD = $(LIBADDS)

df_imageprovider_MT_Decode_SOURCES = df_imageprovider_MT_Decode.c
df_imageprovider_MT_Decode_LDADD = $(LIBADDS)

df_input_SOURCES = df_input.c
df_input_LDADD = $(LIBADDS)

df_joystick_SOURCES = df_joystick.c
df_joystick_LDADD  = $(LIBADDS) -lm

df_layer_SOURCES = df_layer.c
df_layer_LDADD = $(LIBADDS) -lm

df_matrix_SOURCES = df_matrix.c
df_matrix_LDADD = $(LIBADDS) -lm

df_matrix_water_SOURCES = df_matrix_water.c
df_matrix_water_LDADD = $(LIBADDS) -lm

df_netload_SOURCES = df_netload.c
df_netload_LDADD  = $(LIBADDS)

df_dioload_SOURCES = df_dioload.c
df_dioload_LDADD  = $(LIBADDS)

df_palette_SOURCES = df_palette.c
df_palette_LDADD = $(LIBADDS)

df_palette_colorkeyindex_SOURCES = df_palette_colorkeyindex.c
df_palette_colorkeyindex_LDADD = $(LIBADDS)

df_palette_i4_i2_i1_SOURCES = df_palette_i4_i2_i1.c
df_palette_i4_i2_i1_LDADD = $(LIBADDS)

df_particle_SOURCES = df_particle.c
df_particle_LDADD  = $(LIBADDS) -lm

df_porter_SOURCES = df_porter.c
df_porter_LDADD  = $(LIBADDS)

df_line_stretchblit_SOURCES = df_line_stretchblit.c
df_line_stretchblit_LDADD = $(LIBADDS) -lm

df_stress_SOURCES = df_stress.c
df_stress_LDADD = $(LIBADDS)

df_texture_SOURCES = df_texture.c util.h ve.c ve.h
df_texture_LDADD = $(LIBADDS) -lm

df_video_SOURCES = df_video.c
df_video_LDADD  = $(LIBADDS) -lm

df_video_particle_SOURCES = df_video_particle.c
df_video_particle_LDADD  = $(LIBADDS) -lm

df_window_SOURCES = df_window.c
df_window_LDADD  = $(LIBADDS) -lm

df_window_ForceFullUpdate_SOURCES = df_window_ForceFullUpdate.c
df_window_ForceFullUpdate_LDADD  = $(LIBADDS) -lm

df_multi_window_SOURCES = df_multi_window.c
df_multi_window_LDADD = $(LIBADDS) -lm

df_trapezoid_SOURCES = df_trapezoid.c
df_trapezoid_LDADD = $(LIBADDS) -lm

df_source_rectangle_SOURCES = df_source_rectangle.c
df_source_rectangle_LDADD = $(LIBADDS) -lm

df_layer_palette_SOURCES = df_layer_palette.c
df_layer_palette_LDADD = $(LIBADDS) -lm

df_font_stringbreak_SOURCES = df_font_stringbreak.c
df_font_stringbreak_LDADD = $(LIBADDS) -lm

df_font_style_SOURCES = df_font_style.c
df_font_style_LDADD = $(LIBADDS) -lm

pss_SOURCES = pss.c
pss_LDADD = $(LIBADDS) -lm

df_imageprovider_SOURCES = df_imageprovider.c
df_imageprovider_LDADD   = $(LIBADDS) -lm

df_rop2_SOURCES = df_rop2.c
df_rop2_LDADD  = $(LIBADDS) -lm

df_cpptest_SOURCES = df_cpptest.cpp
df_cpptest_LDADD = $(LIBADDS) -lm

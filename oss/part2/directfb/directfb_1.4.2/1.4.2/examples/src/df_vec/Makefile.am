## Makefile.am for DirectFB-examples/src/df_gopc

AM_CFLAGS = @DIRECTFB_CFLAGS@

LIBADDS = @DIRECTFB_LIBS@ -lm

AM_CPPFLAGS = -DDATADIR=\"@DATADIR@\"

bin_PROGRAMS = df_vec

df_vec_SOURCES = df_vec.c
df_vec_LDADD  = $(LIBADDS)

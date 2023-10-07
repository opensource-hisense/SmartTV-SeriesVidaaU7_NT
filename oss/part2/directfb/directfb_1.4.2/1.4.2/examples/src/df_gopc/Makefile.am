## Makefile.am for DirectFB-examples/src/df_gopc

AM_CFLAGS = @DIRECTFB_CFLAGS@

LIBADDS = @DIRECTFB_LIBS@ -lm

AM_CPPFLAGS = -DDATADIR=\"@DATADIR@\"

bin_PROGRAMS = df_gopc

df_gopc_SOURCES = df_gopc.c
df_gopc_LDADD  = $(LIBADDS)

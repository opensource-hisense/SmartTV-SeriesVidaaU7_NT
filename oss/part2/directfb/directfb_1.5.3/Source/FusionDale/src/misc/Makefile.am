## Makefile.am for FusionDale/src/misc

INCLUDES = \
	-I$(top_builddir)/include	\
	-I$(top_srcdir)/include	\
	-I$(top_srcdir)/src 	\
	$(FUSION_CFLAGS)

AM_CPPFLAGS = -DSYSCONFDIR=\"@sysconfdir@\"
        
noinst_LTLIBRARIES = \
	libfusiondalemisc.la

libfusiondalemisc_la_SOURCES = \
	dale_config.c		\
	dale_config.h

## Makefile.am for FusionDale/tools

INCLUDES = \
	-I$(top_builddir)/include	\
	-I$(top_srcdir)/include		\
	-I$(top_srcdir)/src		\
	$(FUSION_CFLAGS)

bin_PROGRAMS = fddump

fddump_SOURCES = fddump.c
fddump_LDADD   = ../src/libfusiondale.la $(FUSION_LIBS)

EXTRA_DIST = \
	gendoc.pl

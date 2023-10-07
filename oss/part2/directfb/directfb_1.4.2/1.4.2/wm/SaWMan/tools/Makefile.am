## Makefile.am for SaWMan/tools

INCLUDES = \
	-I$(top_builddir)/include	\
	-I$(top_builddir)/src		\
	-I$(top_srcdir)/include		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/lib     \
	-I$(top_srcdir)/wm/SaWMan/include \
	-I$(top_srcdir)/wm/SaWMan/src \
	$(DFB_CFLAGS)


bin_PROGRAMS = swmdump

CPPFLAGS += -DSAWMAN_VERSION=\"1.5.3\"

swmdump_SOURCES = swmdump.c
swmdump_LDADD   = $(DFB_LIBS) ../src/libsawman.la \
        $(top_builddir)/lib/direct/libdirect.la \
        $(top_builddir)/src/libdirectfb.la \
        $(top_builddir)/lib/fusion/libfusion.la

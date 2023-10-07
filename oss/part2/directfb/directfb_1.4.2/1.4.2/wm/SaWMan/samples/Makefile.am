## Makefile.am for SaWMan/samples

INCLUDES    = \
	-I$(top_srcdir)/include	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/wm/SaWMan/include \
	$(DFB_CFLAGS)


bin_PROGRAMS = sample1 testman testrun


sample1_SOURCES = sample1.c
sample1_LDADD   = $(DFB_LIBS) ../src/libsawman.la \
        $(top_builddir)/lib/direct/libdirect.la \
        $(top_builddir)/src/libdirectfb.la \
        $(top_builddir)/lib/fusion/libfusion.la

testman_SOURCES = testman.c
testman_LDADD   = $(DFB_LIBS) ../src/libsawman.la \
        $(top_builddir)/lib/direct/libdirect.la \
        $(top_builddir)/src/libdirectfb.la \
        $(top_builddir)/lib/fusion/libfusion.la

testrun_SOURCES = testrun.c
testrun_LDADD   = $(DFB_LIBS) ../src/libsawman.la \
        $(top_builddir)/lib/direct/libdirect.la \
        $(top_builddir)/src/libdirectfb.la \
        $(top_builddir)/lib/fusion/libfusion.la

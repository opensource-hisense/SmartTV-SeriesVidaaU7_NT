## Makefile.am for DirectFB/tests

INCLUDES = \
	-I$(top_builddir)/lib \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-DLINUX_2_6


bin_PROGRAMS = \
        dfbtest_layer \
	dfbtest_blit	\
	dfbtest_reinit	\
	dfbtest_scale	\
	dfbtest_sync	\
	dfbtest_window	\
	dfbtest_window2 \
	dfbtest_mview   \
	direct_stream	\
	direct_test	\
	fusion_fork	\
	fusion_reactor	\
	fusion_skirmish	\
	fusion_stream \
	fifo_read \
	fifo_write

libdirectfb = $(top_builddir)/src/libdirectfb.la
libfusion = $(top_builddir)/lib/fusion/libfusion.la
libdirect = $(top_builddir)/lib/direct/libdirect.la

dfbtest_layer_SOURCES = dfbtest_layer.c
dfbtest_layer_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

dfbtest_blit_SOURCES = dfbtest_blit.c
dfbtest_blit_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

dfbtest_reinit_SOURCES = dfbtest_reinit.c
dfbtest_reinit_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

dfbtest_scale_SOURCES = dfbtest_scale.c
dfbtest_scale_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

dfbtest_sync_SOURCES = dfbtest_sync.c
dfbtest_sync_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

dfbtest_window_SOURCES = dfbtest_window.c
dfbtest_window_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

dfbtest_window2_SOURCES = dfbtest_window2.c
dfbtest_window2_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

dfbtest_mview_SOURCES = dfbtest_mview.c
dfbtest_mview_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

direct_stream_SOURCES = direct_stream.c
direct_stream_LDADD   = $(libdirect)

direct_test_SOURCES = direct_test.c
direct_test_LDADD   = $(libdirect)


fusion_fork_SOURCES = fusion_fork.c
fusion_fork_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

fusion_reactor_SOURCES = fusion_reactor.c
fusion_reactor_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

fusion_skirmish_SOURCES = fusion_skirmish.c
fusion_skirmish_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

fusion_stream_SOURCES = fusion_stream.c
fusion_stream_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

fifo_read_SOURCES = fifo_read.c
fifo_read_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

fifo_write_SOURCES = fifo_write.c
fifo_write_LDADD   = $(libdirectfb) $(libfusion) $(libdirect)

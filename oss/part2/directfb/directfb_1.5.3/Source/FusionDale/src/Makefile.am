## Makefile.am for FusionDale/src

SUBDIRS = coma core messenger misc

INCLUDES = \
	-I$(top_builddir)/include	\
	-I$(top_srcdir)/include	\
	-I$(top_srcdir)/src 	\
	$(FUSION_CFLAGS)


lib_LTLIBRARIES = libfusiondale.la

libfusiondale_la_SOURCES = \
	fusiondale.c		\
	ifusiondale.c		\
	ifusiondale.h

libfusiondale_la_LIBADD = \
	$(FUSION_LIBS)				\
	coma/libfusiondalecoma.la		\
	core/libfusiondalecore.la		\
	messenger/libfusiondalemessenger.la	\
	misc/libfusiondalemisc.la

libfusiondale_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-release $(LT_RELEASE)

#
## and now rebuild the static version with the *correct* object files
#
if BUILD_STATIC

clean-local:
	rm -f libfusiondale_fixed.a

all-local: libfusiondale_fixed.a

libfusiondale_fixed.a: .libs/libfusiondale.a
	rm -f libfusiondale_fixed.a
	${AR} cru libfusiondale_fixed.a `find -name "*.o" | grep -v core | grep -v ifusion`
	${RANLIB} libfusiondale_fixed.a
	cp -af libfusiondale_fixed.a .libs/libfusiondale.a

.libs/libfusiondale.a: libfusiondale.la

else

clean-local:

all-local:

endif

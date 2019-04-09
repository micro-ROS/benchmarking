ACLOCAL_AMFLAGS = -I m4

CFLAGS="-g -O2"
CXXFLAGS="-g -O2"

export EXT_LIBS=  \
	  -L$(abs_top_builddir)/ext/libcjson/.libs/		\
	  -L$(abs_top_builddir)/ext/libswo/libswo/.libs/	\
	  -L$(abs_top_builddir)/ext/libini/.libs/		\
	  -L$(abs_top_builddir)/ext/libmfa/src/.libs/		\
	  -L$(abs_top_builddir)/ext/openocd/src/.libs/		\
	  -L$(abs_top_builddir)/ext/openocd/jimtcl/

SUBDIRS= src \
 	 apps

maintainer-clean-local:
	rm -rf @ac_list_folder_ext@

MAINTAINERCLEANFILES = \
	$(srcdir)/INSTALL \
	$(srcdir)/configure \
	$(srcdir)/Makefile.in \
	$(srcdir)/depcomp \
	$(srcdir)/config.guess \
	$(srcdir)/config.sub \
	$(srcdir)/inc/onfig.h.in \
	$(srcdir)/inc/config.h.in~ \
	$(srcdir)/compile \
	$(srcdir)/ltmain.sh \
	$(srcdir)/missing \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/install-sh 


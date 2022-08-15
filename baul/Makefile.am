AM_CPPFLAGS =						\
	$(WARN_CFLAGS)					\
	-DFR_DATADIR=\"$(datadir)\"			\
	-DCAFELOCALEDIR=\""$(datadir)/locale"\" 	\
	-I$(top_srcdir)					\
	-I$(top_builddir)				\
	$(BAUL_CFLAGS)

baul_extensiondir=$(BAUL_EXTENSION_DIR)

baul_extension_LTLIBRARIES=libbaul-grapa.la

libbaul_grapa_la_SOURCES =     \
	baul-grapa.c           \
	baul-grapa.h           \
	grapa-module.c

libbaul_grapa_la_LDFLAGS = -module -avoid-version -no-undefined
libbaul_grapa_la_LIBADD  = $(BAUL_LIBS)

extensiondir = $(datadir)/baul/extensions
extension_in_files = libbaul-grapa.baul-extension.desktop.in
extension_DATA = $(extension_in_files:.baul-extension.desktop.in=.baul-extension)
$(extension_DATA): $(extension_in_files)
	$(AM_V_GEN) $(MSGFMT) --desktop --keyword=Name --keyword=Description --template $< -d $(top_srcdir)/po -o $@

CLEANFILES = $(extension_DATA)

EXTRA_DIST = $(extension_in_files)
-include $(top_srcdir)/git.mk

SUBDIRS = icons

desktopdir = $(datadir)/applications
desktop_in_in_files = grapa.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in) 
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
$(desktop_DATA): $(desktop_in_files)
	$(AM_V_GEN) $(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

matchdir = $(datadir)/grapa
match_DATA = packages.match

servicedir = $(datadir)/dbus-1/services
service_in_files = org.cafe.Grapa.service.in
service_DATA = $(service_in_files:.service.in=.service)
$(service_DATA): $(service_in_files) Makefile
	$(AM_V_GEN) $(SED) -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

appdatadir = $(datadir)/metainfo
appdata_in_files = grapa.appdata.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)
$(appdata_DATA): $(appdata_in_files)
	$(AM_V_GEN) $(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

gsettingsschema_in_files = org.cafe.grapa.gschema.xml.in
gsettings_SCHEMAS = $(gsettingsschema_in_files:.xml.in=.xml)
.PRECIOUS: $(gsettings_SCHEMAS)

@GSETTINGS_RULES@

man_MANS = grapa.1

ctk_update_icon_cache = ctk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-cache
uninstall-hook: update-cache
update-cache:
	if test -z "$(DESTDIR)"; then \
		echo "Updating Ctk icon cache."; \
		$(ctk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(ctk_update_icon_cache)"; \
	fi

EXTRA_DIST = \
       packages.match.in \
       packages.match \
       $(desktop_in_in_files) \
       $(desktop_in_files) \
       $(desktop_DATA) \
       $(gsettingsschema_in_files) \
       $(appdata_in_files) \
       $(man_MANS) \
       $(service_in_files) \
       $(NULL)

DISTCLEANFILES = \
       $(desktop_in_files) \
       $(desktop_DATA) \
       $(service_DATA) \
       $(NULL)

CLEANFILES = \
       $(gsettings_SCHEMAS) \
       grapa.appdata.xml \
       $(NULL)

dist-hook:
	cd $(distdir); rm -f $(CLEANFILES)

-include $(top_srcdir)/git.mk

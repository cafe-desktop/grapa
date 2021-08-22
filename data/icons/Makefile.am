iconsdir = $(datadir)/icons/hicolor

nobase_dist_icons_DATA = \
	16x16/actions/grapa_add-files-to-archive.png \
	16x16/actions/grapa_add-folder-to-archive.png \
	16x16/actions/grapa_extract-archive.png \
	16x16/apps/grapa.png \
	22x22/apps/grapa.png \
	24x24/actions/grapa_add-files-to-archive.png \
	24x24/actions/grapa_add-folder-to-archive.png \
	24x24/actions/grapa_extract-archive.png \
	24x24/actions/grapa-ok.png \
	24x24/actions/grapa-no.png \
	24x24/apps/grapa.png \
	32x32/apps/grapa.png \
	scalable/apps/grapa.svg

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(iconsdir)

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

-include $(top_srcdir)/git.mk

@YELP_HELP_RULES@

HELP_ID = grapa
HELP_FILES = index.docbook legal.xml
HELP_MEDIA = 					\
	figures/grapa_home.png		\
	figures/grapa_leftarrow.png	\
	figures/grapa_main_window.png	\
	figures/grapa_rightarrow.png	\
	figures/grapa_uparrow.png

# Add linguas to be ignored, e.g. IGNORE_HELP_LINGUAS = ca de es fr
IGNORE_HELP_LINGUAS =
HELP_LINGUAS = $(if $(IGNORE_HELP_LINGUAS), \
	$(filter-out $(IGNORE_HELP_LINGUAS),$(subst /,,$(dir $(wildcard */*.po)))), \
	$(subst /,,$(dir $(wildcard */*.po))) )

-include $(top_srcdir)/git.mk

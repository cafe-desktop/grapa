/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  Grapa
 *
 *  Copyright (C) 2001 The Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <config.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <ctk/ctk.h>

#include "ctk-utils.h"

#define LOAD_BUFFER_SIZE 65536


static void
count_selected (CtkTreeModel *model,
		CtkTreePath  *path,
		CtkTreeIter  *iter,
		gpointer      data)
{
	int *n = data;
	*n = *n + 1;
}


int
_ctk_count_selected (CtkTreeSelection *selection)
{
	int n = 0;

	if (selection == NULL)
		return 0;
	ctk_tree_selection_selected_foreach (selection, count_selected, &n);
	return n;
}



CtkWidget *
grapa_dialog_add_button (CtkDialog   *dialog,
			 const gchar *button_text,
			 const gchar *icon_name,
			       gint   response_id)
{
	CtkWidget *button;

	button = ctk_button_new_with_mnemonic (button_text);
	ctk_button_set_image (CTK_BUTTON (button), ctk_image_new_from_icon_name (icon_name, CTK_ICON_SIZE_BUTTON));

	ctk_button_set_use_underline (CTK_BUTTON (button), TRUE);
	ctk_style_context_add_class (ctk_widget_get_style_context (button), "text-button");
	ctk_widget_set_can_default (button, TRUE);
	ctk_widget_show (button);
	ctk_dialog_add_action_widget (CTK_DIALOG (dialog), button, response_id);

	return button;
}


CtkWidget*
_ctk_message_dialog_new (CtkWindow        *parent,
			 CtkDialogFlags    flags,
			 const char       *icon_name,
			 const char       *message,
			 const char       *secondary_message,
			 const gchar      *first_button_text,
			 ...)
{
	CtkWidget    *dialog;
	CtkWidget    *label;
	CtkWidget    *image;
	CtkWidget    *hbox;
	CtkWidget    *content_area;
	va_list       args;
	const gchar  *text;
	int           response_id;
	char         *markup_text;

	g_return_val_if_fail ((message != NULL) || (secondary_message != NULL), NULL);

	if (icon_name == NULL)
		icon_name = "dialog-information";

	dialog = ctk_dialog_new_with_buttons ("", parent, flags, NULL, NULL);
	content_area = ctk_dialog_get_content_area (CTK_DIALOG (dialog));

	/* Add label and image */

	image = ctk_image_new_from_icon_name (icon_name, CTK_ICON_SIZE_DIALOG);
	ctk_widget_set_valign (image, CTK_ALIGN_START);

	label = ctk_label_new ("");

	if (message != NULL) {
		char *escaped_message;

		escaped_message = g_markup_escape_text (message, -1);
		if (secondary_message != NULL) {
			char *escaped_secondary_message = g_markup_escape_text (secondary_message, -1);
			markup_text = g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s",
						       escaped_message,
						       escaped_secondary_message);
			g_free (escaped_secondary_message);
		}
		else
			markup_text = g_strdup (escaped_message);
		g_free (escaped_message);
	}
	else
		markup_text = g_markup_escape_text (secondary_message, -1);

	ctk_label_set_markup (CTK_LABEL (label), markup_text);
	g_free (markup_text);

	ctk_label_set_line_wrap (CTK_LABEL (label), TRUE);
	ctk_label_set_selectable (CTK_LABEL (label), TRUE);

	hbox = ctk_box_new (CTK_ORIENTATION_HORIZONTAL, 24);
	ctk_container_set_border_width (CTK_CONTAINER (hbox), 5);

	ctk_box_pack_start (CTK_BOX (hbox), image,
			    FALSE, FALSE, 0);

	ctk_box_pack_start (CTK_BOX (hbox), label,
			    TRUE, TRUE, 0);

	ctk_box_pack_start (CTK_BOX (content_area),
			    hbox,
			    FALSE, FALSE, 0);

#if CTK_CHECK_VERSION (3,99,0)
	ctk_widget_show (hbox);
#else
	ctk_widget_show_all (hbox);
#endif

	/* Add buttons */

	if (first_button_text == NULL)
		return dialog;

	va_start (args, first_button_text);

	text = first_button_text;
	response_id = va_arg (args, gint);

	while (text != NULL) {
		ctk_dialog_add_button (CTK_DIALOG (dialog), text, response_id);

		text = va_arg (args, char*);
		if (text == NULL)
			break;
		response_id = va_arg (args, int);
	}

	va_end (args);

	return dialog;
}


static CtkWidget *
create_button (const char *icon_name,
	       const char *text)
{
	CtkIconTheme *icon_theme;
	CtkWidget    *button;

	button = ctk_button_new_with_mnemonic (text);
	icon_theme = ctk_icon_theme_get_default ();
	if (ctk_icon_theme_has_icon (icon_theme, icon_name)) {
		CtkWidget *image;
		image = ctk_image_new_from_icon_name (icon_name, CTK_ICON_SIZE_BUTTON);
		ctk_button_set_image (CTK_BUTTON (button), image);
	}
	ctk_widget_set_can_default (button, TRUE);

	ctk_widget_show (button);

	return button;
}


char *
_ctk_request_dialog_run (CtkWindow        *parent,
			 CtkDialogFlags    flags,
			 const char       *title,
			 const char       *message,
			 const char       *default_value,
			 int               max_length,
			 const gchar      *no_button_text,
			 const gchar      *yes_button_text)
{
	CtkWidget    *dialog;
	CtkWidget    *label;
	CtkWidget    *image;
	CtkWidget    *hbox;
	CtkWidget    *vbox;
	CtkWidget    *entry;
	CtkWidget    *button;
	CtkWidget    *content_area;
	char         *result;

	dialog = ctk_dialog_new_with_buttons (title, parent, flags, NULL, NULL);
	content_area = ctk_dialog_get_content_area (CTK_DIALOG (dialog));

	/* Add label and image */

	image = ctk_image_new_from_icon_name ("dialog-question", CTK_ICON_SIZE_DIALOG);
	ctk_widget_set_halign (image, CTK_ALIGN_CENTER);
	ctk_widget_set_valign (image, CTK_ALIGN_START);

	label = ctk_label_new_with_mnemonic (message);
	ctk_label_set_line_wrap (CTK_LABEL (label), TRUE);
	ctk_label_set_selectable (CTK_LABEL (label), FALSE);
	ctk_label_set_xalign (CTK_LABEL (label), 0.0);
	ctk_label_set_yalign (CTK_LABEL (label), 0.0);

	entry = ctk_entry_new ();
	ctk_entry_set_width_chars (CTK_ENTRY (entry), 50);
	ctk_entry_set_activates_default (CTK_ENTRY (entry), TRUE);
	ctk_entry_set_max_length (CTK_ENTRY (entry), max_length);
	ctk_entry_set_text (CTK_ENTRY (entry), default_value);
	ctk_label_set_mnemonic_widget (CTK_LABEL (label), entry);

	hbox = ctk_box_new (CTK_ORIENTATION_HORIZONTAL, 12);
	vbox = ctk_box_new (CTK_ORIENTATION_VERTICAL, 12);

	ctk_container_set_border_width (CTK_CONTAINER (dialog), 5);
	ctk_container_set_border_width (CTK_CONTAINER (hbox), 5);

	ctk_box_set_spacing (CTK_BOX (content_area), 14); /* 14 + 2 * 5 = 24 */
	ctk_container_set_border_width (CTK_CONTAINER (vbox), 5);
	ctk_box_set_spacing (CTK_BOX (vbox), 6);

	ctk_box_pack_start (CTK_BOX (hbox), image, FALSE, FALSE, 0);
	ctk_box_pack_start (CTK_BOX (hbox), vbox, TRUE, TRUE, 0);
	ctk_box_pack_start (CTK_BOX (vbox), label, FALSE, FALSE, 0);
	ctk_box_pack_start (CTK_BOX (vbox), entry, FALSE, FALSE, 0);
	ctk_box_pack_start (CTK_BOX (content_area), hbox, FALSE, FALSE, 0);

#if CTK_CHECK_VERSION (3,99,0)
	ctk_widget_show (hbox);
#else
	ctk_widget_show_all (hbox);
#endif

	/* Add buttons */

	button = create_button ("process-stop", no_button_text);
	ctk_dialog_add_action_widget (CTK_DIALOG (dialog),
				      button,
				      CTK_RESPONSE_CANCEL);

	button = create_button ("ctk-ok", yes_button_text);
	ctk_dialog_add_action_widget (CTK_DIALOG (dialog),
				      button,
				      CTK_RESPONSE_YES);

	ctk_dialog_set_default_response (CTK_DIALOG (dialog), CTK_RESPONSE_YES);

	/* Run dialog */

	ctk_widget_grab_focus (entry);

	if (ctk_dialog_run (CTK_DIALOG (dialog)) == CTK_RESPONSE_YES)
		result = g_strdup (ctk_entry_get_text (CTK_ENTRY (entry)));
	else
		result = NULL;

	ctk_widget_destroy (dialog);

	return result;
}


CtkWidget*
_ctk_error_dialog_new (CtkWindow        *parent,
		       CtkDialogFlags    flags,
		       GList            *row_output,
		       const char       *primary_text,
		       const char       *secondary_text,
		       ...)
{
	CtkWidget     *dialog;
	CtkWidget     *label;
	CtkWidget     *image;
	CtkWidget     *hbox;
	CtkWidget     *vbox;
	CtkWidget     *text_view;
	CtkWidget     *scrolled = NULL;
	CtkWidget     *expander;
	CtkWidget     *content_area;
	CtkTextBuffer *text_buf;
	CtkTextIter    iter;
	GList         *scan;
	char          *escaped_message, *markup_text;
	va_list        args;
	gboolean       view_output = (row_output != NULL);

	dialog = ctk_dialog_new ();
	ctk_window_set_title (CTK_WINDOW (dialog), "");
	ctk_window_set_transient_for (CTK_WINDOW (dialog), parent);
	if (flags & CTK_DIALOG_DESTROY_WITH_PARENT)
		ctk_window_set_destroy_with_parent (CTK_WINDOW (dialog), TRUE);
	if (flags & CTK_DIALOG_MODAL)
		ctk_window_set_modal (CTK_WINDOW (dialog), TRUE);
	ctk_dialog_add_button (CTK_DIALOG (dialog), CTK_STOCK_OK, CTK_RESPONSE_OK);

	ctk_window_set_resizable (CTK_WINDOW (dialog), FALSE);
	ctk_dialog_set_default_response (CTK_DIALOG (dialog), CTK_RESPONSE_OK);

	content_area = ctk_dialog_get_content_area (CTK_DIALOG (dialog));

	/* Add label and image */

	image = ctk_image_new_from_icon_name ("dialog-error", CTK_ICON_SIZE_DIALOG);
	ctk_widget_set_halign (image, CTK_ALIGN_CENTER);
	ctk_widget_set_valign (image, CTK_ALIGN_START);

	label = ctk_label_new ("");
	ctk_label_set_line_wrap (CTK_LABEL (label), TRUE);
	ctk_label_set_selectable (CTK_LABEL (label), TRUE);
	ctk_label_set_xalign (CTK_LABEL (label), 0.0);
	ctk_label_set_yalign (CTK_LABEL (label), 0.0);

	escaped_message = g_markup_escape_text (primary_text, -1);
	if (secondary_text != NULL) {
		char *secondary_message;
		char *escaped_secondary_message;

		va_start (args, secondary_text);
		secondary_message = g_strdup_vprintf (secondary_text, args);
		va_end (args);
		escaped_secondary_message = g_markup_escape_text (secondary_message, -1);

		markup_text = g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s",
					       escaped_message,
					       escaped_secondary_message);

		g_free (escaped_secondary_message);
		g_free (secondary_message);
	}
	else
		markup_text = g_strdup (escaped_message);
	ctk_label_set_markup (CTK_LABEL (label), markup_text);
	g_free (markup_text);
	g_free (escaped_message);

	if (view_output) {
		ctk_widget_set_size_request (dialog, 500, -1);

		/* Expander */

		expander = ctk_expander_new_with_mnemonic (_("Command _Line Output"));
		ctk_expander_set_expanded (CTK_EXPANDER (expander), secondary_text == NULL);

		/* Add text */

		scrolled = ctk_scrolled_window_new (NULL, NULL);
		ctk_scrolled_window_set_policy (CTK_SCROLLED_WINDOW (scrolled),
						CTK_POLICY_AUTOMATIC,
						CTK_POLICY_AUTOMATIC);
		ctk_scrolled_window_set_shadow_type (CTK_SCROLLED_WINDOW (scrolled),
						     CTK_SHADOW_ETCHED_IN);
		ctk_widget_set_size_request (scrolled, -1, 200);

		text_buf = ctk_text_buffer_new (NULL);
		ctk_text_buffer_create_tag (text_buf, "monospace",
					    "family", "monospace", NULL);
		ctk_text_buffer_get_iter_at_offset (text_buf, &iter, 0);
		for (scan = row_output; scan; scan = scan->next) {
			char *line = scan->data;
			char *utf8_line;
			gsize bytes_written;

			utf8_line = g_locale_to_utf8 (line, -1, NULL, &bytes_written, NULL);
			ctk_text_buffer_insert_with_tags_by_name (text_buf,
								  &iter,
								  utf8_line,
								  bytes_written,
								  "monospace", NULL);
			g_free (utf8_line);

			ctk_text_buffer_insert (text_buf, &iter, "\n", 1);
		}
		text_view = ctk_text_view_new_with_buffer (text_buf);
		g_object_unref (text_buf);
		ctk_text_view_set_editable (CTK_TEXT_VIEW (text_view), FALSE);
		ctk_text_view_set_cursor_visible (CTK_TEXT_VIEW (text_view), FALSE);
	}

	vbox = ctk_box_new (CTK_ORIENTATION_VERTICAL, 6);

	hbox = ctk_box_new (CTK_ORIENTATION_HORIZONTAL, 12);
	ctk_box_pack_start (CTK_BOX (hbox), image, FALSE, FALSE, 0);
	ctk_box_pack_start (CTK_BOX (hbox), label, FALSE, FALSE, 0);
	ctk_box_pack_start (CTK_BOX (vbox), hbox, TRUE, TRUE, 0);

	if (view_output) {
		ctk_container_add (CTK_CONTAINER (scrolled), text_view);
		ctk_container_add (CTK_CONTAINER (expander), scrolled);
		ctk_box_pack_start (CTK_BOX (vbox), expander, TRUE, TRUE, 0);
	}

	ctk_box_pack_start (CTK_BOX (content_area), vbox, FALSE, FALSE, 0);

	ctk_container_set_border_width (CTK_CONTAINER (dialog), 5);
	ctk_container_set_border_width (CTK_CONTAINER (hbox), 5);
	ctk_box_set_spacing (CTK_BOX (content_area), 14); /* 14 + 2 * 5 = 24 */

#if CTK_CHECK_VERSION (3,99,0)
	ctk_widget_show (vbox);
#else
	ctk_widget_show_all (vbox);
#endif

	return dialog;
}


void
_ctk_error_dialog_run (CtkWindow        *parent,
		       const gchar      *main_message,
		       const gchar      *format,
		       ...)
{
	CtkWidget *d;
	char      *message;
	va_list    args;

	va_start (args, format);
	message = g_strdup_vprintf (format, args);
	va_end (args);

	d =  _ctk_message_dialog_new (parent,
				      CTK_DIALOG_MODAL,
				      "dialog-error",
				      main_message,
				      message,
				      CTK_STOCK_CLOSE, CTK_RESPONSE_CANCEL,
				      NULL);
	g_free (message);

	g_signal_connect (G_OBJECT (d), "response",
			  G_CALLBACK (ctk_widget_destroy),
			  NULL);

	ctk_widget_show (d);
}


void
_ctk_entry_set_locale_text (CtkEntry   *entry,
			    const char *text)
{
	char *utf8_text;

	if (text == NULL)
		return;

	utf8_text = g_locale_to_utf8 (text, -1, NULL, NULL, NULL);
	if (utf8_text != NULL)
		ctk_entry_set_text (entry, utf8_text);
	else
		ctk_entry_set_text (entry, "");
	g_free (utf8_text);
}


char *
_ctk_entry_get_locale_text (CtkEntry *entry)
{
	const char *utf8_text;
	char       *text;

	utf8_text = ctk_entry_get_text (entry);
	if (utf8_text == NULL)
		return NULL;

	text = g_locale_from_utf8 (utf8_text, -1, NULL, NULL, NULL);

	return text;
}


void
_ctk_entry_set_filename_text (CtkEntry   *entry,
			      const char *text)
{
	char *utf8_text;

	utf8_text = g_filename_to_utf8 (text, -1, NULL, NULL, NULL);
	if (utf8_text != NULL) {
		ctk_entry_set_text (entry, utf8_text);
		g_free (utf8_text);
	} else
		ctk_entry_set_text (entry, "");
}


static GdkPixbuf *
get_themed_icon_pixbuf (GThemedIcon  *icon,
		        int           size,
		        CtkIconTheme *icon_theme)
{
	char        **icon_names;
	CtkIconInfo  *icon_info;
	GdkPixbuf    *pixbuf;
	GError       *error = NULL;

	g_object_get (icon, "names", &icon_names, NULL);

	icon_info = ctk_icon_theme_choose_icon (icon_theme, (const char **)icon_names, size, 0);
	if (icon_info == NULL)
		icon_info = ctk_icon_theme_lookup_icon (icon_theme, "text-x-generic", size, CTK_ICON_LOOKUP_USE_BUILTIN);

	pixbuf = ctk_icon_info_load_icon (icon_info, &error);
	if (pixbuf == NULL) {
		g_warning ("could not load icon pixbuf: %s\n", error->message);
		g_clear_error (&error);
	}

	g_object_unref (icon_info);
	g_strfreev (icon_names);

	return pixbuf;
}


static GdkPixbuf *
get_file_icon_pixbuf (GFileIcon *icon,
		      int        size)
{
	GFile     *file;
	char      *filename;
	GdkPixbuf *pixbuf;

	file = g_file_icon_get_file (icon);
	filename = g_file_get_path (file);
	pixbuf = gdk_pixbuf_new_from_file_at_size (filename, size, -1, NULL);
	g_free (filename);
	g_object_unref (file);

	return pixbuf;
}


GdkPixbuf *
get_icon_pixbuf (GIcon        *icon,
		 int           size,
		 CtkIconTheme *theme)
{
	if (icon == NULL)
		return NULL;
	if (G_IS_THEMED_ICON (icon))
		return get_themed_icon_pixbuf (G_THEMED_ICON (icon), size, theme);
	if (G_IS_FILE_ICON (icon))
		return get_file_icon_pixbuf (G_FILE_ICON (icon), size);
	return NULL;
}


GdkPixbuf *
get_mime_type_pixbuf (const char   *mime_type,
		      int           icon_size,
		      CtkIconTheme *icon_theme)
{
	GdkPixbuf *pixbuf = NULL;
	GIcon     *icon;

	if (icon_theme == NULL)
		icon_theme = ctk_icon_theme_get_default ();

	icon = g_content_type_get_icon (mime_type);
	pixbuf = get_icon_pixbuf (icon, icon_size, icon_theme);
	g_object_unref (icon);

	return pixbuf;
}


void
show_help_dialog (CtkWindow  *parent,
		  const char *section)
{
	char   *uri;
	GError *error = NULL;

	uri = g_strconcat ("help:grapa", section ? "/" : NULL, section, NULL);
	if (! ctk_show_uri_on_window (parent, uri, CDK_CURRENT_TIME, &error)) {
		CtkWidget *dialog;

		dialog = _ctk_message_dialog_new (parent,
						  CTK_DIALOG_DESTROY_WITH_PARENT,
						  "dialog-error",
						  _("Could not display help"),
						  error->message,
						  CTK_STOCK_OK, CTK_RESPONSE_OK,
						  NULL);
		ctk_dialog_set_default_response (CTK_DIALOG (dialog), CTK_RESPONSE_OK);

		g_signal_connect (G_OBJECT (dialog), "response",
				  G_CALLBACK (ctk_widget_destroy),
				  NULL);

		ctk_window_set_resizable (CTK_WINDOW (dialog), FALSE);

		ctk_widget_show (dialog);

		g_clear_error (&error);
	}
	g_free (uri);
}


CtkBuilder *
_ctk_builder_new_from_resource (const char *resource_path)
{
	CtkBuilder *builder;
	char       *full_path;
	GError     *error = NULL;

	builder = ctk_builder_new ();
	full_path = g_strconcat (GRAPA_RESOURCE_UI_PATH G_DIR_SEPARATOR_S, resource_path, NULL);
        if (! ctk_builder_add_from_resource (builder, full_path, &error)) {
                g_warning ("%s\n", error->message);
                g_clear_error (&error);
        }
	g_free (full_path);

        return builder;
}


CtkWidget *
_ctk_builder_get_widget (CtkBuilder *builder,
			 const char *name)
{
	return (CtkWidget *) ctk_builder_get_object (builder, name);
}


int
_ctk_widget_lookup_for_size (CtkWidget *widget,
                             CtkIconSize icon_size)
{
	int w, h;
	ctk_icon_size_lookup (icon_size,
			      &w, &h);
	return MAX (w, h);
}


gchar *
grapa_file_chooser_get_current_folder_uri (CtkFileChooser *chooser)
{
	GFile *file;
	gchar *uri;

	g_return_val_if_fail (CTK_IS_FILE_CHOOSER (chooser), NULL);

#if CTK_CHECK_VERSION (3,99,0)
	file = ctk_file_chooser_get_current_folder (chooser);
#else
	file = ctk_file_chooser_get_current_folder_file (chooser);
#endif
	if (!file)
		return NULL;

	uri = g_file_get_uri (file);
	g_object_unref (file);

	return uri;
}

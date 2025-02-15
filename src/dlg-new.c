/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  Grapa
 *
 *  Copyright (C) 2008 Free Software Foundation, Inc.
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
#include <math.h>
#include <unistd.h>
#include <gio/gio.h>
#include "dlg-new.h"
#include "file-utils.h"
#include "ctk-utils.h"
#include "fr-init.h"
#include "preferences.h"


#define GET_WIDGET(x) (_ctk_builder_get_widget (data->builder, (x)))
#define DEFAULT_EXTENSION ".tar.gz"
#define MEGABYTE (1024.0 * 1024.0)


/* called when the main dialog is closed. */
static void
destroy_cb (CtkWidget  *widget G_GNUC_UNUSED,
	    DlgNewData *data)
{
	g_object_unref (data->builder);
	g_free (data);
}


static void
update_sensitivity (DlgNewData *data)
{
	ctk_toggle_button_set_inconsistent (CTK_TOGGLE_BUTTON (data->n_encrypt_header_checkbutton), ! data->can_encrypt_header);
	ctk_widget_set_sensitive (data->n_encrypt_header_checkbutton, data->can_encrypt_header);
	ctk_widget_set_sensitive (data->n_volume_spinbutton, ! data->can_create_volumes || ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->n_volume_checkbutton)));
}


static void
update_sensitivity_for_ext (DlgNewData *data,
			    const char *ext)
{
	const char *mime_type;
	int         i;

	data->can_encrypt = FALSE;
	data->can_encrypt_header = FALSE;
	data->can_create_volumes = FALSE;

	mime_type = get_mime_type_from_extension (ext);

	if (mime_type == NULL) {
		ctk_widget_set_sensitive (data->n_password_entry, FALSE);
		ctk_widget_set_sensitive (data->n_password_label, FALSE);
		ctk_widget_set_sensitive (data->n_encrypt_header_checkbutton, FALSE);
		ctk_widget_set_sensitive (data->n_volume_box, FALSE);
		return;
	}

	for (i = 0; mime_type_desc[i].mime_type != NULL; i++) {
		if (strcmp (mime_type_desc[i].mime_type, mime_type) == 0) {
			data->can_encrypt = mime_type_desc[i].capabilities & FR_COMMAND_CAN_ENCRYPT;
			ctk_widget_set_sensitive (data->n_password_entry, data->can_encrypt);
			ctk_widget_set_sensitive (data->n_password_label, data->can_encrypt);

			data->can_encrypt_header = mime_type_desc[i].capabilities & FR_COMMAND_CAN_ENCRYPT_HEADER;
			ctk_widget_set_sensitive (data->n_encrypt_header_checkbutton, data->can_encrypt_header);

			data->can_create_volumes = mime_type_desc[i].capabilities & FR_COMMAND_CAN_CREATE_VOLUMES;
			ctk_widget_set_sensitive (data->n_volume_box, data->can_create_volumes);

			break;
		}
	}

	update_sensitivity (data);
}


static int
get_archive_type (DlgNewData *data)
{
	const char *uri;
	const char *ext;

	uri = ctk_file_chooser_get_uri (CTK_FILE_CHOOSER (data->dialog));
	if (uri == NULL)
		return -1;

	ext = get_archive_filename_extension (uri);
	if (ext == NULL) {
		int idx;

		idx = egg_file_format_chooser_get_format (EGG_FILE_FORMAT_CHOOSER (data->format_chooser), uri);
		/*idx = ctk_combo_box_get_active (CTK_COMBO_BOX (data->n_archive_type_combo_box)) - 1;*/
		if (idx >= 0)
			return data->supported_types[idx];

		ext = DEFAULT_EXTENSION;
	}

	return get_mime_type_index (get_mime_type_from_extension (ext));
}


/* FIXME
static void
archive_type_combo_box_changed_cb (CtkComboBox *combo_box,
				   DlgNewData  *data)
{
	const char *uri;
	const char *ext;
	int         idx;

	uri = ctk_file_chooser_get_uri (CTK_FILE_CHOOSER (data->dialog));

	ext = get_archive_filename_extension (uri);
	idx = ctk_combo_box_get_active (CTK_COMBO_BOX (data->n_archive_type_combo_box)) - 1;
	if ((ext == NULL) && (idx >= 0))
		ext = mime_type_desc[data->supported_types[idx]].default_ext;

	update_sensitivity_for_ext (data, ext);

	if ((idx >= 0) && (uri != NULL)) {
		const char *new_ext;
		const char *basename;
		char       *basename_noext;
		char       *new_basename;
		char       *new_basename_uft8;

		new_ext = mime_type_desc[data->supported_types[idx]].default_ext;
		basename = file_name_from_path (uri);
		if (g_str_has_suffix (basename, ext))
			basename_noext = g_strndup (basename, strlen (basename) - strlen (ext));
		else
			basename_noext = g_strdup (basename);
		new_basename = g_strconcat (basename_noext, new_ext, NULL);
		new_basename_uft8 = g_uri_unescape_string (new_basename, NULL);

		ctk_file_chooser_set_current_name (CTK_FILE_CHOOSER (data->dialog), new_basename_uft8);
		update_sensitivity_for_ext (data, new_ext);

		g_free (new_basename_uft8);
		g_free (new_basename);
		g_free (basename_noext);
	}
}
*/


static void
password_entry_changed_cb (CtkEditable *editable G_GNUC_UNUSED,
			   gpointer     user_data)
{
	update_sensitivity ((DlgNewData *) user_data);
}


static void
volume_toggled_cb (CtkToggleButton *toggle_button G_GNUC_UNUSED,
		   gpointer         user_data)
{
	update_sensitivity ((DlgNewData *) user_data);
}


static void
format_chooser_selection_changed_cb (EggFileFormatChooser *format_chooser G_GNUC_UNUSED,
				     DlgNewData           *data)
{
	const char *uri;
	const char *ext;
	int         n_format;

	uri = ctk_file_chooser_get_uri (CTK_FILE_CHOOSER (data->dialog));
	if (uri == NULL)
		return;

	ext = get_archive_filename_extension (uri);
	n_format = egg_file_format_chooser_get_format (EGG_FILE_FORMAT_CHOOSER (data->format_chooser), uri);
	if (ext == NULL)
		ext = mime_type_desc[data->supported_types[n_format - 1]].default_ext;

	update_sensitivity_for_ext (data, ext);

	if (uri != NULL) {
		const char *new_ext;
		const char *basename;
		char       *basename_noext;
		char       *new_basename;
		char       *new_basename_uft8;

		new_ext = mime_type_desc[data->supported_types[n_format - 1]].default_ext;
		basename = file_name_from_path (uri);
		if (g_str_has_suffix (basename, ext))
			basename_noext = g_strndup (basename, strlen (basename) - strlen (ext));
		else
			basename_noext = g_strdup (basename);
		new_basename = g_strconcat (basename_noext, new_ext, NULL);
		new_basename_uft8 = g_uri_unescape_string (new_basename, NULL);

		ctk_file_chooser_set_current_name (CTK_FILE_CHOOSER (data->dialog), new_basename_uft8);
		update_sensitivity_for_ext (data, new_ext);

		g_free (new_basename_uft8);
		g_free (new_basename);
		g_free (basename_noext);
	}
}


static char *
get_icon_name_for_type (const char *mime_type)
{
	char *name = NULL;

	if (mime_type != NULL) {
		char *s;

		name = g_strconcat ("cafe-mime-", mime_type, NULL);
		for (s = name; *s; ++s)
			if (! g_ascii_isalpha (*s))
				*s = '-';
	}

	if ((name == NULL) || ! ctk_icon_theme_has_icon (ctk_icon_theme_get_default (), name)) {
		g_free (name);
		name = g_strdup ("package-x-generic");
	}

	return name;
}


static void
options_expander_unmap_cb (CtkWidget *widget G_GNUC_UNUSED,
			   gpointer   user_data)
{
	egg_file_format_chooser_emit_size_changed ((EggFileFormatChooser *) user_data);
}


static DlgNewData *
dlg_new_archive (FrWindow  *window,
		int        *supported_types,
		const char *default_name)
{
	DlgNewData    *data;
	CtkWidget     *n_new_button;
        GSettings *settings;
	/*char          *default_ext;*/
	int            i;

	data = g_new0 (DlgNewData, 1);

	data->builder = _ctk_builder_new_from_resource ("new.ui");
	if (data->builder == NULL) {
		g_free (data);
		return NULL;
	}

	data->window = window;
	data->supported_types = supported_types;
	sort_mime_types_by_description (data->supported_types);

	/* Get the widgets. */

	data->dialog = _ctk_builder_get_widget (data->builder, "filechooserdialog");

	data->n_password_entry = _ctk_builder_get_widget (data->builder, "n_password_entry");
	data->n_password_label = _ctk_builder_get_widget (data->builder, "n_password_label");
	data->n_other_options_expander = _ctk_builder_get_widget (data->builder, "n_other_options_expander");
	data->n_encrypt_header_checkbutton = _ctk_builder_get_widget (data->builder, "n_encrypt_header_checkbutton");

	data->n_volume_checkbutton = _ctk_builder_get_widget (data->builder, "n_volume_checkbutton");
	data->n_volume_spinbutton = _ctk_builder_get_widget (data->builder, "n_volume_spinbutton");
	data->n_volume_box = _ctk_builder_get_widget (data->builder, "n_volume_box");

	n_new_button = _ctk_builder_get_widget (data->builder, "n_new_button");

	/* Set widgets data. */

	ctk_dialog_set_default_response (CTK_DIALOG (data->dialog), CTK_RESPONSE_OK);
	ctk_file_chooser_set_current_folder_uri (CTK_FILE_CHOOSER (data->dialog), fr_window_get_open_default_dir (window));

	if (default_name != NULL)
		ctk_file_chooser_set_current_name (CTK_FILE_CHOOSER (data->dialog), default_name);

	/**/

	ctk_button_set_label (CTK_BUTTON (n_new_button), _("C_reate"));
	ctk_button_set_image (CTK_BUTTON (n_new_button),
			      ctk_image_new_from_icon_name ("grapa_add-files-to-archive", CTK_ICON_SIZE_BUTTON));

	ctk_expander_set_expanded (CTK_EXPANDER (data->n_other_options_expander), FALSE);
	settings = g_settings_new (GRAPA_SCHEMA_GENERAL);
        ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (data->n_encrypt_header_checkbutton), g_settings_get_boolean (settings, PREF_GENERAL_ENCRYPT_HEADER));
        g_object_unref (settings);

        settings = g_settings_new (GRAPA_SCHEMA_BATCH_ADD);
        ctk_spin_button_set_value (CTK_SPIN_BUTTON (data->n_volume_spinbutton), g_settings_get_int (settings, PREF_BATCH_ADD_VOLUME_SIZE) / MEGABYTE);
        g_object_unref (settings);

	/* format chooser */

	data->format_chooser = (EggFileFormatChooser *) egg_file_format_chooser_new ();
	for (i = 0; data->supported_types[i] != -1; i++) {
		int   idx = data->supported_types[i];
		const char *exts[4];
		int   e;
		int   n_exts;
		char *icon_name;

		n_exts = 0;
		for (e = 0; (n_exts < 4) && file_ext_type[e].ext != NULL; e++) {
			if (strcmp (file_ext_type[e].ext, mime_type_desc[idx].default_ext) == 0)
				continue;
			if (strcmp (file_ext_type[e].mime_type, mime_type_desc[idx].mime_type) == 0)
				exts[n_exts++] = file_ext_type[e].ext;
		}
		while (n_exts < 4)
			exts[n_exts++] = NULL;

		/* g_print ("%s => %s, %s, %s, %s\n", mime_type_desc[idx].mime_type, exts[0], exts[1], exts[2], exts[3]); */

		icon_name = get_icon_name_for_type (mime_type_desc[idx].mime_type);
		egg_file_format_chooser_add_format (data->format_chooser,
						    0,
						    _(mime_type_desc[idx].name),
						    icon_name,
						    mime_type_desc[idx].default_ext,
						    exts[0],
						    exts[1],
						    exts[2],
						    exts[3],
						    NULL);

		g_free (icon_name);
	}
	egg_file_format_chooser_set_format (data->format_chooser, 0);
	ctk_widget_show (CTK_WIDGET (data->format_chooser));
	ctk_box_pack_start (CTK_BOX (GET_WIDGET ("format_chooser_box")), CTK_WIDGET (data->format_chooser), TRUE, TRUE, 0);
	ctk_widget_set_vexpand (GET_WIDGET ("extra_widget"), FALSE);

	/* Set the signals handlers. */

	/*g_signal_connect (G_OBJECT (data->dialog),
			  "response",
			  G_CALLBACK (new_file_response_cb),
			  data);*/

	g_signal_connect (G_OBJECT (data->dialog),
			  "destroy",
			  G_CALLBACK (destroy_cb),
			  data);

	/*
	g_signal_connect_swapped (G_OBJECT (cancel_button),
				  "clicked",
				  G_CALLBACK (ctk_widget_destroy),
				  G_OBJECT (data->dialog));
	g_signal_connect (G_OBJECT (add_button),
			  "clicked",
			  G_CALLBACK (add_clicked_cb),
			  data);*/

	/* FIXME g_signal_connect (G_OBJECT (data->n_archive_type_combo_box),
			  "changed",
			  G_CALLBACK (archive_type_combo_box_changed_cb),
			  data); */
	g_signal_connect (G_OBJECT (data->n_password_entry),
			  "changed",
			  G_CALLBACK (password_entry_changed_cb),
			  data);
	g_signal_connect (G_OBJECT (data->n_volume_checkbutton),
			  "toggled",
			  G_CALLBACK (volume_toggled_cb),
			  data);
	g_signal_connect (G_OBJECT (data->format_chooser),
			  "selection-changed",
			  G_CALLBACK (format_chooser_selection_changed_cb),
			  data);
	g_signal_connect_after (GET_WIDGET ("other_oprtions_alignment"),
				"unmap",
				G_CALLBACK (options_expander_unmap_cb),
				data->format_chooser);

	/* Run dialog. */

	update_sensitivity (data);

	ctk_window_set_modal (CTK_WINDOW (data->dialog), TRUE);
	ctk_window_set_transient_for (CTK_WINDOW (data->dialog), CTK_WINDOW (data->window));
	/*ctk_window_present (CTK_WINDOW (data->dialog));*/

	return data;
}


DlgNewData *
dlg_new (FrWindow *window)
{
	DlgNewData *data;

	data = dlg_new_archive (window, create_type, NULL);
	ctk_window_set_title (CTK_WINDOW (data->dialog), C_("File", "New"));

	return data;
}


DlgNewData *
dlg_save_as (FrWindow   *window,
	     const char *default_name)
{
	DlgNewData *data;

	data = dlg_new_archive (window, save_type, default_name);
	ctk_window_set_title (CTK_WINDOW (data->dialog), C_("File", "Save"));

	return data;
}


const char *
dlg_new_data_get_password (DlgNewData *data)
{
	const char *password = NULL;
	int         idx;

	idx = get_archive_type (data);
	if (idx < 0)
		return NULL;

	if (mime_type_desc[idx].capabilities & FR_COMMAND_CAN_ENCRYPT)
		password = (char*) ctk_entry_get_text (CTK_ENTRY (data->n_password_entry));

	return password;
}


gboolean
dlg_new_data_get_encrypt_header (DlgNewData *data)
{
	gboolean encrypt_header = FALSE;
	int      idx;

	idx = get_archive_type (data);
	if (idx < 0)
		return FALSE;

	if (mime_type_desc[idx].capabilities & FR_COMMAND_CAN_ENCRYPT) {
		const char *password = ctk_entry_get_text (CTK_ENTRY (data->n_password_entry));
		if (password != NULL) {
			if (strcmp (password, "") != 0) {
				if (mime_type_desc[idx].capabilities & FR_COMMAND_CAN_ENCRYPT_HEADER)
					encrypt_header = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->n_encrypt_header_checkbutton));
			}
		}
	}

	return encrypt_header;
}


int
dlg_new_data_get_volume_size (DlgNewData *data)
{
	guint volume_size = 0;
	int   idx;

	idx = get_archive_type (data);
	if (idx < 0)
		return 0;

	if ((mime_type_desc[idx].capabilities & FR_COMMAND_CAN_CREATE_VOLUMES)
	    && ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->n_volume_checkbutton)))
	{
		double value;

		value = ctk_spin_button_get_value (CTK_SPIN_BUTTON (data->n_volume_spinbutton));
		volume_size = floor (value * MEGABYTE);

	}

	return volume_size;
}

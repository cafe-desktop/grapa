/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  Grapa
 *
 *  Copyright (C) 2001, 2003, 2004, 2005 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <ctk/ctk.h>
#include "file-utils.h"
#include "fr-init.h"
#include "ctk-utils.h"
#include "fr-window.h"
#include "typedefs.h"
#include "dlg-extract.h"

#define GET_WIDGET(x) (_ctk_builder_get_widget (data->builder, (x)))

typedef struct {
	FrWindow     *window;
	GSettings    *settings;
	GList        *selected_files;
	char         *base_dir_for_selection;

	CtkWidget    *dialog;

	CtkBuilder   *builder;

	gboolean      extract_clicked;
} DialogData;


/* called when the main dialog is closed. */
static void
destroy_cb (CtkWidget  *widget G_GNUC_UNUSED,
	    DialogData *data)
{
	if (! data->extract_clicked) {
		fr_window_pop_message (data->window);
		fr_window_stop_batch (data->window);
	}
	g_object_unref (data->builder);
	path_list_free (data->selected_files);
	g_free (data->base_dir_for_selection);
	g_object_unref (data->settings);
	g_free (data);
}


static int
extract_cb (CtkWidget  *w G_GNUC_UNUSED,
	    DialogData *data)
{
	FrWindow   *window = data->window;
	gboolean    do_not_extract = FALSE;
	char       *extract_to_dir;
	gboolean    overwrite;
	gboolean    skip_newer;
	gboolean    selected_files;
	gboolean    pattern_files;
	gboolean    junk_paths;
	GList      *file_list;
	char       *base_dir = NULL;
	GError     *error = NULL;

	data->extract_clicked = TRUE;

	/* collect extraction options. */

	extract_to_dir = ctk_file_chooser_get_uri (CTK_FILE_CHOOSER (data->dialog));

	/* check directory existence. */

	if (! uri_is_dir (extract_to_dir)) {
		if (! ForceDirectoryCreation) {
			CtkWidget *d;
			int        r;
			char      *folder_name;
			char      *msg;

			folder_name = g_filename_display_name (extract_to_dir);
			msg = g_strdup_printf (_("Destination folder \"%s\" does not exist.\n\nDo you want to create it?"), folder_name);
			g_free (folder_name);

			d = _ctk_message_dialog_new (CTK_WINDOW (data->dialog),
						     CTK_DIALOG_MODAL,
						     "dialog-question",
						     msg,
						     NULL,
						     CTK_STOCK_CANCEL, CTK_RESPONSE_CANCEL,
						     _("Create _Folder"), CTK_RESPONSE_YES,
						     NULL);

			ctk_dialog_set_default_response (CTK_DIALOG (d), CTK_RESPONSE_YES);
			r = ctk_dialog_run (CTK_DIALOG (d));
			ctk_widget_destroy (CTK_WIDGET (d));

			g_free (msg);

			if (r != CTK_RESPONSE_YES)
				do_not_extract = TRUE;
		}

		if (! do_not_extract && ! ensure_dir_exists (extract_to_dir, 0755, &error)) {
			CtkWidget  *d;

			d = _ctk_error_dialog_new (CTK_WINDOW (window),
						   CTK_DIALOG_DESTROY_WITH_PARENT,
						   NULL,
						   _("Extraction not performed"),
						   _("Could not create the destination folder: %s."),
						   error->message);
			ctk_dialog_run (CTK_DIALOG (d));
			ctk_widget_destroy (CTK_WIDGET (d));

			g_error_free (error);

			return FALSE;
		}
	}

	if (do_not_extract) {
		CtkWidget *d;

		d = _ctk_message_dialog_new (CTK_WINDOW (window),
					     CTK_DIALOG_DESTROY_WITH_PARENT,
					     "dialog-warning",
					     _("Extraction not performed"),
					     NULL,
					     CTK_STOCK_OK, CTK_RESPONSE_OK,
					     NULL);
		ctk_dialog_set_default_response (CTK_DIALOG (d), CTK_RESPONSE_OK);
		ctk_dialog_run (CTK_DIALOG (d));
		ctk_widget_destroy (CTK_WIDGET (d));

		if (fr_window_is_batch_mode (data->window))
			ctk_widget_destroy (data->dialog);

		return FALSE;
	}

	/* check extraction directory permissions. */

	if (uri_is_dir (extract_to_dir)
	    && ! check_permissions (extract_to_dir, R_OK | W_OK))
	{
		CtkWidget *d;
		char      *utf8_path;

		utf8_path = g_filename_display_name (extract_to_dir);

		d = _ctk_error_dialog_new (CTK_WINDOW (window),
					   CTK_DIALOG_DESTROY_WITH_PARENT,
					   NULL,
					   _("Extraction not performed"),
					   _("You don't have the right permissions to extract archives in the folder \"%s\""),
					   utf8_path);
		ctk_dialog_run (CTK_DIALOG (d));
		ctk_widget_destroy (CTK_WIDGET (d));

		g_free (utf8_path);
		g_free (extract_to_dir);

		return FALSE;
	}

	fr_window_set_extract_default_dir (window, extract_to_dir, TRUE);

	overwrite = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("overwrite_checkbutton")));
	skip_newer = ! ctk_toggle_button_get_inconsistent (CTK_TOGGLE_BUTTON (GET_WIDGET ("not_newer_checkbutton"))) && ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("not_newer_checkbutton")));
	junk_paths = ! ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("recreate_dir_checkbutton")));

	g_settings_set_boolean (data->settings, PREF_EXTRACT_OVERWRITE, overwrite);
	if (! ctk_toggle_button_get_inconsistent (CTK_TOGGLE_BUTTON (GET_WIDGET ("not_newer_checkbutton"))))
		g_settings_set_boolean (data->settings, PREF_EXTRACT_SKIP_NEWER, skip_newer);
	g_settings_set_boolean (data->settings, PREF_EXTRACT_RECREATE_FOLDERS, ! junk_paths);

	selected_files = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("selected_files_radiobutton")));
	pattern_files = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("file_pattern_radiobutton")));

	/* create the file list. */

	file_list = NULL;

	if (selected_files) {
		file_list = data->selected_files;
		data->selected_files = NULL;       /* do not the list when destroying the dialog. */
	}
	else if (pattern_files) {
		const char *pattern;

		pattern = ctk_entry_get_text (CTK_ENTRY (GET_WIDGET ("file_pattern_entry")));
		file_list = fr_window_get_file_list_pattern (window, pattern);
		if (file_list == NULL) {
			ctk_widget_destroy (data->dialog);
			g_free (extract_to_dir);
			return FALSE;
		}
	}

	if (selected_files) {
		base_dir = data->base_dir_for_selection;
		data->base_dir_for_selection = NULL;
	}
	else
		base_dir = NULL;

	/* close the dialog. */

	ctk_widget_destroy (data->dialog);

	/* extract ! */

	fr_window_archive_extract (window,
				   file_list,
				   extract_to_dir,
				   base_dir,
				   skip_newer,
				   overwrite ? FR_OVERWRITE_YES : FR_OVERWRITE_NO,
				   junk_paths,
				   TRUE);

	path_list_free (file_list);
	g_free (extract_to_dir);
	g_free (base_dir);

	return TRUE;
}


static int
file_sel_response_cb (CtkWidget    *widget,
		      int           response,
		      DialogData   *data)
{
	if ((response == CTK_RESPONSE_CANCEL) || (response == CTK_RESPONSE_DELETE_EVENT)) {
		ctk_widget_destroy (data->dialog);
		return TRUE;
	}

	if (response == CTK_RESPONSE_HELP) {
		show_help_dialog (CTK_WINDOW (data->dialog), "grapa-extract-options");
		return TRUE;
	}

	if (response == CTK_RESPONSE_OK)
		return extract_cb (widget, data);

	return FALSE;
}


static void
files_entry_changed_cb (CtkWidget  *widget G_GNUC_UNUSED,
			DialogData *data)
{
	if (! ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("file_pattern_radiobutton"))))
		ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("file_pattern_radiobutton")), TRUE);
}


static void
overwrite_toggled_cb (CtkToggleButton *button,
		      DialogData      *data)
{
	gboolean active = ctk_toggle_button_get_active (button);
	ctk_toggle_button_set_inconsistent (CTK_TOGGLE_BUTTON (GET_WIDGET ("not_newer_checkbutton")), !active);
	ctk_widget_set_sensitive (GET_WIDGET ("not_newer_checkbutton"), active);
}


static void
dlg_extract__common (FrWindow *window,
	             GList    *selected_files,
	             char     *base_dir_for_selection)
{
	DialogData *data;

	data = g_new0 (DialogData, 1);
	if ((data->builder = _ctk_builder_new_from_resource ("dlg-extract.ui")) == NULL) {
		g_free (data);
		return;
	}
	data->settings = g_settings_new (GRAPA_SCHEMA_EXTRACT);
	data->window = window;
	data->selected_files = selected_files;
	data->base_dir_for_selection = base_dir_for_selection;
	data->extract_clicked = FALSE;
	data->dialog = GET_WIDGET ("dialog_extract");

	/* Set widgets data. */

	ctk_file_chooser_set_current_folder_uri (CTK_FILE_CHOOSER (data->dialog), fr_window_get_extract_default_dir (window));

	if (data->selected_files != NULL)
		ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("selected_files_radiobutton")), TRUE);
	else {
		ctk_widget_set_sensitive (GET_WIDGET ("selected_files_radiobutton"), FALSE);
		ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("all_files_radiobutton")), TRUE);
	}

	ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("overwrite_checkbutton")), g_settings_get_boolean (data->settings, PREF_EXTRACT_OVERWRITE));
	ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("not_newer_checkbutton")), g_settings_get_boolean (data->settings, PREF_EXTRACT_SKIP_NEWER));

	if (!ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("overwrite_checkbutton")))) {
		ctk_toggle_button_set_inconsistent (CTK_TOGGLE_BUTTON (GET_WIDGET ("not_newer_checkbutton")), TRUE);
		ctk_widget_set_sensitive (GET_WIDGET ("not_newer_checkbutton"), FALSE);
	}

	ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (GET_WIDGET ("recreate_dir_checkbutton")), g_settings_get_boolean (data->settings, PREF_EXTRACT_RECREATE_FOLDERS));

	/* Set the signals handlers. */

	ctk_builder_add_callback_symbols (data->builder,
	                                  "on_dialog_extract_destroy", G_CALLBACK (destroy_cb),
	                                  "on_dialog_extract_response", G_CALLBACK (file_sel_response_cb),
	                                  "on_overwrite_checkbutton_toggled", G_CALLBACK (overwrite_toggled_cb),
	                                  "on_file_pattern_entry_changed", G_CALLBACK (files_entry_changed_cb),
	                                  NULL);
	ctk_builder_connect_signals (data->builder, data);

	/* Run dialog. */

	ctk_window_set_modal (CTK_WINDOW (data->dialog), TRUE);
	ctk_widget_show (data->dialog);
}


void
dlg_extract (CtkWidget *widget G_GNUC_UNUSED,
	     gpointer   callback_data)
{
	FrWindow *window = callback_data;
	GList    *files;
	char     *base_dir;

	files = fr_window_get_selection (window, FALSE, &base_dir);
	dlg_extract__common (window, files, base_dir);
}


void
dlg_extract_folder_from_sidebar (CtkWidget *widget G_GNUC_UNUSED,
	     			 gpointer   callback_data)
{
	FrWindow *window = callback_data;
	GList    *files;
	char     *base_dir;

	files = fr_window_get_selection (window, TRUE, &base_dir);
	dlg_extract__common (window, files, base_dir);
}

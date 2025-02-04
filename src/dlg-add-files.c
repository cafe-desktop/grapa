/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  Grapa
 *
 *  Copyright (C) 2001, 2003, 2004 Free Software Foundation, Inc.
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
#include <glib/gi18n.h>
#include <ctk/ctk.h>
#include "file-utils.h"
#include "fr-window.h"
#include "ctk-utils.h"
#include "preferences.h"
#include "dlg-add-files.h"

typedef struct {
	FrWindow  *window;
	GSettings *settings;
	CtkWidget *dialog;
	CtkWidget *choice;
	CtkWidget *add_if_newer_checkbutton;
} DialogData;


static void
open_file_destroy_cb (CtkWidget  *file_sel G_GNUC_UNUSED,
		      DialogData *data)
{
	g_object_unref (data->settings);
	g_free (data);
}


static int
file_sel_response_cb (CtkWidget  *widget G_GNUC_UNUSED,
		      int         response,
		      DialogData *data)
{
	CtkFileChooser *file_sel = CTK_FILE_CHOOSER (data->choice);
	FrWindow       *window = data->window;
	char           *current_folder;
	char           *uri;
	gboolean        update;
	GSList         *selections, *iter;
	GList          *item_list = NULL;

	current_folder = ctk_file_chooser_get_current_folder_uri (file_sel);
	uri = ctk_file_chooser_get_uri (file_sel);

	if (current_folder != NULL) {
		g_settings_set_string (data->settings, PREF_ADD_CURRENT_FOLDER, current_folder);
		fr_window_set_add_default_dir (window, current_folder);
	}

	if (uri != NULL)
	{
		g_settings_set_string (data->settings, PREF_ADD_FILENAME, uri);
		g_free (uri);
	}


	if ((response == CTK_RESPONSE_CANCEL) || (response == CTK_RESPONSE_DELETE_EVENT)) {
		ctk_widget_destroy (data->dialog);
		g_free (current_folder);
		return TRUE;
	}

	if (response == CTK_RESPONSE_HELP) {
		show_help_dialog (CTK_WINDOW (data->dialog), "grapa-add-options");
		g_free (current_folder);
		return TRUE;
	}

	/* check folder permissions. */

	if (uri_is_dir (current_folder) && ! check_permissions (current_folder, R_OK)) {
		CtkWidget *d;
		char      *utf8_path;

		utf8_path = g_filename_display_name (current_folder);

		d = _ctk_error_dialog_new (CTK_WINDOW (window),
					   CTK_DIALOG_MODAL,
					   NULL,
					   _("Could not add the files to the archive"),
					   _("You don't have the right permissions to read files from folder \"%s\""),
					   utf8_path);
		ctk_dialog_run (CTK_DIALOG (d));
		ctk_widget_destroy (CTK_WIDGET (d));

		g_free (utf8_path);
		g_free (current_folder);
		return FALSE;
	}

	update = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->add_if_newer_checkbutton));

	/**/

	selections = ctk_file_chooser_get_uris (file_sel);
	for (iter = selections; iter != NULL; iter = iter->next)
		item_list = g_list_prepend (item_list, g_file_new_for_uri (iter->data));

	if (item_list != NULL)
		fr_window_archive_add_files (window, item_list, update);

	gio_file_list_free (item_list);
	g_slist_free_full (selections, g_free);
	g_free (current_folder);

	ctk_widget_destroy (data->dialog);

	return TRUE;
}


static gboolean
add_files_window_unrealize_cb (CtkWidget  *widget,
			       gpointer    data G_GNUC_UNUSED)
{
	pref_util_save_window_geometry (CTK_WINDOW (widget), "addfiles");
	return FALSE;
}


/* create the "add" dialog. */
void
add_files_cb (CtkWidget *widget G_GNUC_UNUSED,
	      void      *callback_data)
{
	CtkWidget  *file_sel;
	DialogData *data;
	CtkWidget  *main_box;
	CtkWidget  *content_area;
	CtkWidget  *filechooser;
	char       *folder;

	data = g_new0 (DialogData, 1);
	data->window = callback_data;
	data->settings = g_settings_new (GRAPA_SCHEMA_ADD);
	data->dialog = file_sel = ctk_dialog_new ();
	ctk_window_set_title (CTK_WINDOW (file_sel), _("Add Files"));
	ctk_window_set_transient_for (CTK_WINDOW (file_sel), CTK_WINDOW (data->window));
	ctk_window_set_modal (CTK_WINDOW (file_sel), TRUE);
	ctk_dialog_add_button_with_icon_name (CTK_DIALOG (file_sel), _("_Help"), "help-browser", CTK_RESPONSE_HELP);
	ctk_dialog_add_button_with_icon_name (CTK_DIALOG (file_sel), _("_Cancel"), "process-stop", CTK_RESPONSE_CANCEL);
	ctk_dialog_add_button_with_icon_name (CTK_DIALOG (file_sel), _("_Add"), "grapa_add-files-to-archive", CTK_RESPONSE_OK);

	ctk_window_set_default_size (CTK_WINDOW (file_sel), 530, 450);

	data->choice = filechooser = ctk_file_chooser_widget_new (CTK_FILE_CHOOSER_ACTION_OPEN);
	ctk_file_chooser_set_select_multiple (CTK_FILE_CHOOSER (filechooser), TRUE);
	ctk_file_chooser_set_local_only (CTK_FILE_CHOOSER (filechooser), FALSE);
	ctk_dialog_set_default_response (CTK_DIALOG (file_sel), CTK_RESPONSE_OK);

	/* Translators: add a file to the archive only if the disk version is
	 * newer than the archive version. */
	data->add_if_newer_checkbutton = ctk_check_button_new_with_mnemonic (_("Add only if _newer"));

	main_box = ctk_box_new (CTK_ORIENTATION_VERTICAL, 6);
	ctk_widget_set_margin_top (CTK_WIDGET(main_box) , 2);
	ctk_widget_set_halign (data->add_if_newer_checkbutton, CTK_ALIGN_START);
	content_area = ctk_dialog_get_content_area (CTK_DIALOG (file_sel));
	ctk_container_set_border_width (CTK_CONTAINER (main_box), 0);
	ctk_box_pack_start (CTK_BOX (main_box), filechooser,
			    TRUE, TRUE, 0);
	ctk_box_pack_start (CTK_BOX (main_box), data->add_if_newer_checkbutton,
			    FALSE, FALSE, 6);
	ctk_box_pack_start (CTK_BOX (content_area),
			    main_box,
			    TRUE, TRUE, 0);
	ctk_widget_show_all (main_box);

	/* set data */

	folder = g_settings_get_string (data->settings, PREF_ADD_CURRENT_FOLDER);
	if ((folder == NULL) || (strcmp (folder, "") == 0))
		folder = g_strdup (fr_window_get_add_default_dir (data->window));
	ctk_file_chooser_set_current_folder_uri (CTK_FILE_CHOOSER (filechooser), folder);
	g_free (folder);

	/* signals */

	g_signal_connect (G_OBJECT (file_sel),
			  "destroy",
			  G_CALLBACK (open_file_destroy_cb),
			  data);

	g_signal_connect (G_OBJECT (file_sel),
			  "response",
			  G_CALLBACK (file_sel_response_cb),
			  data);

	g_signal_connect (G_OBJECT (file_sel),
			  "unrealize",
			  G_CALLBACK (add_files_window_unrealize_cb),
			  NULL);

	ctk_window_set_modal (CTK_WINDOW (file_sel), TRUE);
	pref_util_restore_window_geometry (CTK_WINDOW (file_sel), "addfiles");
	ctk_widget_show (file_sel);
}

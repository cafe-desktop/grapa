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
#include <gtk/gtk.h>
#include "file-utils.h"
#include "fr-window.h"
#include "gtk-utils.h"
#include "preferences.h"
#include "dlg-add-files.h"

typedef struct {
	FrWindow  *window;
	GSettings *settings;
	GtkWidget *dialog;
	GtkWidget *choice;
	GtkWidget *add_if_newer_checkbutton;
} DialogData;


static void
open_file_destroy_cb (GtkWidget  *file_sel,
		      DialogData *data)
{
	g_object_unref (data->settings);
	g_free (data);
}


static int
file_sel_response_cb (GtkWidget      *widget,
		      int             response,
		      DialogData     *data)
{
	GtkFileChooser *file_sel = GTK_FILE_CHOOSER (data->choice);
	FrWindow       *window = data->window;
	char           *current_folder;
	char           *uri;
	gboolean        update;
#if GTK_CHECK_VERSION (3,99,0)
	GListModel     *files;
	guint           i, n;
#else
	GSList         *selections, *iter;
#endif
	GList          *item_list = NULL;

	current_folder = grapa_file_chooser_get_current_folder_uri (file_sel);
	uri = grapa_file_chooser_get_uri (file_sel);

	if (current_folder != NULL) {
		g_settings_set_string (data->settings, PREF_ADD_CURRENT_FOLDER, current_folder);
		fr_window_set_add_default_dir (window, current_folder);
	}

	if (uri != NULL)
	{
		g_settings_set_string (data->settings, PREF_ADD_FILENAME, uri);
		g_free (uri);
	}


	if ((response == GTK_RESPONSE_CANCEL) || (response == GTK_RESPONSE_DELETE_EVENT)) {
		gtk_widget_destroy (data->dialog);
		g_free (current_folder);
		return TRUE;
	}

	if (response == GTK_RESPONSE_HELP) {
		show_help_dialog (GTK_WINDOW (data->dialog), "grapa-add-options");
		g_free (current_folder);
		return TRUE;
	}

	/* check folder permissions. */

	if (uri_is_dir (current_folder) && ! check_permissions (current_folder, R_OK)) {
		GtkWidget *d;
		char      *utf8_path;

		utf8_path = g_filename_display_name (current_folder);

		d = _gtk_error_dialog_new (GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   NULL,
					   _("Could not add the files to the archive"),
					   _("You don't have the right permissions to read files from folder \"%s\""),
					   utf8_path);
		gtk_dialog_run (GTK_DIALOG (d));
		gtk_widget_destroy (GTK_WIDGET (d));

		g_free (utf8_path);
		g_free (current_folder);
		return FALSE;
	}

	update = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->add_if_newer_checkbutton));

	/**/

#if GTK_CHECK_VERSION (3,99,0)
	files = gtk_file_chooser_get_files (file_sel);
	n = g_list_model_get_n_items (files);
	for (i = 0; i < n; i++)
		item_list = g_list_prepend (item_list, g_file_new_for_uri (g_list_model_get_item (files, i)));
#else
	selections = gtk_file_chooser_get_uris (file_sel);
	for (iter = selections; iter != NULL; iter = iter->next)
		item_list = g_list_prepend (item_list, g_file_new_for_uri (iter->data));
#endif

	if (item_list != NULL)
		fr_window_archive_add_files (window, item_list, update);

	gio_file_list_free (item_list);
#if GTK_CHECK_VERSION (3,99,0)
	g_object_unref (files);
#else
	g_slist_free_full (selections, g_free);
#endif
	g_free (current_folder);

	gtk_widget_destroy (data->dialog);

	return TRUE;
}


static gboolean
add_files_window_unrealize_cb (GtkWidget  *widget,
			       gpointer    data)
{
	pref_util_save_window_geometry (GTK_WINDOW (widget), "add");
	return FALSE;
}


/* create the "add" dialog. */
void
add_files_cb (GtkWidget *widget,
	      void      *callback_data)
{
	GtkWidget  *file_sel;
	DialogData *data;
	GtkWidget  *main_box;
#if !GTK_CHECK_VERSION (3,99,0)
	GtkWidget  *content_area;
#endif
	GtkWidget  *filechooser;
	char       *folder;

	data = g_new0 (DialogData, 1);
	data->window = callback_data;
	data->settings = g_settings_new (GRAPA_SCHEMA_ADD);
	data->dialog = file_sel = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (file_sel), _("Add Files"));
	gtk_window_set_transient_for (GTK_WINDOW (file_sel), GTK_WINDOW (data->window));
	gtk_window_set_modal (GTK_WINDOW (file_sel), TRUE);
	grapa_dialog_add_button (GTK_DIALOG (file_sel), _("_Help"), "help-browser", GTK_RESPONSE_HELP);
	grapa_dialog_add_button (GTK_DIALOG (file_sel), _("_Cancel"), "process-stop", GTK_RESPONSE_CANCEL);
	grapa_dialog_add_button (GTK_DIALOG (file_sel), _("_Add"), "grapa_add-files-to-archive", GTK_RESPONSE_OK);

	gtk_window_set_default_size (GTK_WINDOW (file_sel), 530, 450);

	data->choice = filechooser = gtk_file_chooser_widget_new (GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (filechooser), TRUE);
#if !GTK_CHECK_VERSION (3,99,0)
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (filechooser), FALSE);
#endif
	gtk_dialog_set_default_response (GTK_DIALOG (file_sel), GTK_RESPONSE_OK);

	/* Translators: add a file to the archive only if the disk version is
	 * newer than the archive version. */
	data->add_if_newer_checkbutton = gtk_check_button_new_with_mnemonic (_("Add only if _newer"));

	main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
	gtk_widget_set_margin_top (GTK_WIDGET(main_box) , 2);
	gtk_widget_set_halign (data->add_if_newer_checkbutton, GTK_ALIGN_START);
#if GTK_CHECK_VERSION (3,99,0)
	gtk_window_set_child (GTK_WINDOW (file_sel), main_box);
	gtk_box_append (GTK_BOX (main_box), filechooser);
	gtk_box_append (GTK_BOX (main_box), data->add_if_newer_checkbutton);
#else
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (file_sel));
	gtk_container_set_border_width (GTK_CONTAINER (main_box), 0);
	gtk_box_pack_start (GTK_BOX (main_box), filechooser,
			    TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (main_box), data->add_if_newer_checkbutton,
			    FALSE, FALSE, 6);
	gtk_box_pack_start (GTK_BOX (content_area),
			    main_box,
			    TRUE, TRUE, 0);
	gtk_widget_show_all (main_box);
#endif

	/* set data */

	folder = g_settings_get_string (data->settings, PREF_ADD_CURRENT_FOLDER);
	if ((folder == NULL) || (strcmp (folder, "") == 0))
		folder = g_strdup (fr_window_get_add_default_dir (data->window));
	grapa_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (filechooser), folder);
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

	gtk_window_set_modal (GTK_WINDOW (file_sel), TRUE);
	pref_util_restore_window_geometry (GTK_WINDOW (file_sel), "add");
	gtk_widget_show (file_sel);
}

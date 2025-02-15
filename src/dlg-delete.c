/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  Grapa
 *
 *  Copyright (C) 2001, 2003 Free Software Foundation, Inc.
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
#include <ctk/ctk.h>
#include "fr-window.h"
#include "ctk-utils.h"
#include "file-utils.h"
#include "dlg-delete.h"


typedef struct {
	FrWindow  *window;
	GList     *selected_files;
	CtkBuilder *builder;

	CtkWidget *dialog;
	CtkWidget *d_all_files_radio;
	CtkWidget *d_selected_files_radio;
	CtkWidget *d_files_radio;
	CtkWidget *d_files_entry;
} DialogData;


/* called when the main dialog is closed. */
static void
destroy_cb (CtkWidget  *widget G_GNUC_UNUSED,
	    DialogData *data)
{
	path_list_free (data->selected_files);
	g_object_unref (G_OBJECT (data->builder));
	g_free (data);
}


/* called when the "ok" button is pressed. */
static void
ok_clicked_cb (CtkWidget  *widget G_GNUC_UNUSED,
	       DialogData *data)
{
	gboolean  selected_files;
	gboolean  pattern_files;
	FrWindow *window = data->window;
	GList    *file_list = NULL;
	gboolean  do_not_remove_if_null = FALSE;

	selected_files = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->d_selected_files_radio));
	pattern_files = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->d_files_radio));

	/* create the file list. */

	if (selected_files) {
		file_list = data->selected_files;
		data->selected_files = NULL;       /* do not the list when destroying the dialog. */
	}
	else if (pattern_files) {
		const char *pattern;

		pattern = ctk_entry_get_text (CTK_ENTRY (data->d_files_entry));
		file_list = fr_window_get_file_list_pattern (window, pattern);
		if (file_list == NULL)
			do_not_remove_if_null = TRUE;
	}

	/* close the dialog. */

	ctk_widget_destroy (data->dialog);

	/* remove ! */

	if (! do_not_remove_if_null || (file_list != NULL))
		fr_window_archive_remove (window, file_list);

	path_list_free (file_list);
}


static void
entry_changed_cb (CtkWidget  *widget G_GNUC_UNUSED,
		  DialogData *data)
{
	if (! ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->d_files_radio)))
		ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (data->d_files_radio), TRUE);
}


static void
dlg_delete__common (FrWindow *window,
	            GList    *selected_files)
{
	DialogData *data;
	CtkWidget  *cancel_button;
	CtkWidget  *ok_button;

	data = g_new (DialogData, 1);
	data->window = window;
	data->selected_files = selected_files;

	data->builder = _ctk_builder_new_from_resource ("delete.ui");
	if (data->builder == NULL) {
		g_free (data);
		return;
	}

	/* Get the widgets. */

	data->dialog = _ctk_builder_get_widget (data->builder, "delete_dialog");
	data->d_all_files_radio = _ctk_builder_get_widget (data->builder, "d_all_files_radio");
	data->d_selected_files_radio = _ctk_builder_get_widget (data->builder, "d_selected_files_radio");
	data->d_files_radio = _ctk_builder_get_widget (data->builder, "d_files_radio");
	data->d_files_entry = _ctk_builder_get_widget (data->builder, "d_files_entry");

	ok_button = _ctk_builder_get_widget (data->builder, "d_ok_button");
	cancel_button = _ctk_builder_get_widget (data->builder, "d_cancel_button");

	/* Set widgets data. */

	if (data->selected_files != NULL)
		ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (data->d_selected_files_radio), TRUE);
	else {
		ctk_widget_set_sensitive (data->d_selected_files_radio, FALSE);
		ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (data->d_all_files_radio), TRUE);
	}

	/* Set the signals handlers. */

	g_signal_connect (G_OBJECT (data->dialog),
			  "destroy",
			  G_CALLBACK (destroy_cb),
			  data);
	g_signal_connect_swapped (G_OBJECT (cancel_button),
				  "clicked",
				  G_CALLBACK (ctk_widget_destroy),
				  G_OBJECT (data->dialog));
	g_signal_connect (G_OBJECT (ok_button),
			  "clicked",
			  G_CALLBACK (ok_clicked_cb),
			  data);
	g_signal_connect (G_OBJECT (data->d_files_entry),
			  "changed",
			  G_CALLBACK (entry_changed_cb),
			  data);

	/* Run dialog. */

	ctk_window_set_transient_for (CTK_WINDOW (data->dialog),
				      CTK_WINDOW (window));
	ctk_window_set_modal         (CTK_WINDOW (data->dialog), TRUE);

	ctk_widget_show (data->dialog);
}


void
dlg_delete (CtkWidget *widget G_GNUC_UNUSED,
	    gpointer   callback_data)
{
	FrWindow *window = callback_data;
	dlg_delete__common (window,
			    fr_window_get_file_list_selection (window, TRUE, NULL));
}


void
dlg_delete_from_sidebar (CtkWidget *widget G_GNUC_UNUSED,
			 gpointer   callback_data)
{
	FrWindow *window = callback_data;
	dlg_delete__common (window,
			    fr_window_get_folder_tree_selection (window, TRUE, NULL));
}

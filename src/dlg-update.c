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
#include <ctk/ctk.h>
#include "dlg-update.h"
#include "file-utils.h"
#include "glib-utils.h"
#include "ctk-utils.h"
#include "fr-init.h"
#include "fr-window.h"


enum {
	IS_SELECTED_COLUMN,
	NAME_COLUMN,
	DATA_COLUMN,
	N_COLUMNS
};

typedef struct {
	FrWindow     *window;
	CtkBuilder *builder;

	CtkWidget    *update_file_dialog;
	CtkWidget    *update_file_primary_text_label;
	CtkWidget    *update_file_secondary_text_label;

	CtkWidget    *update_files_dialog;
	CtkWidget    *update_files_primary_text_label;
	CtkWidget    *update_files_secondary_text_label;
	CtkWidget    *update_files_treeview;
	CtkWidget    *update_files_ok_button;

	GList        *file_list;
	CtkTreeModel *list_model;
} DialogData;


/* called when the main dialog is closed. */
static void
dlg_update__destroy_cb (CtkWidget  *widget,
		        DialogData *data)
{
	fr_window_update_dialog_closed (data->window);
	g_object_unref (G_OBJECT (data->builder));
	if (data->file_list != NULL)
		g_list_free (data->file_list);
	g_free (data);
}


static GList*
get_selected_files (DialogData *data)
{
	GList       *selection = NULL;
	CtkTreeIter  iter;

	if (! ctk_tree_model_get_iter_first (data->list_model, &iter))
		return NULL;

	do {
		gboolean  is_selected;
		OpenFile *file;

                ctk_tree_model_get (data->list_model, &iter,
                		    IS_SELECTED_COLUMN, &is_selected,
                		    DATA_COLUMN, &file,
                		    -1);
                if (is_selected)
                	selection = g_list_prepend (selection, file);
	} while (ctk_tree_model_iter_next (data->list_model, &iter));

	return g_list_reverse (selection);
}


static void
update_cb (CtkWidget *widget,
	   gpointer   callback_data)
{
	DialogData *data = callback_data;
	GList      *selection;

	selection = get_selected_files (data);
	if (fr_window_update_files (data->window, selection)) {
		int n_files;

		n_files = g_list_length (data->file_list);
		if (n_files == 1)
			ctk_widget_destroy (data->update_file_dialog);
		else
			ctk_widget_destroy (data->update_files_dialog);
	}
	if (selection != NULL)
		g_list_free (selection);
}


static void
update_file_list (DialogData *data)
{
	gboolean     n_files;
	GList       *scan;
	CtkTreeIter  iter;

	n_files = g_list_length (data->file_list);

	/* update the file list */

	ctk_list_store_clear (CTK_LIST_STORE (data->list_model));
	for (scan = data->file_list; scan; scan = scan->next) {
		char     *utf8_name;
		OpenFile *file = scan->data;

		ctk_list_store_append (CTK_LIST_STORE (data->list_model),
				       &iter);

		utf8_name = g_filename_display_name (file_name_from_path (file->path));
		ctk_list_store_set (CTK_LIST_STORE (data->list_model),
				    &iter,
				    IS_SELECTED_COLUMN, TRUE,
				    NAME_COLUMN, utf8_name,
				    DATA_COLUMN, file,
				    -1);
		g_free (utf8_name);
	}

	/* update the labels */

	if (n_files == 1) {
		OpenFile *file = data->file_list->data;
		char     *file_name;
		char     *unescaped;
		char     *archive_name;
		char     *label;
		char     *markup;

		/* primary text */

		file_name = g_filename_display_name (file_name_from_path (file->path));
		unescaped = g_uri_unescape_string (fr_window_get_archive_uri (data->window), NULL);
		archive_name = g_path_get_basename (unescaped);
		label = g_markup_printf_escaped (_("Update the file \"%s\" in the archive \"%s\"?"), file_name, archive_name);
		markup = g_strdup_printf ("<big><b>%s</b></big>", label);
		ctk_label_set_markup (CTK_LABEL (data->update_file_primary_text_label), markup);

		g_free (markup);
		g_free (label);
		g_free (archive_name);
		g_free (unescaped);
		g_free (file_name);

		/* secondary text */

		label = g_strdup_printf (ngettext ("The file has been modified with an external application. If you don't update the file in the archive, all of your changes will be lost.",
						   "%d files have been modified with an external application. If you don't update the files in the archive, all of your changes will be lost.",
						   n_files),
					 n_files);
		ctk_label_set_text (CTK_LABEL (data->update_file_secondary_text_label), label);
		g_free (label);
	}
	else if (n_files > 1) {
		char *unescaped;
		char *archive_name;
		char *label;
		char *markup;

		/* primary text */

		unescaped = g_uri_unescape_string (fr_window_get_archive_uri (data->window), NULL);
		archive_name = g_path_get_basename (unescaped);
		label = g_markup_printf_escaped (_("Update the files in the archive \"%s\"?"), archive_name);
		markup = g_strdup_printf ("<big><b>%s</b></big>", label);
		ctk_label_set_markup (CTK_LABEL (data->update_files_primary_text_label), markup);

		g_free (markup);
		g_free (label);
		g_free (archive_name);
		g_free (unescaped);

		/* secondary text */

		label = g_strdup_printf (ngettext ("The file has been modified with an external application. If you don't update the file in the archive, all of your changes will be lost.",
						   "%d files have been modified with an external application. If you don't update the files in the archive, all of your changes will be lost.",
						   n_files),
					 n_files);
		ctk_label_set_text (CTK_LABEL (data->update_files_secondary_text_label), label);
		g_free (label);
	}

	/* show the appropriate dialog */

	if (n_files == 1) {
		/*ctk_window_set_modal (CTK_WINDOW (data->update_files_dialog), FALSE);*/
		ctk_widget_hide (data->update_files_dialog);
		/*ctk_window_set_modal (CTK_WINDOW (data->update_file_dialog), TRUE);*/
		ctk_widget_show (data->update_file_dialog);
	}
	else if (n_files > 1) {
		/*ctk_window_set_modal (CTK_WINDOW (data->update_file_dialog), FALSE);*/
		ctk_widget_hide (data->update_file_dialog);
		/*ctk_window_set_modal (CTK_WINDOW (data->update_files_dialog), TRUE);*/
		ctk_widget_show (data->update_files_dialog);
	}
	else { /* n_files == 0 */
		/*ctk_window_set_modal (CTK_WINDOW (data->update_files_dialog), FALSE);*/
		ctk_widget_hide (data->update_files_dialog);
		/*ctk_window_set_modal (CTK_WINDOW (data->update_file_dialog), FALSE);*/
		ctk_widget_hide (data->update_file_dialog);
	}
}


static int
n_selected (DialogData *data)
{
	int         n = 0;
	CtkTreeIter iter;

	if (! ctk_tree_model_get_iter_first (data->list_model, &iter))
		return 0;

	do {
		gboolean is_selected;
                ctk_tree_model_get (data->list_model, &iter, IS_SELECTED_COLUMN, &is_selected, -1);
                if (is_selected)
                	n++;
	} while (ctk_tree_model_iter_next (data->list_model, &iter));

	return n;
}


static void
is_selected_toggled (CtkCellRendererToggle *cell,
		     char                  *path_string,
		     gpointer               callback_data)
{
	DialogData   *data  = callback_data;
	CtkTreeModel *model = CTK_TREE_MODEL (data->list_model);
	CtkTreeIter   iter;
	CtkTreePath  *path = ctk_tree_path_new_from_string (path_string);
	guint         value;

	ctk_tree_model_get_iter (model, &iter, path);
	value = ! ctk_cell_renderer_toggle_get_active (cell);
	ctk_list_store_set (CTK_LIST_STORE (model), &iter, IS_SELECTED_COLUMN, value, -1);

	ctk_tree_path_free (path);

	ctk_widget_set_sensitive (data->update_files_ok_button, n_selected (data) > 0);
}


gpointer
dlg_update (FrWindow *window)
{
	DialogData        *data;
	CtkWidget         *update_file_ok_button;
	CtkWidget         *update_file_cancel_button;
	CtkWidget         *update_files_cancel_button;
	CtkCellRenderer   *renderer;
	CtkTreeViewColumn *column;

	data = g_new0 (DialogData, 1);

	data->builder = _ctk_builder_new_from_resource ("update.ui");
	if (data->builder == NULL) {
		g_free (data);
		return NULL;
	}

	data->file_list = NULL;
	data->window = window;

	/* Get the widgets. */

	data->update_file_dialog = _ctk_builder_get_widget (data->builder, "update_file_dialog");
	data->update_file_primary_text_label = _ctk_builder_get_widget (data->builder, "update_file_primary_text_label");
	data->update_file_secondary_text_label = _ctk_builder_get_widget (data->builder, "update_file_secondary_text_label");

	update_file_ok_button = _ctk_builder_get_widget (data->builder, "update_file_ok_button");
	update_file_cancel_button = _ctk_builder_get_widget (data->builder, "update_file_cancel_button");

	data->update_files_dialog = _ctk_builder_get_widget (data->builder, "update_files_dialog");
	data->update_files_primary_text_label = _ctk_builder_get_widget (data->builder, "update_files_primary_text_label");
	data->update_files_secondary_text_label = _ctk_builder_get_widget (data->builder, "update_files_secondary_text_label");
	data->update_files_treeview = _ctk_builder_get_widget (data->builder, "update_files_treeview");
	data->update_files_ok_button = _ctk_builder_get_widget (data->builder, "update_files_ok_button");
	update_files_cancel_button = _ctk_builder_get_widget (data->builder, "update_files_cancel_button");

	/* Set the signals handlers. */

	g_signal_connect (G_OBJECT (data->update_file_dialog),
			  "destroy",
			  G_CALLBACK (dlg_update__destroy_cb),
			  data);
	g_signal_connect (G_OBJECT (update_file_ok_button),
			  "clicked",
			  G_CALLBACK (update_cb),
			  data);
	g_signal_connect_swapped (G_OBJECT (update_file_cancel_button),
				  "clicked",
				  G_CALLBACK (ctk_widget_destroy),
				  G_OBJECT (data->update_file_dialog));
	g_signal_connect (G_OBJECT (data->update_files_dialog),
			  "destroy",
			  G_CALLBACK (dlg_update__destroy_cb),
			  data);
	g_signal_connect (G_OBJECT (data->update_files_ok_button),
			  "clicked",
			  G_CALLBACK (update_cb),
			  data);
	g_signal_connect_swapped (G_OBJECT (update_files_cancel_button),
				  "clicked",
				  G_CALLBACK (ctk_widget_destroy),
				  G_OBJECT (data->update_files_dialog));

	/* Set dialog data. */

	data->list_model = CTK_TREE_MODEL (ctk_list_store_new (N_COLUMNS,
							       G_TYPE_BOOLEAN,
							       G_TYPE_STRING,
							       G_TYPE_POINTER));
	ctk_tree_sortable_set_sort_column_id (CTK_TREE_SORTABLE (data->list_model),
					      NAME_COLUMN,
					      CTK_SORT_ASCENDING);
	ctk_tree_view_set_model (CTK_TREE_VIEW (data->update_files_treeview),
				 data->list_model);
	g_object_unref (G_OBJECT (data->list_model));

	column = ctk_tree_view_column_new ();

	renderer = ctk_cell_renderer_toggle_new ();
        g_signal_connect (G_OBJECT (renderer),
                          "toggled",
                          G_CALLBACK (is_selected_toggled),
                          data);
        ctk_tree_view_column_pack_start (column, renderer, FALSE);
        ctk_tree_view_column_set_attributes (column, renderer,
                                             "active", IS_SELECTED_COLUMN,
                                             NULL);

	renderer = ctk_cell_renderer_text_new ();
	ctk_tree_view_column_pack_start (column, renderer, TRUE);
	ctk_tree_view_column_set_attributes (column, renderer,
					     "text", NAME_COLUMN,
					     NULL);

	ctk_tree_view_append_column (CTK_TREE_VIEW (data->update_files_treeview), column);

	/* Run dialog. */

	ctk_window_set_transient_for (CTK_WINDOW (data->update_file_dialog),
				      CTK_WINDOW (window));
	ctk_window_set_transient_for (CTK_WINDOW (data->update_files_dialog),
				      CTK_WINDOW (window));

	update_file_list (data);

	return data;
}


void
dlg_update_add_file (gpointer  dialog,
		     OpenFile *file)
{
	DialogData *data = dialog;
	GList      *scan;

	/* avoid duplicates */

	for (scan = data->file_list; scan; scan = scan->next) {
		OpenFile *test = scan->data;
		if (uricmp (test->extracted_uri, file->extracted_uri) == 0)
			return;
	}

	/**/

	data->file_list = g_list_append (data->file_list, file);
	update_file_list (data);
}

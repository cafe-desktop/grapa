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
#include <gio/gio.h>
#include "dlg-add-folder.h"
#include "file-utils.h"
#include "fr-window.h"
#include "ctk-utils.h"
#include "preferences.h"

#ifdef __GNUC__
#define UNUSED_VARIABLE __attribute__ ((unused))
#else
#define UNUSED_VARIABLE
#endif

typedef struct {
	FrWindow    *window;
	GSettings   *settings;
	CtkWidget   *dialog;
	CtkWidget   *include_subfold_checkbutton;
	CtkWidget   *choice;
	CtkWidget   *add_if_newer_checkbutton;
	CtkWidget   *exclude_symlinks;
	CtkWidget   *include_files_checkbutton;
	CtkWidget   *include_files_entry;
	CtkWidget   *include_files_label;
	CtkWidget   *exclude_files_entry;
	CtkWidget   *exclude_files_label;
	CtkWidget   *exclude_folders_entry;
	CtkWidget   *exclude_folders_label;
	CtkWidget   *load_button;
	CtkWidget   *save_button;
	CtkWidget   *clear_button;
	char        *last_options;
} DialogData;


static void
open_file_destroy_cb (CtkWidget  *widget G_GNUC_UNUSED,
		      DialogData *data)
{
	g_object_unref (data->settings);
	g_free (data->last_options);
	g_free (data);
}


static gboolean
utf8_only_spaces (const char *text)
{
	const char *scan;

	if (text == NULL)
		return TRUE;

	for (scan = text; *scan != 0; scan = g_utf8_next_char (scan)) {
		gunichar c = g_utf8_get_char (scan);
		if (! g_unichar_isspace (c))
			return FALSE;
	}

	return TRUE;
}


static void dlg_add_folder_save_last_options (DialogData *data);


static int
file_sel_response_cb (CtkWidget  *widget G_GNUC_UNUSED,
		      int         response,
		      DialogData *data)
{
	CtkFileChooser *file_sel = CTK_FILE_CHOOSER (data->choice);
	FrWindow       *window = data->window;
	char           *selected_folder;
	gboolean        update, UNUSED_VARIABLE recursive, follow_links;
	const char     *include_files;
	const char     *exclude_files;
	const char     *exclude_folders;
	char           *dest_dir;
	char           *local_filename;


	dlg_add_folder_save_last_options (data);

	if ((response == CTK_RESPONSE_CANCEL) || (response == CTK_RESPONSE_DELETE_EVENT)) {
		ctk_widget_destroy (data->dialog);
		return TRUE;
	}

	if (response == CTK_RESPONSE_HELP) {
		show_help_dialog (CTK_WINDOW (data->dialog), "grapa-add-options");
		return TRUE;
	}

	selected_folder = ctk_file_chooser_get_uri (file_sel);

	/* check folder permissions. */

	if (! check_permissions (selected_folder, R_OK)) {
		CtkWidget *d;
		char      *utf8_path;

		utf8_path = g_filename_display_name (selected_folder);

		d = _ctk_error_dialog_new (CTK_WINDOW (window),
					   CTK_DIALOG_MODAL,
					   NULL,
					   _("Could not add the files to the archive"),
					   _("You don't have the right permissions to read files from folder \"%s\""),
					   utf8_path);
		ctk_dialog_run (CTK_DIALOG (d));
		ctk_widget_destroy (CTK_WIDGET (d));

		g_free (utf8_path);
		g_free (selected_folder);

		return FALSE;
	}

	update = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->add_if_newer_checkbutton));
	recursive = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->include_subfold_checkbutton));
	follow_links = ! ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->exclude_symlinks));

	include_files = ctk_entry_get_text (CTK_ENTRY (data->include_files_entry));
	if (utf8_only_spaces (include_files))
		include_files = "*";

	exclude_files = ctk_entry_get_text (CTK_ENTRY (data->exclude_files_entry));
	if (utf8_only_spaces (exclude_files))
		exclude_files = NULL;

	exclude_folders = ctk_entry_get_text (CTK_ENTRY (data->exclude_folders_entry));
	if (utf8_only_spaces (exclude_folders))
		exclude_folders = NULL;

	local_filename = g_filename_from_uri (selected_folder, NULL, NULL);
	dest_dir = build_uri (fr_window_get_current_location (window),
			      file_name_from_path (local_filename),
			      NULL);

	fr_window_archive_add_with_wildcard (window,
					     include_files,
					     exclude_files,
					     exclude_folders,
					     selected_folder,
					     dest_dir,
					     update,
					     follow_links);

	g_free (local_filename);
	g_free (dest_dir);
	g_free (selected_folder);

	ctk_widget_destroy (data->dialog);

	return TRUE;
}


static int
include_subfold_toggled_cb (CtkWidget *widget,
			    gpointer   callback_data)
{
	DialogData *data = callback_data;

	ctk_widget_set_sensitive (data->exclude_symlinks,
				  ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (widget)));

	return FALSE;
}


static void load_options_cb (CtkWidget *w, DialogData *data);
static void save_options_cb (CtkWidget *w, DialogData *data);
static void clear_options_cb (CtkWidget *w, DialogData *data);
static void dlg_add_folder_load_last_options (DialogData *data);


static gboolean
add_folder_window_unrealize_cb (CtkWidget  *widget,
				gpointer    data G_GNUC_UNUSED)
{
	pref_util_save_window_geometry (CTK_WINDOW (widget), "addfolder");
	return FALSE;
}


/* create the "add" dialog. */
void
add_folder_cb (CtkWidget *widget G_GNUC_UNUSED,
	       void      *callback_data)
{
	CtkWidget   *file_sel;
	DialogData  *data;
	CtkWidget   *full_box;
	CtkWidget   *main_box;
	CtkWidget   *vbox;
	CtkWidget   *grid;
	CtkWidget   *filechooser;
	CtkWidget   *content_area;

	data = g_new0 (DialogData, 1);
	data->settings = g_settings_new (GRAPA_SCHEMA_ADD);
	data->window = callback_data;

	data->dialog = file_sel = ctk_dialog_new ();
	ctk_window_set_title (CTK_WINDOW (file_sel), _("Add a Folder"));
	ctk_window_set_transient_for (CTK_WINDOW (file_sel), CTK_WINDOW (data->window));
	ctk_window_set_modal (CTK_WINDOW (file_sel), TRUE);
	ctk_dialog_add_button_with_icon_name (CTK_DIALOG (file_sel), _("_Help"), "help-browser", CTK_RESPONSE_HELP);
	ctk_dialog_add_button_with_icon_name (CTK_DIALOG (file_sel), _("_Cancel"), "process-stop", CTK_RESPONSE_CANCEL);
	ctk_dialog_add_button_with_icon_name (CTK_DIALOG (file_sel), _("_Add"), "grapa_add-folder-to-archive", CTK_RESPONSE_OK);

	ctk_window_set_default_size (CTK_WINDOW (file_sel), 530, 510);

	data->choice = filechooser = ctk_file_chooser_widget_new (CTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	ctk_file_chooser_set_select_multiple (CTK_FILE_CHOOSER (filechooser), FALSE);

	ctk_file_chooser_set_local_only (CTK_FILE_CHOOSER (filechooser), FALSE);
	ctk_dialog_set_default_response (CTK_DIALOG (file_sel), CTK_RESPONSE_OK);

	data->add_if_newer_checkbutton = ctk_check_button_new_with_mnemonic (_("Add only if _newer"));
	data->include_subfold_checkbutton = ctk_check_button_new_with_mnemonic (_("_Include subfolders"));
	data->exclude_symlinks = ctk_check_button_new_with_mnemonic (_("Exclude folders that are symbolic lin_ks"));
	ctk_widget_set_halign (data->exclude_symlinks, CTK_ALIGN_START);
	ctk_widget_set_valign (data->exclude_symlinks, CTK_ALIGN_START);
	ctk_widget_set_margin_start (data->exclude_symlinks, 12);

	data->include_files_entry = ctk_entry_new ();
	ctk_widget_set_tooltip_text (data->include_files_entry, _("example: *.o; *.bak"));
	data->include_files_label = ctk_label_new_with_mnemonic (_("Include _files:"));
	ctk_label_set_xalign (CTK_LABEL (data->include_files_label), 0.0);
	ctk_label_set_mnemonic_widget (CTK_LABEL (data->include_files_label), data->include_files_entry);

	data->exclude_files_entry = ctk_entry_new ();
	ctk_widget_set_tooltip_text (data->exclude_files_entry, _("example: *.o; *.bak"));
	data->exclude_files_label = ctk_label_new_with_mnemonic (_("E_xclude files:"));
	ctk_label_set_xalign (CTK_LABEL (data->exclude_files_label), 0.0);
	ctk_label_set_mnemonic_widget (CTK_LABEL (data->exclude_files_label), data->exclude_files_entry);

	data->exclude_folders_entry = ctk_entry_new ();
	ctk_widget_set_tooltip_text (data->exclude_folders_entry, _("example: *.o; *.bak"));
	data->exclude_folders_label = ctk_label_new_with_mnemonic (_("_Exclude folders:"));
	ctk_label_set_xalign (CTK_LABEL (data->exclude_folders_label), 0.0);
	ctk_label_set_mnemonic_widget (CTK_LABEL (data->exclude_folders_label), data->exclude_folders_entry);

	data->load_button = ctk_button_new_with_mnemonic (_("_Load Options"));
	data->save_button = ctk_button_new_with_mnemonic (_("Sa_ve Options"));
	data->clear_button = ctk_button_new_with_mnemonic (_("_Reset Options"));

	main_box = ctk_box_new (CTK_ORIENTATION_HORIZONTAL, 20);
	ctk_container_set_border_width (CTK_CONTAINER (main_box), 0);
 	ctk_widget_set_margin_start (CTK_WIDGET (main_box) , 6);

	content_area = ctk_dialog_get_content_area (CTK_DIALOG (file_sel));
	full_box = ctk_box_new (CTK_ORIENTATION_VERTICAL, 6);
 	ctk_widget_set_margin_top (CTK_WIDGET (full_box) , 2);
	ctk_container_set_border_width (CTK_CONTAINER (full_box), 0);
	ctk_box_pack_start (CTK_BOX (full_box), filechooser,
			    TRUE, TRUE, 0);
	ctk_box_pack_start (CTK_BOX (full_box), main_box, FALSE, FALSE, 0);
	ctk_box_pack_start (CTK_BOX (content_area),
			    full_box,
			    TRUE, TRUE, 0);

	vbox = ctk_box_new (CTK_ORIENTATION_VERTICAL, 6);
	ctk_container_set_border_width (CTK_CONTAINER (vbox), 0);
	ctk_box_pack_start (CTK_BOX (main_box), vbox, FALSE, FALSE, 0);

	ctk_box_pack_start (CTK_BOX (vbox), data->include_subfold_checkbutton,
			    FALSE, FALSE, 0);

	ctk_box_pack_start (CTK_BOX (vbox), data->exclude_symlinks, FALSE, FALSE, 0);

	ctk_box_pack_start (CTK_BOX (vbox), data->add_if_newer_checkbutton,
			    FALSE, FALSE, 0);

	grid = ctk_grid_new ();
	ctk_grid_set_row_spacing (CTK_GRID (grid), 6);
	ctk_grid_set_column_spacing (CTK_GRID (grid), 6);
	ctk_box_pack_start (CTK_BOX (vbox), grid,
			    TRUE, TRUE, 0);

	ctk_grid_attach (CTK_GRID (grid),
			  data->include_files_label,
			  0, 0,
			  1, 1);
	ctk_grid_attach (CTK_GRID (grid),
			  data->include_files_entry,
			  1, 0,
			  3, 1);
	ctk_grid_attach (CTK_GRID (grid),
			  data->exclude_files_label,
			  0, 1,
			  1, 1);
	ctk_grid_attach (CTK_GRID (grid),
			  data->exclude_files_entry,
			  1, 1,
			  1, 1);
	ctk_grid_attach (CTK_GRID (grid),
			  data->exclude_folders_label,
			  2, 1,
			  1, 1);
	ctk_grid_attach (CTK_GRID (grid),
			  data->exclude_folders_entry,
			  3, 1,
			  1, 1);

	/**/

	vbox = ctk_box_new (CTK_ORIENTATION_VERTICAL, 5);
	ctk_container_set_border_width (CTK_CONTAINER (vbox), 0);
	ctk_box_pack_start (CTK_BOX (main_box), vbox, FALSE, FALSE, 0);

	ctk_box_pack_start (CTK_BOX (vbox), data->load_button,
			    FALSE, FALSE, 0);
	ctk_box_pack_start (CTK_BOX (vbox), data->save_button,
			    FALSE, FALSE, 0);
	ctk_box_pack_start (CTK_BOX (vbox), data->clear_button,
			    FALSE, FALSE, 0);

	ctk_widget_show_all (full_box);

	/* set data */

	dlg_add_folder_load_last_options (data);

	/* signals */

	g_signal_connect (G_OBJECT (file_sel),
			  "destroy",
			  G_CALLBACK (open_file_destroy_cb),
			  data);
	g_signal_connect (G_OBJECT (file_sel),
			  "response",
			  G_CALLBACK (file_sel_response_cb),
			  data);
	g_signal_connect (G_OBJECT (data->include_subfold_checkbutton),
			  "toggled",
			  G_CALLBACK (include_subfold_toggled_cb),
			  data);
	g_signal_connect (G_OBJECT (data->load_button),
			  "clicked",
			  G_CALLBACK (load_options_cb),
			  data);
	g_signal_connect (G_OBJECT (data->save_button),
			  "clicked",
			  G_CALLBACK (save_options_cb),
			  data);
	g_signal_connect (G_OBJECT (data->clear_button),
			  "clicked",
			  G_CALLBACK (clear_options_cb),
			  data);

	g_signal_connect (G_OBJECT (file_sel),
			  "unrealize",
			  G_CALLBACK (add_folder_window_unrealize_cb),
			  NULL);

	ctk_window_set_modal (CTK_WINDOW (file_sel),TRUE);
	pref_util_restore_window_geometry (CTK_WINDOW (file_sel), "addfolder");
	ctk_widget_show_all (file_sel);
}


/* load/save the dialog options */


static void
dlg_add_folder_save_last_used_options (DialogData *data,
			               const char *options_path)
{
	g_free (data->last_options);
	data->last_options = g_strdup (file_name_from_path (options_path));
}


static void
sync_widgets_with_options (DialogData *data,
			   const char *base_dir,
			   const char *filename,
			   const char *include_files,
			   const char *exclude_files,
			   const char *exclude_folders,
			   gboolean    update,
			   gboolean    recursive,
			   gboolean    no_symlinks)
{
	if ((base_dir == NULL) || (strcmp (base_dir, "") == 0))
		base_dir = fr_window_get_add_default_dir (data->window);

	if ((filename != NULL) && (strcmp (filename, base_dir) != 0))
		ctk_file_chooser_select_uri (CTK_FILE_CHOOSER (data->choice), filename);
	else
		ctk_file_chooser_set_current_folder_uri (CTK_FILE_CHOOSER (data->choice), base_dir);

	if (include_files != NULL)
		ctk_entry_set_text (CTK_ENTRY (data->include_files_entry), include_files);
	if (exclude_files != NULL)
		ctk_entry_set_text (CTK_ENTRY (data->exclude_files_entry), exclude_files);
	if (exclude_folders != NULL)
		ctk_entry_set_text (CTK_ENTRY (data->exclude_folders_entry), exclude_folders);
	ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (data->add_if_newer_checkbutton), update);
	ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (data->include_subfold_checkbutton), recursive);
	ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (data->exclude_symlinks), no_symlinks);
}


static void
clear_options_cb (CtkWidget  *w G_GNUC_UNUSED,
		  DialogData *data)
{
	sync_widgets_with_options (data,
				   ctk_file_chooser_get_current_folder_uri (CTK_FILE_CHOOSER (data->choice)),
				   ctk_file_chooser_get_uri (CTK_FILE_CHOOSER (data->choice)),
				   "",
				   "",
				   "",
				   FALSE,
				   TRUE,
				   FALSE);
}


static gboolean
dlg_add_folder_load_options (DialogData *data,
			     const char *name)
{
	GFile     *options_dir;
	GFile     *options_file;
	char      *file_path;
	GKeyFile  *key_file;
	GError    *error = NULL;
	char      *base_dir = NULL;
	char      *filename = NULL;
	char      *include_files = NULL;
	char      *exclude_files = NULL;
	char      *exclude_folders = NULL;
	gboolean   update;
	gboolean   recursive;
	gboolean   no_symlinks;

	options_dir = get_user_config_subdirectory (ADD_FOLDER_OPTIONS_DIR, TRUE);
	options_file = g_file_get_child (options_dir, name);
	file_path = g_file_get_path (options_file);
	key_file = g_key_file_new ();
	if (! g_key_file_load_from_file (key_file, file_path, G_KEY_FILE_KEEP_COMMENTS, &error)) {
		if (error->code != G_IO_ERROR_NOT_FOUND)
			g_warning ("Could not load options file: %s\n", error->message);
		g_clear_error (&error);
		g_object_unref (options_file);
		g_object_unref (options_dir);
		g_key_file_free (key_file);
		return FALSE;
	}

	base_dir = g_key_file_get_string (key_file, "Options", "base_dir", NULL);
	filename = g_key_file_get_string (key_file, "Options", "filename", NULL);
	include_files = g_key_file_get_string (key_file, "Options", "include_files", NULL);
	exclude_files = g_key_file_get_string (key_file, "Options", "exclude_files", NULL);
	exclude_folders = g_key_file_get_string (key_file, "Options", "exclude_folders", NULL);
	update = g_key_file_get_boolean (key_file, "Options", "update", NULL);
	recursive = g_key_file_get_boolean (key_file, "Options", "recursive", NULL);
	no_symlinks = g_key_file_get_boolean (key_file, "Options", "no_symlinks", NULL);

	sync_widgets_with_options (data,
			   	   base_dir,
			   	   filename,
			   	   include_files,
			   	   exclude_files,
			   	   exclude_folders,
			   	   update,
			   	   recursive,
			   	   no_symlinks);

	dlg_add_folder_save_last_used_options (data, file_path);

	g_free (base_dir);
	g_free (filename);
	g_free (include_files);
	g_free (exclude_files);
	g_free (exclude_folders);
	g_key_file_free (key_file);
	g_free (file_path);
	g_object_unref (options_file);
	g_object_unref (options_dir);

	return TRUE;
}


static void
dlg_add_folder_load_last_options (DialogData *data)
{
	char     *base_dir = NULL;
	char     *filename = NULL;
	char     *include_files = NULL;
	char     *exclude_files = NULL;
	char     *exclude_folders = NULL;
	gboolean  update;
	gboolean  recursive;
	gboolean  no_symlinks;

	base_dir = g_settings_get_string (data->settings, PREF_ADD_CURRENT_FOLDER);
	filename = g_settings_get_string (data->settings, PREF_ADD_FILENAME);
	include_files = g_settings_get_string (data->settings, PREF_ADD_INCLUDE_FILES);
	exclude_files = g_settings_get_string (data->settings, PREF_ADD_EXCLUDE_FILES);
	exclude_folders = g_settings_get_string (data->settings, PREF_ADD_EXCLUDE_FOLDERS);
	update = g_settings_get_boolean (data->settings, PREF_ADD_UPDATE);
	recursive = g_settings_get_boolean (data->settings, PREF_ADD_RECURSIVE);
	no_symlinks = g_settings_get_boolean (data->settings, PREF_ADD_NO_SYMLINKS);

	sync_widgets_with_options (data,
			   	   base_dir,
			   	   filename,
			   	   include_files,
			   	   exclude_files,
			   	   exclude_folders,
			   	   update,
			   	   recursive,
			   	   no_symlinks);

	g_free (base_dir);
	g_free (filename);
	g_free (include_files);
	g_free (exclude_files);
	g_free (exclude_folders);
}


static void
get_options_from_widgets (DialogData  *data,
			  char       **base_dir,
			  char       **filename,
			  const char **include_files,
			  const char **exclude_files,
			  const char **exclude_folders,
			  gboolean    *update,
			  gboolean    *recursive,
			  gboolean    *no_symlinks)
{
	*base_dir = ctk_file_chooser_get_current_folder_uri (CTK_FILE_CHOOSER (data->choice));
	*filename = ctk_file_chooser_get_uri (CTK_FILE_CHOOSER (data->choice));
	*update = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->add_if_newer_checkbutton));
	*recursive = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->include_subfold_checkbutton));
	*no_symlinks = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->exclude_symlinks));

	*include_files = ctk_entry_get_text (CTK_ENTRY (data->include_files_entry));
	if (utf8_only_spaces (*include_files))
		*include_files = "";

	*exclude_files = ctk_entry_get_text (CTK_ENTRY (data->exclude_files_entry));
	if (utf8_only_spaces (*exclude_files))
		*exclude_files = "";

	*exclude_folders = ctk_entry_get_text (CTK_ENTRY (data->exclude_folders_entry));
	if (utf8_only_spaces (*exclude_folders))
		*exclude_folders = "";
}


static void
dlg_add_folder_save_current_options (DialogData *data,
				     GFile      *options_file)
{
	char       *base_dir;
	char       *filename;
	const char *include_files;
	const char *exclude_files;
	const char *exclude_folders;
	gboolean    update;
	gboolean    recursive;
	gboolean    no_symlinks;
	GKeyFile   *key_file;

	get_options_from_widgets (data,
				  &base_dir,
				  &filename,
				  &include_files,
				  &exclude_files,
				  &exclude_folders,
				  &update,
				  &recursive,
				  &no_symlinks);

	fr_window_set_add_default_dir (data->window, base_dir);

	key_file = g_key_file_new ();
	g_key_file_set_string (key_file, "Options", "base_dir", base_dir);
	g_key_file_set_string (key_file, "Options", "filename", filename);
	g_key_file_set_string (key_file, "Options", "include_files", include_files);
	g_key_file_set_string (key_file, "Options", "exclude_files", exclude_files);
	g_key_file_set_string (key_file, "Options", "exclude_folders", exclude_folders);
	g_key_file_set_boolean (key_file, "Options", "update", update);
	g_key_file_set_boolean (key_file, "Options", "recursive", recursive);
	g_key_file_set_boolean (key_file, "Options", "no_symlinks", no_symlinks);

	g_key_file_save (key_file, options_file);

	g_key_file_free (key_file);
	g_free (base_dir);
	g_free (filename);
}


static void
dlg_add_folder_save_last_options (DialogData *data)
{
	char       *base_dir;
	char       *filename;
	const char *include_files;
	const char *exclude_files;
	const char *exclude_folders;
	gboolean    update;
	gboolean    recursive;
	gboolean    no_symlinks;

	get_options_from_widgets (data,
				  &base_dir,
				  &filename,
				  &include_files,
				  &exclude_files,
				  &exclude_folders,
				  &update,
				  &recursive,
				  &no_symlinks);

	g_settings_set_string (data->settings, PREF_ADD_CURRENT_FOLDER, base_dir);
	g_settings_set_string (data->settings, PREF_ADD_FILENAME, filename);
	g_settings_set_string (data->settings, PREF_ADD_INCLUDE_FILES, include_files);
	g_settings_set_string (data->settings, PREF_ADD_EXCLUDE_FILES, exclude_files);
	g_settings_set_string (data->settings, PREF_ADD_EXCLUDE_FOLDERS, exclude_folders);
	g_settings_set_boolean (data->settings, PREF_ADD_UPDATE, update);
	g_settings_set_boolean (data->settings, PREF_ADD_RECURSIVE, recursive);
	g_settings_set_boolean (data->settings, PREF_ADD_NO_SYMLINKS, no_symlinks);

	g_free (base_dir);
	g_free (filename);
}


typedef struct {
	DialogData   *data;
	CtkBuilder *builder;
	CtkWidget    *dialog;
	CtkWidget    *aod_treeview;
	CtkTreeModel *aod_model;
} LoadOptionsDialogData;


static void
aod_destroy_cb (CtkWidget             *widget G_GNUC_UNUSED,
		LoadOptionsDialogData *aod_data)
{
	g_object_unref (aod_data->builder);
	g_free (aod_data);
}


static void
aod_apply_cb (CtkWidget *widget G_GNUC_UNUSED,
	      gpointer   callback_data)
{
	LoadOptionsDialogData *aod_data = callback_data;
	DialogData            *data = aod_data->data;
	CtkTreeSelection      *selection;
	CtkTreeIter            iter;
	char                  *options_name;

	selection = ctk_tree_view_get_selection (CTK_TREE_VIEW (aod_data->aod_treeview));
	if (! ctk_tree_selection_get_selected (selection, NULL, &iter))
		return;

	ctk_tree_model_get (aod_data->aod_model, &iter, 1, &options_name, -1);

	dlg_add_folder_load_options (data, options_name);
	g_free (options_name);

	ctk_widget_destroy (aod_data->dialog);
}


static void
aod_activated_cb (CtkTreeView       *tree_view G_GNUC_UNUSED,
		  CtkTreePath       *path G_GNUC_UNUSED,
		  CtkTreeViewColumn *column G_GNUC_UNUSED,
		  gpointer           callback_data)
{
	aod_apply_cb (NULL, callback_data);
}


static void
aod_update_option_list (LoadOptionsDialogData *aod_data)
{
	CtkListStore    *list_store = CTK_LIST_STORE (aod_data->aod_model);
	GFile           *options_dir;
	GFileEnumerator *file_enum;
	GFileInfo       *info;
	GError          *err = NULL;

	ctk_list_store_clear (list_store);

	options_dir = get_user_config_subdirectory (ADD_FOLDER_OPTIONS_DIR, TRUE);
	make_directory_tree (options_dir, 0700, NULL);

	file_enum = g_file_enumerate_children (options_dir, G_FILE_ATTRIBUTE_STANDARD_NAME, 0, NULL, &err);
	if (err != NULL) {
		g_warning ("Failed to enumerate children: %s", err->message);
		g_clear_error (&err);
		g_object_unref (options_dir);
		return;
	}

	while ((info = g_file_enumerator_next_file (file_enum, NULL, &err)) != NULL) {
		const char  *name;
		char        *display_name;
		CtkTreeIter  iter;

		if (err != NULL) {
			g_warning ("Failed to get info while enumerating: %s", err->message);
			g_clear_error (&err);
			continue;
		}

		name = g_file_info_get_name (info);
		display_name = g_filename_display_name (name);

		ctk_list_store_append (CTK_LIST_STORE (aod_data->aod_model), &iter);
		ctk_list_store_set (CTK_LIST_STORE (aod_data->aod_model), &iter,
				    0, name,
				    1, display_name,
				    -1);

		g_free (display_name);
		g_object_unref (info);
	}

	if (err != NULL) {
		g_warning ("Failed to get info after enumeration: %s", err->message);
		g_clear_error (&err);
	}

	g_object_unref (options_dir);
}


static void
aod_remove_cb (CtkWidget             *widget G_GNUC_UNUSED,
	       LoadOptionsDialogData *aod_data)
{
	CtkTreeSelection *selection;
	CtkTreeIter       iter;
	char             *filename;
	GFile            *options_dir;
	GFile            *options_file;
	GError           *error = NULL;

	selection = ctk_tree_view_get_selection (CTK_TREE_VIEW (aod_data->aod_treeview));
	if (! ctk_tree_selection_get_selected (selection, NULL, &iter))
		return;

	ctk_tree_model_get (aod_data->aod_model, &iter, 1, &filename, -1);
	ctk_list_store_remove (CTK_LIST_STORE (aod_data->aod_model), &iter);

	options_dir = get_user_config_subdirectory (ADD_FOLDER_OPTIONS_DIR, TRUE);
	options_file = g_file_get_child (options_dir, filename);
	if (! g_file_delete (options_file, NULL, &error)) {
		g_warning ("could not delete the options: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (options_file);
	g_object_unref (options_dir);
	g_free (filename);
}


static void
load_options_cb (CtkWidget  *w G_GNUC_UNUSED,
		 DialogData *data)
{
	LoadOptionsDialogData *aod_data;
	CtkWidget             *ok_button;
	CtkWidget             *cancel_button;
	CtkWidget             *remove_button;
	CtkCellRenderer       *renderer;
	CtkTreeViewColumn     *column;

	aod_data = g_new0 (LoadOptionsDialogData, 1);

	aod_data->data = data;
	aod_data->builder = _ctk_builder_new_from_resource ("add-options.ui");
	if (aod_data->builder == NULL) {
		g_free (aod_data);
		return;
	}

	/* Get the widgets. */

	aod_data->dialog = _ctk_builder_get_widget (aod_data->builder, "add_options_dialog");
	aod_data->aod_treeview = _ctk_builder_get_widget (aod_data->builder, "aod_treeview");

	ok_button = _ctk_builder_get_widget (aod_data->builder, "aod_okbutton");
	cancel_button = _ctk_builder_get_widget (aod_data->builder, "aod_cancelbutton");
	remove_button = _ctk_builder_get_widget (aod_data->builder, "aod_remove_button");

	/* Set the signals handlers. */

	g_signal_connect (G_OBJECT (aod_data->dialog),
			  "destroy",
			  G_CALLBACK (aod_destroy_cb),
			  aod_data);
	g_signal_connect (G_OBJECT (aod_data->aod_treeview),
			  "row_activated",
			  G_CALLBACK (aod_activated_cb),
			  aod_data);
	g_signal_connect_swapped (G_OBJECT (cancel_button),
				  "clicked",
				  G_CALLBACK (ctk_widget_destroy),
				  G_OBJECT (aod_data->dialog));
	g_signal_connect (G_OBJECT (ok_button),
			  "clicked",
			  G_CALLBACK (aod_apply_cb),
			  aod_data);
	g_signal_connect (G_OBJECT (remove_button),
			  "clicked",
			  G_CALLBACK (aod_remove_cb),
			  aod_data);

	/* Set data. */

	aod_data->aod_model = CTK_TREE_MODEL (ctk_list_store_new (2,
								  G_TYPE_STRING,
								  G_TYPE_STRING));
	ctk_tree_sortable_set_sort_column_id (CTK_TREE_SORTABLE (aod_data->aod_model),
					      0,
					      CTK_SORT_ASCENDING);
	ctk_tree_view_set_model (CTK_TREE_VIEW (aod_data->aod_treeview),
				 aod_data->aod_model);
	g_object_unref (aod_data->aod_model);

	/**/

	renderer = ctk_cell_renderer_text_new ();
	column = ctk_tree_view_column_new_with_attributes (NULL,
							   renderer,
							   "text", 0,
							   NULL);
	ctk_tree_view_column_set_sort_column_id (column, 0);
	ctk_tree_view_append_column (CTK_TREE_VIEW (aod_data->aod_treeview),
				     column);

	aod_update_option_list (aod_data);

	/* Run */

	ctk_window_set_transient_for (CTK_WINDOW (aod_data->dialog),
				      CTK_WINDOW (data->dialog));
	ctk_window_set_modal (CTK_WINDOW (aod_data->dialog), TRUE);
	ctk_widget_show (aod_data->dialog);
}


static void
save_options_cb (CtkWidget  *w G_GNUC_UNUSED,
		 DialogData *data)
{
	GFile *options_dir;
	GFile *options_file;
	char  *opt_filename;

	options_dir = get_user_config_subdirectory (ADD_FOLDER_OPTIONS_DIR, TRUE);
	make_directory_tree (options_dir, 0700, NULL);

	opt_filename = _ctk_request_dialog_run (
				CTK_WINDOW (data->dialog),
				CTK_DIALOG_MODAL,
				_("Save Options"),
				_("_Options Name:"),
				(data->last_options != NULL) ? data->last_options : "",
				1024,
				_("_Cancel"),
				_("_Save"));
	if (opt_filename == NULL)
		return;

	options_file = g_file_get_child_for_display_name (options_dir, opt_filename, NULL);
	dlg_add_folder_save_current_options (data, options_file);
	dlg_add_folder_save_last_used_options (data, opt_filename);

	g_free (opt_filename);
	g_object_unref (options_file);
	g_object_unref (options_dir);
}

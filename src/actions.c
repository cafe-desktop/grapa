/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  Grapa
 *
 *  Copyright (C) 2004 Free Software Foundation, Inc.
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
#include <glib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "actions.h"
#include "dlg-add-files.h"
#include "dlg-add-folder.h"
#include "dlg-extract.h"
#include "dlg-delete.h"
#include "dlg-new.h"
#include "dlg-open-with.h"
#include "dlg-password.h"
#include "dlg-prop.h"
#include "ctk-utils.h"
#include "fr-window.h"
#include "file-utils.h"
#include "fr-process.h"
#include "glib-utils.h"
#include "fr-init.h"
#include "typedefs.h"


/* -- new archive -- */


static void
new_archive (DlgNewData *data,
         char       *uri)
{
    CtkWidget  *archive_window;
    gboolean    new_window;
    const char *password;
    gboolean    encrypt_header;
    int         volume_size;

    new_window = fr_window_archive_is_present (data->window) && ! fr_window_is_batch_mode (data->window);
    if (new_window)
        archive_window = fr_window_new ();
    else
        archive_window = (CtkWidget *) data->window;

    password = dlg_new_data_get_password (data);
    encrypt_header = dlg_new_data_get_encrypt_header (data);
    volume_size = dlg_new_data_get_volume_size (data);

    fr_window_set_password (FR_WINDOW (archive_window), password);
    fr_window_set_encrypt_header (FR_WINDOW (archive_window), encrypt_header);
    fr_window_set_volume_size (FR_WINDOW (archive_window), volume_size);

    if (fr_window_archive_new (FR_WINDOW (archive_window), uri)) {
        ctk_widget_destroy (data->dialog);
        if (! fr_window_is_batch_mode (FR_WINDOW (archive_window)))
            ctk_window_present (CTK_WINDOW (archive_window));
    }
    else if (new_window)
        ctk_widget_destroy (archive_window);
}


/* when on Automatic the user provided extension needs to be supported,
   otherwise an existing unsupported archive can be deleted (if the user
   provided name matches with its name) before we find out that the
   archive is unsupported
*/
static gboolean
is_supported_extension (CtkWidget *file_sel G_GNUC_UNUSED,
			char      *filename,
			int       *file_type)
{
    int i;
    for (i = 0; file_type[i] != -1; i++)
        if (file_extension_is (filename, mime_type_desc[file_type[i]].default_ext))
            return TRUE;
    return FALSE;
}


static char *
get_full_uri (DlgNewData *data)
{
    char        *full_uri = NULL;
    char        *uri;
    const char  *filename;
    int          idx;

    uri = ctk_file_chooser_get_uri (CTK_FILE_CHOOSER (data->dialog));

    if ((uri == NULL) || (*uri == 0))
        return NULL;

    filename = file_name_from_path (uri);
    if ((filename == NULL) || (*filename == 0)) {
        g_free (uri);
        return NULL;
    }

    idx = egg_file_format_chooser_get_format (EGG_FILE_FORMAT_CHOOSER (data->format_chooser), uri);
    if (idx > 0) {
        const char *uri_ext;
        const char *default_ext;

        uri_ext = get_archive_filename_extension (uri);
        default_ext = mime_type_desc[data->supported_types[idx-1]].default_ext;
        if (strcmp_null_tolerant (uri_ext, default_ext) != 0)
            full_uri = g_strconcat (uri, default_ext, NULL);
    }
    if (full_uri == NULL)
        full_uri = uri;
    else
        g_free (uri);

    return full_uri;
}


static char *
get_archive_filename_from_selector (DlgNewData *data)
{
    char      *uri = NULL;
    GFile     *file, *dir;
    GFileInfo *info;
    GError    *err = NULL;

    uri = get_full_uri (data);
    if ((uri == NULL) || (*uri == 0)) {
        CtkWidget *dialog;

        g_free (uri);

        dialog = _ctk_error_dialog_new (CTK_WINDOW (data->dialog),
                        CTK_DIALOG_DESTROY_WITH_PARENT,
                        NULL,
                        _("Could not create the archive"),
                        "%s",
                        _("You have to specify an archive name."));
        ctk_dialog_run (CTK_DIALOG (dialog));
        ctk_widget_destroy (CTK_WIDGET (dialog));

        return NULL;
    }

    file = g_file_new_for_uri (uri);

    dir = g_file_get_parent (file);
    info = g_file_query_info (dir,
                  G_FILE_ATTRIBUTE_ACCESS_CAN_READ ","
                  G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE ","
                  G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE,
                  0, NULL, &err);
    if (err != NULL) {
        g_warning ("Failed to get permission for extraction dir: %s",
               err->message);
        g_clear_error (&err);
        g_object_unref (info);
        g_object_unref (dir);
        g_object_unref (file);
        g_free (uri);
        return NULL;
    }

    if (! g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE)) {
        CtkWidget *dialog;

        g_object_unref (info);
        g_object_unref (dir);
        g_object_unref (file);
        g_free (uri);

        dialog = _ctk_error_dialog_new (CTK_WINDOW (data->dialog),
                        CTK_DIALOG_DESTROY_WITH_PARENT,
                        NULL,
                        _("Could not create the archive"),
                        "%s",
                        _("You don't have permission to create an archive in this folder"));
        ctk_dialog_run (CTK_DIALOG (dialog));
        ctk_widget_destroy (CTK_WIDGET (dialog));
        return NULL;
    }
    g_object_unref (info);
    g_object_unref (dir);

    /* if the user did not specify a valid extension use the filetype combobox current type
     * or tar.gz if automatic is selected. */
    if (get_archive_filename_extension (uri) == NULL) {
        int   idx;
        char *new_uri;
        const char *ext = NULL;

        idx = egg_file_format_chooser_get_format (EGG_FILE_FORMAT_CHOOSER (data->format_chooser), uri);
        if (idx > 0)
            ext = mime_type_desc[data->supported_types[idx-1]].default_ext;
        else
            ext = ".tar.gz";
        new_uri = g_strconcat (uri, ext, NULL);
        g_free (uri);
        uri = new_uri;
    }

    debug (DEBUG_INFO, "create/save %s\n", uri);

    if (uri_exists (uri)) {

        if (! is_supported_extension (data->dialog, uri, data->supported_types)) {
            CtkWidget *dialog;
            dialog = _ctk_error_dialog_new (CTK_WINDOW (data->dialog),
                            CTK_DIALOG_MODAL,
                            NULL,
                            _("Could not create the archive"),
                            "%s",
                            _("Archive type not supported."));
            ctk_dialog_run (CTK_DIALOG (dialog));
            ctk_widget_destroy (CTK_WIDGET (dialog));
            g_free (uri);

            return NULL;
        }

        g_file_delete (file, NULL, &err);
        if (err != NULL) {
            CtkWidget *dialog;
            dialog = _ctk_error_dialog_new (CTK_WINDOW (data->dialog),
                            CTK_DIALOG_DESTROY_WITH_PARENT,
                            NULL,
                            _("Could not delete the old archive."),
                            "%s",
                            err->message);
            ctk_dialog_run (CTK_DIALOG (dialog));
            ctk_widget_destroy (CTK_WIDGET (dialog));
            g_error_free (err);
            g_free (uri);
            g_object_unref (file);
            return NULL;
        }
    }

    g_object_unref (file);

    return uri;
}


static void
new_file_response_cb (CtkWidget  *w G_GNUC_UNUSED,
		      int         response,
		      DlgNewData *data)
{
    char *path;

    if ((response == CTK_RESPONSE_CANCEL) || (response == CTK_RESPONSE_DELETE_EVENT)) {
        fr_archive_action_completed (data->window->archive,
                         FR_ACTION_CREATING_NEW_ARCHIVE,
                         FR_PROC_ERROR_STOPPED,
                         NULL);
        ctk_widget_destroy (data->dialog);
        return;
    }

    if (response == CTK_RESPONSE_HELP) {
        show_help_dialog (CTK_WINDOW (data->dialog), "grapa-create");
        return;
    }

    path = get_archive_filename_from_selector (data);
    if (path != NULL) {
        new_archive (data, path);
        g_free (path);
    }
}


void
show_new_archive_dialog (FrWindow   *window,
             const char *archive_name)
{
    DlgNewData *data;

    if (archive_name != NULL)
        data = dlg_save_as (window, archive_name);
    else
        data = dlg_new (window);

    g_signal_connect (G_OBJECT (data->dialog),
              "response",
              G_CALLBACK (new_file_response_cb),
              data);
    ctk_window_present (CTK_WINDOW (data->dialog));
}


void
activate_action_new (CtkAction *action G_GNUC_UNUSED,
		     gpointer   data)
{
    show_new_archive_dialog ((FrWindow*)data, NULL);
}


/* -- open archive -- */


static void
window_archive_loaded_cb (FrWindow  *window,
			  gboolean   success,
			  CtkWidget *file_sel)
{
    if (success) {
        g_signal_handlers_disconnect_by_data (window, file_sel);
        ctk_widget_destroy (file_sel);
    }
    else {
        FrWindow *original_window =  g_object_get_data (G_OBJECT (file_sel), "fr_window");
        if (window != original_window)
            fr_window_destroy_with_error_dialog (window);
    }
}


static void
open_file_response_cb (CtkWidget *w G_GNUC_UNUSED,
		       int        response,
		       CtkWidget *file_sel)
{
    FrWindow *window = NULL;
    char     *uri;

    if ((response == CTK_RESPONSE_CANCEL) || (response == CTK_RESPONSE_DELETE_EVENT)) {
        ctk_widget_destroy (file_sel);
        return;
    }

    window = g_object_get_data (G_OBJECT (file_sel), "fr_window");
    uri = ctk_file_chooser_get_uri (CTK_FILE_CHOOSER (file_sel));

    if ((window == NULL) || (uri == NULL))
        return;

    if (fr_window_archive_is_present (window))
        window = (FrWindow *) fr_window_new ();
    g_signal_connect (G_OBJECT (window),
              "archive_loaded",
              G_CALLBACK (window_archive_loaded_cb),
              file_sel);
    fr_window_archive_open (window, uri, CTK_WINDOW (file_sel));

    g_free (uri);
}


void
activate_action_open (CtkAction *action G_GNUC_UNUSED,
		      gpointer   data)
{
    CtkWidget     *file_sel;
    FrWindow      *window = data;
    CtkFileFilter *filter;
    int            i;

    file_sel = ctk_file_chooser_dialog_new (_("Open"),
                        CTK_WINDOW (window),
                        CTK_FILE_CHOOSER_ACTION_OPEN,
                        CTK_STOCK_CANCEL, CTK_RESPONSE_CANCEL,
                        CTK_STOCK_OPEN, CTK_RESPONSE_OK,
                        NULL);
    ctk_dialog_set_default_response (CTK_DIALOG (file_sel), CTK_RESPONSE_OK);
    ctk_file_chooser_set_local_only (CTK_FILE_CHOOSER (file_sel), FALSE);
    ctk_file_chooser_set_current_folder_uri (CTK_FILE_CHOOSER (file_sel), fr_window_get_open_default_dir (window));

    filter = ctk_file_filter_new ();
    ctk_file_filter_set_name (filter, _("All archives"));
    for (i = 0; open_type[i] != -1; i++)
        ctk_file_filter_add_mime_type (filter, mime_type_desc[open_type[i]].mime_type);
    ctk_file_chooser_add_filter (CTK_FILE_CHOOSER (file_sel), filter);
    ctk_file_chooser_set_filter (CTK_FILE_CHOOSER (file_sel), filter);

    filter = ctk_file_filter_new ();
    ctk_file_filter_set_name (filter, _("All files"));
    ctk_file_filter_add_pattern (filter, "*");
    ctk_file_chooser_add_filter (CTK_FILE_CHOOSER (file_sel), filter);

    /**/

    g_object_set_data (G_OBJECT (file_sel), "fr_window", window);

    g_signal_connect (G_OBJECT (file_sel),
              "response",
              G_CALLBACK (open_file_response_cb),
              file_sel);

    ctk_window_set_modal (CTK_WINDOW (file_sel), TRUE);
    ctk_widget_show (file_sel);
}


/* -- save archive -- */


static void
save_file_response_cb (CtkWidget  *w G_GNUC_UNUSED,
		       gint        response,
		       DlgNewData *data)
{
    char       *path;
    const char *password;
    gboolean    encrypt_header;
    int         volume_size;
    GSettings  *settings;

    if ((response == CTK_RESPONSE_CANCEL) || (response == CTK_RESPONSE_DELETE_EVENT)) {
        ctk_widget_destroy (data->dialog);
        return;
    }

    if (response == CTK_RESPONSE_HELP) {
        show_help_dialog (CTK_WINDOW (data->dialog), "grapa-create#grapa-convert-archive");
        return;
    }

    path = get_archive_filename_from_selector (data);
    if (path == NULL)
        return;

    password = dlg_new_data_get_password (data);
    encrypt_header = dlg_new_data_get_encrypt_header (data);
    volume_size = dlg_new_data_get_volume_size (data);

    settings = g_settings_new (GRAPA_SCHEMA_BATCH_ADD);
    g_settings_set_int (settings, PREF_BATCH_ADD_VOLUME_SIZE, volume_size);
    g_object_unref (settings);

    fr_window_archive_save_as (data->window, path, password, encrypt_header, volume_size);
    ctk_widget_destroy (data->dialog);

    g_free (path);
}


void
activate_action_save_as (CtkAction *action G_GNUC_UNUSED,
			 gpointer   callback_data)
{
    FrWindow   *window = callback_data;
    DlgNewData *data;
    char       *archive_name = NULL;

    if (fr_window_get_archive_uri (window)) {
        const char *uri;
        GFile      *file;
        GFileInfo  *info;
        GError     *err = NULL;

        uri = fr_window_get_archive_uri (window);
        file = g_file_new_for_uri (uri);
        info = g_file_query_info (file,
                      G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
                      0, NULL, &err);

        if (err != NULL) {
            g_warning ("Failed to get display name for uri %s: %s", uri, err->message);
            g_clear_error (&err);
        }
        else
            archive_name = g_strdup (g_file_info_get_display_name (info));

        g_object_unref (info);
        g_object_unref (file);
    }

    data = dlg_save_as (window, archive_name);
    g_signal_connect (G_OBJECT (data->dialog),
              "response",
              G_CALLBACK (save_file_response_cb),
              data);
    ctk_window_present (CTK_WINDOW (data->dialog));

    g_free (archive_name);
}


void
activate_action_test_archive (CtkAction *action G_GNUC_UNUSED,
			      gpointer   data)
{
    FrWindow *window = data;

    fr_window_archive_test (window);
}


void
activate_action_properties (CtkAction *action G_GNUC_UNUSED,
			    gpointer   data)
{
    FrWindow *window = data;

    dlg_prop (window);
}


void
activate_action_close (CtkAction *action G_GNUC_UNUSED,
		       gpointer   data)
{
    FrWindow *window = data;

    fr_window_close (window);
}


void
activate_action_add_files (CtkAction *action G_GNUC_UNUSED,
			   gpointer   data)
{
    add_files_cb (NULL, data);
}


void
activate_action_add_folder (CtkAction *action G_GNUC_UNUSED,
			    gpointer   data)
{
    add_folder_cb (NULL, data);
}


void
activate_action_extract (CtkAction *action G_GNUC_UNUSED,
			 gpointer   data)
{
    dlg_extract (NULL, data);
}


void
activate_action_extract_folder_from_sidebar (CtkAction *action G_GNUC_UNUSED,
					     gpointer   data)
{
    dlg_extract_folder_from_sidebar (NULL, data);
}


void
activate_action_copy (CtkAction *action G_GNUC_UNUSED,
		      gpointer   data)
{
    fr_window_copy_selection ((FrWindow*) data, FALSE);
}


void
activate_action_cut (CtkAction *action G_GNUC_UNUSED,
		     gpointer   data)
{
    fr_window_cut_selection ((FrWindow*) data, FALSE);
}


void
activate_action_paste (CtkAction *action G_GNUC_UNUSED,
		       gpointer   data)
{
    fr_window_paste_selection ((FrWindow*) data, FALSE);
}


void
activate_action_rename (CtkAction *action G_GNUC_UNUSED,
			gpointer   data)
{
    fr_window_rename_selection ((FrWindow*) data, FALSE);
}


void
activate_action_delete (CtkAction *action G_GNUC_UNUSED,
			gpointer   data)
{
    dlg_delete (NULL, data);
}


void
activate_action_copy_folder_from_sidebar (CtkAction *action G_GNUC_UNUSED,
					  gpointer   data)
{
    fr_window_copy_selection ((FrWindow*) data, TRUE);
}


void
activate_action_cut_folder_from_sidebar (CtkAction *action G_GNUC_UNUSED,
					 gpointer   data)
{
    fr_window_cut_selection ((FrWindow*) data, TRUE);
}


void
activate_action_paste_folder_from_sidebar (CtkAction *action G_GNUC_UNUSED,
					   gpointer   data)
{
    fr_window_paste_selection ((FrWindow*) data, TRUE);
}


void
activate_action_rename_folder_from_sidebar (CtkAction *action G_GNUC_UNUSED,
					    gpointer   data)
{
    fr_window_rename_selection ((FrWindow*) data, TRUE);
}


void
activate_action_delete_folder_from_sidebar (CtkAction *action G_GNUC_UNUSED,
					    gpointer   data)
{
    dlg_delete_from_sidebar (NULL, data);
}


void
activate_action_find (CtkAction *action G_GNUC_UNUSED,
		      gpointer   data)
{
    FrWindow *window = data;

    fr_window_find (window);
}


void
activate_action_select_all (CtkAction *action G_GNUC_UNUSED,
			    gpointer   data)
{
    FrWindow *window = data;

    fr_window_select_all (window);
}


void
activate_action_deselect_all (CtkAction *action G_GNUC_UNUSED,
			      gpointer   data)
{
    FrWindow *window = data;

    fr_window_unselect_all (window);
}


void
activate_action_open_with (CtkAction *action G_GNUC_UNUSED,
			   gpointer   data)
{
    open_with_cb (NULL, (FrWindow*) data);
}


void
activate_action_view_or_open (CtkAction *action G_GNUC_UNUSED,
			      gpointer   data)
{
    FrWindow *window = data;
    GList    *file_list;

    file_list = fr_window_get_file_list_selection (window, FALSE, NULL);
    if (file_list == NULL)
        return;
    fr_window_open_files (window, file_list, FALSE);
    path_list_free (file_list);
}


void
activate_action_open_folder (CtkAction *action G_GNUC_UNUSED,
			     gpointer   data)
{
    FrWindow *window = data;
    fr_window_current_folder_activated (window, FALSE);
}


void
activate_action_open_folder_from_sidebar (CtkAction *action G_GNUC_UNUSED,
					  gpointer   data)
{
    FrWindow *window = data;
    fr_window_current_folder_activated (window, TRUE);
}


void
activate_action_password (CtkAction *action G_GNUC_UNUSED,
			  gpointer   data)
{
    dlg_password (NULL, (FrWindow*) data);
}


void
activate_action_view_toolbar (CtkAction *action,
			      gpointer   data G_GNUC_UNUSED)
{
    GSettings *settings;

    settings = g_settings_new (GRAPA_SCHEMA_UI);
    g_settings_set_boolean (settings, PREF_UI_VIEW_TOOLBAR, ctk_toggle_action_get_active (CTK_TOGGLE_ACTION (action)));
    g_object_unref (settings);
}


void
activate_action_view_statusbar (CtkAction *action,
				gpointer   data G_GNUC_UNUSED)
{
    GSettings *settings;

    settings = g_settings_new (GRAPA_SCHEMA_UI);
    g_settings_set_boolean (settings, PREF_UI_VIEW_STATUSBAR, ctk_toggle_action_get_active (CTK_TOGGLE_ACTION (action)));
    g_object_unref (settings);
}


void
activate_action_view_folders (CtkAction *action,
			      gpointer   data G_GNUC_UNUSED)
{
    GSettings *settings;

    settings = g_settings_new (GRAPA_SCHEMA_UI);
    g_settings_set_boolean (settings, PREF_UI_VIEW_FOLDERS, ctk_toggle_action_get_active (CTK_TOGGLE_ACTION (action)));
    g_object_unref (settings);
}


void
activate_action_stop (CtkAction *action G_GNUC_UNUSED,
		      gpointer   data)
{
    FrWindow *window = data;
    fr_window_stop (window);
}


void
activate_action_reload (CtkAction *action G_GNUC_UNUSED,
			gpointer   data)
{
    FrWindow *window = data;

    fr_window_archive_reload (window);
}


void
activate_action_sort_reverse_order (CtkAction *action,
				    gpointer   data)
{
    FrWindow *window = data;

    fr_window_set_sort_type (window, ctk_toggle_action_get_active (CTK_TOGGLE_ACTION (action)) ? CTK_SORT_DESCENDING : CTK_SORT_ASCENDING);
}


void
activate_action_last_output (CtkAction *action G_GNUC_UNUSED,
			     gpointer   data)
{
    FrWindow *window = data;
    fr_window_view_last_output (window, _("Last Output"));
}


void
activate_action_go_back (CtkAction *action G_GNUC_UNUSED,
			 gpointer   data)
{
    FrWindow *window = data;
    fr_window_go_back (window);
}


void
activate_action_go_forward (CtkAction *action G_GNUC_UNUSED,
			    gpointer   data)
{
    FrWindow *window = data;
    fr_window_go_forward (window);
}


void
activate_action_go_up (CtkAction *action G_GNUC_UNUSED,
		       gpointer   data)
{
    FrWindow *window = data;
    fr_window_go_up_one_level (window);
}


void
activate_action_go_home (CtkAction *action G_GNUC_UNUSED,
			 gpointer   data)
{
    FrWindow *window = data;
    fr_window_go_to_location (window, "/", FALSE);
}


void
activate_action_manual (CtkAction *action G_GNUC_UNUSED,
			gpointer   data)
{
    FrWindow *window = data;

    show_help_dialog (CTK_WINDOW (window) , NULL);
}


#define ABOUT_GROUP "About"
#define EMAILIFY(string) (g_strdelimit ((string), "%", '@'))

void
activate_action_about (CtkAction *action G_GNUC_UNUSED,
		       gpointer   gp)
{
    FrWindow *window = gp;
    const char *documenters [] = {
        "Alexander Kirillov",
        "Breda McColgan",
        NULL
    };
    const char *license[] = {
        N_("Grapa is free software; you can redistribute it and/or modify "
        "it under the terms of the GNU General Public License as published by "
        "the Free Software Foundation; either version 2 of the License, or "
        "(at your option) any later version."),
        N_("Grapa is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
        "GNU General Public License for more details."),
        N_("You should have received a copy of the GNU General Public License "
        "along with Grapa; if not, write to the Free Software Foundation, Inc., "
        "51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA")
    };
    char *license_text;
    GKeyFile *key_file;
    GBytes *bytes;
    const guint8 *data;
    gsize data_len;
    GError *error = NULL;
    char **authors;
    gsize n_authors = 0, i;

    bytes = g_resources_lookup_data (GRAPA_RESOURCE_UI_PATH G_DIR_SEPARATOR_S "grapa.about", G_RESOURCE_LOOKUP_FLAGS_NONE, &error);
    g_assert_no_error (error);

    data = g_bytes_get_data (bytes, &data_len);
    key_file = g_key_file_new ();
    g_key_file_load_from_data (key_file, (const char *) data, data_len, 0, &error);
    g_assert_no_error (error);

    authors = g_key_file_get_string_list (key_file, ABOUT_GROUP, "Authors", &n_authors, NULL);

    g_key_file_free (key_file);
    g_bytes_unref (bytes);

    for (i = 0; i < n_authors; ++i)
        authors[i] = EMAILIFY (authors[i]);

    license_text =  g_strjoin ("\n\n", _(license[0]), _(license[1]), _(license[2]), NULL);

    ctk_show_about_dialog (CTK_WINDOW (window),
                   "version", VERSION,
                   "copyright", _("Copyright \xc2\xa9 2001–2010 Free Software Foundation, Inc.\n"
                                  "Copyright \xc2\xa9 2012–2020 The MATE developers\n"
                                  "Copyright \xc2\xa9 2020-2025 Pablo Barciela\n\n"
                                  "grapa icon made by https://freeicons.io/profile/3"),
                   "comments", _("An archive manager for CAFE"),
                   "authors", authors,
                   "documenters", documenters,
                   "translator-credits", _("translator-credits"),
                   "logo-icon-name", "grapa",
                   "license", license_text,
                   "wrap-license", TRUE,
                   "website", "https://cafe-desktop.org",
                   NULL);

    g_strfreev (authors);
    g_free (license_text);
}

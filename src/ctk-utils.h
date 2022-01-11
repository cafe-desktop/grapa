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

#ifndef CTK_UTILS_H
#define CTK_UTILS_H

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <ctk/ctk.h>


int         _ctk_count_selected             (CtkTreeSelection *selection);

CtkWidget*  _ctk_message_dialog_new         (CtkWindow        *parent,
					     CtkDialogFlags    flags,
					     const char       *icon_name,
					     const char       *message,
					     const char       *secondary_message,
					     const char       *first_button_text,
					     ...);

gchar*      _ctk_request_dialog_run         (CtkWindow        *parent,
					     CtkDialogFlags    flags,
					     const char       *title,
					     const char       *message,
					     const char       *default_value,
					     int               max_length,
					     const char       *no_button_text,
					     const char       *yes_button_text);

CtkWidget*  _ctk_error_dialog_new           (CtkWindow        *parent,
					     CtkDialogFlags    flags,
					     GList            *row_output,
					     const char       *primary_text,
					     const char       *secondary_text,
					     ...) G_GNUC_PRINTF (5, 6);

void        _ctk_error_dialog_run           (CtkWindow        *parent,
					     const gchar      *main_message,
					     const gchar      *format,
					     ...);

void        _ctk_entry_set_locale_text      (CtkEntry   *entry,
					     const char *text);

char *      _ctk_entry_get_locale_text      (CtkEntry   *entry);

void        _ctk_entry_set_filename_text    (CtkEntry   *entry,
					     const char *text);

CdkPixbuf * get_icon_pixbuf                 (GIcon        *icon,
		 			     int           size,
		 			     CtkIconTheme *icon_theme);

CdkPixbuf * get_mime_type_pixbuf            (const char   *mime_type,
		                             int           icon_size,
		                             CtkIconTheme *icon_theme);

void        show_help_dialog                (CtkWindow    *parent,
					     const char   *section);
CtkBuilder *
	   _ctk_builder_new_from_resource   (const char   *resource_path);
CtkWidget *
	    _ctk_builder_get_widget         (CtkBuilder   *builder,
			 		     const char   *name);

int	    _ctk_widget_lookup_for_size	    (CtkWidget *widget,
					     CtkIconSize icon_size);

CtkWidget * grapa_dialog_add_button         (CtkDialog   *dialog,
					     const gchar *button_text,
					     const gchar *icon_name,
					     gint         response_id);
gchar *
grapa_file_chooser_get_current_folder_uri   (CtkFileChooser *chooser);
gboolean
grapa_file_chooser_set_current_folder_uri   (CtkFileChooser *chooser,
					     const gchar    *uri);
gchar *
grapa_file_chooser_get_uri                  (CtkFileChooser *chooser);
#endif

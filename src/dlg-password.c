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
#include <ctk/ctk.h>
#include "fr-window.h"
#include "ctk-utils.h"
#include "preferences.h"
#include "dlg-password.h"


typedef struct {
	CtkBuilder *builder;
	FrWindow  *window;
	CtkWidget *dialog;
	CtkWidget *pw_password_entry;
	CtkWidget *pw_encrypt_header_checkbutton;
} DialogData;


/* called when the main dialog is closed. */
static void
destroy_cb (CtkWidget  *widget G_GNUC_UNUSED,
	    DialogData *data)
{
	g_object_unref (data->builder);
	g_free (data);
}


static void
response_cb (CtkWidget  *dialog G_GNUC_UNUSED,
	     int         response_id,
	     DialogData *data)
{
	char     *password;
	gboolean  encrypt_header;

	switch (response_id) {
	case CTK_RESPONSE_OK:
		password = _ctk_entry_get_locale_text (CTK_ENTRY (data->pw_password_entry));
		fr_window_set_password (data->window, password);
		g_free (password);

		encrypt_header = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (data->pw_encrypt_header_checkbutton));
		{
			GSettings *settings;

			settings = g_settings_new (GRAPA_SCHEMA_GENERAL);
			g_settings_set_boolean (settings, PREF_GENERAL_ENCRYPT_HEADER, encrypt_header);
			g_object_unref (settings);
		}
		fr_window_set_encrypt_header (data->window, encrypt_header);
		break;
	default:
		break;
	}

	ctk_widget_destroy (data->dialog);
}


void
dlg_password (CtkWidget *widget G_GNUC_UNUSED,
	      gpointer   callback_data)
{
	FrWindow   *window = callback_data;
	DialogData *data;

	data = g_new0 (DialogData, 1);

	data->builder = _ctk_builder_new_from_resource ("password.ui");
	if (data->builder == NULL) {
		g_free (data);
		return;
	}

	data->window = window;

	/* Get the widgets. */

	data->dialog = _ctk_builder_get_widget (data->builder, "password_dialog");
	data->pw_password_entry = _ctk_builder_get_widget (data->builder, "pw_password_entry");
	data->pw_encrypt_header_checkbutton = _ctk_builder_get_widget (data->builder, "pw_encrypt_header_checkbutton");

	/* Set widgets data. */

	_ctk_entry_set_locale_text (CTK_ENTRY (data->pw_password_entry), fr_window_get_password (window));
	ctk_toggle_button_set_active (CTK_TOGGLE_BUTTON (data->pw_encrypt_header_checkbutton), fr_window_get_encrypt_header (window));

	/* Set the signals handlers. */

	g_signal_connect (G_OBJECT (data->dialog),
			  "destroy",
			  G_CALLBACK (destroy_cb),
			  data);

	g_signal_connect (G_OBJECT (data->dialog),
			  "response",
			  G_CALLBACK (response_cb),
			  data);

	/* Run dialog. */

	ctk_widget_grab_focus (data->pw_password_entry);
	if (ctk_widget_get_realized (CTK_WIDGET (window)))
		ctk_window_set_transient_for (CTK_WINDOW (data->dialog),
					      CTK_WINDOW (window));
	ctk_window_set_modal (CTK_WINDOW (data->dialog), TRUE);

	ctk_widget_show (data->dialog);
}

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
#include "dlg-ask-password.h"
#include "file-utils.h"
#include "fr-window.h"
#include "glib-utils.h"
#include "ctk-utils.h"


typedef enum {
	FR_PASSWORD_TYPE_MAIN,
	FR_PASSWORD_TYPE_PASTE_FROM
} FrPasswordType;

typedef struct {
	CtkBuilder     *builder;
	FrWindow       *window;
	FrPasswordType  pwd_type;
	CtkWidget      *dialog;
	CtkWidget      *pw_password_entry;
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
ask_password__response_cb (CtkWidget  *dialog G_GNUC_UNUSED,
			   int         response_id,
			   DialogData *data)
{
	char *password;

	switch (response_id) {
	case CTK_RESPONSE_OK:
		password = _ctk_entry_get_locale_text (CTK_ENTRY (data->pw_password_entry));
		if (data->pwd_type == FR_PASSWORD_TYPE_MAIN)
			fr_window_set_password (data->window, password);
		else if (data->pwd_type == FR_PASSWORD_TYPE_PASTE_FROM)
			fr_window_set_password_for_paste (data->window, password);
		g_free (password);
		if (fr_window_is_batch_mode (data->window))
			fr_window_resume_batch (data->window);
		else
			fr_window_restart_current_batch_action (data->window);
		break;

	default:
		if (fr_window_is_batch_mode (data->window))
			ctk_widget_destroy (CTK_WIDGET (data->window));
		else
			fr_window_reset_current_batch_action (data->window);
		break;
	}

	ctk_widget_destroy (data->dialog);
}


static void
dlg_ask_password__common (FrWindow       *window,
			  FrPasswordType  pwd_type)
{
	DialogData *data;
	CtkWidget  *label;
	char       *text;
	char       *name = NULL;

	data = g_new0 (DialogData, 1);

	data->builder = _ctk_builder_new_from_resource ("batch-password.ui");
	if (data->builder == NULL) {
		g_free (data);
		return;
	}

	data->window = window;
	data->pwd_type = pwd_type;

	/* Get the widgets. */

	data->dialog = _ctk_builder_get_widget (data->builder, "password_dialog");
	data->pw_password_entry = _ctk_builder_get_widget (data->builder, "pw_password_entry");

	label = _ctk_builder_get_widget (data->builder, "pw_password_label");

	/* Set widgets data. */

	if (data->pwd_type == FR_PASSWORD_TYPE_MAIN)
		name = g_uri_display_basename (fr_window_get_archive_uri (window));
	else if (data->pwd_type == FR_PASSWORD_TYPE_PASTE_FROM)
		name = g_uri_display_basename (fr_window_get_paste_archive_uri (window));
        g_assert (name != NULL);
	text = g_strdup_printf (_("Enter the password for the archive '%s'."), name);
	ctk_label_set_label (CTK_LABEL (label), text);
	g_free (text);

	if (fr_window_get_password (window) != NULL)
		_ctk_entry_set_locale_text (CTK_ENTRY (data->pw_password_entry),
					    fr_window_get_password (window));

	/* Set the signals handlers. */

	g_signal_connect (G_OBJECT (data->dialog),
			  "destroy",
			  G_CALLBACK (destroy_cb),
			  data);

	g_signal_connect (G_OBJECT (data->dialog),
			  "response",
			  G_CALLBACK (ask_password__response_cb),
			  data);

	/* Run dialog. */

	ctk_widget_grab_focus (data->pw_password_entry);
	if (ctk_widget_get_realized (CTK_WIDGET (window))) {
		ctk_window_set_transient_for (CTK_WINDOW (data->dialog),
					      CTK_WINDOW (window));
		ctk_window_set_modal (CTK_WINDOW (data->dialog), TRUE);
	}
	else
		ctk_window_set_title (CTK_WINDOW (data->dialog), name);
	g_free (name);

	ctk_widget_show (data->dialog);
}


void
dlg_ask_password (FrWindow *window)
{
	dlg_ask_password__common (window, FR_PASSWORD_TYPE_MAIN);
}


void
dlg_ask_password_for_paste_operation (FrWindow *window)
{
	dlg_ask_password__common (window, FR_PASSWORD_TYPE_PASTE_FROM);
}

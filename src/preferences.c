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

#include <string.h>
#include "typedefs.h"
#include "preferences.h"
#include "fr-init.h"
#include "file-utils.h"
#include "fr-window.h"


void
pref_util_save_window_geometry (CtkWindow  *window,
				const char *dialog_id)
{
	char *schema;
	GSettings *settings;
	int width;
	int height;
	char *dialog_width;
	char *dialog_height;

	if (g_strcmp0 (dialog_id, "addfiles") == 0) {
		dialog_width = "width-addfiles";
		dialog_height = "height-addfiles";
	}
	else if (g_strcmp0 (dialog_id, "addfolder") == 0) {
		dialog_width = "width-addfolder";
		dialog_height = "height-addfolder";
	}
	else {
		dialog_width = "width";
		dialog_height = "height";
	}

	if (g_str_has_prefix (dialog_id, "add"))
		schema = g_strconcat (GRAPA_SCHEMA_DIALOGS, ".", "add", NULL);
	else
		schema = g_strconcat (GRAPA_SCHEMA_DIALOGS, ".", dialog_id, NULL);
	settings = g_settings_new (schema);

	ctk_window_get_size (window, &width, &height);
	g_settings_set_int (settings, dialog_width, width);
	g_settings_set_int (settings, dialog_height, height);
	g_object_unref (settings);
	g_free (schema);
}


void
pref_util_restore_window_geometry (CtkWindow  *window,
				   const char *dialog_id)
{
	char *schema;
	GSettings *settings;
	int width;
	int height;
	char *dialog_width;
	char *dialog_height;

	if (g_strcmp0 (dialog_id, "addfiles") == 0) {
		dialog_width = "width-addfiles";
		dialog_height = "height-addfiles";
	}
	else if (g_strcmp0 (dialog_id, "addfolder") == 0) {
		dialog_width = "width-addfolder";
		dialog_height = "height-addfolder";
	}
	else {
		dialog_width = "width";
		dialog_height = "height";
	}

	if (g_str_has_prefix (dialog_id, "add"))
		schema = g_strconcat (GRAPA_SCHEMA_DIALOGS, ".", "add", NULL);
	else
		schema = g_strconcat (GRAPA_SCHEMA_DIALOGS, ".", dialog_id, NULL);
	settings = g_settings_new (schema);

	width = g_settings_get_int (settings, dialog_width);
	height = g_settings_get_int (settings, dialog_height);
	if ((width != -1) && (height != 1))
		ctk_window_set_default_size (window, width, height);

	ctk_window_present (window);

	g_object_unref (settings);
	g_free (schema);
}

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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <ctk/ctk.h>
#include "typedefs.h"
#include "fr-window.h"

#define GRAPA_SCHEMA			"org.cafe.grapa"
#define GRAPA_SCHEMA_LISTING		GRAPA_SCHEMA	".listing"
#define GRAPA_SCHEMA_UI		GRAPA_SCHEMA	".ui"
#define GRAPA_SCHEMA_GENERAL		GRAPA_SCHEMA	".general"
#define GRAPA_SCHEMA_DIALOGS		GRAPA_SCHEMA	".dialogs"
#define GRAPA_SCHEMA_ADD		GRAPA_SCHEMA_DIALOGS	".add"
#define GRAPA_SCHEMA_BATCH_ADD 	GRAPA_SCHEMA_DIALOGS ".batch-add"
#define GRAPA_SCHEMA_EXTRACT		GRAPA_SCHEMA_DIALOGS ".extract"
#define GRAPA_SCHEMA_LAST_OUTPUT	GRAPA_SCHEMA_DIALOGS ".last-output"

#define PREF_LISTING_SORT_METHOD	"sort-method"
#define PREF_LISTING_SORT_TYPE		"sort-type"
#define PREF_LISTING_LIST_MODE		"list-mode"
#define PREF_LISTING_SHOW_TYPE		"show-type"
#define PREF_LISTING_SHOW_SIZE		"show-size"
#define PREF_LISTING_SHOW_TIME		"show-time"
#define PREF_LISTING_SHOW_PATH		"show-path"
#define PREF_LISTING_USE_MIME_ICONS	"use-mime-icons"
#define PREF_LISTING_NAME_COLUMN_WIDTH	"name-column-width"

#define PREF_UI_WINDOW_WIDTH		"window-width"
#define PREF_UI_WINDOW_HEIGHT		"window-height"
#define PREF_UI_SIDEBAR_WIDTH		"sidebar-width"
#define PREF_UI_HISTORY_LEN		"history-len"
#define PREF_UI_VIEW_TOOLBAR		"view-toolbar"
#define PREF_UI_VIEW_STATUSBAR		"view-statusbar"
#define PREF_UI_VIEW_FOLDERS		"view-folders"

#define PREF_GENERAL_EDITORS		"editors"
#define PREF_GENERAL_COMPRESSION_LEVEL	"compression-level"
#define PREF_GENERAL_ENCRYPT_HEADER	"encrypt-header"

#define PREF_EXTRACT_OVERWRITE		"overwrite"
#define PREF_EXTRACT_SKIP_NEWER		"skip-newer"
#define PREF_EXTRACT_RECREATE_FOLDERS	"recreate-folders"

#define PREF_ADD_CURRENT_FOLDER		"current-folder"
#define PREF_ADD_FILENAME		"filename"
#define PREF_ADD_INCLUDE_FILES		"include-files"
#define PREF_ADD_EXCLUDE_FILES		"exclude-files"
#define PREF_ADD_EXCLUDE_FOLDERS	"exclude-folders"
#define PREF_ADD_UPDATE			"update"
#define PREF_ADD_RECURSIVE		"recursive"
#define PREF_ADD_NO_SYMLINKS		"no-symlinks"

#define PREF_BATCH_ADD_DEFAULT_EXTENSION "default-extension"
#define PREF_BATCH_ADD_OTHER_OPTIONS	 "other-options"
#define PREF_BATCH_ADD_VOLUME_SIZE	 "volume-size"

#define BAUL_SCHEMA			"org.cafe.baul.preferences"
#define BAUL_CLICK_POLICY		"click-policy"

void pref_util_save_window_geometry (CtkWindow *window,
                                     const char *dialog_id);
void pref_util_restore_window_geometry (CtkWindow *window,
                                       const char *dialog_id);

#endif /* PREFERENCES_H */

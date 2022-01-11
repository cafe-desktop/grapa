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

#ifndef ACTIONS_H
#define ACTIONS_H

#include <ctk/ctk.h>
#include "fr-window.h"

void show_new_archive_dialog (FrWindow *window, const char *archive_name);

void activate_action_new (CtkAction *action, gpointer data);
void activate_action_open (CtkAction *action, gpointer data);
void activate_action_save_as (CtkAction *action, gpointer data);
void activate_action_test_archive (CtkAction *action, gpointer data);
void activate_action_properties (CtkAction *action, gpointer data);
void activate_action_close (CtkAction *action, gpointer data);
void activate_action_quit (CtkAction *action, gpointer data);

void activate_action_add_files (CtkAction *action, gpointer data);
void activate_action_add_folder (CtkAction *action, gpointer data);
void activate_action_extract (CtkAction *action, gpointer data);
void activate_action_extract_folder_from_sidebar (CtkAction *action, gpointer data);

void activate_action_copy (CtkAction *action, gpointer data);
void activate_action_cut (CtkAction *action, gpointer data);
void activate_action_paste (CtkAction *action, gpointer data);
void activate_action_rename (CtkAction *action, gpointer data);
void activate_action_delete (CtkAction *action, gpointer data);

void activate_action_copy_folder_from_sidebar (CtkAction *action, gpointer data);
void activate_action_cut_folder_from_sidebar (CtkAction *action, gpointer data);
void activate_action_paste_folder_from_sidebar (CtkAction *action, gpointer data);
void activate_action_rename_folder_from_sidebar (CtkAction *action, gpointer data);
void activate_action_delete_folder_from_sidebar (CtkAction *action, gpointer data);

void activate_action_find (CtkAction *action, gpointer data);
void activate_action_select_all (CtkAction *action, gpointer data);
void activate_action_deselect_all (CtkAction *action, gpointer data);
void activate_action_open_with (CtkAction *action, gpointer data);
void activate_action_view_or_open (CtkAction *action, gpointer data);
void activate_action_open_folder (CtkAction *action, gpointer data);
void activate_action_open_folder_from_sidebar (CtkAction *action, gpointer data);
void activate_action_password (CtkAction *action, gpointer data);

void activate_action_view_toolbar (CtkAction *action, gpointer data);
void activate_action_view_statusbar (CtkAction *action, gpointer data);
void activate_action_view_folders (CtkAction *action, gpointer data);
void activate_action_stop (CtkAction *action, gpointer data);
void activate_action_reload (CtkAction *action, gpointer data);
void activate_action_sort_reverse_order (CtkAction *action, gpointer data);
void activate_action_last_output (CtkAction *action, gpointer data);

void activate_action_go_back (CtkAction *action, gpointer data);
void activate_action_go_forward (CtkAction *action, gpointer data);
void activate_action_go_up (CtkAction *action, gpointer data);
void activate_action_go_home (CtkAction *action, gpointer data);

void activate_action_manual (CtkAction *action, gpointer data);
void activate_action_about (CtkAction *action, gpointer data);


#endif /* ACTIONS_H */

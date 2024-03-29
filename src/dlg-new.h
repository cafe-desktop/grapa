/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  Grapa
 *
 *  Copyright (C) 2008 The Free Software Foundation, Inc.
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

#ifndef DLG_NEW_H
#define DLG_NEW_H

#include <ctk/ctk.h>
#include "eggfileformatchooser.h"
#include "fr-window.h"


typedef struct {
	FrWindow   *window;
	int        *supported_types;
	gboolean    can_encrypt;
	gboolean    can_encrypt_header;
	gboolean    can_create_volumes;
	CtkBuilder *builder;

	CtkWidget  *dialog;
	/*CtkWidget  *n_archive_type_combo_box;*/
	CtkWidget  *n_other_options_expander;
	CtkWidget  *n_password_entry;
	CtkWidget  *n_password_label;
	CtkWidget  *n_encrypt_header_checkbutton;
	CtkWidget  *n_volume_checkbutton;
	CtkWidget  *n_volume_spinbutton;
	CtkWidget  *n_volume_box;
	EggFileFormatChooser *format_chooser;
} DlgNewData;


DlgNewData *    dlg_new                          (FrWindow   *window);
DlgNewData *    dlg_save_as                      (FrWindow   *window,
	 		                          const char *default_name);
const char *    dlg_new_data_get_password        (DlgNewData *data);
gboolean        dlg_new_data_get_encrypt_header  (DlgNewData *data);
int             dlg_new_data_get_volume_size     (DlgNewData *data);

#endif /* DLG_NEW_H */

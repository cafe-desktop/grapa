/*
 *  Grapa
 *
 *  Copyright (C) 2004 Free Software Foundation, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  Author: Paolo Bacchilega <paobac@cvs.cafe.org>
 *
 */

#include <config.h>
#include <libbaul-extension/baul-extension-types.h>
#include <libbaul-extension/baul-column-provider.h>
#include <glib/gi18n-lib.h>
#include "baul-grapa.h"


void
baul_module_initialize (GTypeModule*module)
{
	baul_fr_register_type (module);

	bindtextdomain (GETTEXT_PACKAGE, CAFELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}


void
baul_module_shutdown (void)
{
}


void
baul_module_list_types (const GType **types,
			    int          *num_types)
{
	static GType type_list[1];

	type_list[0] = BAUL_TYPE_FR;
	*types = type_list;
	*num_types = 1;
}

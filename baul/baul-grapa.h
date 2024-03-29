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

#ifndef BAUL_RNGRAMPA_H
#define BAUL_RNGRAMPA_H

#include <glib-object.h>

G_BEGIN_DECLS

#define BAUL_TYPE_FR  (baul_fr_get_type ())
#define BAUL_FR(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), BAUL_TYPE_FR, BaulFr))
#define BAUL_IS_FR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), BAUL_TYPE_FR))

typedef struct _BaulFr      BaulFr;
typedef struct _BaulFrClass BaulFrClass;

struct _BaulFr {
	GObject __parent;
};

struct _BaulFrClass {
	GObjectClass __parent;
};

GType baul_fr_get_type      (void);
void  baul_fr_register_type (GTypeModule *module);

G_END_DECLS

#endif /* BAUL_RNGRAMPA_H */

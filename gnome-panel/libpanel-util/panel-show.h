/*
 * panel-show.h: a helper around gtk_show_uri
 *
 * Copyright (C) 2008 Novell, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *	Vincent Untz <vuntz@gnome.org>
 */

#ifndef PANEL_SHOW_H
#define PANEL_SHOW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

gboolean panel_show_uri (GdkScreen    *screen,
			 const gchar  *uri,
			 guint32       timestamp,
			 GError      **error);

gboolean panel_show_uri_force_mime_type (GdkScreen    *screen,
					 const gchar  *uri,
					 const gchar  *mime_type,
					 guint32       timestamp,
					 GError      **error);

G_END_DECLS

#endif /* PANEL_SHOW_H */

/*
 * Pidgin Save Conversation Order
 * Copyright (C) 2010 Konrad Gräfe
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301, USA.
 */

#ifndef _UTIL_H
#define _UTIL_H

#include <gtk/gtk.h>
#include <gtkconv.h>

gboolean gtk_conv_configure_cb(GtkWidget *w, GdkEventConfigure *event, PidginConversation *conv);
PurpleBlistNode *find_blist_node(PidginConversation *conv);
void pidgin_conv_set_position_size(PidginWindow *win, int conv_x, int conv_y, int conv_width, int conv_height);

#endif /* _UTIL_H */

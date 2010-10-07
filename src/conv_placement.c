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

#include "save-conv-order.h"

#include "conv_placement.h"

#include <gtkconvwin.h>
#include <pidginstock.h>
#include <debug.h>

PidginWindow *win_mix = NULL;
static gboolean gtk_conv_configure_cb(GtkWidget *w, GdkEventConfigure *event, gpointer data) {
	int x, y;

	if (GTK_WIDGET_VISIBLE(w))
		gtk_window_get_position(GTK_WINDOW(w), &x, &y);
	else
		return FALSE; /* carry on normally */

	/* Workaround for GTK+ bug # 169811 - "configure_event" is fired
	* when the window is being maximized */
	if (gdk_window_get_state(w->window) & GDK_WINDOW_STATE_MAXIMIZED)
		return FALSE;

	/* don't save off-screen positioning */
	if (x + event->width < 0 ||
	    y + event->height < 0 ||
	    x > gdk_screen_width() ||
	    y > gdk_screen_height())
		return FALSE; /* carry on normally */

	/* store the position */
	purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/im/x", x);
	purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/im/y", y);
	purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/im/width",  event->width);
	purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/im/height", event->height);

	/* continue to handle event normally */
	return FALSE;

}

static void destroy_win_cb(GtkWidget *w, gpointer d) {
	PidginWindow *win = d;

	purple_debug_info(PLUGIN_STATIC_NAME, "hier!\n");

	if(win == win_mix) win_mix = NULL;
}

void conv_placement_fnc(PidginConversation *conv) {
	PidginWindow *win = NULL;

	win = win_mix;

	if(!win) {
		win = pidgin_conv_window_new();
		win_mix = win;

		g_signal_connect(G_OBJECT(win->window), "configure_event", G_CALLBACK(gtk_conv_configure_cb), NULL);

		/* TODO: autrennen zwischen "im" und "chat" */
		g_signal_connect(G_OBJECT(win->window), "destroy", G_CALLBACK(destroy_win_cb), win_mix);


		pidgin_conv_window_add_gtkconv(win, conv);
		pidgin_conv_window_show(win);
	} else {
		pidgin_conv_window_add_gtkconv(win, conv);
	}
}



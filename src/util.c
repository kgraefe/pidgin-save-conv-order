/*
 * Pidgin Save Conversation Order
 * Copyright (C) 2010-2016 Konrad Gräfe
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

#include "util.h"

gboolean gtk_conv_configure_cb(GtkWidget *w, GdkEventConfigure *event, PidginConversation *conv) {
	int x, y;
	PurpleConversationType type = purple_conversation_get_type(conv->active_conv);
	GList *all;

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

	for (all = conv->convs; all != NULL; all = all->next) {
		if (type != purple_conversation_get_type(all->data)) {
			/* this window has different types of conversation, don't save */
			return FALSE;
		}
	}

	/* store the position */
	if (type == PURPLE_CONV_TYPE_IM) {
		purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/im/x", x);
		purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/im/y", y);
		purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/im/width",  event->width);
		purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/im/height", event->height);
	} else if (type == PURPLE_CONV_TYPE_CHAT) {
		purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/chat/x", x);
		purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/chat/y", y);
		purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/chat/width",  event->width);
		purple_prefs_set_int(PIDGIN_PREFS_ROOT "/conversations/chat/height", event->height);
	}

	/* continue to handle event normally */
	return FALSE;

}

void pidgin_conv_set_position_size(PidginWindow *win, int conv_x, int conv_y, int conv_width, int conv_height)
{
	 /* if the window exists, is hidden, we're saving positions, and the
	  * position is sane... */
	if (win && win->window &&
			!GTK_WIDGET_VISIBLE(win->window) && conv_width != 0) {

#ifdef _WIN32  /* only override window manager placement on Windows */
		/* ...check position is on screen... */
		if (conv_x >= gdk_screen_width())
			conv_x = gdk_screen_width() - 100;
		else if (conv_x + conv_width < 0)
			conv_x = 100;

		if (conv_y >= gdk_screen_height())
			conv_y = gdk_screen_height() - 100;
		else if (conv_y + conv_height < 0)
			conv_y = 100;

		/* ...and move it back. */
		gtk_window_move(GTK_WINDOW(win->window), conv_x, conv_y);
#endif
		gtk_window_resize(GTK_WINDOW(win->window), conv_width, conv_height);
	}
}


PurpleBlistNode *find_blist_node(PidginConversation *conv) {
	if(purple_conversation_get_type(conv->active_conv) == PURPLE_CONV_TYPE_IM) {
		return (PurpleBlistNode *)purple_find_buddy(conv->active_conv->account, conv->active_conv->name);
	} else if(purple_conversation_get_type(conv->active_conv) == PURPLE_CONV_TYPE_CHAT) {
		return (PurpleBlistNode *)purple_blist_find_chat(conv->active_conv->account, conv->active_conv->name);
	} else {
		return NULL;
	}
}



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

#include "conv_placement.h"
#include "util.h"

#include <gtkconv.h>
#include <gtkconvwin.h>
#include <pidginstock.h>
#include <debug.h>

PidginWindow *win_mix = NULL;
PidginWindow *win_im = NULL;
PidginWindow *win_chat = NULL;

static gboolean reordered_by_plugin = FALSE;
static const gchar *conv_placement_fnc_ori = NULL;

static void destroy_win_cb(GtkWidget *w, gpointer d) {
	PidginWindow *win = d;

	if(win == win_mix) win_mix = NULL;
	if(win == win_im) win_im = NULL;
	if(win == win_chat) win_chat = NULL;
}

static void notebook_reordered_cb(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer user_data) {
	PurpleBlistNode *node;
	PidginWindow *win = (PidginWindow *)user_data;
	gint i;

	if(reordered_by_plugin) return;

	purple_debug_info(PLUGIN_STATIC_NAME, "notebook_reordered_cb()\n");

	/* update all indices */
	for(i = 0; i < pidgin_conv_window_get_gtkconv_count(win); i++) {
		node = find_blist_node(pidgin_conv_window_get_gtkconv_at_index(win, i));

		if(node) {
			/* +1 to not let it be 0 (since this is the indicator of an unset setting) */
			purple_blist_node_set_int(node, "tab_index", i + 1);
		}
	}
}

static void conv_placement_fnc(PidginConversation *conv) {
	PidginWindow *win = NULL;
	gboolean separated, changed;
	PidginConversation *cur, *prev;
	PurpleBlistNode *cur_node, *prev_node;
	gint i;

	separated = purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/separate_im_and_chat");

	if(separated && purple_conversation_get_type(conv->active_conv) == PURPLE_CONV_TYPE_IM) {
		win = win_im;
	} else if(separated && purple_conversation_get_type(conv->active_conv) == PURPLE_CONV_TYPE_CHAT) {
		win = win_chat;
	} else {
		win = win_mix;
	}

	if(!win) {
		win = pidgin_conv_window_new();

		if(separated && purple_conversation_get_type(conv->active_conv) == PURPLE_CONV_TYPE_IM) {
			win_im = win;
		} else if(separated && purple_conversation_get_type(conv->active_conv) == PURPLE_CONV_TYPE_CHAT) {
			win_chat = win;
		} else {
			win_mix = win;
		}

		if(purple_conversation_get_type(conv->active_conv) == PURPLE_CONV_TYPE_IM || purple_prefs_get_int(PIDGIN_PREFS_ROOT "/conversations/chat/width") == 0) {
			pidgin_conv_set_position_size(win,
				purple_prefs_get_int(PIDGIN_PREFS_ROOT "/conversations/im/x"),
				purple_prefs_get_int(PIDGIN_PREFS_ROOT "/conversations/im/y"),
				purple_prefs_get_int(PIDGIN_PREFS_ROOT "/conversations/im/width"),
				purple_prefs_get_int(PIDGIN_PREFS_ROOT "/conversations/im/height"));
		} else if(purple_conversation_get_type(conv->active_conv) == PURPLE_CONV_TYPE_CHAT) {
			pidgin_conv_set_position_size(win,
				purple_prefs_get_int(PIDGIN_PREFS_ROOT "/conversations/chat/x"),
				purple_prefs_get_int(PIDGIN_PREFS_ROOT "/conversations/chat/y"),
				purple_prefs_get_int(PIDGIN_PREFS_ROOT "/conversations/chat/width"),
				purple_prefs_get_int(PIDGIN_PREFS_ROOT "/conversations/chat/height"));
		}


		g_signal_connect(G_OBJECT(win->window), "configure_event", G_CALLBACK(gtk_conv_configure_cb), conv);

		g_signal_connect(G_OBJECT(win->window), "destroy", G_CALLBACK(destroy_win_cb), win);

		g_signal_connect(G_OBJECT(win->notebook), "page-reordered", G_CALLBACK(notebook_reordered_cb), win);


		pidgin_conv_window_show(win);
	}

	pidgin_conv_window_add_gtkconv(win, conv);

	/* now the new tab is at the bottom */
	/* let's do a (reverted) bubble sort */
	changed = TRUE;
	reordered_by_plugin = TRUE;
	while(changed) {
		changed = FALSE;

		for(i = pidgin_conv_window_get_gtkconv_count(win) - 1; i > 0; i--) {
			cur = pidgin_conv_window_get_gtkconv_at_index(win, i);
			prev = pidgin_conv_window_get_gtkconv_at_index(win, i - 1);

			cur_node = find_blist_node(cur);
			prev_node = find_blist_node(prev);

			if(cur_node && prev_node) {
				/* +1 to not let it be 0 (since this is the indicator of an unset setting) */
				if(!purple_blist_node_get_int(cur_node, "tab_index")) purple_blist_node_set_int(cur_node, "tab_index", i + 1);
				if(!purple_blist_node_get_int(prev_node, "tab_index")) purple_blist_node_set_int(prev_node, "tab_index", i);

				if(purple_blist_node_get_int(cur_node, "tab_index") < purple_blist_node_get_int(prev_node, "tab_index")) {
					gtk_notebook_reorder_child(GTK_NOTEBOOK(win->notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(win->notebook), i), i - 1);

					changed = TRUE;
				}
			} else if(cur_node) {
				gtk_notebook_reorder_child(GTK_NOTEBOOK(win->notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(win->notebook), i), i - 1);

				changed = TRUE;
			}
			
		}
	}
	reordered_by_plugin = FALSE;


}

void conv_placement_init(void) {
	pidgin_conv_placement_add_fnc(PLUGIN_STATIC_NAME, _("Save Conversation Order"), conv_placement_fnc);

	conv_placement_fnc_ori = purple_prefs_get_string(PIDGIN_PREFS_ROOT "/conversations/placement");

	purple_prefs_set_string(PIDGIN_PREFS_ROOT "/conversations/placement", PLUGIN_STATIC_NAME);
	pidgin_conv_placement_set_current_func(pidgin_conv_placement_get_fnc(PLUGIN_STATIC_NAME));

}

void conv_placement_uninit(void) {
	purple_prefs_set_string(PIDGIN_PREFS_ROOT "/conversations/placement", conv_placement_fnc_ori);
	pidgin_conv_placement_set_current_func(pidgin_conv_placement_get_fnc(conv_placement_fnc_ori));

	pidgin_conv_placement_remove_fnc(PLUGIN_STATIC_NAME);
}



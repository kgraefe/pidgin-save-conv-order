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

#include <gtkconv.h>
#include <gtkconvwin.h>
#include <pidginstock.h>
#include <debug.h>

typedef struct _pidgin_conv_desc {
	gchar *key;
	PidginWindow *win;
} PidginConversationDescription;

PidginWindow *win_mix = NULL;
PidginWindow *win_im = NULL;
PidginWindow *win_chat = NULL;

GList *conv_descriptions = NULL;

static gchar *get_key_from_conversation(PidginConversation *gtkconv) {
	gchar *ret, *c;
	const gchar *conv_name, *account_username, *protocol;
	gchar type;
	PurpleAccount *account;
	PurpleConversation *conv = gtkconv->active_conv;

	if(purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_IM) {
		type = 'I';
	} else if(purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_CHAT) {
		type = 'C';
	} else {
		type = 'U';
	}

	conv_name = purple_conversation_get_name((PurpleConversation *)conv);

	account = purple_conversation_get_account((PurpleConversation *)conv);
	account_username = purple_account_get_username(account);
	protocol = purple_account_get_protocol_name(account);


	ret = g_strdup_printf("%c_%s_%s_%s", type, conv_name, account_username, protocol);

	c = ret;
	while(*c != '\0') {
		if(*c == ' ' || *c == '\n' || *c == '\r') *c = '_';
		c++;
	}

	return ret;

}

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

	if(win == win_mix) win_mix = NULL;
	if(win == win_im) win_im = NULL;
	if(win == win_chat) win_chat = NULL;
}

static void notebook_reordered_cb(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer user_data) {
	PidginConversation *gtkconv;
	gchar *key;
	PidginWindow *win = (PidginWindow *)user_data;
	GList *cur, *before;
	PidginConversationDescription *desc;

	gtkconv = pidgin_conv_window_get_gtkconv_at_index(win, page_num);
	if(!gtkconv) return;

	key = get_key_from_conversation(gtkconv);

	/* find conversation */
	desc = NULL;
	cur = conv_descriptions;
	while(cur && !desc) {
		desc = (PidginConversationDescription *)cur->data;

		if(purple_utf8_strcasecmp(desc->key, key) != 0) {
			desc = NULL;
		}
		
		cur = cur->next;
	}

	if(!desc) return;

	conv_descriptions = g_list_remove(conv_descriptions, desc);

	before = g_list_nth(conv_descriptions, page_num);

	conv_descriptions = g_list_insert_before(conv_descriptions, before, desc);
}

static void conv_placement_fnc(PidginConversation *conv) {
	PidginWindow *win = NULL;
	gboolean separated;
	PidginConversationDescription *desc, *curdesc;
	gchar *key;
	gint pos, curpos;
	GList *cur, *cur_conv_win, *cur_conv_list, *desc_listentry;
	PidginConversation *cur_gtkconv;

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

		/* TODO: pidgin_conv_set_position_size(...) */

		g_signal_connect(G_OBJECT(win->window), "configure_event", G_CALLBACK(gtk_conv_configure_cb), NULL);

		/* TODO: auftrennen zwischen "im" und "chat" */
		g_signal_connect(G_OBJECT(win->window), "destroy", G_CALLBACK(destroy_win_cb), win);

		g_signal_connect(G_OBJECT(win->notebook), "page-reordered", G_CALLBACK(notebook_reordered_cb), win);


		pidgin_conv_window_show(win);
	}

	key = get_key_from_conversation(conv);

	/* find conversation */
	desc = NULL;
	desc_listentry = NULL;
	cur = conv_descriptions;
	while(cur && !desc) {
		curdesc = (PidginConversationDescription *)cur->data;

		if(purple_utf8_strcasecmp(curdesc->key, key) == 0) {
			desc = curdesc;
			desc_listentry = cur;
		}
		
		cur = cur->next;
	}

	if(!desc) {
		desc = g_malloc(sizeof(PidginConversationDescription));
		desc->key = key;
		conv_descriptions = g_list_append(conv_descriptions, desc);
		desc_listentry = g_list_last(conv_descriptions);
	}

	pidgin_conv_window_add_gtkconv(win, conv);
	desc->win = win;

	/* now the new tab is at the bottom */
	/* search the highest conv that is under the new tab in the list */
	cur_conv_list = desc_listentry->next;
	pos = -1;
	while(cur_conv_list && pos == -1) {
		curdesc = (PidginConversationDescription *) cur_conv_list->data;

		cur_conv_win = win->gtkconvs;
		curpos = 0;
		while(cur_conv_win && pos == -1) {
			cur_gtkconv = (PidginConversation *) cur_conv_win->data;

			if(purple_utf8_strcasecmp(curdesc->key, get_key_from_conversation(cur_gtkconv)) == 0) {
				pos = curpos;
			}

			cur_conv_win = cur_conv_win->next;
			curpos++;
		}

		cur_conv_list = cur_conv_list->next;
	}

	gtk_notebook_reorder_child(GTK_NOTEBOOK(win->notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(win->notebook), -1), pos);

}

static void conv_descriptions_load(void) {
	GList *keys;
	gchar *key;
	PidginConversationDescription *desc;

	/* TODO: PidginConversationDescription's freigeben! */
	if(conv_descriptions) g_list_free(conv_descriptions);
	conv_descriptions = NULL;

	keys = purple_prefs_get_string_list(PLUGIN_PREFS_PREFIX "/conversation_list");
	while(keys) {
		key = (gchar *)keys->data;

		desc = g_malloc(sizeof(PidginConversationDescription));
		desc->key = key;
		conv_descriptions = g_list_append(conv_descriptions, desc);

		keys = keys->next;
	}
}

static void conv_descriptions_save(void) {
	GList *keys, *cur;
	PidginConversationDescription *desc;

	cur = conv_descriptions;
	keys = NULL;
	while(cur) {
		desc = (PidginConversationDescription *)cur->data;

		keys = g_list_append(keys, desc->key);

		cur = cur->next;
	}

	purple_prefs_set_string_list(PLUGIN_PREFS_PREFIX "/conversation_list", keys);

	/* TODO: keys freigeben? */

}

void conv_placement_init(void) {
	conv_descriptions_load();

	pidgin_conv_placement_add_fnc(PLUGIN_STATIC_NAME, _("Save Conversation Order"), conv_placement_fnc);

	/* TODO: save&restore old setting */
	purple_prefs_set_string(PIDGIN_PREFS_ROOT "/conversations/placement", PLUGIN_STATIC_NAME);
	pidgin_conv_placement_set_current_func(pidgin_conv_placement_get_fnc(PLUGIN_STATIC_NAME));

}

void conv_placement_uninit(void) {
	pidgin_conv_placement_remove_fnc(PLUGIN_STATIC_NAME);

	conv_descriptions_save();
}



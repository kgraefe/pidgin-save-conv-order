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

#include <gtkplugin.h>
#include <version.h>
#include <util.h>
#include <prefs.h>

PurplePlugin *plugin;
static gboolean reordered_by_plugin = FALSE;
static GList *windows;

static void window_reorder(PidginWindow *win) {
	gboolean changed;
	PidginConversation *cur_conv, *prev_conv;
	PurpleBlistNode *cur_node, *prev_node;
	gint cur_tabidx, prev_tabidx, pos;

	if(reordered_by_plugin) {
		return;
	}
	reordered_by_plugin = TRUE;

	/* Bubble sort:
	 *	- Larger tab index goes to the end.
	 *	- New conversations (tab index = 0) go to the end as well.
	 */
	do {
		changed = FALSE;

		for(pos = pidgin_conv_window_get_gtkconv_count(win) - 1; pos > 0; pos--) {
			cur_conv = pidgin_conv_window_get_gtkconv_at_index(win, pos);
			prev_conv = pidgin_conv_window_get_gtkconv_at_index(win, pos - 1);

			cur_node = find_blist_node(cur_conv);
			prev_node = find_blist_node(prev_conv);

			if(cur_node && prev_node) {
				cur_tabidx = purple_blist_node_get_int(cur_node, "tab_index");
				prev_tabidx = purple_blist_node_get_int(prev_node, "tab_index");

				if(
					(cur_tabidx && cur_tabidx < prev_tabidx) ||
					prev_tabidx == 0
				) {
					gtk_notebook_reorder_child(
						GTK_NOTEBOOK(win->notebook),
						gtk_notebook_get_nth_page(GTK_NOTEBOOK(win->notebook), pos),
						pos - 1
					);

					changed = TRUE;
				}
			} else if(cur_node) {
				gtk_notebook_reorder_child(
					GTK_NOTEBOOK(win->notebook),
					gtk_notebook_get_nth_page(GTK_NOTEBOOK(win->notebook), pos),
					pos - 1
				);

				changed = TRUE;
			}
		}
	} while(changed);

	reordered_by_plugin = FALSE;
}
static void window_reordered_cb(
	GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer d
) {
	gint pos, tabidx, next_tabidx;
	PidginWindow *win = d;
	PurpleBlistNode *node;

	if(reordered_by_plugin) {
		return;
	}

	next_tabidx = 1; /* 0 is reserved for 'not set' */
	for(pos = 0; pos < pidgin_conv_window_get_gtkconv_count(win); pos++) {
		node = find_blist_node(pidgin_conv_window_get_gtkconv_at_index(win, pos));

		if(node) {
			tabidx = purple_blist_node_get_int(node, "tab_index");
			if(tabidx > next_tabidx) {
				next_tabidx = tabidx + 1;
			} else {
				purple_blist_node_set_int(node, "tab_index", next_tabidx);
				next_tabidx++;
			}
		}
	}
}
static void window_destroyed_cb(GtkWidget *w, gpointer d) {
	PidginWindow *win = d;

	g_signal_handlers_disconnect_matched(
		G_OBJECT(win->notebook),
		G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
		0, 0, 0,
		G_CALLBACK(window_reordered_cb), win
	);

	g_signal_handlers_disconnect_matched(
		G_OBJECT(win->window),
		G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
		0, 0, 0,
		G_CALLBACK(window_destroyed_cb), win
	);

	windows = g_list_remove(windows, win);
}
static void window_add(PidginWindow *win) {
	if(g_list_find(windows, win) == NULL) {
		windows = g_list_append(windows, win);

		g_signal_connect(
			G_OBJECT(win->notebook), "page-reordered", 
			G_CALLBACK(window_reordered_cb), win
		);
		g_signal_connect(
			G_OBJECT(win->window), "destroy", 
			G_CALLBACK(window_destroyed_cb), win
		);
	}
	window_reorder(win);
}

static void conversation_displayed_cb(PidginConversation *gtkconv) {
	if(gtkconv->win) {
		window_add(gtkconv->win);
	}
}

static gboolean plugin_load(PurplePlugin *_plugin) {
	GList *wins;
	void *gtk_conv_handle = pidgin_conversations_get_handle();

	plugin = _plugin;

	purple_signal_connect(
		gtk_conv_handle, "conversation-displayed",
		plugin, PURPLE_CALLBACK(conversation_displayed_cb),
		NULL
	);

	wins = pidgin_conv_windows_get_list();
	while(wins) {
		window_add(wins->data);

		wins = wins->next;
	}

	
	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *_plugin) {
	void *gtk_conv_handle = pidgin_conversations_get_handle();

	purple_signal_disconnect(
		gtk_conv_handle, "conversation-displayed",
		plugin, PURPLE_CALLBACK(conversation_displayed_cb)
	);

	return TRUE;
}

static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,			/**< type           */
	PIDGIN_PLUGIN_TYPE,			/**< ui_requirement */
	0,					/**< flags          */
	NULL,					/**< dependencies   */
	PURPLE_PRIORITY_DEFAULT,		/**< priority       */

	PLUGIN_ID,				/**< id             */
	NULL,					/**< name           */
	PLUGIN_VERSION,				/**< version        */
	NULL,					/**  summary        */
				
	NULL,					/**  description    */
	PLUGIN_AUTHOR,				/**< author         */
	PLUGIN_WEBSITE,				/**< homepage       */

	plugin_load,				/**< load           */
	plugin_unload,				/**< unload         */
	NULL,					/**< destroy        */

	NULL,					/**< ui_info        */
	NULL,					/**< extra_info     */
	NULL,					/**< prefs_info     */
	NULL,					/**< actions        */
	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static void init_plugin(PurplePlugin *plugin) {
	const char *str = "Save Conversation Order";
	gchar *plugins_locale_dir;

#ifdef ENABLE_NLS
	plugins_locale_dir = g_build_filename(purple_user_dir(), "locale", NULL);

	bindtextdomain(GETTEXT_PACKAGE, plugins_locale_dir);
	if(str == _(str)) {
		bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	}
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

	g_free(plugins_locale_dir);
#endif /* ENABLE_NLS */

	info.name        = _("Save Conversation Order");
	info.summary     = _("This plugin saves the order of chats and IMs and restores it the next time you open a conversation.");
	info.description = _("This plugin saves the order of chats and IMs and restores it the next time you open a conversation.");
}

PURPLE_INIT_PLUGIN(PLUGIN_STATIC_NAME, init_plugin, info)


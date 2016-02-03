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

#include "config.h"
#include "internal.h"

#include "prefs.h"

#include <pidgin.h>

static void toggle_cb(GtkWidget *widget, gpointer data) {
        gboolean value;
        gchar *pref;

        value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
        pref = (gchar *) data;

        purple_prefs_set_bool(pref, value);
}

GtkWidget *get_config_frame(PurplePlugin *plugin) {
	GtkWidget *ret, *vbox, *toggle;

	ret = gtk_vbox_new(FALSE, PIDGIN_HIG_CAT_SPACE);
	gtk_container_set_border_width(GTK_CONTAINER(ret), 12);

	vbox = gtk_vbox_new(FALSE, PIDGIN_HIG_BOX_SPACE);
	gtk_box_pack_start(GTK_BOX(ret), vbox, TRUE, TRUE, 0);

	toggle = gtk_check_button_new_with_mnemonic(_("Separate IMs and Chats"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/separate_im_and_chat"));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), PLUGIN_PREFS_PREFIX "/separate_im_and_chat");

	return ret;
}


void init_prefs(void) {
	purple_prefs_add_none(PLUGIN_PREFS_PREFIX);

	purple_prefs_add_bool(PLUGIN_PREFS_PREFIX "/separate_im_and_chat", FALSE);
	purple_prefs_add_string_list(PLUGIN_PREFS_PREFIX "/conversation_list", NULL);
}


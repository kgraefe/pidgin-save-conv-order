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

#include <gtkplugin.h>
#include <version.h>
#include <util.h>
#include <prefs.h>

#include "conv_placement.h"

PurplePlugin *plugin;

static gboolean plugin_load(PurplePlugin *_plugin) {
	plugin = _plugin;

	pidgin_conv_placement_add_fnc(PLUGIN_STATIC_NAME, _("Save Conversation Order"), conv_placement_fnc);

	/* TODO: save&restore old setting */
	purple_prefs_set_string(PIDGIN_PREFS_ROOT "/conversations/placement", PLUGIN_STATIC_NAME);
	pidgin_conv_placement_set_current_func(pidgin_conv_placement_get_fnc(PLUGIN_STATIC_NAME));

	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *_plugin) {
	pidgin_conv_placement_remove_fnc(PLUGIN_STATIC_NAME);

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
	info.summary     = _(" ");
	info.description = _(" ");
}

PURPLE_INIT_PLUGIN(PLUGIN_STATIC_NAME, init_plugin, info)


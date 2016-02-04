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

PurpleBlistNode *find_blist_node(PidginConversation *conv) {
	if(purple_conversation_get_type(conv->active_conv) == PURPLE_CONV_TYPE_IM) {
		return (PurpleBlistNode *)purple_find_buddy(
			conv->active_conv->account, conv->active_conv->name
		);
	} else if(purple_conversation_get_type(conv->active_conv) == PURPLE_CONV_TYPE_CHAT) {
		return (PurpleBlistNode *)purple_blist_find_chat(
			conv->active_conv->account, conv->active_conv->name
		);
	} else {
		return NULL;
	}
}


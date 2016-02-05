#!/bin/bash

test -f configure.ac || exit 1
test -f VERSION || exit 1

GETTEXT_PACKAGE=$(grep 'GETTEXT_PACKAGE=' configure.ac | cut -d= -f2)
PLUGIN_AUTHOR=$(grep 'PLUGIN_AUTHOR' configure.ac | cut -d\" -f2)
PLUGIN_ID=$(grep 'PLUGIN_ID' configure.ac | cut -d\" -f2)
PLUGIN_STATIC_NAME=$(grep 'PLUGIN_STATIC_NAME' configure.ac | cut -d\" -f2)
PLUGIN_WEBSITE=$(grep 'PLUGIN_WEBSITE' configure.ac | cut -d\" -f2)
PLUGIN_VERSION=$(cat VERSION)


cat << EOF
/* Generated by scripts/gen_mingw_config_h.sh
 * from files VERSION and configure.ac.
 */
#define ENABLE_NLS 1
#define GETTEXT_PACKAGE "$GETTEXT_PACKAGE"
#define PLUGIN_AUTHOR "$PLUGIN_AUTHOR"
#define PLUGIN_ID "$PLUGIN_ID"
#define PLUGIN_STATIC_NAME "$PLUGIN_STATIC_NAME"
#define PLUGIN_VERSION "$PLUGIN_VERSION"
#define PLUGIN_WEBSITE "$PLUGIN_WEBSITE"
EOF

plugindir = $(libdir)/pidgin
DEFS = -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)\"

plugin_LTLIBRARIES = save_conv_order.la
save_conv_order_la_SOURCES = \
	conv_placement.c \
	prefs.c \
	util.c \
	save-conv-order.c

save_conv_order_la_LDFLAGS = -module -avoid-version


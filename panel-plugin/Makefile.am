INCLUDES = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"xfce4-sample-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# Sample plugin
#
plugin_LTLIBRARIES = \
	libactiveapp.la

plugindir = \
	$(libdir)/xfce4/panel/plugins

libactiveapp_la_SOURCES = \
	activeapp.c \
	activeapp.h \
	activeapp-dialogs.c \
	activeapp-dialogs.h

libactiveapp_la_CFLAGS = \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(PLATFORM_CFLAGS)

libactiveapp_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       -export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
       $(PLATFORM_LDFLAGS)

libactiveapp_la_LIBADD = \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBWNCK_LIBS) \
	$(LIBXFCE4PANEL_LIBS)

#
# Desktop file
#
desktopdir =								\
	$(datadir)/xfce4/panel/plugins

desktop_DATA =								\
	activeapp.desktop

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	activeapp.desktop.in

CLEANFILES =								\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

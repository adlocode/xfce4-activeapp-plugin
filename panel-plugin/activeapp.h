/*  $Id$
 *
 *  Copyright (C) 2015 adlo
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __SAMPLE_H__
#define __SAMPLE_H__
#include <libwnck/libwnck.h>
#include <X11/Xlib.h>

#define XFCE_PANEL_IS_SMALL (xfce_panel_plugin_get_size (activeapp->plugin) < 23)

G_BEGIN_DECLS

/* plugin structure */
typedef struct
{
    XfcePanelPlugin *plugin;

    /* panel widgets */
    GtkWidget       *ebox;
    GtkWidget       *hvbox;
    GtkWidget       *label;
    GtkWidget       *icon;
    GdkPixbuf       *pixbuf;
    GtkWidget       *menu;
    GtkWidget       *action_menu;
    
    Display 		*dpy;
    WnckScreen      *screen;
    WnckWindow      *wnck_window;
    gulong          icon_changed_tag;
    gulong			name_changed_tag;
    const gchar * const * system_data_dirs;

    GError **error;

    /* sample settings */
    gchar           *setting1;
    gint             setting2;
    gboolean         setting3;
    gboolean         show_tooltips;
}
ActiveAppPlugin;



void
sample_save (XfcePanelPlugin *plugin,
             ActiveAppPlugin    *activeapp);

G_END_DECLS

#endif /* !__SAMPLE_H__ */

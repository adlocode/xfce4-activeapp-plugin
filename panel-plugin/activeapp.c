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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif


#include <gtk/gtk.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-hvbox.h>
#include <libwnck/libwnck.h>
#include <X11/Xlib.h>
#include <stdlib.h>

#include "activeapp.h"
#include "activeapp-dialogs.h"

/* default settings */
#define DEFAULT_SETTING1 NULL
#define DEFAULT_SETTING2 1
#define DEFAULT_SETTING3 FALSE
#define MAX_WIDTH_CHARS 30
#define BUF_LENGTH 1024


/* prototypes */
static void
sample_construct (XfcePanelPlugin *plugin);
static void
activeapp_on_active_window_changed (WnckScreen *screen, WnckWindow *previous_window,  ActiveAppPlugin *activeapp);

/* register the plugin */
XFCE_PANEL_PLUGIN_REGISTER (sample_construct);

static char*
latin1_to_utf8 (const char *latin1)
{
  GString *str;
  const char *p;

  str = g_string_new (NULL);

  p = latin1;
  while (*p)
    {
      g_string_append_unichar (str, (gunichar) *p);
      ++p;
    }

  return g_string_free (str, FALSE);
}

const gchar *
activeapp_get_app_name
(const gchar * const * system_data_dir, gchar *filename)
{
	const gchar *app_name = NULL;
	gboolean success = FALSE;
	GKeyFile *key_file = g_key_file_new ();

	gchar *full_filename;

	full_filename = g_strconcat (latin1_to_utf8 (system_data_dir),
				"/applications/", filename, NULL);


	success = g_key_file_load_from_file (key_file,
				full_filename,
				G_KEY_FILE_NONE,
				NULL);
	if (success)
		{
			app_name = (g_key_file_get_locale_string (key_file,
					"Desktop Entry",
					"Name",
					NULL,
					NULL));
		}
	else
		{
			filename = g_ascii_strdown (filename, -1);
			full_filename = g_strconcat (latin1_to_utf8 (system_data_dir),
				"/applications/", filename, NULL);


			success = g_key_file_load_from_file (key_file,
				full_filename,
				G_KEY_FILE_NONE,
				NULL);

			app_name = (g_key_file_get_locale_string (key_file,
					"Desktop Entry",
					"Name",
					NULL,
					NULL));
		}
	return app_name;
}



static void
activeapp_on_icon_changed (WnckWindow *window, ActiveAppPlugin *activeapp)
{
	WnckWindowType type;

	type=wnck_window_get_window_type (activeapp->wnck_window);

	if (!(type == WNCK_WINDOW_DESKTOP || type == WNCK_WINDOW_DOCK || type == WNCK_WINDOW_UTILITY))
	{
		activeapp->pixbuf=wnck_window_get_icon (activeapp->wnck_window);
		xfce_panel_image_set_from_pixbuf(XFCE_PANEL_IMAGE(activeapp->icon),activeapp->pixbuf);
	}
	else
	{
		return;
	}
}
static void
activeapp_on_name_changed (WnckWindow *window, ActiveAppPlugin *activeapp)
{
	if (activeapp->show_tooltips && gtk_widget_get_has_tooltip (activeapp->ebox))
		gtk_widget_set_tooltip_text (activeapp->ebox, wnck_window_get_name (activeapp->wnck_window));
}

static void
activeapp_on_active_window_changed (WnckScreen *screen, WnckWindow *previous_window,  ActiveAppPlugin *activeapp)

	{
		GDesktopAppInfo *app_info;

		if (activeapp->action_menu)
		{
			gtk_widget_destroy (activeapp->action_menu);
			activeapp->action_menu = NULL;
		}

		if (activeapp->icon_changed_tag)
		{
			g_signal_handler_disconnect (activeapp->wnck_window, activeapp->icon_changed_tag);
			activeapp->icon_changed_tag = 0;

		}
		if (activeapp->name_changed_tag)
		{
			g_signal_handler_disconnect (activeapp->wnck_window, activeapp->name_changed_tag);
			activeapp->name_changed_tag = 0;

		}

		gint status = 0;

		activeapp->wnck_window = wnck_screen_get_active_window(activeapp->screen);

		activeapp->icon_changed_tag = g_signal_connect (activeapp->wnck_window, "icon-changed",
                    G_CALLBACK (activeapp_on_icon_changed), activeapp);

        activeapp->name_changed_tag = g_signal_connect (activeapp->wnck_window, "name-changed",
                    G_CALLBACK (activeapp_on_name_changed), activeapp);

		WnckWindowType type;
		WnckApplication *app;

		xfce_panel_image_clear(XFCE_PANEL_IMAGE(activeapp->icon));
		gtk_widget_set_has_tooltip (activeapp->ebox, FALSE);

		if (activeapp->wnck_window)
		{
			type=wnck_window_get_window_type (activeapp->wnck_window);
			
			if (type == WNCK_WINDOW_DESKTOP || type == WNCK_WINDOW_DOCK || type == WNCK_WINDOW_UTILITY)
			{
				gtk_label_set_text(GTK_LABEL(activeapp->label),"");
			}

			else
			{

				app=wnck_window_get_application(activeapp->wnck_window);

				//Get class group name
				XClassHint ch;
				status = XGetClassHint (activeapp->dpy, wnck_window_get_xid(activeapp->wnck_window),
									&ch);

				gchar *filename = NULL;

				filename = g_strconcat (latin1_to_utf8 (ch.res_name), ".desktop", NULL);

				if (wnck_window_is_skip_pager(activeapp->wnck_window))
				{
					gtk_label_set_text(GTK_LABEL(activeapp->label),"");


				}
				else
				{
					gchar *app_name = NULL;

					gboolean success;

				int i;

				success = FALSE;

				for (i=0; activeapp->system_data_dirs [i]; i++)

				{


					if (activeapp->system_data_dirs [i])
					{

						app_name = activeapp_get_app_name (activeapp->system_data_dirs [i],
										   filename);

					if (app_name == NULL)
						{
							filename = g_strconcat (latin1_to_utf8 (ch.res_class), ".desktop", NULL);
							app_name = activeapp_get_app_name (activeapp->system_data_dirs [i],
										   filename);

						}
					}
				}

					if (app_name == NULL)
						{
							app_name = g_strdup (latin1_to_utf8 (ch.res_class));
						}


					gtk_label_set_text(GTK_LABEL(activeapp->label),app_name);
					
					if (XFCE_PANEL_IS_SMALL)
					{
						activeapp->pixbuf= g_object_ref (wnck_window_get_mini_icon (activeapp->wnck_window));
					}
					else
					{
						activeapp->pixbuf= g_object_ref (wnck_window_get_icon (activeapp->wnck_window));
					}
					
					xfce_panel_image_set_from_pixbuf(XFCE_PANEL_IMAGE(activeapp->icon),activeapp->pixbuf);
					
					if (activeapp->show_tooltips)
					{
						gtk_widget_set_tooltip_text (activeapp->ebox, wnck_window_get_name (activeapp->wnck_window));
					}
				
					if (app_name)
						g_free (app_name);
					
					g_object_unref (activeapp->pixbuf);
				
				}
			
			if (status != 0)
				XFree (ch.res_name);
			
			if (status != 0)	
				XFree (ch.res_class);
			
			g_free (filename);

			
			}
}
	else
	{gtk_label_set_text(GTK_LABEL(activeapp->label),"");
		xfce_panel_image_clear(XFCE_PANEL_IMAGE(activeapp->icon));
		}
	}
	
static gint
activeapp_popup_handler (GtkWidget *widget, GdkEventButton *event, ActiveAppPlugin *activeapp)
{
	if ((event->button == 1) 
	&& (wnck_window_get_window_type (activeapp->wnck_window) != WNCK_WINDOW_DESKTOP))
	{
		if (!activeapp->action_menu)
		{
			activeapp->action_menu = wnck_action_menu_new (activeapp->wnck_window);
			gtk_menu_attach_to_widget (GTK_MENU (activeapp->action_menu), activeapp->ebox, NULL);
			
		}
		
		GdkEventButton *event_button = (GdkEventButton *) event;
		gtk_menu_popup (GTK_MENU (activeapp->action_menu), NULL, NULL, 
					xfce_panel_plugin_position_menu,
					activeapp->plugin,
					event_button->button, event_button->time);
	}
	
	else
	{
		
	}			
}

static int activeapp_xhandler_xerror (Display *dpy, XErrorEvent *e)
{
	return 0;
}

void
sample_save (XfcePanelPlugin *plugin,
             ActiveAppPlugin    *activeapp)
{
  XfceRc *rc;
  gchar  *file;

  /* get the config file location */
  file = xfce_panel_plugin_save_location (plugin, TRUE);

  if (G_UNLIKELY (file == NULL))
    {
       DBG ("Failed to open config file");
       return;
    }

  /* open the config file, read/write */
  rc = xfce_rc_simple_open (file, FALSE);
  g_free (file);

  if (G_LIKELY (rc != NULL))
    {
      /* save the settings */
      DBG(".");
      if (activeapp->setting1)
        xfce_rc_write_entry    (rc, "setting1", activeapp->setting1);

      xfce_rc_write_int_entry  (rc, "setting2", activeapp->setting2);
      xfce_rc_write_bool_entry (rc, "setting3", activeapp->setting3);

      /* close the rc file */
      xfce_rc_close (rc);
    }
}



static void
sample_read (ActiveAppPlugin *activeapp)
{
  XfceRc      *rc;
  gchar       *file;
  const gchar *value;

  /* get the plugin config file location */
  file = xfce_panel_plugin_save_location (activeapp->plugin, TRUE);

  if (G_LIKELY (file != NULL))
    {
      /* open the config file, readonly */
      rc = xfce_rc_simple_open (file, TRUE);

      /* cleanup */
      g_free (file);

      if (G_LIKELY (rc != NULL))
        {
          /* read the settings */
          value = xfce_rc_read_entry (rc, "setting1", DEFAULT_SETTING1);
          activeapp->setting1 = g_strdup (value);

          activeapp->setting2 = xfce_rc_read_int_entry (rc, "setting2", DEFAULT_SETTING2);
          activeapp->setting3 = xfce_rc_read_bool_entry (rc, "setting3", DEFAULT_SETTING3);

          /* cleanup */
          xfce_rc_close (rc);

          /* leave the function, everything went well */
          return;
        }
    }

  /* something went wrong, apply default values */
  DBG ("Applying default settings");

  activeapp->setting1 = g_strdup (DEFAULT_SETTING1);
  activeapp->setting2 = DEFAULT_SETTING2;
  activeapp->setting3 = DEFAULT_SETTING3;
}



static ActiveAppPlugin *
sample_new (XfcePanelPlugin *plugin)
{
  ActiveAppPlugin   *activeapp;
  GtkOrientation  orientation;
  GtkWidget      *label;

  /* allocate memory for the plugin structure */
  activeapp = panel_slice_new0 (ActiveAppPlugin);

  /* pointer to plugin */
  activeapp->plugin = plugin;

  /* read the user settings */
  sample_read (activeapp);
  
  /*Initialize variables */
  activeapp->show_tooltips = TRUE;
  
  activeapp->icon_changed_tag = 0;
  activeapp->name_changed_tag = 0;
  
  activeapp->screen = wnck_screen_get_default ();
  
  activeapp->dpy = gdk_x11_get_default_xdisplay();
  
  activeapp->system_data_dirs = g_get_system_data_dirs ();
  
  activeapp->action_menu = NULL;
  
  XSetErrorHandler (activeapp_xhandler_xerror);
  

  /* get the current orientation */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* create some panel widgets */

  
  activeapp->ebox = gtk_event_box_new ();
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (activeapp->ebox), FALSE);
  gtk_widget_show (activeapp->ebox);

  activeapp->hvbox = xfce_hvbox_new (orientation, FALSE, 2);
  gtk_widget_show (activeapp->hvbox);
  gtk_container_add (GTK_CONTAINER (activeapp->ebox), activeapp->hvbox);

  /* some sample widgets */
  activeapp->icon = xfce_panel_image_new ();
  gtk_widget_show (activeapp->icon);
  gtk_box_pack_start (GTK_BOX (activeapp->hvbox), activeapp->icon, FALSE, FALSE, 0);
  
  activeapp->label = gtk_label_new ("");
  gtk_label_set_max_width_chars (GTK_LABEL(activeapp->label), MAX_WIDTH_CHARS);
  gtk_label_set_ellipsize (GTK_LABEL (activeapp->label), PANGO_ELLIPSIZE_END);
  gtk_widget_show (activeapp->label);
  gtk_box_pack_start (GTK_BOX (activeapp->hvbox), activeapp->label, FALSE, FALSE, 0);
  
  activeapp_on_active_window_changed (NULL, NULL, activeapp);

  return activeapp;
}



static void
sample_free (XfcePanelPlugin *plugin,
             ActiveAppPlugin    *sample)
{
  GtkWidget *dialog;

  /* check if the dialog is still open. if so, destroy it */
  dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
  if (G_UNLIKELY (dialog != NULL))
    gtk_widget_destroy (dialog);

  /* destroy the panel widgets */
  gtk_widget_destroy (sample->hvbox);

  /* cleanup the settings */
  if (G_LIKELY (sample->setting1 != NULL))
    g_free (sample->setting1);

  /* free the plugin structure */
  panel_slice_free (ActiveAppPlugin, sample);
}



static void
sample_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation,
                            ActiveAppPlugin    *sample)
{
  /* change the orienation of the box */
  xfce_hvbox_set_orientation (XFCE_HVBOX (sample->hvbox), orientation);
}



static gboolean
sample_size_changed (XfcePanelPlugin *plugin,
                     gint             size,
                     ActiveAppPlugin    *sample)
{
  GtkOrientation orientation;

  /* get the orientation of the plugin */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* set the widget size */
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, size);
  else
    gtk_widget_set_size_request (GTK_WIDGET (plugin), size, -1);

  /* we handled the orientation */
  return TRUE;
}



static void
sample_construct (XfcePanelPlugin *plugin)
{
  ActiveAppPlugin *sample;

  /* setup transation domain */
  xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* create the plugin */
  sample = sample_new (plugin);
  
  

  /* add the ebox to the panel */
  gtk_container_add (GTK_CONTAINER (plugin), sample->ebox);

  /* show the panel's right-click menu on this ebox */
  xfce_panel_plugin_add_action_widget (plugin, sample->ebox);

  /* connect plugin signals */
  g_signal_connect (G_OBJECT (plugin), "free-data",
                    G_CALLBACK (sample_free), sample);

  g_signal_connect (G_OBJECT (plugin), "save",
                    G_CALLBACK (sample_save), sample);

  g_signal_connect (G_OBJECT (plugin), "size-changed",
                    G_CALLBACK (sample_size_changed), sample);

  g_signal_connect (G_OBJECT (plugin), "orientation-changed",
                    G_CALLBACK (sample_orientation_changed), sample);

  /* show the configure menu item and connect signal */
  xfce_panel_plugin_menu_show_configure (plugin);
  g_signal_connect (G_OBJECT (plugin), "configure-plugin",
                    G_CALLBACK (sample_configure), sample);

  /* show the about menu item and connect signal */
  xfce_panel_plugin_menu_show_about (plugin);
  g_signal_connect (G_OBJECT (plugin), "about",
                    G_CALLBACK (sample_about), NULL);
                    
  
  
  
  g_signal_connect (sample->screen, "active-window-changed",
                    G_CALLBACK (activeapp_on_active_window_changed), sample);
                    
  g_signal_connect (sample->ebox, "button-press-event", 
			G_CALLBACK(activeapp_popup_handler), sample);
}

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

char 
*activeapp_get_app_name 
(const gchar * const * system_data_dirs, char *filename, GError **error, WnckApplication *app, gboolean *freeable)
{				
				*freeable = FALSE;
				
				int i;
				for (i=0; system_data_dirs [i]; i++)
  
				{
					
					
					if (system_data_dirs [i])
					{
							char *dir = malloc (sizeof (char)* BUF_LENGTH);
							
							strncpy (dir, latin1_to_utf8 (system_data_dirs [i]), BUF_LENGTH - 1);
							dir[BUF_LENGTH - 1] = '\0';
							
							GKeyFile *key_file = g_key_file_new ();
							
							
							
							g_key_file_load_from_file (key_file,
								strcat (strcat (dir, "/applications/"), filename),
								G_KEY_FILE_NONE,
								error);
								
							char *name = (g_key_file_get_string (key_file,
								"Desktop Entry",
								"Name",
								error));
								
				

							
							if (name)
							{
								freeable = TRUE;
								free (dir);
								g_key_file_free (key_file);
								return name;
							
							}
							
							else
							
							{
								free (name);
								free (dir);
								g_key_file_free (key_file);
							}
							
							
							
							
					}
				
				
				
					
					
				}

		
		return wnck_application_get_name (app);
			
}

static void
activeapp_on_icon_changed (WnckWindow *window, ActiveAppPlugin *sample)
{
	sample->pixbuf=wnck_window_get_icon (sample->wnck_window);
	gtk_image_set_from_pixbuf(GTK_IMAGE(sample->icon),sample->pixbuf);
}

static void
activeapp_on_active_window_changed (WnckScreen *screen, WnckWindow *previous_window,  ActiveAppPlugin *sample)

	{
		if (sample->icon_changed_tag)
		{
			g_signal_handler_disconnect (sample->wnck_window, sample->icon_changed_tag);
			sample->icon_changed_tag = 0;
			
		}
		
		WnckWindow *active_window;
		sample->wnck_window = wnck_screen_get_active_window(sample->screen);
				
		sample->icon_changed_tag = g_signal_connect (sample->wnck_window, "icon-changed",
                    G_CALLBACK (activeapp_on_icon_changed), sample);
                    
                    
		WnckWindowType type;
		WnckApplication *app;
		
		gtk_image_clear(GTK_IMAGE(sample->icon));
		
		if (sample->wnck_window)
		{
			type=wnck_window_get_window_type (sample->wnck_window);
			if (type == WNCK_WINDOW_DESKTOP || type == WNCK_WINDOW_DOCK || type == WNCK_WINDOW_UTILITY)
			{	
				gtk_label_set_text(GTK_LABEL(sample->label),"");
			}
			
			else
			{
							
				app=wnck_window_get_application(sample->wnck_window);
				
				//Get class group name
				XClassHint ch;
				XGetClassHint (sample->dpy, wnck_window_get_xid(sample->wnck_window),
					&ch);
				
				//Build up path	
				char *class_group_name = malloc (sizeof (char)*BUF_LENGTH);
				strncpy (class_group_name, latin1_to_utf8 (ch.res_name), BUF_LENGTH - 1);
				class_group_name [BUF_LENGTH - 1] = '\0';
				
				char *filename =  malloc (sizeof (char)*BUF_LENGTH);
				filename = strcat (class_group_name, ".desktop");
				
								
				if (wnck_window_is_skip_pager(sample->wnck_window))
				{gtk_label_set_text(GTK_LABEL(sample->label),"");
				
			}
			else
			{
				gboolean freeable;
				char *app_name = 
					activeapp_get_app_name
						(sample->system_data_dirs, filename, sample->error, app, &freeable);
						
				gtk_label_set_text(GTK_LABEL(sample->label),app_name);
				sample->pixbuf=wnck_window_get_icon (sample->wnck_window);
				gtk_image_set_from_pixbuf(GTK_IMAGE(sample->icon),sample->pixbuf);
				
				if (freeable && app_name)
					free (app_name);
			}
			
			free (class_group_name);
			
			}
}
	else
	{gtk_label_set_text(GTK_LABEL(sample->label),"");
		gtk_image_clear(GTK_IMAGE(sample->icon));
		}
	}
	
static gint
activeapp_popup_handler (GtkWidget *widget, GdkEventButton *event, ActiveAppPlugin *sample)
{
	if ((event->button == 1) 
	&& (wnck_window_get_window_type (sample->wnck_window) != WNCK_WINDOW_DESKTOP))
	{
		sample->action_menu = wnck_action_menu_new (sample->wnck_window);
		gtk_menu_attach_to_widget (GTK_MENU (sample->action_menu), sample->ebox, NULL);
		GdkEventButton *event_button = (GdkEventButton *) event;
	
		gtk_menu_popup (GTK_MENU (sample->action_menu), NULL, NULL, 
					xfce_panel_plugin_position_menu,
					sample->plugin,
					event_button->button, event_button->time);
	}
	
	else
	{
	}			
}

void
sample_save (XfcePanelPlugin *plugin,
             ActiveAppPlugin    *sample)
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
      if (sample->setting1)
        xfce_rc_write_entry    (rc, "setting1", sample->setting1);

      xfce_rc_write_int_entry  (rc, "setting2", sample->setting2);
      xfce_rc_write_bool_entry (rc, "setting3", sample->setting3);

      /* close the rc file */
      xfce_rc_close (rc);
    }
}



static void
sample_read (ActiveAppPlugin *sample)
{
  XfceRc      *rc;
  gchar       *file;
  const gchar *value;

  /* get the plugin config file location */
  file = xfce_panel_plugin_save_location (sample->plugin, TRUE);

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
          sample->setting1 = g_strdup (value);

          sample->setting2 = xfce_rc_read_int_entry (rc, "setting2", DEFAULT_SETTING2);
          sample->setting3 = xfce_rc_read_bool_entry (rc, "setting3", DEFAULT_SETTING3);

          /* cleanup */
          xfce_rc_close (rc);

          /* leave the function, everything went well */
          return;
        }
    }

  /* something went wrong, apply default values */
  DBG ("Applying default settings");

  sample->setting1 = g_strdup (DEFAULT_SETTING1);
  sample->setting2 = DEFAULT_SETTING2;
  sample->setting3 = DEFAULT_SETTING3;
}



static ActiveAppPlugin *
sample_new (XfcePanelPlugin *plugin)
{
  ActiveAppPlugin   *sample;
  GtkOrientation  orientation;
  GtkWidget      *label;

  /* allocate memory for the plugin structure */
  sample = panel_slice_new0 (ActiveAppPlugin);

  /* pointer to plugin */
  sample->plugin = plugin;

  /* read the user settings */
  sample_read (sample);
  
  sample->icon_changed_tag = 0;

  /* get the current orientation */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* create some panel widgets */
  sample->ebox = gtk_event_box_new ();
  gtk_widget_show (sample->ebox);

  sample->hvbox = xfce_hvbox_new (orientation, FALSE, 2);
  gtk_widget_show (sample->hvbox);
  gtk_container_add (GTK_CONTAINER (sample->ebox), sample->hvbox);

  /* some sample widgets */
  sample->icon = gtk_image_new ();
  gtk_widget_show (sample->icon);
  gtk_box_pack_start (GTK_BOX (sample->hvbox), sample->icon, FALSE, FALSE, 0);
  
  sample->label = gtk_label_new ("");
  gtk_label_set_max_width_chars (GTK_LABEL(sample->label), MAX_WIDTH_CHARS);
  gtk_label_set_ellipsize (GTK_LABEL (sample->label), PANGO_ELLIPSIZE_END);
  gtk_widget_show (sample->label);
  gtk_box_pack_start (GTK_BOX (sample->hvbox), sample->label, FALSE, FALSE, 0);
  
  activeapp_on_active_window_changed (NULL, NULL, sample);

  return sample;
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
                    
  sample->screen = wnck_screen_get_default ();
  
  sample->dpy = gdk_x11_get_default_xdisplay();
  
  sample->system_data_dirs = g_get_system_data_dirs ();
  
  
  g_signal_connect (sample->screen, "active-window-changed",
                    G_CALLBACK (activeapp_on_active_window_changed), sample);
                    
  g_signal_connect (sample->ebox, "button-press-event", 
			G_CALLBACK(activeapp_popup_handler), sample);
}

/* Eye Of Gnome - General Utilities 
 *
 * Copyright (C) 2006 The Free Software Foundation
 *
 * Author: Lucas Rocha <lucasr@gnome.org>
 *
 * Based on code by:
 *	- Jens Finke <jens@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STRPTIME
#define _XOPEN_SOURCE
#endif /* HAVE_STRPTIME */

#include <time.h>

#include "eog-util.h"

#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libgnome/gnome-help.h>
#include <libgnome/gnome-desktop-item.h>
#include <libgnomevfs/gnome-vfs.h>

void 
eog_util_show_help (const gchar *section, GtkWindow *parent)
{
	GError *error = NULL;

	gnome_help_display ("eog.xml", section, &error);

	if (error) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (parent,
						 0,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_OK,
						 _("Could not display help for Eye of GNOME"));

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
							  error->message);

		g_signal_connect_swapped (dialog, "response",
					  G_CALLBACK (gtk_widget_destroy),
					  dialog);
		gtk_widget_show (dialog);

		g_error_free (error);
	}
}

gchar *
eog_util_make_valid_utf8 (const gchar *str)
{
	gchar *utf_str;

	if (g_utf8_validate (str, -1, NULL)) {
		utf_str = g_strdup (str);
	} else {
		utf_str = g_locale_to_utf8 (str, -1, NULL, NULL, NULL);
	}

	return utf_str;
}

GSList*
eog_util_string_list_to_uri_list (GSList *string_list)
{
	GSList *it = NULL;
	GSList *uri_list = NULL;

	for (it = string_list; it != NULL; it = it->next) {
		char *uri_str;

		uri_str = (gchar *) it->data;
		
		uri_list = g_slist_prepend (uri_list, 
					    gnome_vfs_uri_new (uri_str));
	}

	return g_slist_reverse (uri_list);
}

#ifdef HAVE_DBUS
GSList*
eog_util_strings_to_uri_list (gchar **strings)
{
	int i;
 	GSList *uri_list = NULL;

	for (i = 0; strings[i]; i++) {
 		uri_list = g_slist_prepend (uri_list, 
					    gnome_vfs_uri_new (strings[i]));
 	}
 
 	return g_slist_reverse (uri_list);
}
#endif

GSList*
eog_util_string_array_to_list (const gchar **files, gboolean create_uri)
{
	gint i;
	GSList *list = NULL;

	if (files == NULL) return list;

	for (i = 0; files[i]; i++) {
		char *str;

		if (create_uri) {
			str = gnome_vfs_make_uri_from_input_with_dirs (files[i], 
								       GNOME_VFS_MAKE_URI_DIR_CURRENT);
		} else {
			str = g_strdup (files[i]);
		}

		if (str) {
			list = g_slist_prepend (list, g_strdup (str));
			g_free (str);
		}
	}

	return g_slist_reverse (list);
}

gchar **
eog_util_string_array_make_absolute (gchar **files)
{
	int i;
	gchar **abs_files;

	if (files == NULL)
		return NULL;
	
	int size = g_strv_length (files);
	
	abs_files = g_new0 (gchar *, size);
	
	for (i = 0; i < size; i++) {
		abs_files[i] = gnome_vfs_make_uri_from_input_with_dirs (files[i],
									GNOME_VFS_MAKE_URI_DIR_CURRENT);
	}
	
	return abs_files;
}

static int
launch_desktop_item (const char *desktop_file,
                     guint32 user_time,
                     GError **error)
{
	GnomeDesktopItem *item = NULL;
	GList *uris = NULL;
	int ret = -1;
	
	item = gnome_desktop_item_new_from_file (desktop_file, 0, NULL);

	if (item == NULL) return FALSE;
	
	gnome_desktop_item_set_launch_time (item, user_time);

	ret = gnome_desktop_item_launch (item, uris, 0, error);
	
	g_list_foreach (uris, (GFunc) g_free, NULL);
	g_list_free (uris);
	gnome_desktop_item_unref (item);
	
	return ret;
}

gboolean
eog_util_launch_desktop_file (const gchar *filename,
                              guint32      user_time)
{
	GError *error = NULL;
	const char * const *dirs;
	char *path = NULL;
	int i, ret = -1;
	
	dirs = g_get_system_data_dirs ();

	if (dirs == NULL) return FALSE;
	
	for (i = 0; dirs[i] != NULL; i++) {
	        path = g_build_filename (dirs[i], 
					 "applications", 
					 filename, 
					 NULL);

	        if (g_file_test (path, G_FILE_TEST_IS_REGULAR)) break;
	
	        g_free (path);
		path = NULL;
	}
	
	if (path != NULL) {
	        ret = launch_desktop_item (path, user_time, &error);
	
	        if (ret == -1 || error != NULL) {
	                g_warning ("Cannot launch desktop item '%s': %s\n",
	                           path, 
				   error ? error->message : "unknown error");

	                g_clear_error (&error);
	        }
	
	        g_free (path);
	}
	
	return ret >= 0;
}
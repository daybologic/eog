/* Eye Of Gnome - Erro Message Area 
 *
 * Copyright (C) 2007 The Free Software Foundation
 *
 * Author: Lucas Rocha <lucasr@gnome.org>
 *
 * Based on gedit code (gedit/gedit-message-area.h) by: 
 * 	- Paolo Maggi <paolo@gnome.org>
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
#include <config.h>
#endif

#include "eog-error-message-area.h"
#include "eog-message-area.h"
#include "eog-image.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <gtk/gtk.h>

static void
set_message_area_text_and_icon (EogMessageArea   *message_area,
				const gchar      *icon_stock_id,
				const gchar      *primary_text,
				const gchar      *secondary_text)
{
	GtkWidget *hbox_content;
	GtkWidget *image;
	GtkWidget *vbox;
	gchar *primary_markup;
	gchar *secondary_markup;
	GtkWidget *primary_label;
	GtkWidget *secondary_label;

	hbox_content = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox_content);

	image = gtk_image_new_from_stock (icon_stock_id, GTK_ICON_SIZE_DIALOG);
	gtk_widget_show (image);
	gtk_box_pack_start (GTK_BOX (hbox_content), image, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0);

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (hbox_content), vbox, TRUE, TRUE, 0);

	primary_markup = g_strdup_printf ("<b>%s</b>", primary_text);
	primary_label = gtk_label_new (primary_markup);
	g_free (primary_markup);

	gtk_widget_show (primary_label);

	gtk_box_pack_start (GTK_BOX (vbox), primary_label, TRUE, TRUE, 0);
	gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
	gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (primary_label), 0, 0.5);

	GTK_WIDGET_SET_FLAGS (primary_label, GTK_CAN_FOCUS);

	gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);

  	if (secondary_text != NULL) {
  		secondary_markup = g_strdup_printf ("<small>%s</small>",
  						    secondary_text);
		secondary_label = gtk_label_new (secondary_markup);
		g_free (secondary_markup);

		gtk_widget_show (secondary_label);

		gtk_box_pack_start (GTK_BOX (vbox), secondary_label, TRUE, TRUE, 0);

		GTK_WIDGET_SET_FLAGS (secondary_label, GTK_CAN_FOCUS);

		gtk_label_set_use_markup (GTK_LABEL (secondary_label), TRUE);
		gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
		gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (secondary_label), 0, 0.5);
	}

	eog_message_area_set_contents (EOG_MESSAGE_AREA (message_area),
				       hbox_content);
}

static GtkWidget *
create_image_load_error_message_area (const gchar *primary_text,
				      const gchar *secondary_text)
{
	GtkWidget *message_area;

	message_area = eog_message_area_new_with_buttons (
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					NULL);

	set_message_area_text_and_icon (EOG_MESSAGE_AREA (message_area),
					GTK_STOCK_DIALOG_ERROR,
					primary_text,
					secondary_text);

	return message_area;
}

GtkWidget *
eog_image_load_error_message_area_new (const gchar  *caption,
				       const GError *error)
{
	GtkWidget *message_area;
	gchar *error_message = NULL;
	gchar *message_details = NULL;

	g_return_val_if_fail (caption != NULL, NULL);
	g_return_val_if_fail (error != NULL, NULL);
	//g_return_val_if_fail (error->domain == EOG_IMAGE_ERROR, NULL);

	error_message = g_strdup_printf (_("Could not load image '%s'."),
					 caption);

	message_details = g_strdup (error->message); 

	message_area = create_image_load_error_message_area (error_message,
							     message_details);

	g_free (error_message);
	g_free (message_details);

	return message_area;
}
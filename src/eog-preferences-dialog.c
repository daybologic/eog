/* Eye Of Gnome - EOG Preferences Dialog 
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

#include "eog-preferences-dialog.h"
#include "eog-util.h"
#include "eog-config-keys.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#define EOG_PREFERENCES_DIALOG_GET_PRIVATE(object) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((object), EOG_TYPE_PREFERENCES_DIALOG, EogPreferencesDialogPrivate))

G_DEFINE_TYPE (EogPreferencesDialog, eog_preferences_dialog, EOG_TYPE_DIALOG);

enum {
        PROP_0,
        PROP_GCONF_CLIENT,
};

#define GCONF_OBJECT_KEY	"GCONF_KEY"
#define GCONF_OBJECT_VALUE	"GCONF_VALUE"

struct _EogPreferencesDialogPrivate {
	GConfClient   *client;
};

static GObject *instance = NULL;

static void
pd_check_toggle_cb (GtkWidget *widget, gpointer data)
{
	char *key = NULL;

	key = g_object_get_data (G_OBJECT (widget), GCONF_OBJECT_KEY);

	if (key == NULL) return;

	gconf_client_set_bool (GCONF_CLIENT (data),
			       key,
			       gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)),
			       NULL);
}

static void
pd_spin_button_changed_cb (GtkWidget *widget, gpointer data)
{
	char *key = NULL;

	key = g_object_get_data (G_OBJECT (widget), GCONF_OBJECT_KEY);

	if (key == NULL) return;

	gconf_client_set_int (GCONF_CLIENT (data),
			      key,
			      gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget)),
			      NULL);
}

static void
pd_color_change_cb (GtkColorButton *button, gpointer data)
{
	GdkColor color;
	char *key = NULL;
	char *value = NULL;

	gtk_color_button_get_color (button, &color);

	value = g_strdup_printf ("#%02X%02X%02X",
				 color.red / 256,
				 color.green / 256,
				 color.blue / 256);

	key = g_object_get_data (G_OBJECT (button), GCONF_OBJECT_KEY);

	if (key == NULL || value == NULL) 
		return;

	gconf_client_set_string (GCONF_CLIENT (data),
				 key,
				 value,
				 NULL);
	g_free (value);
}

static void
pd_radio_toggle_cb (GtkWidget *widget, gpointer data)
{
	char *key = NULL;
	char *value = NULL;
	
	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	    return;

	key = g_object_get_data (G_OBJECT (widget), GCONF_OBJECT_KEY);
	value = g_object_get_data (G_OBJECT (widget), GCONF_OBJECT_VALUE);

	if (key == NULL || value == NULL) 
		return;

	gconf_client_set_string (GCONF_CLIENT (data),
				 key,
				 value,
				 NULL);
}

static void
eog_preferences_response_cb (GtkDialog *dlg, gint res_id, gpointer data)
{
	switch (res_id) {
		case GTK_RESPONSE_HELP:
			eog_util_show_help ("eog-prefs", NULL);
			break;
		default:
			gtk_widget_destroy (GTK_WIDGET (dlg));
			instance = NULL;
	}
}

static void
eog_preferences_dialog_set_property (GObject      *object,
				     guint         prop_id,
				     const GValue *value,
				     GParamSpec   *pspec)
{
	EogPreferencesDialog *pref_dlg = EOG_PREFERENCES_DIALOG (object);

	switch (prop_id) {
		case PROP_GCONF_CLIENT:
			pref_dlg->priv->client = g_value_get_object (value);
			break;
	}
}

static void
eog_preferences_dialog_get_property (GObject    *object,
				     guint       prop_id,
				     GValue     *value,
				     GParamSpec *pspec)
{
	EogPreferencesDialog *pref_dlg = EOG_PREFERENCES_DIALOG (object);

	switch (prop_id) {
		case PROP_GCONF_CLIENT:
			g_value_set_object (value, pref_dlg->priv->client);
			break;
	}
}

static GObject *
eog_preferences_dialog_constructor (GType type,
				    guint n_construct_properties,
				    GObjectConstructParam *construct_params)

{
	EogPreferencesDialogPrivate *priv;
	GtkWidget *dlg;
	GtkWidget *interpolate_check;
	GtkWidget *color_radio;
	GtkWidget *checkpattern_radio;
	GtkWidget *background_radio;
	GtkWidget *color_button;
	GtkWidget *upscale_check;
	GtkWidget *loop_check;
	GtkWidget *seconds_spin;
	GObject *object;
	GdkColor color;
	gchar *value;

	object = G_OBJECT_CLASS (eog_preferences_dialog_parent_class)->constructor
			(type, n_construct_properties, construct_params);

	priv = EOG_PREFERENCES_DIALOG (object)->priv;

	eog_dialog_construct (EOG_DIALOG (object),
			      "eog.glade",
			      "eog_preferences_dialog");
 
	eog_dialog_get_controls (EOG_DIALOG (object), 
			         "eog_preferences_dialog", &dlg,
			         "interpolate_check", &interpolate_check,
			         "color_radio", &color_radio,
			         "checkpattern_radio", &checkpattern_radio,
			         "background_radio", &background_radio,
			         "color_button", &color_button,
			         "upscale_check", &upscale_check,
			         "loop_check", &loop_check,
			         "seconds_spin", &seconds_spin,
			         NULL);

	g_signal_connect (G_OBJECT (dlg), 
			  "response",
			  G_CALLBACK (eog_preferences_response_cb), 
			  dlg);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (interpolate_check), 
				      gconf_client_get_bool (priv->client, 
							     EOG_CONF_VIEW_INTERPOLATE, 
							     NULL));

	g_object_set_data (G_OBJECT (interpolate_check), 
			   GCONF_OBJECT_KEY, 
			   EOG_CONF_VIEW_INTERPOLATE);

	g_signal_connect (G_OBJECT (interpolate_check), 
			  "toggled", 
			  G_CALLBACK (pd_check_toggle_cb), 
			  priv->client);

	g_object_set_data (G_OBJECT (color_radio), 
			   GCONF_OBJECT_KEY, 
			   EOG_CONF_VIEW_TRANSPARENCY);

	g_object_set_data (G_OBJECT (color_radio), 
			   GCONF_OBJECT_VALUE, 
			   "COLOR");

	g_signal_connect (G_OBJECT (color_radio), 
			  "toggled", 
			  G_CALLBACK (pd_radio_toggle_cb), 
			  priv->client);

	g_object_set_data (G_OBJECT (checkpattern_radio), 
			   GCONF_OBJECT_KEY, 
			   EOG_CONF_VIEW_TRANSPARENCY);

	g_object_set_data (G_OBJECT (checkpattern_radio), 
			   GCONF_OBJECT_VALUE, 
			   "CHECK_PATTERN");

	g_signal_connect (G_OBJECT (checkpattern_radio), 
			  "toggled", 
			  G_CALLBACK (pd_radio_toggle_cb), 
			  priv->client);

	g_object_set_data (G_OBJECT (background_radio), 
			   GCONF_OBJECT_KEY, 
			   EOG_CONF_VIEW_TRANSPARENCY);

	g_object_set_data (G_OBJECT (background_radio), 
			   GCONF_OBJECT_VALUE, 
			   "NONE");

	g_signal_connect (G_OBJECT (background_radio), 
			  "toggled", 
			  G_CALLBACK (pd_radio_toggle_cb), 
			  priv->client);

	value = gconf_client_get_string (priv->client, 
					 EOG_CONF_VIEW_TRANSPARENCY, 
					 NULL);

	if (g_ascii_strcasecmp (value, "COLOR") == 0) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (color_radio), TRUE);
	}
	else if (g_ascii_strcasecmp (value, "CHECK_PATTERN") == 0) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkpattern_radio), TRUE);
	}
	else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (background_radio), TRUE);
	}

	g_free (value);

	value = gconf_client_get_string (priv->client, 
					 EOG_CONF_VIEW_TRANS_COLOR, 
					 NULL);

	if (gdk_color_parse (value, &color)) {
		gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button),
					    &color);
	}

	g_object_set_data (G_OBJECT (color_button), 
			   GCONF_OBJECT_KEY, 
			   EOG_CONF_VIEW_TRANS_COLOR);

	g_signal_connect (G_OBJECT (color_button),
			  "color-set",
			  G_CALLBACK (pd_color_change_cb),
			  priv->client);

	g_free (value);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (upscale_check), 
				      gconf_client_get_bool (priv->client, 
							     EOG_CONF_FULLSCREEN_UPSCALE, 
							     NULL));

	g_object_set_data (G_OBJECT (upscale_check), 
			   GCONF_OBJECT_KEY, 
			   EOG_CONF_FULLSCREEN_UPSCALE);

	g_signal_connect (G_OBJECT (upscale_check), 
			  "toggled", 
			  G_CALLBACK (pd_check_toggle_cb), 
			  priv->client);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (loop_check), 
				      gconf_client_get_bool (priv->client, 
							     EOG_CONF_FULLSCREEN_LOOP, 
							     NULL));

	g_object_set_data (G_OBJECT (loop_check), 
			   GCONF_OBJECT_KEY, 
			   EOG_CONF_FULLSCREEN_LOOP);

	g_signal_connect (G_OBJECT (loop_check), 
			  "toggled", 
			  G_CALLBACK (pd_check_toggle_cb), 
			  priv->client);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (seconds_spin), 
				   gconf_client_get_int (priv->client, 
							 EOG_CONF_FULLSCREEN_SECONDS, 
							 NULL));

	g_object_set_data (G_OBJECT (seconds_spin), 
			   GCONF_OBJECT_KEY, 
			   EOG_CONF_FULLSCREEN_SECONDS);

	g_signal_connect (G_OBJECT (seconds_spin), 
			  "changed", 
			  G_CALLBACK (pd_spin_button_changed_cb), 
			  priv->client);

	return object;	
}

static void
eog_preferences_dialog_class_init (EogPreferencesDialogClass *class)
{
	GObjectClass *g_object_class = (GObjectClass *) class;

	g_object_class->constructor = eog_preferences_dialog_constructor;
	g_object_class->set_property = eog_preferences_dialog_set_property;
	g_object_class->get_property = eog_preferences_dialog_get_property;

	g_object_class_install_property (g_object_class,
					 PROP_GCONF_CLIENT,
					 g_param_spec_object ("gconf-client",
							      "GConf Client",
							      "GConf Client",
							      GCONF_TYPE_CLIENT,
							      G_PARAM_READWRITE | 
							      G_PARAM_CONSTRUCT_ONLY | 
							      G_PARAM_STATIC_NAME | 
							      G_PARAM_STATIC_NICK | 
							      G_PARAM_STATIC_BLURB));

	g_type_class_add_private (g_object_class, sizeof (EogPreferencesDialogPrivate));
}

static void
eog_preferences_dialog_init (EogPreferencesDialog *pref_dlg)
{
	pref_dlg->priv = EOG_PREFERENCES_DIALOG_GET_PRIVATE (pref_dlg);

	pref_dlg->priv->client = NULL;
}

GObject *
eog_preferences_dialog_get_instance (GtkWindow *parent, GConfClient *client)
{
	if (instance == NULL) {
		instance = g_object_new (EOG_TYPE_PREFERENCES_DIALOG, 
				 	 "parent-window", parent,
				 	 "gconf-client", client,
				 	 NULL);
	}

	return instance;
}
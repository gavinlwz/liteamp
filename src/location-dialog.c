/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "liteamp.h"
#include "location-dialog.h"
#include "util.h"

/* private-----------------------------------------------*/

struct _LocationDialog {
    GObject parent;

    GladeXML *gladexml;
    GtkWidget *dialog;		//GtkDialog
    GtkWidget *location_entry;	//GtkEntry

    GString *location;
};

struct _LocationDialogClass {
    GObjectClass parent_class;
};

static void update_data(LocationDialog * self, gboolean save)
{
    if (save) {
	g_string_assign(self->location,
			gtk_entry_get_text(GTK_ENTRY
					   (self->location_entry)));
    } else {
	gtk_entry_set_text(GTK_ENTRY(self->location_entry),
			   self->location->str);
    }
}

static void response_cb(GtkDialog * dialog, gint response,
			LocationDialog * self)
{
    if (response == GTK_RESPONSE_OK) {
	update_data(self, TRUE);
    }

    gtk_widget_destroy(GTK_WIDGET(self->dialog));
    return;
}

static gboolean delete_event_cb(GtkWidget * widget, GdkEvent * event,
				LocationDialog * self)
{
    g_object_unref(self);
    return FALSE;
}

/* public-----------------------------------------------*/

LocationDialog *location_dialog_new(GtkWindow * parent)
{
    LocationDialog *self;

    self = g_object_new(TYPE_LOCATION_DIALOG, NULL);

    self->gladexml =
	glade_xml_new(PKGDATADIR "/glade/location-dialog.glade", NULL, NULL);

    self->dialog = glade_xml_get_widget(self->gladexml, "location_dialog");
    self->location_entry =
	glade_xml_get_widget(self->gladexml, "location_entry");

    gtk_window_set_transient_for(GTK_WINDOW(self->dialog), parent);
    gtk_dialog_set_default_response(GTK_DIALOG(self->dialog),
				    GTK_RESPONSE_OK);

    g_signal_connect(self->dialog, "response",
		     G_CALLBACK(response_cb), self);
    g_signal_connect(self->dialog, "delete_event",
		     G_CALLBACK(delete_event_cb), self);

    return self;
}

void location_dialog_show(LocationDialog * self, const gchar * location)
{
    self->location = g_string_new(location);

    update_data(self, FALSE);

    gtk_widget_show_all(self->dialog);
}

const gchar *location_dialog_get_location(LocationDialog * self)
{
    return self->location->str;
}

/* boilerplates -----------------------------------------*/

LA_TYPE_BOILERPLATE(LocationDialog, location_dialog, G_TYPE_OBJECT)

/* location-dialog.c */

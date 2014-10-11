/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if ENABLE_TRAY
#include <eggtrayicon.h>
#endif

#include "liteamp.h"
#include "tray.h"
#include "util.h"

/* private-----------------------------------------------*/

struct _Tray {
    GObject parent;

#ifdef ENABLE_TRAY
    GtkWidget *icon;		//EggTrayIcon
    GtkWidget *eventbox;	//GtkEventBox
    GtkWidget *image;		//GtkImage

    GtkMenu *popup;		//GtkMenu
#endif
};

struct _TrayClass {
    GObjectClass parent_class;
};


#ifdef ENABLE_TRAY
static void on_button_press_event(GtkWidget * widget,
				  GdkEventButton * event, Tray * self)
{
    GtkWindow *window = get_window(app);

    switch (event->button) {
    case 1:
	if (GTK_WIDGET_VISIBLE(window)) {
	    if (gdk_window_get_state(GTK_WIDGET(window)->window) &
		GDK_WINDOW_STATE_ICONIFIED) {
		gtk_window_present(window);
	    } else {
		gtk_widget_hide(GTK_WIDGET(window));
	    }
	} else {
	    gtk_widget_show(GTK_WIDGET(window));
	}
	break;
    case 2:
	break;
    case 3:
	gtk_menu_popup(self->popup, NULL, NULL, NULL, NULL, 3,
		       gtk_get_current_event_time());
	break;
    }
}
#endif

/* public------------------------------------------------*/

Tray *tray_new(void)
{
    Tray *self;

    self = g_object_new(TYPE_TRAY, NULL);

#ifdef ENABLE_TRAY
    self->icon = GTK_WIDGET(egg_tray_icon_new(PACKAGE));

    self->eventbox = gtk_event_box_new();
    self->image = gtk_image_new_from_file(PKGDATADIR "/pixmaps/stop.png");

    gtk_container_add(GTK_CONTAINER(self->eventbox), self->image);
    gtk_container_add(GTK_CONTAINER(self->icon), self->eventbox);

    self->popup = GTK_MENU(lookup_action_widget(app, "/TrayPopup"));

    g_signal_connect(self->eventbox,
		     "button_press_event",
		     G_CALLBACK(on_button_press_event), self);

    gtk_widget_show_all(self->icon);
#endif

    return self;
}

void tray_set_image(Tray * self, const gchar * image)
{
#ifdef ENABLE_TRAY
    gtk_image_set_from_file(GTK_IMAGE(self->image), image);
#endif
}

/* boilerplates -----------------------------------------*/

LA_TYPE_BOILERPLATE(Tray, tray, G_TYPE_OBJECT)

/* tray.c */

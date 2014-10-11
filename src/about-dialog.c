/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "liteamp.h"
#include "about-dialog.h"

GtkWidget *about_dialog_new(void)
{
    const gchar *authors[] = {
	"Dongsu Jang <iolo@hellocity.net>",
	"Jaeman Jang <jacojang@korea.com>",
	NULL
    };
    const gchar *documenters[] = { NULL };
    gchar *translators = _("translator_credits");

    GtkWidget *dialog;
    GdkPixbuf *logo_pixbuf;
    GdkPixbuf *icon_pixbuf;

    if (!strcmp(translators, "translator_credits"))
	translators = NULL;

    logo_pixbuf =
	gdk_pixbuf_new_from_file(PKGDATADIR "/pixmaps/liteamp-logo.png", NULL);

    dialog = gnome_about_new(PACKAGE, VERSION,
			     _("(C) 2002 Dongsu Jang"),
			     _("A Light-Weight Music Player for GNOME"),
			     authors, documenters, translators,
			     logo_pixbuf);

    icon_pixbuf =
	gdk_pixbuf_new_from_file(PKGDATADIR "/pixmaps/liteamp.png", NULL);

    if (icon_pixbuf) {
	gtk_window_set_icon(GTK_WINDOW(dialog), icon_pixbuf);
	gdk_pixbuf_unref(icon_pixbuf);
    }

    return dialog;
}

void about_dialog_show(void)
{
    static GtkWidget *dialog = NULL;

    if (dialog != NULL) {
	g_assert(GTK_WIDGET_REALIZED(dialog));
	gdk_window_show(dialog->window);
	gdk_window_raise(dialog->window);
    } else {
	dialog = about_dialog_new();

	g_signal_connect(G_OBJECT(dialog),
			 "destroy",
			 G_CALLBACK(gtk_widget_destroyed), &dialog);

	gtk_widget_show_all(dialog);
    }
}

/* about-dialog.c */

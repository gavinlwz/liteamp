/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "stock.h"

static const char* items[] = {
	LA_STOCK_ADD_DIRECTORY,
	LA_STOCK_ADD_FILE,
	LA_STOCK_ADD_LOCATION,
	LA_STOCK_ADD_PLAYLIST,
	LA_STOCK_BACK,
	LA_STOCK_FORWARD,
	LA_STOCK_LITEAMP_LOGO,
	LA_STOCK_LITEAMP,
	LA_STOCK_MUTE,
	LA_STOCK_NEXT,
	LA_STOCK_PAUSE,
	LA_STOCK_PLAYLIST,
	LA_STOCK_PREVIOUS,
	LA_STOCK_REMOVE_PLAYLIST,
	LA_STOCK_REPEAT,
	LA_STOCK_SHUFFLE,
	LA_STOCK_START,
	LA_STOCK_STOP,
	LA_STOCK_VOLUME_DOWN,
	LA_STOCK_VOLUME_HI,
	LA_STOCK_VOLUME_LO,
	LA_STOCK_VOLUME_UP,
	LA_STOCK_VOLUME,
};

void stock_init(void)
{
	GtkIconFactory* factory;
	int i;

	factory = gtk_icon_factory_new();
	gtk_icon_factory_add_default(factory);

	for(i = 0; i < G_N_ELEMENTS(items); i++) {
		GtkIconSet* icon_set;
		GdkPixbuf* pixbuf;
		char* filename;

		// + 3 to skip "la-" prefix ;)
		filename = g_strconcat(PKGDATADIR "/pixmaps/", items[i] + 3, ".png", NULL);
		pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
		g_free(filename);

		icon_set = gtk_icon_set_new_from_pixbuf(pixbuf);
		gtk_icon_factory_add(factory, items[i], icon_set);
		gtk_icon_set_unref(icon_set);

		g_object_unref(G_OBJECT(pixbuf));
	}
	g_object_unref (G_OBJECT(factory));
}


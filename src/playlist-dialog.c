/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "liteamp.h"
#include "playlist-dialog.h"
#include "util.h"

/* private-----------------------------------------------*/

struct _PlaylistDialog {
    GObject parent;

    GladeXML *gladexml;
    GtkWidget *dialog;		//GtkDialog
    GtkWidget *name_entry;	//GtkEntry
    GtkWidget *uri_entry;	//GtkEntry
    GtkWidget *icon_entry;	//GtkEntry

    LaPlaylist *playlist;
};

struct _PlaylistDialogClass {
    GObjectClass parent_class;
};

static void update_data(PlaylistDialog * self, gboolean save)
{
    if (save) {
	la_playlist_set_name(self->playlist,
			     gtk_entry_get_text(GTK_ENTRY
						(self->name_entry)));
	la_playlist_set_uri(self->playlist,
			    gtk_entry_get_text(GTK_ENTRY
					       (self->uri_entry)));
	la_playlist_set_icon(self->playlist,
			     gtk_entry_get_text(GTK_ENTRY
						(self->icon_entry)));
    } else {
	gtk_entry_set_text(GTK_ENTRY(self->name_entry),
			   la_playlist_get_name(self->playlist));
	gtk_entry_set_text(GTK_ENTRY(self->uri_entry),
			   la_playlist_get_uri(self->playlist));
	gtk_entry_set_text(GTK_ENTRY(self->icon_entry),
			   la_playlist_get_icon(self->playlist));
    }
}

static void on_response(GtkDialog * dialog, gint response,
			PlaylistDialog * self)
{
    if (response == GTK_RESPONSE_OK) {
	if (self->playlist) {
	    // change properties of the playlist
	    update_data(self, TRUE);
	    sidebar_update_playlist(get_sidebar(app), self->playlist);
	} else {
	    // create a new playlist
	    self->playlist = la_playlist_new();
	    update_data(self, TRUE);
	    sidebar_add_playlist(get_sidebar(app), self->playlist);
	}
	playlist_set_playlist(get_playlist(app), self->playlist);
	playlist_write(get_playlist(app));
    }

    gtk_widget_destroy(GTK_WIDGET(self->dialog));
}

static gboolean on_delete_event(GtkWidget * widget, GdkEvent * event,
				PlaylistDialog * self)
{
    g_object_unref(self);
    return FALSE;
}

/* public-----------------------------------------------*/

PlaylistDialog *playlist_dialog_new(GtkWindow * parent)
{
    PlaylistDialog *self;

    self = g_object_new(TYPE_PLAYLIST_DIALOG, NULL);

    self->gladexml =
	glade_xml_new(PKGDATADIR "/glade/playlist-dialog.glade", NULL, NULL);

    self->dialog = glade_xml_get_widget(self->gladexml, "playlist_dialog");
    self->name_entry = glade_xml_get_widget(self->gladexml, "name_entry");
    self->uri_entry = glade_xml_get_widget(self->gladexml, "uri_entry");
    //FIXME:why this doesn't work?
    //self->icon_entry = glade_xml_get_widget(self->gladexml, "icon_entry");
    self->icon_entry =
	GTK_WIDGET(gnome_pixmap_entry_gtk_entry
		   (GNOME_PIXMAP_ENTRY
		    (glade_xml_get_widget
		     (self->gladexml, "icon_pixmapentry"))));

    gtk_window_set_transient_for(GTK_WINDOW(self->dialog), parent);
    gtk_dialog_set_default_response(GTK_DIALOG(self->dialog),
				    GTK_RESPONSE_OK);

    g_signal_connect(self->dialog, "response",
		     G_CALLBACK(on_response), self);
    g_signal_connect(self->dialog, "delete-event",
		     G_CALLBACK(on_delete_event), self);

    return self;
}

void playlist_dialog_show(PlaylistDialog * self, LaPlaylist * playlist)
{
    static int playlist_count = 0;
    gchar name[PATH_MAX];
    gchar uri[PATH_MAX];
    gchar icon[PATH_MAX];

    self->playlist = playlist;

    if (self->playlist) {
	update_data(self, FALSE);
    } else {
	// set default attributes for new item
	g_snprintf(name, sizeof(name), _("Playlist-%d"), ++playlist_count);
	g_snprintf(uri, sizeof(uri), "%s/.liteamp/playlist-%s.pls",
		   g_get_home_dir(), la_timestamp());
	g_strlcpy(icon, PKGDATADIR "pixmaps/playlist.png", sizeof(icon));
	gtk_entry_set_text(GTK_ENTRY(self->name_entry), name);
	gtk_entry_set_text(GTK_ENTRY(self->uri_entry), uri);
	gtk_entry_set_text(GTK_ENTRY(self->icon_entry), icon);
    }

    gtk_widget_show_all(GTK_WIDGET(self->dialog));
}

LaPlaylist *playlist_dialog_get_playlist(PlaylistDialog * self)
{
    return self->playlist;
}

/* boilerplates -----------------------------------------*/

LA_TYPE_BOILERPLATE(PlaylistDialog, playlist_dialog, G_TYPE_OBJECT)

/* playlist-dialog.c */

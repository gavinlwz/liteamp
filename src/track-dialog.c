/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "liteamp.h"
#include "track-dialog.h"
#include "util.h"

/* private-----------------------------------------------*/

struct _TrackDialog {
    GObject parent;

    GladeXML *gladexml;
    GtkWidget *dialog;		//GtkDialog
    GtkWidget *title_entry;	//GtkEntry
    GtkWidget *artist_entry;	//GtkEntry
    GtkWidget *album_entry;	//GtkEntry
    GtkWidget *year_entry;	//GtkEntry
    GtkWidget *track_entry;	//GtkEntry
    GtkWidget *genre_entry;	//GtkEntry
    GtkWidget *rating_hscale;	//GtkHScale
    GtkWidget *comment_entry;	//GtkEntry
    GtkWidget *title_checkbtn;	//GtkCheckButton
    GtkWidget *artist_checkbtn;	//GtkCheckButton
    GtkWidget *album_checkbtn;	//GtkCheckButton
    GtkWidget *year_checkbtn;	//GtkCheckButton
    GtkWidget *track_checkbtn;	//GtkCheckButton
    GtkWidget *genre_checkbtn;	//GtkCheckButton
    GtkWidget *rating_checkbtn;	//GtkCheckButton
    GtkWidget *comment_checkbtn;	//GtkCheckButton
    GtkWidget *location_value_label;	//GtkLabel
    GtkWidget *quality_value_label;	//GtkLabel
    GtkWidget *time_value_label;	//GtkLabel
    GtkWidget *size_value_label;	//GtkLabel
    GtkWidget *date_value_label;	//GtkLabel

    LaTrack *track;
    GSList *tracks;
};

struct _TrackDialogClass {
    GObjectClass parent_class;
};

static void update_data_to_track(TrackDialog * self, LaTrack * track)
{
    if (LA_GET_TOGGLE_BUTTON(self->title_checkbtn)) {
	la_track_set_title(track,
			   gtk_entry_get_text(GTK_ENTRY
					      (self->title_entry)));
    }
    if (LA_GET_TOGGLE_BUTTON(self->artist_checkbtn)) {
	la_track_set_artist(track,
			    gtk_entry_get_text(GTK_ENTRY
					       (self->artist_entry)));
    }
    if (LA_GET_TOGGLE_BUTTON(self->album_checkbtn)) {
	la_track_set_album(track,
			   gtk_entry_get_text(GTK_ENTRY
					      (self->album_entry)));
    }
    if (LA_GET_TOGGLE_BUTTON(self->year_checkbtn)) {
	la_track_set_year(track,
			  gtk_entry_get_text(GTK_ENTRY(self->year_entry)));
    }
    if (LA_GET_TOGGLE_BUTTON(self->track_checkbtn)) {
	la_track_set_track(track,
			   gtk_entry_get_text(GTK_ENTRY
					      (self->track_entry)));
    }
    if (LA_GET_TOGGLE_BUTTON(self->genre_checkbtn)) {
	la_track_set_genre(track,
			   gtk_entry_get_text(GTK_ENTRY
					      (self->genre_entry)));
    }
    if (LA_GET_TOGGLE_BUTTON(self->rating_checkbtn)) {
	la_track_set_rating(track,
			    gtk_range_get_value(GTK_RANGE
						(self->rating_hscale)));
    }
    if (LA_GET_TOGGLE_BUTTON(self->comment_checkbtn)) {
	la_track_set_comment(track,
			     gtk_entry_get_text(GTK_ENTRY
						(self->comment_entry)));
    }
    tag_write(track);
}

static void update_data_from_track(TrackDialog * self, LaTrack * track)
{
    gtk_entry_set_text(GTK_ENTRY(self->title_entry),
		       la_track_get_title(track));
    gtk_entry_set_text(GTK_ENTRY(self->genre_entry),
		       la_track_get_genre(track));
    gtk_entry_set_text(GTK_ENTRY(self->artist_entry),
		       la_track_get_artist(track));
    gtk_entry_set_text(GTK_ENTRY(self->album_entry),
		       la_track_get_album(track));
    gtk_entry_set_text(GTK_ENTRY(self->year_entry),
		       la_track_get_year(track));
    gtk_entry_set_text(GTK_ENTRY(self->track_entry),
		       la_track_get_track(track));
    gtk_entry_set_text(GTK_ENTRY(self->comment_entry),
		       la_track_get_comment(track));
    gtk_range_set_value(GTK_RANGE(self->rating_hscale),
			la_track_get_rating(track));
    gtk_label_set_text(GTK_LABEL(self->location_value_label),
		       la_track_get_uri(track));
    gtk_label_set_text(GTK_LABEL(self->quality_value_label),
		       la_track_get_quality_str(track));
    gtk_label_set_text(GTK_LABEL(self->time_value_label),
		       la_track_get_time_str(track));
    gtk_label_set_text(GTK_LABEL(self->size_value_label),
		       la_track_get_size_str(track));
    gtk_label_set_text(GTK_LABEL(self->date_value_label),
		       la_track_get_date_str(track));
}

static void update_data(TrackDialog * self, gboolean save)
{
    GSList *tracks;
    LaTrack *track;

    tracks = self->tracks;

    if (save) {
	while (tracks) {
	    track = tracks->data;
	    update_data_to_track(self, track);
	    tracks = g_slist_next(tracks);
	}
    } else {
	if (g_slist_length(self->tracks) == 1) {
	    gchar *title;
	    title = g_strdup_printf("%s - %s",
				    gtk_window_get_title(GTK_WINDOW
							 (self->dialog)),
				    la_track_get_title(self->track));
	    gtk_window_set_title(GTK_WINDOW(self->dialog), title);
	    g_free(title);
	}

	update_data_from_track(self, self->track);
    }
}

static void response_cb(GtkDialog * dialog, gint response,
			TrackDialog * self)
{
    GSList *tracks;
    LaTrack *track;

    switch (response) {
    case 1:			// up
	/*
	g_assert(!g_slist_next(self->tracks));
	update_data(self, TRUE);
	self->track = playlist_get_previous_track(get_playlist(app));
	update_data(self, FALSE);
	*/
	return;

    case 2:			// down
	/*
	g_assert(!g_slist_next(self->tracks));
	update_data(self, TRUE);
	track = playlist_get_next_track(get_playlist(app));
	update_data(self, FALSE);
	*/
	return;

    case 3:			// clear
	tracks = self->tracks;
	while (tracks) {
	    track = tracks->data;

	    la_track_set_title(track, "");
	    la_track_set_artist(track, "");
	    la_track_set_album(track, "");
	    la_track_set_year(track, "");
	    la_track_set_track(track, "");
	    la_track_set_genre(track, "");
	    la_track_set_rating(track, 0);
	    la_track_set_comment(track, "");

	    tag_remove(track);

	    tag_read(track);

	    tracks = g_slist_next(tracks);
	}
	update_data(self, FALSE);
	return;

    case GTK_RESPONSE_CLOSE:
	update_data(self, TRUE);
	//@@la_iter_changed(liteamp->library->tracks, self->track);
	break;
    }

    gtk_widget_destroy(GTK_WIDGET(self->dialog));
    return;
}

static gboolean delete_event_cb(GtkWidget * widget, GdkEvent * event,
				TrackDialog * self)
{
    g_slist_free(self->tracks);
    g_object_unref(self);
    return FALSE;
}

/* public-----------------------------------------------*/

TrackDialog *track_dialog_new(GtkWindow * parent)
{
    TrackDialog *self;

    self = g_object_new(TYPE_TRACK_DIALOG, NULL);

    self->gladexml =
	glade_xml_new(PKGDATADIR "/glade/track-dialog.glade", NULL, NULL);

    self->dialog = glade_xml_get_widget(self->gladexml, "track_dialog");

    self->title_entry =
	glade_xml_get_widget(self->gladexml, "title_entry");
    self->artist_entry =
	glade_xml_get_widget(self->gladexml, "artist_entry");
    self->album_entry =
	glade_xml_get_widget(self->gladexml, "album_entry");
    self->year_entry = glade_xml_get_widget(self->gladexml, "year_entry");
    self->track_entry =
	glade_xml_get_widget(self->gladexml, "track_entry");
    self->genre_entry =
	glade_xml_get_widget(self->gladexml, "genre_entry");
    self->rating_hscale =
	glade_xml_get_widget(self->gladexml, "rating_hscale");
    self->comment_entry =
	glade_xml_get_widget(self->gladexml, "comment_entry");

    self->title_checkbtn =
	glade_xml_get_widget(self->gladexml, "title_checkbtn");
    self->artist_checkbtn =
	glade_xml_get_widget(self->gladexml, "artist_checkbtn");
    self->album_checkbtn =
	glade_xml_get_widget(self->gladexml, "album_checkbtn");
    self->year_checkbtn =
	glade_xml_get_widget(self->gladexml, "year_checkbtn");
    self->track_checkbtn =
	glade_xml_get_widget(self->gladexml, "track_checkbtn");
    self->genre_checkbtn =
	glade_xml_get_widget(self->gladexml, "genre_checkbtn");
    self->rating_checkbtn =
	glade_xml_get_widget(self->gladexml, "rating_checkbtn");
    self->comment_checkbtn =
	glade_xml_get_widget(self->gladexml, "comment_checkbtn");

    self->location_value_label =
	glade_xml_get_widget(self->gladexml, "location_value_label");
    self->quality_value_label =
	glade_xml_get_widget(self->gladexml, "quality_value_label");
    self->time_value_label =
	glade_xml_get_widget(self->gladexml, "time_value_label");
    self->size_value_label =
	glade_xml_get_widget(self->gladexml, "size_value_label");
    self->date_value_label =
	glade_xml_get_widget(self->gladexml, "date_value_label");

    gtk_window_set_transient_for(GTK_WINDOW(self->dialog), parent);
    gtk_dialog_set_default_response(GTK_DIALOG(self->dialog),
				    GTK_RESPONSE_CLOSE);

    g_signal_connect(self->dialog, "response",
		     G_CALLBACK(response_cb), self);
    g_signal_connect(self->dialog, "delete_event",
		     G_CALLBACK(delete_event_cb), self);

    return self;
}

void track_dialog_show(TrackDialog * self, GSList * tracks)
{
    gint count;
    gchar *title;
    GtkWidget *widget;

    g_return_if_fail(tracks && tracks->data);

    self->tracks = tracks;
    self->track = tracks->data;

    count = g_slist_length(tracks);
    if (count > 1) {
	// for multiple selection,
	// check off all fields by default.
	LA_SET_TOGGLE_BUTTON(self->title_checkbtn, FALSE);
	LA_SET_TOGGLE_BUTTON(self->artist_checkbtn, FALSE);
	LA_SET_TOGGLE_BUTTON(self->album_checkbtn, FALSE);
	LA_SET_TOGGLE_BUTTON(self->year_checkbtn, FALSE);
	LA_SET_TOGGLE_BUTTON(self->track_checkbtn, FALSE);
	LA_SET_TOGGLE_BUTTON(self->genre_checkbtn, FALSE);
	LA_SET_TOGGLE_BUTTON(self->rating_checkbtn, FALSE);
	LA_SET_TOGGLE_BUTTON(self->comment_checkbtn, FALSE);

	widget = glade_xml_get_widget(self->gladexml, "up_button");
	gtk_widget_set_sensitive(widget, FALSE);

	widget = glade_xml_get_widget(self->gladexml, "down_button");
	gtk_widget_set_sensitive(widget, FALSE);

	title = g_strdup_printf(_("%s - %d Track(s)"),
				gtk_window_get_title(GTK_WINDOW
						     (self->dialog)),
				count);
	gtk_window_set_title(GTK_WINDOW(self->dialog), title);
	g_free(title);
    }

    update_data(self, FALSE);

    gtk_widget_show_all(self->dialog);
}

LaTrack *track_dialog_get_track(TrackDialog * self)
{
    return self->track;
}

/* boilerplates -----------------------------------------*/

LA_TYPE_BOILERPLATE(TrackDialog, track_dialog, G_TYPE_OBJECT)

/* track-dialog.c */

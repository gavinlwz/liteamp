/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include "la-track-column.h"
#include "util.h"
#include "liteamp.h"
#include "player.h"

/* private-----------------------------------------------*/

static const GEnumValue self_values[] = {
    {LA_TRACK_COLUMN_URI, "URI", "uri"},
    {LA_TRACK_COLUMN_TITLE, "Title", "title"},
    {LA_TRACK_COLUMN_GENRE, "Genre", "genre"},
    {LA_TRACK_COLUMN_ARTIST, "Artist", "artist"},
    {LA_TRACK_COLUMN_ALBUM, "Album", "album"},
    {LA_TRACK_COLUMN_TRACK, "Track", "track"},
    {LA_TRACK_COLUMN_YEAR, "Year", "year"},
    {LA_TRACK_COLUMN_COMMENT, "Comment", "comment"},
    {LA_TRACK_COLUMN_BITRATE, "Bitrate", "bitrate"},
    {LA_TRACK_COLUMN_SAMPLERATE, "Samplerate", "samplerate"},
    {LA_TRACK_COLUMN_FRAMES, "Frames", "frames"},
    {LA_TRACK_COLUMN_CHANNELS, "Channels", "channels"},
    {LA_TRACK_COLUMN_SECONDS, "Seconds", "seconds"},
    {LA_TRACK_COLUMN_BYTES, "Bytes", "bytes"},
    {LA_TRACK_COLUMN_ICON, "Icon", "icon"},
    {LA_TRACK_COLUMN_RATING, "Rating", "rating"},
    {LA_TRACK_COLUMN_CHECKED, "Checked", "checked"},
    {LA_TRACK_COLUMN_TIMESTAMP, "Timestamp", "timestamp"},
    {LA_TRACK_COLUMN_TIME_STR, "TimeStr", "time-str"},
    {LA_TRACK_COLUMN_SIZE_STR, "SizeStr", "size-str"},
    {LA_TRACK_COLUMN_DATE_STR, "DateStr", "date-str"},
    {LA_TRACK_COLUMN_RATING_STR, "RatingStr", "rating-str"},
    {LA_TRACK_COLUMN_QUALITY_STR, "QualityStr", "quality-str"},
    {LA_TRACK_COLUMN_SUMMARY_STR, "SummaryStr", "summary-str"},
    {LA_TRACK_COLUMN_SELF, "Self", "self"},
    {0, NULL, NULL}
};

static const gchar *column_titles[] = {
    N_("URI"),
    N_("Title"),
    N_("Genre"),
    N_("Artist"),
    N_("Album"),
    N_("Track"),
    N_("Year"),
    N_("Comment"),
    N_("Bitrate"),
    N_("Samplerate"),
    N_("Frames"),
    N_("Channels"),
    N_("Seconds"),
    N_("Bytes"),
    N_(" "),			//icon
    N_("Rating"),
    N_(" "),			//checked
    N_("Timestamp"),
    N_("Time"),
    N_("Size"),
    N_("Date"),
    N_("Rating"),
    N_("Quality"),
    N_("Summary"),
    N_(" "),			//self
};

/* public------------------------------------------------*/

const gchar *la_track_column_get_name(LaTrackColumn index)
{
    g_return_val_if_fail(index >= 0 && index < LA_N_TRACK_COLUMNS, NULL);
    return _(self_values[index].value_nick);
}

LaTrackColumn la_track_column_from_name(const gchar * name)
{
    LaTrackColumn index;
    for (index = 0; index < LA_N_TRACK_COLUMNS; index++) {
	if (!strcmp(name, self_values[index].value_nick)
	    || !strcmp(name, self_values[index].value_name))
	    return index;
    }
    return -1;
}

const gchar *la_track_column_get_column_title(LaTrackColumn index)
{
    g_return_val_if_fail(index >= 0 && index < LA_N_TRACK_COLUMNS, NULL);
    return _(column_titles[index]);

}

GType la_track_column_get_column_type(LaTrackColumn index)
{
    switch (index) {
    case LA_TRACK_COLUMN_BITRATE:
    case LA_TRACK_COLUMN_SAMPLERATE:
    case LA_TRACK_COLUMN_FRAMES:
    case LA_TRACK_COLUMN_CHANNELS:
    case LA_TRACK_COLUMN_SECONDS:
    case LA_TRACK_COLUMN_BYTES:
    case LA_TRACK_COLUMN_RATING:
    case LA_TRACK_COLUMN_TIMESTAMP:
	return G_TYPE_ULONG;

    case LA_TRACK_COLUMN_ICON:
	return GDK_TYPE_PIXBUF;

    case LA_TRACK_COLUMN_CHECKED:
	return G_TYPE_BOOLEAN;

    case LA_TRACK_COLUMN_SELF:
	return G_TYPE_INT;	//G_TYPE_POINTER;

    default:
	return G_TYPE_STRING;
    }
}

GtkTreeViewColumn *la_track_column_new_tree_view_column(LaTrackColumn
							index)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    const gchar *title;

    column = gtk_tree_view_column_new();

    title = la_track_column_get_column_title(index);
    gtk_tree_view_column_set_title(column, _(title));

    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_sort_column_id(column, index);

    switch (index) {
    case LA_TRACK_COLUMN_ICON:
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer,
					   "pixbuf", LA_TRACK_COLUMN_ICON);
	gtk_tree_view_column_set_min_width(column, 20);
	gtk_tree_view_column_set_max_width(column, 20);
	gtk_tree_view_column_set_fixed_width(column, 20);
	break;

    case LA_TRACK_COLUMN_CHECKED:
	renderer = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer,
					   "active",
					   LA_TRACK_COLUMN_CHECKED);
	gtk_tree_view_column_set_min_width(column, 20);
	gtk_tree_view_column_set_max_width(column, 20);
	gtk_tree_view_column_set_fixed_width(column, 20);
	break;

    default:
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer,
					   "text", index);
	gtk_tree_view_column_set_min_width(column, 50);
	gtk_tree_view_column_set_fixed_width(column, 50);
	gtk_tree_view_column_set_resizable(column, TRUE);
	break;
    }

    return column;
}

gint la_track_column_compare_column(LaTrackColumn index, LaTrack * track1,
				    LaTrack * track2)
{
    const gchar *str1;
    const gchar *str2;

    switch (index) {
    case LA_TRACK_COLUMN_URI:
	str1 = la_track_get_uri(track1);
	str2 = la_track_get_uri(track2);
	break;

    case LA_TRACK_COLUMN_TITLE:
	str1 = la_track_get_title(track1);
	str2 = la_track_get_title(track2);
	break;

    case LA_TRACK_COLUMN_GENRE:
	str1 = la_track_get_genre(track1);
	str2 = la_track_get_genre(track2);
	break;

    case LA_TRACK_COLUMN_ARTIST:
	str1 = la_track_get_artist(track1);
	str2 = la_track_get_artist(track2);
	break;

    case LA_TRACK_COLUMN_ALBUM:
	str1 = la_track_get_album(track1);
	str2 = la_track_get_album(track2);
	break;

    case LA_TRACK_COLUMN_TRACK:
	str1 = la_track_get_track(track1);
	str2 = la_track_get_track(track2);
	break;

    case LA_TRACK_COLUMN_YEAR:
	str1 = la_track_get_year(track1);
	str2 = la_track_get_year(track2);
	break;

    case LA_TRACK_COLUMN_COMMENT:
	str1 = la_track_get_comment(track1);
	str2 = la_track_get_comment(track2);
	break;

    case LA_TRACK_COLUMN_BITRATE:
	return la_track_get_bitrate(track1) - la_track_get_bitrate(track2);

    case LA_TRACK_COLUMN_SAMPLERATE:
	return la_track_get_samplerate(track1) -
	    la_track_get_samplerate(track2);

    case LA_TRACK_COLUMN_FRAMES:
	return la_track_get_frames(track1) - la_track_get_frames(track2);

    case LA_TRACK_COLUMN_CHANNELS:
	return la_track_get_channels(track1) -
	    la_track_get_channels(track2);

    case LA_TRACK_COLUMN_SECONDS:
    case LA_TRACK_COLUMN_TIME_STR:
	return la_track_get_seconds(track1) - la_track_get_seconds(track2);

    case LA_TRACK_COLUMN_BYTES:
    case LA_TRACK_COLUMN_SIZE_STR:
	return la_track_get_bytes(track1) - la_track_get_bytes(track2);

    case LA_TRACK_COLUMN_RATING:
    case LA_TRACK_COLUMN_RATING_STR:
	return la_track_get_rating(track1) - la_track_get_rating(track2);

    case LA_TRACK_COLUMN_CHECKED:
	return la_track_get_checked(track1) - la_track_get_checked(track2);

    case LA_TRACK_COLUMN_TIMESTAMP:
    case LA_TRACK_COLUMN_DATE_STR:
	return la_track_get_timestamp(track1) -
	    la_track_get_timestamp(track2);

    case LA_TRACK_COLUMN_QUALITY_STR:
	str1 = la_track_get_quality_str(track1);
	str2 = la_track_get_quality_str(track2);
	break;

    case LA_TRACK_COLUMN_SUMMARY_STR:
	str1 = la_track_get_summary_str(track1);
	str2 = la_track_get_summary_str(track2);
	break;

    default:
	return 0;
    }

    if (LA_STR_IS_EMPTY(str1))
	return -1;
    if (LA_STR_IS_EMPTY(str2))
	return 1;
    return strcmp(str1, str2);
}

void la_track_column_get_column_value(LaTrackColumn index,
				      LaTrack * track, GValue * value)
{
    g_value_init(value, la_track_column_get_column_type(index));
    if (!track)
	return;

    switch (index) {
    case LA_TRACK_COLUMN_URI:
	g_value_set_string(value, la_track_get_uri(track));
	break;

    case LA_TRACK_COLUMN_TITLE:
	g_value_set_string(value, la_track_get_title(track));
	break;

    case LA_TRACK_COLUMN_GENRE:
	g_value_set_string(value, la_track_get_genre(track));
	break;

    case LA_TRACK_COLUMN_ARTIST:
	g_value_set_string(value, la_track_get_artist(track));
	break;

    case LA_TRACK_COLUMN_ALBUM:
	g_value_set_string(value, la_track_get_album(track));
	break;

    case LA_TRACK_COLUMN_TRACK:
	g_value_set_string(value, la_track_get_track(track));
	break;

    case LA_TRACK_COLUMN_YEAR:
	g_value_set_string(value, la_track_get_year(track));
	break;

    case LA_TRACK_COLUMN_COMMENT:
	g_value_set_string(value, la_track_get_comment(track));
	break;

    case LA_TRACK_COLUMN_BITRATE:
	g_value_set_ulong(value, la_track_get_bitrate(track));
	break;

    case LA_TRACK_COLUMN_SAMPLERATE:
	g_value_set_ulong(value, la_track_get_samplerate(track));
	break;

    case LA_TRACK_COLUMN_FRAMES:
	g_value_set_ulong(value, la_track_get_frames(track));
	break;

    case LA_TRACK_COLUMN_CHANNELS:
	g_value_set_ulong(value, la_track_get_channels(track));
	break;

    case LA_TRACK_COLUMN_SECONDS:
	g_value_set_ulong(value, la_track_get_seconds(track));
	break;

    case LA_TRACK_COLUMN_BYTES:
	g_value_set_ulong(value, la_track_get_bytes(track));
	break;

    case LA_TRACK_COLUMN_ICON:
	{
	    if (track == player_get_track(get_player(app))) {
		g_value_set_object(value, player_get_status_icon(get_player(app)));

	    } else {
		g_value_set_object(value, NULL);
	    }
	}
	/*
	g_value_set_object(value,
			   la_pixbuf_new(la_track_get_icon(track),
					 12, 12));
					 */
	break;

    case LA_TRACK_COLUMN_RATING:
	g_value_set_ulong(value, la_track_get_rating(track));
	break;

    case LA_TRACK_COLUMN_CHECKED:
	g_value_set_boolean(value, la_track_get_checked(track));
	break;

    case LA_TRACK_COLUMN_TIMESTAMP:
	g_value_set_ulong(value, la_track_get_timestamp(track));
	break;

    case LA_TRACK_COLUMN_TIME_STR:
	g_value_set_string(value, la_track_get_time_str(track));
	break;

    case LA_TRACK_COLUMN_SIZE_STR:
	g_value_set_string(value, la_track_get_size_str(track));
	break;

    case LA_TRACK_COLUMN_DATE_STR:
	g_value_set_string(value, la_track_get_date_str(track));
	break;

    case LA_TRACK_COLUMN_RATING_STR:
	g_value_set_string(value, la_track_get_rating_str(track));
	break;

    case LA_TRACK_COLUMN_QUALITY_STR:
	g_value_set_string(value, la_track_get_quality_str(track));
	break;

    case LA_TRACK_COLUMN_SUMMARY_STR:
	g_value_set_string(value, la_track_get_summary_str(track));
	break;

    case LA_TRACK_COLUMN_SELF:
	g_value_set_int(value, GPOINTER_TO_INT(track));
	break;

    default:
	g_value_reset(value);
	break;
    }
}

/* boilerplates -----------------------------------------*/

GType la_track_column_get_type(void)
{
    static GType self_type = 0;
    if (self_type == 0) {
	self_type = g_enum_register_static("LaTrackColumn", self_values);
    }
    return self_type;
}

/* la-track-column.c */

/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __LA_TRACK_COLUMN_H__
#define __LA_TRACK_COLUMN_H__

#include <gtk/gtk.h>
#include "la-track.h"

G_BEGIN_DECLS
#define LA_TYPE_TRACK_COLUMN		(la_track_column_get_type())
typedef enum _LaTrackColumn LaTrackColumn;

enum _LaTrackColumn {
    LA_TRACK_COLUMN_URI,
    LA_TRACK_COLUMN_TITLE,
    LA_TRACK_COLUMN_GENRE,
    LA_TRACK_COLUMN_ARTIST,
    LA_TRACK_COLUMN_ALBUM,
    LA_TRACK_COLUMN_TRACK,
    LA_TRACK_COLUMN_YEAR,
    LA_TRACK_COLUMN_COMMENT,
    LA_TRACK_COLUMN_BITRATE,
    LA_TRACK_COLUMN_SAMPLERATE,
    LA_TRACK_COLUMN_FRAMES,
    LA_TRACK_COLUMN_CHANNELS,
    LA_TRACK_COLUMN_SECONDS,
    LA_TRACK_COLUMN_BYTES,
    LA_TRACK_COLUMN_ICON,
    LA_TRACK_COLUMN_RATING,
    LA_TRACK_COLUMN_CHECKED,
    LA_TRACK_COLUMN_TIMESTAMP,
    LA_TRACK_COLUMN_TIME_STR,
    LA_TRACK_COLUMN_SIZE_STR,
    LA_TRACK_COLUMN_DATE_STR,
    LA_TRACK_COLUMN_RATING_STR,
    LA_TRACK_COLUMN_QUALITY_STR,
    LA_TRACK_COLUMN_SUMMARY_STR,
    LA_TRACK_COLUMN_SELF,
    LA_N_TRACK_COLUMNS,
};

GType la_track_column_get_type(void) G_GNUC_CONST;

const gchar *la_track_column_get_name(LaTrackColumn index);
LaTrackColumn la_track_column_from_name(const gchar * name);

const gchar *la_track_column_get_column_title(LaTrackColumn index);
GType la_track_column_get_column_type(LaTrackColumn index);
GtkTreeViewColumn *la_track_column_new_tree_view_column(LaTrackColumn
							index);
gint la_track_column_compare_column(LaTrackColumn index, LaTrack * track1,
				    LaTrack * track2);
void la_track_column_get_column_value(LaTrackColumn index,
				      LaTrack * track, GValue * value);

#endif				/* __LA_TRACK_COLUMN_H__ */

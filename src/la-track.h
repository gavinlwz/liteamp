/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __LA_TRACK_H__
#define __LA_TRACK_H__

#include <glib-object.h>
#include <libgnomevfs/gnome-vfs.h>

G_BEGIN_DECLS
#define LA_TYPE_TRACK			(la_track_get_type())
typedef struct _LaTrack LaTrack;

GType la_track_get_type(void) G_GNUC_CONST;
LaTrack *la_track_new(const gchar * uri);

LaTrack *la_track_ref(LaTrack * self);
void la_track_unref(LaTrack * self);

// setters
void la_track_set_title(LaTrack * self, const gchar * title);
void la_track_set_genre(LaTrack * self, const gchar * genre);
void la_track_set_artist(LaTrack * self, const gchar * artist);
void la_track_set_album(LaTrack * self, const gchar * album);
void la_track_set_track(LaTrack * self, const gchar * track);
void la_track_set_year(LaTrack * self, const gchar * year);
void la_track_set_comment(LaTrack * self, const gchar * comment);
void la_track_set_bitrate(LaTrack * self, gint bitrate);
void la_track_set_samplerate(LaTrack * self, gint samplerate);
void la_track_set_frames(LaTrack * self, gint frames);
void la_track_set_channels(LaTrack * self, gint channels);
void la_track_set_seconds(LaTrack * self, gint seconds);
void la_track_set_bytes(LaTrack * self, gint bytes);
void la_track_set_icon(LaTrack * self, const gchar * icon);
void la_track_set_rating(LaTrack * self, gint rating);
void la_track_set_checked(LaTrack * self, gboolean checked);
void la_track_set_timestamp(LaTrack * self, gulong timestamp);

// getters
const gchar *la_track_get_uri(LaTrack * self);
const gchar *la_track_get_title(LaTrack * self);
const gchar *la_track_get_genre(LaTrack * self);
const gchar *la_track_get_artist(LaTrack * self);
const gchar *la_track_get_album(LaTrack * self);
const gchar *la_track_get_track(LaTrack * self);
const gchar *la_track_get_year(LaTrack * self);
const gchar *la_track_get_comment(LaTrack * self);
gint la_track_get_bitrate(LaTrack * self);
gint la_track_get_samplerate(LaTrack * self);
gint la_track_get_frames(LaTrack * self);
gint la_track_get_channels(LaTrack * self);
gint la_track_get_seconds(LaTrack * self);
gint la_track_get_bytes(LaTrack * self);
const gchar *la_track_get_icon(LaTrack * self);
gint la_track_get_rating(LaTrack * self);
gboolean la_track_get_checked(LaTrack * self);
gulong la_track_get_timestamp(LaTrack * self);

// convenient methods
gboolean la_track_set_quality(LaTrack * self, gint bitrate,
			      gint samplerate, gint frames,
			      gint channels, gint seconds);
const gchar *la_track_get_time_str(LaTrack * self);
const gchar *la_track_get_size_str(LaTrack * self);
const gchar *la_track_get_date_str(LaTrack * self);
const gchar *la_track_get_rating_str(LaTrack * self);
const gchar *la_track_get_quality_str(LaTrack * self);
const gchar *la_track_get_summary_str(LaTrack * self);
GQuark la_track_get_quark(LaTrack * self);
GnomeVFSURI *la_track_get_vfs_uri(LaTrack * self);
GnomeVFSFileInfo *la_track_get_vfs_info(LaTrack * self);

G_END_DECLS
#endif				/* __LA_TRACK_H__ */

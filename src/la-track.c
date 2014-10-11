/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include "la-track.h"
#include "util.h"

/* private------------------------------------------------*/

struct _LaTrack {
    gchar *uri;			//PK

    gchar *title;
    gchar *genre;
    gchar *artist;
    gchar *album;
    gchar *track;
    gchar *year;
    gchar *comment;
    gint bitrate;
    gint samplerate;
    gint frames;
    gint channels;
    gint seconds;
    gint bytes;
    gchar *icon;
    gint rating;
    gboolean checked;
    gulong timestamp;

    gchar *quality_str;
    gchar *time_str;
    gchar *size_str;
    gchar *date_str;
    gchar *rating_str;
    gchar *summary_str;
    GQuark quark;
    GnomeVFSURI *vfs_uri;
    GnomeVFSFileInfo *vfs_info;
    gint refcount;
};

static const gchar *QUALITY_FORMAT = N_("%.1fKBPS %.1fKHz %d / %d");
static const gchar *SUMMARY_FORMAT =
N_("Bit Rate:%d Bit/Sec\nSample Rate:%d Hz\nFrames:%d\nChannels:%d");
static const gchar RATING_CHAR = '*';

static void invalidate_quality_str(LaTrack * self)
{
    if (self->quality_str) {
	g_free(self->quality_str);
	self->quality_str = NULL;
    }
    if (self->summary_str) {
	g_free(self->summary_str);
	self->summary_str = NULL;
    }
}

/* public-------------------------------------------------*/

LaTrack *la_track_new(const gchar * uri)
{
    LaTrack *self;
    self = g_new0(LaTrack, 1);

    self->uri = g_strdup(uri);
    self->quark = g_quark_from_string(uri);

    self->refcount = 1;
    return self;
}

LaTrack *la_track_ref(LaTrack * self)
{
    ++self->refcount;
    return self;
}

void la_track_unref(LaTrack * self)
{
    if (--self->refcount > 0)
	return;

    if (self->uri)
	g_free(self->uri);

    if (self->title)
	g_free(self->title);
    if (self->genre)
	g_free(self->genre);
    if (self->artist)
	g_free(self->artist);
    if (self->album)
	g_free(self->album);
    if (self->track)
	g_free(self->track);
    if (self->year)
	g_free(self->year);
    if (self->comment)
	g_free(self->comment);
    if (self->icon)
	g_free(self->icon);

    if (self->size_str)
	g_free(self->size_str);
    if (self->time_str)
	g_free(self->time_str);
    if (self->date_str)
	g_free(self->date_str);
    if (self->rating_str)
	g_free(self->rating_str);
    if (self->quality_str)
	g_free(self->quality_str);
    if (self->summary_str)
	g_free(self->summary_str);

    if (self->vfs_uri)
	gnome_vfs_uri_unref(self->vfs_uri);
    if (self->vfs_info)
	gnome_vfs_file_info_unref(self->vfs_info);

    g_free(self);
}

void la_track_set_title(LaTrack * self, const gchar * title)
{
    if (self->title)
	g_free(self->title);
    self->title = g_strdup(title);
}

void la_track_set_genre(LaTrack * self, const gchar * genre)
{
    if (self->genre)
	g_free(self->genre);
    self->genre = g_strdup(genre);
}

void la_track_set_artist(LaTrack * self, const gchar * artist)
{
    if (self->artist)
	g_free(self->artist);
    self->artist = g_strdup(artist);
}

void la_track_set_album(LaTrack * self, const gchar * album)
{
    if (self->album)
	g_free(self->album);
    self->album = g_strdup(album);
}

void la_track_set_track(LaTrack * self, const gchar * track)
{
    if (self->track)
	g_free(self->track);
    self->track = g_strdup(track);
}

void la_track_set_year(LaTrack * self, const gchar * year)
{
    if (self->year)
	g_free(self->year);
    self->year = g_strdup(year);
}

void la_track_set_comment(LaTrack * self, const gchar * comment)
{
    if (self->comment)
	g_free(self->comment);
    self->comment = g_strdup(comment);
}

void la_track_set_bitrate(LaTrack * self, gint bitrate)
{
    self->bitrate = bitrate;

    invalidate_quality_str(self);
}

void la_track_set_samplerate(LaTrack * self, gint samplerate)
{
    self->samplerate = samplerate;

    invalidate_quality_str(self);
}

void la_track_set_frames(LaTrack * self, gint frames)
{
    self->frames = frames;

    invalidate_quality_str(self);
}

void la_track_set_channels(LaTrack * self, gint channels)
{
    self->channels = channels;

    invalidate_quality_str(self);
}

void la_track_set_seconds(LaTrack * self, gint seconds)
{
    self->seconds = seconds;

    if (self->time_str) {
	g_free(self->time_str);
	self->time_str = NULL;
    }
}

void la_track_set_bytes(LaTrack * self, gint bytes)
{
    self->bytes = bytes;

    if (self->size_str) {
	g_free(self->size_str);
	self->size_str = NULL;
    }
}

void la_track_set_icon(LaTrack * self, const gchar * icon)
{
    if (self->icon)
	g_free(self->icon);
    self->icon = g_strdup(icon);
}

void la_track_set_rating(LaTrack * self, gint rating)
{
    self->rating = rating;

    if (self->rating_str) {
	g_free(self->rating_str);
	self->rating_str = NULL;
    }
}

void la_track_set_checked(LaTrack * self, gboolean checked)
{
    self->checked = checked;
}

void la_track_set_timestamp(LaTrack * self, gulong timestamp)
{
    self->timestamp = timestamp;
}

const gchar *la_track_get_uri(LaTrack * self)
{
    if (!self->uri) return "";
    return self->uri;
}

const gchar *la_track_get_title(LaTrack * self)
{
    if (!self->title) return "";
    return self->title;
}

const gchar *la_track_get_genre(LaTrack * self)
{
    if (!self->genre) return "";
    return self->genre;
}

const gchar *la_track_get_artist(LaTrack * self)
{
    if (!self->artist) return "";
    return self->artist;
}

const gchar *la_track_get_album(LaTrack * self)
{
    if (!self->album) return "";
    return self->album;
}

const gchar *la_track_get_track(LaTrack * self)
{
    if (!self->track) return "";
    return self->track;
}

const gchar *la_track_get_year(LaTrack * self)
{
    if (!self->year) return "";
    return self->year;
}

const gchar *la_track_get_comment(LaTrack * self)
{
    if (!self->comment) return "";
    return self->comment;
}

gint la_track_get_bitrate(LaTrack * self)
{
    return self->bitrate;
}

gint la_track_get_samplerate(LaTrack * self)
{
    return self->samplerate;
}

gint la_track_get_frames(LaTrack * self)
{
    return self->frames;
}

gint la_track_get_channels(LaTrack * self)
{
    return self->channels;
}

gint la_track_get_seconds(LaTrack * self)
{
    return self->seconds;
}

gint la_track_get_bytes(LaTrack * self)
{
    return self->bytes;
}

const gchar *la_track_get_icon(LaTrack * self)
{
    if (!self->comment) return "";
    return self->icon;
}

gint la_track_get_rating(LaTrack * self)
{
    return self->rating;
}

gboolean la_track_get_checked(LaTrack * self)
{
    return self->checked;
}

gulong la_track_get_timestamp(LaTrack * self)
{
    return self->timestamp;
}

gboolean la_track_set_quality(LaTrack * self, gint bitrate,
			      gint samplerate, gint frames,
			      gint channels, gint seconds)
{
    if (bitrate == self->bitrate && samplerate == self->samplerate
	&& frames == self->frames && channels == self->channels
	&& seconds == self->seconds)
	return FALSE;		//not changed

    self->bitrate = bitrate;
    self->samplerate = samplerate;
    self->frames = frames;
    self->channels = channels;
    self->seconds = seconds;

    invalidate_quality_str(self);
    return TRUE;		//changed
}

const gchar *la_track_get_time_str(LaTrack * self)
{
    if (!self->time_str && self->seconds > 0) {
	self->time_str = la_format_time_str_new(self->seconds);
    }
    return self->time_str;
}

const gchar *la_track_get_size_str(LaTrack * self)
{
    if (!self->size_str && self->bytes > 0) {
	self->size_str = la_format_size_str_new(self->bytes);
    }
    return self->size_str;
}

const gchar *la_track_get_date_str(LaTrack * self)
{
    if (!self->date_str && self->timestamp > 0) {
	self->date_str = la_format_date_str_new(self->timestamp);
    }
    return self->date_str;
}

const gchar *la_track_get_rating_str(LaTrack * self)
{
    if (!self->rating_str && self->rating > 0) {
	self->rating_str = g_strnfill(self->rating, RATING_CHAR);
    }
    return self->rating_str;
}

const gchar *la_track_get_quality_str(LaTrack * self)
{
    if (!self->rating_str
	&& (self->bitrate > 0 || self->samplerate > 0
	    || self->frames > 0 || self->channels > 0)) {
	self->quality_str =
	    g_strdup_printf(_(QUALITY_FORMAT),
			    self->bitrate / 1024,
			    self->samplerate / 1000, self->frames,
			    self->channels);
    }
    return self->quality_str;
}

const gchar *la_track_get_summary_str(LaTrack * self)
{
    if (!self->summary_str
	&& (self->bitrate > 0 || self->samplerate > 0
	    || self->frames > 0 || self->channels > 0)) {
	self->summary_str =
	    g_strdup_printf(_(SUMMARY_FORMAT),
			    self->bitrate,
			    self->samplerate, self->frames,
			    self->channels);
    }
    return self->summary_str;
}

GQuark la_track_get_quark(LaTrack * self)
{
    return self->quark;
}

GnomeVFSURI *la_track_get_vfs_uri(LaTrack * self)
{
    if (!self->vfs_uri) {
	self->vfs_uri = gnome_vfs_uri_new(self->uri);
    }
    gnome_vfs_uri_ref(self->vfs_uri);
    return self->vfs_uri;
}

GnomeVFSFileInfo *la_track_get_vfs_info(LaTrack * self)
{
    if (!self->vfs_info) {
	self->vfs_info = gnome_vfs_file_info_new();
	gnome_vfs_get_file_info(self->uri, self->vfs_info,
				GNOME_VFS_FILE_INFO_GET_MIME_TYPE
				| GNOME_VFS_FILE_INFO_FOLLOW_LINKS);
    }
    gnome_vfs_file_info_ref(self->vfs_info);
    return self->vfs_info;
}

/* boilerplates -----------------------------------------*/

GType la_track_get_type(void)
{
    static GType self_type = 0;
    if (self_type == 0)
	self_type =
	    g_boxed_type_register_static("LaTrack", (GBoxedCopyFunc)
					 la_track_ref, (GBoxedFreeFunc)
					 la_track_unref);
    return self_type;
}

/* la-track.c */

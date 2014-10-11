/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "la-playlist.h"
#include "la-track.h"
#include "la-iter.h"
#include "la-tracks-iter.h"
#include "util.h"

/* private------------------------------------------------*/

static gboolean track_added_cb(LaIter * iter, LaTrack * track,
			       LaPlaylist * self)
{
}

static gboolean track_removed_cb(LaIter * iter, LaTrack * track,
				 LaPlaylist * self)
{
}

static gboolean track_changed_cb(LaIter * iter, LaTrack * track,
				 LaPlaylist * self)
{
}

/* public-------------------------------------------------*/

LaPlaylist *la_playlist_new(void)
{
    LaPlaylist *self = g_object_new(LA_TYPE_PLAYLIST, NULL);

    self->tracks = la_tracks_iter_new();

    g_signal_connect(self->tracks, "added",
		     G_CALLBACK(track_added_cb), self);
    g_signal_connect(self->tracks, "removed",
		     G_CALLBACK(track_removed_cb), self);
    g_signal_connect(self->tracks, "changed",
		     G_CALLBACK(track_changed_cb), self);

    return self;
}

void la_playlist_set_name(LaPlaylist * self, const gchar * name)
{
    if (self->name)
	g_free(self->name);
    self->name = g_strdup(name);
}

void la_playlist_set_icon(LaPlaylist * self, const gchar * icon)
{
    if (self->icon)
	g_free(self->icon);
    self->icon = g_strdup(icon);
}

void la_playlist_set_uri(LaPlaylist * self, const gchar * uri)
{
    if (self->uri)
	g_free(self->uri);
    self->uri = g_strdup(uri);
}

inline const gchar *la_playlist_get_name(LaPlaylist * self)
{
    return self->name;
}

inline const gchar *la_playlist_get_icon(LaPlaylist * self)
{
    return self->icon;
}

inline const gchar *la_playlist_get_uri(LaPlaylist * self)
{
    return self->uri;
}

inline LaIter *la_playlist_get_tracks(LaPlaylist * self)
{
    return self->tracks;
}

inline gint la_playlist_get_num_tracks(LaPlaylist * self)
{
    return la_iter_length(self->tracks);
}

inline LaTrack *la_playlist_nth_track(LaPlaylist * self, gint index)
{
    return la_iter_nth(self->tracks, index);
}

inline LaTrack *la_playlist_lookup_track(LaPlaylist * self,
					 const gchar * uri)
{
    return la_tracks_iter_lookup(LA_TRACKS_ITER(self->tracks), uri);
}

/* boilerplates -----------------------------------------*/

static void la_playlist_class_init(LaPlaylistClass * klass);
static void la_playlist_instance_init(LaPlaylist * self);
static void la_playlist_finalize(GObject * gobject);

GType la_playlist_get_type()
{
    static const GTypeInfo self_type_info = {
	sizeof(LaPlaylistClass),
	NULL,
	NULL,
	(GClassInitFunc) la_playlist_class_init,
	NULL,
	NULL,
	sizeof(LaPlaylist),
	0,
	(GInstanceInitFunc) la_playlist_instance_init,
    };

    static GType self_type = 0;

    if (self_type)
	return self_type;

    self_type = g_type_register_static(G_TYPE_OBJECT, "LaPlaylist",
				       &self_type_info, 0);

    return self_type;
}

static void la_playlist_class_init(LaPlaylistClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = la_playlist_finalize;
}

static void la_playlist_instance_init(LaPlaylist * self)
{
    self->name = NULL;
    self->icon = NULL;
    self->uri = NULL;

    self->tracks = NULL;
}

static void la_playlist_finalize(GObject * gobject)
{
    LaPlaylist *self = LA_PLAYLIST(gobject);

    if (self->tracks)
	g_object_unref(self->tracks);

    if (self->name)
	g_free(self->name);
    if (self->icon)
	g_free(self->icon);
    if (self->uri)
	g_free(self->uri);
}

/* la-playlist.c */

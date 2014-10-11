/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __LA_PLAYLIST_H__
#define __LA_PLAYLIST_H__

#include <glib-object.h>
#include "la-iter.h"
#include "la-track.h"

G_BEGIN_DECLS
#define LA_TYPE_PLAYLIST		(la_playlist_get_type())
#define LA_PLAYLIST(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), LA_TYPE_PLAYLIST, LaPlaylist))
#define LA_PLAYLIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), LA_TYPE_PLAYLIST, LaPlaylistClass))
#define LA_IS_PLAYLIST(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), LA_TYPE_PLAYLIST))
#define LA_IS_PLAYLIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), LA_TYPE_PLAYLIST))
typedef struct _LaPlaylist LaPlaylist;
typedef struct _LaPlaylistClass LaPlaylistClass;

struct _LaPlaylist {
    GObject parent;

    gchar *name;		//PK
    gchar *icon;
    gchar *uri;

    LaIter *tracks;
};

struct _LaPlaylistClass {
    GObjectClass parent_class;
};

GType la_playlist_get_type(void) G_GNUC_CONST;
LaPlaylist *la_playlist_new(void);

// setters
void la_playlist_set_name(LaPlaylist * self, const gchar * name);
void la_playlist_set_icon(LaPlaylist * self, const gchar * icon);
void la_playlist_set_uri(LaPlaylist * self, const gchar * uri);

// getters
inline const gchar *la_playlist_get_name(LaPlaylist * self);
inline const gchar *la_playlist_get_icon(LaPlaylist * self);
inline const gchar *la_playlist_get_uri(LaPlaylist * self);

// tracks management
inline LaIter *la_playlist_get_tracks(LaPlaylist * self);
inline gint la_playlist_get_num_tracks(LaPlaylist * self);
inline LaTrack *la_playlist_nth_track(LaPlaylist * self, gint index);
inline LaTrack *la_playlist_lookup_track(LaPlaylist * self,
					 const gchar * uri);

G_END_DECLS
#endif				/* __LA_PLAYLIST_H__ */

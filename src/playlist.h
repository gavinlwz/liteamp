/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __PLAYLIST_H__
#define __PLAYLIST_H__

#include <gnome.h>

#include "la-playlist.h"
#include "la-track.h"

G_BEGIN_DECLS
#define TYPE_PLAYLIST		(playlist_get_type())
#define PLAYLIST(object)	(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_PLAYLIST, Playlist))
#define PLAYLIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_PLAYLIST, PlaylistClass))
#define IS_PLAYLIST(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_PLAYLIST))
#define IS_PLAYLIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_PLAYLIST))
typedef struct _Playlist Playlist;
typedef struct _PlaylistClass PlaylistClass;

GType playlist_get_type(void) G_GNUC_CONST;
Playlist *playlist_new(void);

inline void playlist_set_visible(Playlist * self, gboolean visible);
inline gboolean playlist_get_visible(Playlist * self);

void playlist_update_columns(Playlist * self);

void playlist_set_playlist(Playlist * self, LaPlaylist * playlist);
LaPlaylist *playlist_get_playlist(Playlist * self);

void playlist_set_track(Playlist * self, LaTrack * track);
LaTrack *playlist_get_track(Playlist * self);

gboolean playlist_is_writable(Playlist * self);
gboolean playlist_clipboard_has_data(Playlist * self);

LaTrack *playlist_get_cursor_track(Playlist * self);
LaTrack *playlist_get_previous_track(Playlist * self);
LaTrack *playlist_get_next_track(Playlist * self);

LaTrack *playlist_get_track_by_uri(Playlist * self, const gchar * uri);

void playlist_read(Playlist * self);
void playlist_write(Playlist * self);

void playlist_add_file(Playlist * self, const gchar * filename,
		       gboolean checked);
void playlist_add_directory(Playlist * self, const gchar * dirname,
			    gboolean recursive, gboolean checked);
void playlist_remove_file(Playlist * self, const gchar * filename);

void playlist_import_pls(Playlist * self, const gchar * filename);
void playlist_export_pls(Playlist * self, const gchar * filename);

void playlist_cut_selection(Playlist * self);
void playlist_copy_selection(Playlist * self);
void playlist_paste_selection(Playlist * self);
void playlist_clear_selection(Playlist * self);
void playlist_select_all(Playlist * self);
void playlist_select_none(Playlist * self);
void playlist_invert_selection(Playlist * self);

void playlist_previous(Playlist * self);
void playlist_next(Playlist * self);

void playlist_shuffle(Playlist * self);

void playlist_jump_to(Playlist * self);
void playlist_refresh(Playlist * self);

GSList *playlist_get_selected_tracks(Playlist *self);

G_END_DECLS
#endif				/*playlist.h */

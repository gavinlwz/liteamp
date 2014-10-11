/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __PLAYLIST_DIALOG_H__
#define __PLAYLIST_DIALOG_H__

#include "la-playlist.h"

G_BEGIN_DECLS
#define TYPE_PLAYLIST_DIALOG		(playlist_dialog_get_type())
#define PLAYLIST_DIALOG(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_PLAYLIST_DIALOG, PlaylistDialog))
#define PLAYLIST_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_PLAYLIST_DIALOG, PlaylistDialogClass))
#define IS_PLAYLIST_DIALOG(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_PLAYLIST_DIALOG))
#define IS_PLAYLIST_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_PLAYLIST_DIALOG))
typedef struct _PlaylistDialog PlaylistDialog;
typedef struct _PlaylistDialogClass PlaylistDialogClass;

GType playlist_dialog_get_type(void) G_GNUC_CONST;
PlaylistDialog *playlist_dialog_new(GtkWindow * parent);

void playlist_dialog_show(PlaylistDialog * self, LaPlaylist * playlist);
LaPlaylist *playlist_dialog_get_playlist(PlaylistDialog * self);

G_END_DECLS
#endif /*__PLAYLIST_DIALOG__*/

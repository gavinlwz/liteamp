/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __LA_TRACKS_STORE_H__
#define __LA_TRACKS_STORE_H__

#include <gtk/gtk.h>
#include "la-iter.h"
#include "la-track.h"

G_BEGIN_DECLS
#define LA_TYPE_TRACKS_STORE		(la_tracks_store_get_type())
#define LA_TRACKS_STORE(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), LA_TYPE_TRACKS_STORE, LaTracksStore))
#define LA_TRACKS_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), LA_TYPE_TRACKS_STORE, LaTracksStoreClass))
#define LA_IS_TRACKS_STORE(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), LA_TYPE_TRACKS_STORE))
#define LA_IS_TRACKS_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), LA_TYPE_TRACKS_STORE))
typedef struct _LaTracksStore LaTracksStore;
typedef struct _LaTracksStoreClass LaTracksStoreClass;

GType la_tracks_store_get_type(void) G_GNUC_CONST;
LaTracksStore *la_tracks_store_new(LaIter * iter);

gint la_tracks_store_get_sort_column_id(LaTracksStore * self);
GtkSortType la_tracks_store_get_sort_type(LaTracksStore * self);

LaIter *la_tracks_store_get_iter(LaTracksStore * self);
gboolean la_tracks_store_add_track(LaTracksStore * self, LaTrack * track);
gboolean la_tracks_store_remove_track(LaTracksStore * self,
				      LaTrack * track);

void la_tracks_store_update_summary(LaTracksStore * self);
void la_tracks_store_toggle_checked(LaTracksStore * self,
				    GtkTreePath * path);
gchar *la_tracks_store_get_summary(LaTracksStore * self);

G_END_DECLS
#endif				/* __LA_TRACKS_STORE_H__ */

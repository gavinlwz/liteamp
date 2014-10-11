/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __LA_TRACKS_ITER_H__
#define __LA_TRACKS_ITER_H__

#include <glib-object.h>
#include "la-iter.h"
#include "la-track.h"

G_BEGIN_DECLS
#define LA_TYPE_TRACKS_ITER	(la_tracks_iter_get_type())
#define LA_TRACKS_ITER(object)	(G_TYPE_CHECK_INSTANCE_CAST((object), LA_TYPE_TRACKS_ITER, LaTracksIter))
#define LA_TRACKS_ITER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), LA_TYPE_TRACKS_ITER, LaTracksIterClass))
#define LA_IS_TRACKS_ITER(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), LA_TYPE_TRACKS_ITER))
#define LA_IS_TRACKS_ITER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), LA_TYPE_TRACKS_ITER))
typedef struct _LaTracksIter LaTracksIter;
typedef struct _LaTracksIterClass LaTracksIterClass;

struct _LaTracksIter {
    GObject parent;
    GSList *slist;
    GData *datalist;
    gint length;
};

struct _LaTracksIterClass {
    GObjectClass parent_class;
};

GType la_tracks_iter_get_type(void) G_GNUC_CONST;
LaIter *la_tracks_iter_new(void);

LaTrack *la_tracks_iter_lookup(LaTracksIter * self, const gchar * uri);

G_END_DECLS
#endif				/* __LA_TRACKS_ITER_H__ */

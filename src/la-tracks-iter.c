/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "la-tracks-iter.h"

/* private------------------------------------------------*/

static gint iter_length(LaTracksIter * self)
{
    return self->length;
    //return g_slist_length(self->slist);
}

static LaTrack *iter_nth(LaTracksIter * self, gint index)
{
    return g_slist_nth_data(self->slist, index);
}

static gint iter_index(LaTracksIter * self, LaTrack * track)
{
    if (!track)
	return -1;
    return g_slist_index(self->slist, track);
}

static gboolean iter_add(LaTracksIter * self, LaTrack * track)
{
    GQuark quark;

    if (!track)
	return FALSE;

    quark = la_track_get_quark(track);
    if (g_datalist_id_get_data(&self->datalist, quark))
	return FALSE;		// duplicate!

    self->slist = g_slist_append(self->slist, track);
    g_datalist_id_set_data(&self->datalist, quark, track);
    ++self->length;

    la_iter_added(LA_ITER(self), track);
    return TRUE;
}

static gboolean iter_remove(LaTracksIter * self, LaTrack * track)
{
    GQuark quark;

    if (!track)
	return FALSE;

    quark = la_track_get_quark(track);
    if (!g_datalist_id_get_data(&self->datalist, quark))
	return FALSE;		// not exist!

    self->slist = g_slist_remove(self->slist, track);
    g_datalist_id_remove_data(&self->datalist, quark);
    --self->length;

    la_iter_removed(LA_ITER(self), track);
    return TRUE;
}

static void iter_sort(LaTracksIter * self, GCompareDataFunc compare_func,
		      gpointer user_data)
{
    self->slist = g_slist_sort_with_data(self->slist,
					 compare_func, user_data);
}

static void iter_foreach(LaTracksIter * self, GFunc func,
			 gpointer user_data)
{
    g_slist_foreach(self->slist, func, user_data);
}

/* public-------------------------------------------------*/

LaIter *la_tracks_iter_new(void)
{
    LaTracksIter *self;
    self = g_object_new(LA_TYPE_TRACKS_ITER, NULL);
    self->slist = NULL;
    self->length = 0;
    g_datalist_init(&self->datalist);
    return LA_ITER(self);
}

inline LaTrack *la_tracks_iter_lookup(LaTracksIter * self,
				      const gchar * uri)
{
    return g_datalist_get_data(&self->datalist, uri);
}

/* boilerplates -----------------------------------------*/

static void la_tracks_iter_class_init(LaTracksIterClass * klass);
static void la_tracks_iter_iter_iface_init(LaIterIface * iface);
static void la_tracks_iter_instance_init(LaTracksIter * self);
static void la_tracks_iter_finalize(GObject * gobject);

GType la_tracks_iter_get_type()
{
    static const GTypeInfo self_type_info = {
	sizeof(LaTracksIterClass),
	NULL,
	NULL,
	(GClassInitFunc) la_tracks_iter_class_init,
	NULL,
	NULL,
	sizeof(LaTracksIter),
	0,
	(GInstanceInitFunc) la_tracks_iter_instance_init,
    };

    static const GInterfaceInfo list_iface_info = {
	(GInterfaceInitFunc) la_tracks_iter_iter_iface_init,
	NULL,
	NULL
    };

    static GType self_type = 0;

    if (self_type)
	return self_type;

    self_type = g_type_register_static(G_TYPE_OBJECT, "LaTracksIter",
				       &self_type_info, 0);

    g_type_add_interface_static(self_type, LA_TYPE_ITER, &list_iface_info);

    return self_type;
}

static void la_tracks_iter_class_init(LaTracksIterClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = la_tracks_iter_finalize;
}

static void la_tracks_iter_iter_iface_init(LaIterIface * iface)
{
    iface->length = (gpointer) iter_length;
    iface->nth = (gpointer) iter_nth;
    iface->index = (gpointer) iter_index;
    iface->add = (gpointer) iter_add;
    iface->remove = (gpointer) iter_remove;
    iface->sort = (gpointer) iter_sort;
    iface->foreach = (gpointer) iter_foreach;
}

static void la_tracks_iter_instance_init(LaTracksIter * self)
{
    self->slist = NULL;
    self->datalist = NULL;
}

static void la_tracks_iter_finalize(GObject * gobject)
{
    LaTracksIter *self = LA_TRACKS_ITER(gobject);

    g_datalist_clear(&self->datalist);
    g_slist_free(self->slist);
}

/* la-tracks-iter.c */

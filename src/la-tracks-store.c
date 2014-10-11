/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include "la-tracks-store.h"
#include "la-track-column.h"

/* private-----------------------------------------------*/

struct _LaTracksStore {
    GObject parent;

    gint stamp;

    gint sort_column_id;
    GtkSortType sort_type;

    LaIter *iter;

    GString *summary;
    gint total_count;
    gint total_seconds;
    gint total_bytes;
    gint checked_count;
    gint checked_seconds;
    gint checked_bytes;
};

struct _LaTracksStoreClass {
    GObjectClass parent_class;
};

static const gchar *summary_fmt =
N_("Titles: %d / %d, Play Time: %s / %s, File Size: %s / %s");

static gint track_compare_func(LaTrack * track1, LaTrack * track2,
			       LaTracksStore * self)
{
    gint ret;
    ret =
	la_track_column_compare_column(self->sort_column_id, track1,
				       track2);
    return (self->sort_type == GTK_SORT_ASCENDING) ? ret : -ret;
}

static gint tree_iter_compare_func(LaTracksStore * self, GtkTreeIter * a,
				   GtkTreeIter * b, gpointer data)
{
    gint index1;
    gint index2;
    LaTrack *track1;
    LaTrack *track2;

    index1 = GPOINTER_TO_INT(a->user_data);
    g_return_val_if_fail(index1 >= 0, 0);
    track1 = la_iter_nth(self->iter, index1);
    g_return_val_if_fail(track1, 0);

    index2 = GPOINTER_TO_INT(a->user_data);
    g_return_val_if_fail(index2 >= 0, 0);
    track2 = la_iter_nth(self->iter, index2);
    g_return_val_if_fail(track2, 0);

    return track_compare_func(track1, track2, self);
}

static void update_summary(LaTracksStore * self, LaTrack * track,
			   gboolean is_added, gboolean is_removed)
{
    gboolean checked;
    gint seconds, bytes;

    checked = la_track_get_checked(track);
    seconds = MAX(la_track_get_seconds(track), 0);
    bytes = MAX(la_track_get_bytes(track), 0);

    if (is_added) {
	++self->total_count;
	self->total_seconds += seconds;
	self->total_bytes += bytes;
	if (checked) {
	    ++self->checked_count;
	    self->checked_seconds += seconds;
	    self->checked_bytes += bytes;
	}
    } else if (is_removed) {
	--self->total_count;
	self->total_seconds -= seconds;
	self->total_bytes -= bytes;
	if (checked) {
	    --self->checked_count;
	    self->checked_seconds -= seconds;
	    self->checked_bytes -= bytes;
	}
    } else {
	if (checked) {
	    ++self->checked_count;
	    self->checked_seconds += seconds;
	    self->checked_bytes += bytes;
	} else {
	    --self->checked_count;
	    self->checked_seconds -= seconds;
	    self->checked_bytes -= bytes;
	}
    }
}

static void update_summary_func(LaTrack * track, LaTracksStore * self)
{
    update_summary(self, track, TRUE, FALSE);
}

static void track_added_cb(LaIter * iter, LaTrack * track,
			   LaTracksStore * self)
{
    gint index;
    GtkTreeIter tree_iter;
    GtkTreePath *path;

    //@@gdk_threads_enter();

    index = la_iter_index(iter, track);
    if (index >= 0) {
	tree_iter.stamp = self->stamp;
	tree_iter.user_data = GINT_TO_POINTER(index);
	path = gtk_tree_path_new_from_indices(index, -1);
	gtk_tree_model_row_inserted(GTK_TREE_MODEL(self), path,
				    &tree_iter);
	gtk_tree_path_free(path);

	update_summary(self, track, TRUE, FALSE);
    }
    //@@gdk_threads_leave();
}

static void track_removed_cb(LaIter * tracks, LaTrack * track,
			     LaTracksStore * self)
{
    gint index;
    GtkTreePath *path;

    //@@gdk_threads_enter();

    index = la_iter_index(tracks, track);
    if (index >= 0) {
	path = gtk_tree_path_new_from_indices(index, -1);

	gtk_tree_model_row_deleted(GTK_TREE_MODEL(self), path);

	gtk_tree_path_free(path);

	update_summary(self, track, FALSE, TRUE);
    }
    //@@gdk_threads_leave();
}

static void track_changed_cb(LaIter * iter, LaTrack * track,
			     LaTracksStore * self)
{
    gint index;
    GtkTreeIter tree_iter;
    GtkTreePath *path;

    //@@gdk_threads_enter();

    index = la_iter_index(iter, track);
    if (index >= 0) {
	tree_iter.stamp = self->stamp;
	tree_iter.user_data = GINT_TO_POINTER(index);
	path = gtk_tree_path_new_from_indices(index, -1);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(self), path, &tree_iter);
	gtk_tree_path_free(path);

	update_summary(self, track, FALSE, FALSE);
    }
    //@@gdk_threads_leave();
}

//
// prototypes
//

//
// GtkTreeModel
//

static GtkTreeModelFlags get_flags(LaTracksStore * self);
static gint get_n_columns(LaTracksStore * self);
static GType get_column_type(LaTracksStore * self, gint index);
static gboolean get_iter(LaTracksStore * self,
			 GtkTreeIter * iter, GtkTreePath * path);
static GtkTreePath *get_path(LaTracksStore * self, GtkTreeIter * iter);
static void get_value(LaTracksStore * self,
		      GtkTreeIter * iter, gint column, GValue * value);
static gboolean iter_next(LaTracksStore * self, GtkTreeIter * iter);
static gboolean iter_children(LaTracksStore * self,
			      GtkTreeIter * iter, GtkTreeIter * parent);
static gboolean iter_has_child(LaTracksStore * self, GtkTreeIter * iter);
static gint iter_n_children(LaTracksStore * self, GtkTreeIter * iter);
static gboolean iter_nth_child(LaTracksStore * self,
			       GtkTreeIter * iter,
			       GtkTreeIter * parent, gint n);
static gboolean iter_parent(LaTracksStore * self,
			    GtkTreeIter * iter, GtkTreeIter * child);

//
// GtkTreeDragSource
//

static gboolean real_row_draggable(LaTracksStore * self,
				   GtkTreePath * path);
static gboolean drag_data_delete(LaTracksStore * self, GtkTreePath * path);
static gboolean drag_data_get(LaTracksStore * self, GtkTreePath * path,
			      GtkSelectionData * selection_data);

//
// GtkTreeDragDest
//

static gboolean drag_data_received(LaTracksStore * self,
				   GtkTreePath * dest,
				   GtkSelectionData * selection_data);
static gboolean row_drop_possible(LaTracksStore * self,
				  GtkTreePath * dest_path,
				  GtkSelectionData * selection_data);

//
// GtkTreeSortable
// 

static gboolean get_sort_column_id(LaTracksStore * self,
				   gint * sort_column_id,
				   GtkSortType * order);
static void set_sort_column_id(LaTracksStore * self,
			       gint sort_column_id, GtkSortType order);
static void set_sort_func(LaTracksStore * self, gint sort_column_id,
			  GtkTreeIterCompareFunc func, gpointer data,
			  GtkDestroyNotify destroy);
static void set_default_sort_func(LaTracksStore * self,
				  GtkTreeIterCompareFunc func,
				  gpointer data, GtkDestroyNotify destroy);
static gboolean has_default_sort_func(LaTracksStore * self);

//
// implementations
//

//
// GtkTreeModel
//

static GtkTreeModelFlags get_flags(LaTracksStore * self)
{
    return GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST;
}

static gint get_n_columns(LaTracksStore * self)
{
    return LA_N_TRACK_COLUMNS;
}

static GType get_column_type(LaTracksStore * self, gint index)
{
    return la_track_column_get_column_type(index);
}

static gboolean
get_iter(LaTracksStore * self, GtkTreeIter * iter, GtkTreePath * path)
{
    gint index;

    g_return_val_if_fail(LA_IS_TRACKS_STORE(self), FALSE);
    g_return_val_if_fail(gtk_tree_path_get_depth(path) > 0, FALSE);

    self = LA_TRACKS_STORE(self);

    index = gtk_tree_path_get_indices(path)[0];

    if (index >= la_iter_length(self->iter))
	return FALSE;

    iter->stamp = self->stamp;
    iter->user_data = GINT_TO_POINTER(index);

    return TRUE;
}

static GtkTreePath *get_path(LaTracksStore * self, GtkTreeIter * iter)
{
    gint index;

    g_return_val_if_fail(LA_IS_TRACKS_STORE(self), FALSE);
    g_return_val_if_fail(iter->stamp == self->stamp, FALSE);

    index = GPOINTER_TO_INT(iter->user_data);

    if (index >= la_iter_length(self->iter))
	return NULL;

    return gtk_tree_path_new_from_indices(index, -1);
}

static void
get_value(LaTracksStore * self, GtkTreeIter * iter,
	  gint column, GValue * value)
{
    gint index;
    LaTrack *track;

    g_return_if_fail(LA_IS_TRACKS_STORE(self));
    g_return_if_fail(column < LA_N_TRACK_COLUMNS);
    g_return_if_fail(iter->stamp == self->stamp);

    index = GPOINTER_TO_INT(iter->user_data);

    track = la_iter_nth(self->iter, index);

    if (track)
	la_track_column_get_column_value(column, track, value);
    else
	g_value_init(value, la_track_column_get_column_type(column));
}

static gboolean iter_next(LaTracksStore * self, GtkTreeIter * iter)
{
    gint index;

    g_return_val_if_fail(LA_IS_TRACKS_STORE(self), FALSE);
    g_return_val_if_fail(iter->stamp == self->stamp, FALSE);

    index = GPOINTER_TO_INT(iter->user_data);

    if (++index < la_iter_length(self->iter)) {
	iter->user_data = GINT_TO_POINTER(index);
	return TRUE;
    }
    return FALSE;
}

static gboolean
iter_children(LaTracksStore * self, GtkTreeIter * iter,
	      GtkTreeIter * parent)
{
    if (parent)
	return FALSE;

    if (la_iter_length(self->iter) == 0)
	return FALSE;

    iter->stamp = self->stamp;
    iter->user_data = GINT_TO_POINTER(0);
    return TRUE;
}

static gboolean iter_has_child(LaTracksStore * self, GtkTreeIter * iter)
{
    return FALSE;
}

static gint iter_n_children(LaTracksStore * self, GtkTreeIter * iter)
{
    g_return_val_if_fail(LA_IS_TRACKS_STORE(self), 0);

    if (iter == NULL)
	return la_iter_length(self->iter);

    g_return_val_if_fail(iter->stamp == self->stamp, -1);
    return 0;
}

static gboolean
iter_nth_child(LaTracksStore * self, GtkTreeIter * iter,
	       GtkTreeIter * parent, gint n)
{
    g_return_val_if_fail(LA_IS_TRACKS_STORE(self), FALSE);

    if (parent)
	return FALSE;

    if (n < la_iter_length(self->iter)) {
	iter->stamp = self->stamp;
	iter->user_data = GINT_TO_POINTER(n);
	return TRUE;
    } else
	return FALSE;
}

static gboolean
iter_parent(LaTracksStore * self, GtkTreeIter * iter, GtkTreeIter * child)
{
    return FALSE;
}

//
// GtkTreeDragSource
//

static gboolean row_draggable(LaTracksStore * self, GtkTreePath * path)
{
}

static gboolean drag_data_delete(LaTracksStore * self, GtkTreePath * path)
{
}

static gboolean drag_data_get(LaTracksStore * self,
			      GtkTreePath * path,
			      GtkSelectionData * selection_data)
{
}

//
// GtkTreeDragDest
//

static gboolean
drag_data_received(LaTracksStore * self, GtkTreePath * dest,
		   GtkSelectionData * selection_data)
{
}

static gboolean
row_drop_possible(LaTracksStore * self, GtkTreePath * dest_path,
		  GtkSelectionData * selection_data)
{
}

//
// GtkTreeSortable
//

static gboolean
get_sort_column_id(LaTracksStore * self, gint * sort_column_id,
		   GtkSortType * order)
{
    if (self->sort_column_id == GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID)
	return FALSE;

    if (sort_column_id)
	*sort_column_id = self->sort_column_id;

    if (order)
	*order = self->sort_type;

    return TRUE;
}

static void
set_sort_column_id(LaTracksStore * self, gint sort_column_id,
		   GtkSortType order)
{
    if (self->sort_column_id == sort_column_id && self->sort_type == order)
	return;

    self->sort_column_id = sort_column_id;
    self->sort_type = order;

    gtk_tree_sortable_sort_column_changed(GTK_TREE_SORTABLE(self));

    la_iter_sort(self->iter, (GCompareDataFunc) track_compare_func, self);
}

static void
set_sort_func(LaTracksStore * self, gint sort_column_id,
	      GtkTreeIterCompareFunc func, gpointer data,
	      GtkDestroyNotify destroy)
{
    if (sort_column_id == self->sort_column_id)
	la_iter_sort(self->iter, (GCompareDataFunc) track_compare_func,
		     self);
}

static void
set_default_sort_func(LaTracksStore * self,
		      GtkTreeIterCompareFunc func, gpointer data,
		      GtkDestroyNotify destroy)
{
    if (self->sort_column_id == GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID)
	la_iter_sort(self->iter, (GCompareDataFunc) track_compare_func,
		     self);
}

static gboolean has_default_sort_func(LaTracksStore * self)
{
    return FALSE;
}

/* public------------------------------------------------*/

LaTracksStore *la_tracks_store_new(LaIter * iter)
{
    LaTracksStore *self;

    self = g_object_new(LA_TYPE_TRACKS_STORE, NULL);

    self->iter = iter;

    g_signal_connect(self->iter, "added",
		     G_CALLBACK(track_added_cb), self);
    g_signal_connect(self->iter, "removed",
		     G_CALLBACK(track_removed_cb), self);
    g_signal_connect(self->iter, "changed",
		     G_CALLBACK(track_changed_cb), self);

    la_tracks_store_update_summary(self);

    return self;
}

gint la_tracks_store_get_sort_column_id(LaTracksStore * self)
{
    return self->sort_column_id;
}

GtkSortType la_tracks_store_get_sort_type(LaTracksStore * self)
{
    return self->sort_type;
}

LaIter *la_tracks_store_get_iter(LaTracksStore * self)
{
    return self->iter;
}

gboolean la_tracks_store_add_track(LaTracksStore * self, LaTrack * track)
{
    return la_iter_add(self->iter, track);
}

gboolean la_tracks_store_remove_track(LaTracksStore * self,
				      LaTrack * track)
{
    return la_iter_remove(self->iter, track);
}

void la_tracks_store_update_summary(LaTracksStore * self)
{
    self->checked_seconds = self->checked_bytes = self->checked_count =
	self->total_seconds = self->total_bytes = self->total_count = 0;
    la_iter_foreach(self->iter, (GFunc) update_summary_func, self);
}

void la_tracks_store_toggle_checked(LaTracksStore * self,
				    GtkTreePath * path)
{
    gint index;
    LaTrack *track;

    index = gtk_tree_path_get_indices(path)[0];

    track = la_iter_nth(self->iter, index);
    g_return_if_fail(track);

    la_track_set_checked(track, !la_track_get_checked(track));

    la_iter_changed(self->iter, track);
}

gchar *la_tracks_store_get_summary(LaTracksStore * self)
{
    gchar checked_time[20];
    gchar checked_size[20];
    gchar total_time[20];
    gchar total_size[20];

    la_format_time_str(checked_time, sizeof(checked_time),
		       self->checked_seconds);
    la_format_size_str(checked_size, sizeof(checked_size),
		       self->checked_bytes);
    la_format_time_str(total_time, sizeof(total_time),
		       self->total_seconds);
    la_format_size_str(total_size, sizeof(total_size), self->total_bytes);

    if (!self->summary)
	self->summary = g_string_new("");

    g_string_printf(self->summary, _(summary_fmt), self->checked_count,
		    self->total_count, checked_time, total_time,
		    checked_size, total_size);

    return self->summary->str;
}

/* boilerplates -----------------------------------------*/

static void la_tracks_store_class_init(LaTracksStoreClass * klass);
static void la_tracks_store_instance_init(LaTracksStore * self);
static void la_tracks_store_finalize(GObject * gobject);
static void la_tracks_store_tree_model_init(GtkTreeModelIface * iface);
static void la_tracks_store_drag_source_init(GtkTreeDragSourceIface *
					     iface);
static void la_tracks_store_drag_dest_init(GtkTreeDragDestIface * iface);
static void la_tracks_store_sortable_init(GtkTreeSortableIface * iface);

GType la_tracks_store_get_type(void)
{
    static const GTypeInfo self_type_info = {
	sizeof(LaTracksStoreClass),
	NULL,
	NULL,
	(GClassInitFunc) la_tracks_store_class_init,
	NULL,
	NULL,
	sizeof(LaTracksStore),
	0,
	(GInstanceInitFunc) la_tracks_store_instance_init,
    };

    static const GInterfaceInfo tree_model_info = {
	(GInterfaceInitFunc)
	    la_tracks_store_tree_model_init,
	NULL,
	NULL
    };

    static const GInterfaceInfo drag_source_info = {
	(GInterfaceInitFunc)
	    la_tracks_store_drag_source_init,
	NULL,
	NULL
    };

    static const GInterfaceInfo drag_dest_info = {
	(GInterfaceInitFunc) la_tracks_store_drag_dest_init,
	NULL,
	NULL
    };

    static const GInterfaceInfo sortable_info = {
	(GInterfaceInitFunc) la_tracks_store_sortable_init,
	NULL,
	NULL
    };

    static GType self_type = 0;

    if (self_type)
	return self_type;

    self_type = g_type_register_static(G_TYPE_OBJECT, "LaTracksStore",
				       &self_type_info, 0);

    // implements
    g_type_add_interface_static(self_type,
				GTK_TYPE_TREE_MODEL, &tree_model_info);
    g_type_add_interface_static(self_type,
				GTK_TYPE_TREE_DRAG_SOURCE,
				&drag_source_info);
    g_type_add_interface_static(self_type,
				GTK_TYPE_TREE_DRAG_DEST, &drag_dest_info);
    g_type_add_interface_static(self_type,
				GTK_TYPE_TREE_SORTABLE, &sortable_info);
    return self_type;
}

static void la_tracks_store_class_init(LaTracksStoreClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = la_tracks_store_finalize;
}

static void la_tracks_store_instance_init(LaTracksStore * self)
{
    // while the odds are against us getting 0...
    do {
	self->stamp = g_random_int();
    } while (self->stamp == 0);

    self->sort_column_id = -2;	// not sorting
    self->sort_type = GTK_SORT_ASCENDING;

    self->iter = NULL;

    self->summary = NULL;
}

static void la_tracks_store_finalize(GObject * gobject)
{
    LaTracksStore *self = LA_TRACKS_STORE(gobject);

    if (self->summary)
	g_string_free(self->summary, TRUE);
}

static void la_tracks_store_tree_model_init(GtkTreeModelIface * iface)
{
    iface->get_flags = (gpointer) get_flags;
    iface->get_n_columns = (gpointer) get_n_columns;
    iface->get_column_type = (gpointer) get_column_type;
    iface->get_iter = (gpointer) get_iter;
    iface->get_path = (gpointer) get_path;
    iface->get_value = (gpointer) get_value;
    iface->iter_next = (gpointer) iter_next;
    iface->iter_children = (gpointer) iter_children;
    iface->iter_has_child = (gpointer) iter_has_child;
    iface->iter_n_children = (gpointer) iter_n_children;
    iface->iter_nth_child = (gpointer) iter_nth_child;
    iface->iter_parent = (gpointer) iter_parent;
}

static void la_tracks_store_drag_source_init(GtkTreeDragSourceIface *
					     iface)
{
    iface->row_draggable = (gpointer) row_draggable;
    iface->drag_data_delete = (gpointer) drag_data_delete;
    iface->drag_data_get = (gpointer) drag_data_get;
}

static void la_tracks_store_drag_dest_init(GtkTreeDragDestIface * iface)
{
    iface->drag_data_received = (gpointer) drag_data_received;
    iface->row_drop_possible = (gpointer) row_drop_possible;
}

static void la_tracks_store_sortable_init(GtkTreeSortableIface * iface)
{
    iface->get_sort_column_id = (gpointer) get_sort_column_id;
    iface->set_sort_column_id = (gpointer) set_sort_column_id;
    iface->set_sort_func = (gpointer) set_sort_func;
    iface->set_default_sort_func = (gpointer) set_default_sort_func;
    iface->has_default_sort_func = (gpointer) has_default_sort_func;
}

/* la-tracks-store.c */

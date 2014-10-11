/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __LA_ITER_H__
#define __LA_ITER_H__

#include <glib-object.h>

G_BEGIN_DECLS
#define LA_TYPE_ITER		(la_iter_get_type())
#define LA_ITER(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), LA_TYPE_ITER, LaIter))
#define LA_IS_ITER(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), LA_TYPE_ITER))
#define LA_ITER_GET_IFACE(object) 	(G_TYPE_INSTANCE_GET_INTERFACE((object), LA_TYPE_ITER, LaIterIface))
typedef struct _LaIter LaIter;	//dummy
typedef struct _LaIterIface LaIterIface;

struct _LaIterIface {
    GTypeInterface iface;

    // signals
    void (*added) (LaIter * self, gpointer element);
    void (*removed) (LaIter * self, gpointer element);
    void (*changed) (LaIter * self, gpointer element);

    // methods
     gint(*length) (LaIter * self);
     gpointer(*nth) (LaIter * self, gint index);
     gint(*index) (LaIter * self, gpointer element);
     gboolean(*add) (LaIter * self, gpointer element);
     gboolean(*remove) (LaIter * self, gpointer element);
    void (*sort) (LaIter * self, GCompareDataFunc compare_func,
		  gpointer user_data);
    void (*foreach) (LaIter * self, GFunc func, gpointer user_data);
};

GType la_iter_get_type(void) G_GNUC_CONST;

// signals
void la_iter_added(LaIter * self, gpointer element);
void la_iter_removed(LaIter * self, gpointer element);
void la_iter_changed(LaIter * self, gpointer element);

// methods
gint la_iter_length(LaIter * self);
gpointer la_iter_nth(LaIter * self, gint index);
gint la_iter_index(LaIter * self, gpointer element);
gboolean la_iter_add(LaIter * self, gpointer element);
gboolean la_iter_remove(LaIter * self, gpointer element);
void la_iter_sort(LaIter * self, GCompareDataFunc compare_func,
		  gpointer user_data);
void la_iter_foreach(LaIter * self, GFunc func, gpointer user_data);

G_END_DECLS
#endif				/* __LA_ITER_H__ */

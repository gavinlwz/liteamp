/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "la-iter.h"

/* private-----------------------------------------------*/

// signal id
enum {
    SELF_SIG_ADDED,
    SELF_SIG_REMOVED,
    SELF_SIG_CHANGED,
    N_SELF_SIG,
};

static guint self_signals[N_SELF_SIG] = { 0 };

/* public------------------------------------------------*/

void la_iter_added(LaIter * self, gpointer element)
{
    g_signal_emit(self, self_signals[SELF_SIG_ADDED], 0, element);
}

void la_iter_removed(LaIter * self, gpointer element)
{
    g_signal_emit(self, self_signals[SELF_SIG_REMOVED], 0, element);
}

void la_iter_changed(LaIter * self, gpointer element)
{
    g_signal_emit(self, self_signals[SELF_SIG_CHANGED], 0, element);
}

gint la_iter_length(LaIter * self)
{
    return LA_ITER_GET_IFACE(self)->length(self);
}

gpointer la_iter_nth(LaIter * self, gint index)
{
    return LA_ITER_GET_IFACE(self)->nth(self, index);
}

gint la_iter_index(LaIter * self, gpointer element)
{
    return LA_ITER_GET_IFACE(self)->index(self, element);
}

gboolean la_iter_add(LaIter * self, gpointer element)
{
    return LA_ITER_GET_IFACE(self)->add(self, element);
}

gboolean la_iter_remove(LaIter * self, gpointer element)
{
    return LA_ITER_GET_IFACE(self)->remove(self, element);
}

void la_iter_sort(LaIter * self, GCompareDataFunc compare_func,
		  gpointer user_data)
{
    LA_ITER_GET_IFACE(self)->sort(self, compare_func, user_data);
}

void la_iter_foreach(LaIter * self, GFunc func, gpointer user_data)
{
    LA_ITER_GET_IFACE(self)->foreach(self, func, user_data);
}

/* boilerplates -----------------------------------------*/

static void la_iter_base_init(gpointer g_class);

GType la_iter_get_type()
{
    static const GTypeInfo self_type_info = {
	sizeof(LaIterIface),
	la_iter_base_init,
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	0,
	NULL,
    };
    static GType self_type = 0;

    if (self_type)
	return self_type;

    self_type =
	g_type_register_static(G_TYPE_INTERFACE, "LaIter",
			       &self_type_info, 0);
    return self_type;
}

static void la_iter_base_init(gpointer g_class)
{
    static gboolean initialized = FALSE;

    if (initialized)
	return;

    self_signals[SELF_SIG_ADDED] =
	g_signal_new("added", LA_TYPE_ITER,
		     G_SIGNAL_RUN_LAST,
		     G_STRUCT_OFFSET(LaIterIface, added), NULL,
		     NULL, g_cclosure_marshal_VOID__POINTER,
		     G_TYPE_NONE, 1, G_TYPE_POINTER);
    self_signals[SELF_SIG_REMOVED] =
	g_signal_new("removed", LA_TYPE_ITER,
		     G_SIGNAL_RUN_LAST,
		     G_STRUCT_OFFSET(LaIterIface, removed), NULL,
		     NULL, g_cclosure_marshal_VOID__POINTER,
		     G_TYPE_NONE, 1, G_TYPE_POINTER);
    self_signals[SELF_SIG_CHANGED] =
	g_signal_new("changed", LA_TYPE_ITER,
		     G_SIGNAL_RUN_LAST,
		     G_STRUCT_OFFSET(LaIterIface, changed), NULL,
		     NULL, g_cclosure_marshal_VOID__POINTER,
		     G_TYPE_NONE, 1, G_TYPE_POINTER);

    initialized = TRUE;
}

/* la-iter.c */

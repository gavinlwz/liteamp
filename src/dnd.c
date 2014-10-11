/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>

#include "liteamp.h"
#include "dnd.h"
#include "mime.h"
#include "util.h"

/*-----------------------------------------------------------------*/

static void dnd_drag_data_received_cb(GtkWidget * widget,
				      GdkDragContext * context, gint x,
				      gint y,
				      GtkSelectionData * selection_data,
				      guint info, guint time,
				      gpointer data);

// drag and drop targets
// playlist receives text/uri-list only from nautilus
static const GtkTargetEntry dnd_targets[] = {
    {MIME_TYPE_URI_LIST, 0, DND_TARGET_ID_URI_LIST},
};

/*-----------------------------------------------------------------*/

void dnd_connect(GtkWidget * widget, GCallback callback,
		 gpointer user_data)
{
    g_signal_connect(widget, "drag-data-received", callback, user_data);
}


void dnd_enable(GtkWidget * widget)
{
    gtk_drag_dest_set(widget, GTK_DEST_DEFAULT_ALL,
		      dnd_targets, G_N_ELEMENTS(dnd_targets),
		      GDK_ACTION_COPY);
}


void dnd_disable(GtkWidget * widget)
{
    gtk_drag_dest_unset(widget);
}

/*dnd.c*/

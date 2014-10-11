/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgnomevfs/gnome-vfs.h>

#include "visualizer.h"
#include "liteamp.h"
#include "dnd.h"
#include "util.h"

/* private-----------------------------------------------*/

// TODO: modulize visualizer

struct _Visualizer {
    GObject parent;

    GtkWidget *drawingarea;	//GtkDrawingArea
    GtkWidget *eventbox;	//GtkEventBox

    VisualizerStyle style;
    gint x;
    gint y;
    gint width;
    gint height;
    GdkDrawable *drawable;
    guint timeout_handler_id;
};

struct _VisualizerClass {
    GObjectClass parent_class;
};

#define UPDATE_INTERVAL (0.1*1000)

static void invalidate(Visualizer * self)
{
    // prone to expose-event
    gtk_widget_queue_draw_area(self->drawingarea,
			       0, 0, self->width, self->height);
}

static void draw_icon(Visualizer * self)
{
    LaPlaylist *playlist;
    const gchar *icon;
    GdkPixbuf *pixbuf;

    playlist = playlist_get_playlist(get_playlist(app));
    if (playlist) {
	icon = la_playlist_get_icon(playlist);
	// load icon image
	pixbuf = la_pixbuf_new(icon, self->width, self->height);
	// when error, try to load fallback image
	if (!pixbuf)
	    pixbuf =
		la_pixbuf_new(PKGDATADIR "/pixmaps/liteamp.png",
			      self->width, self->height);
	if (pixbuf) {
	    gdk_draw_pixbuf(self->drawable, NULL, pixbuf,
			    0, 0, 0, 0, self->width,
			    self->height, GDK_RGB_DITHER_NORMAL, 0, 0);
	    g_object_unref(pixbuf);
	}
    }
}

static void draw_analyzer(Visualizer * self)
{
    static int x = 0;
    GdkGC *gc = gdk_gc_new(self->drawable);

    x = (x + 1) % 100;

    gdk_draw_rectangle(self->drawable, gc, TRUE, 0, 0, x, x);
    g_object_unref(gc);
}

static void draw_scope(Visualizer * self)
{
    static int x = 0;
    GdkGC *gc = gdk_gc_new(self->drawable);

    x = (x + 1) % 64;

    gdk_draw_arc(self->drawable,
		 gc, TRUE, 0, 0, self->width, self->height, 0, x * 360);

    g_object_unref(gc);
}

static void draw(Visualizer * self)
{
    switch (self->style) {
    case VISUALIZER_OFF:
	draw_icon(self);
	break;
    case VISUALIZER_ANALYZER:
	draw_analyzer(self);
	break;
    case VISUALIZER_SCOPE:
	draw_scope(self);
	break;
    }

    invalidate(self);
}

static void clear(Visualizer * self)
{
    gdk_draw_rectangle(self->drawable,
		       self->drawingarea->style->
		       bg_gc[GTK_WIDGET_STATE(self->drawingarea)],
		       TRUE, 0, 0, self->width, self->height);
}

static void init(Visualizer * self)
{
    if (self->drawable != NULL) {
	g_object_unref(self->drawable);
	self->drawable = NULL;
    }
    // prepare offscreen drawable

    self->drawable = gdk_pixmap_new(self->drawingarea->window,
				    self->width, self->height, -1);

    clear(self);
    draw(self);
}

static gboolean on_drawingarea_configure_event(GtkWidget * widget,
					       GdkEventConfigure * event,
					       Visualizer * self)
{
    self->x = event->x;
    self->y = event->y;
    self->width = event->width;
    self->height = event->height;

    init(self);

    return TRUE;
}

static gboolean on_drawingarea_expose_event(GtkWidget * widget,
					    GdkEventExpose * event,
					    Visualizer * self)
{
    if (self->drawable != NULL) {
	// blit offscreen drawable to window
	gdk_draw_drawable(widget->window,
			  widget->style->
			  fg_gc[GTK_WIDGET_STATE(widget)],
			  self->drawable, 0, 0, 0, 0, self->width,
			  self->height);
    }

    return TRUE;
}

static gboolean on_eventbox_button_press_event(GtkWidget * widget,
					       GdkEventButton * event,
					       Visualizer * self)
{
    if (event->button == 1) {
	// change visualizer style
	visualizer_set_style(self, (self->style + 1) % 3);
    }
    return TRUE;
}

static void on_eventbox_drag_data_received(GtkWidget * widget,
					   GdkDragContext * context,
					   gint x, gint y,
					   GtkSelectionData * data,
					   guint info, guint time,
					   Visualizer * self)
{
    GList *uri_list;
    gchar *uri_str;
    gchar *local_path;
    gchar *mime_type;

    GdkPixbuf *pixbuf;
    LaPlaylist *playlist;

    if (info == DND_TARGET_ID_URI_LIST) {
	uri_list = gnome_vfs_uri_list_parse(data->data);
	if (uri_list) {
	    g_assert(uri_list->data);
	    uri_str =
		gnome_vfs_uri_to_string(uri_list->data,
					GNOME_VFS_URI_HIDE_NONE);
	    mime_type = (gchar *) gnome_vfs_get_mime_type(uri_str);
	    local_path = gnome_vfs_get_local_path_from_uri(uri_str);

	    pixbuf = la_pixbuf_new(local_path, 100, 100);
	    if (pixbuf) {
		playlist = playlist_get_playlist(get_playlist(app));
		if (playlist) {
		    la_playlist_set_icon(playlist, local_path);
		    sidebar_write(get_sidebar(app));
		    visualizer_update(self);
		}
		g_object_unref(pixbuf);
	    }

	    g_free(local_path);
	    g_free(uri_str);
	    gnome_vfs_uri_list_free(uri_list);
	}
	//gtk_drag_finish(context, TRUE, FALSE, time);
    }
    //gtk_drag_finish(context, FALSE, FALSE, time);
}

static gboolean on_timeout(Visualizer * self)
{
    draw(self);
}

static void turn_on(Visualizer * self)
{
    if (self->timeout_handler_id == -1 && self->style != VISUALIZER_OFF)
	self->timeout_handler_id =
	    gtk_timeout_add(UPDATE_INTERVAL,
			    (GtkFunction) on_timeout, self);
}

static void turn_off(Visualizer * self)
{
    if (self->timeout_handler_id != -1) {
	gtk_timeout_remove(self->timeout_handler_id);
	self->timeout_handler_id = -1;
    }
}

/* public-----------------------------------------------*/

Visualizer *visualizer_new(void)
{
    Visualizer *self;

    self = g_object_new(TYPE_VISUALIZER, NULL);

    self->drawingarea = lookup_widget(app, "visualizer_drawingarea");
    self->eventbox = lookup_widget(app, "visualizer_eventbox");

    self->x = self->drawingarea->allocation.x;
    self->y = self->drawingarea->allocation.y;
    self->width = self->drawingarea->allocation.width;
    self->height = self->drawingarea->allocation.height;
    self->style = VISUALIZER_OFF;
    self->drawable = NULL;
    self->timeout_handler_id = -1;

    g_signal_connect(self->drawingarea,
		     "configure-event",
		     G_CALLBACK(on_drawingarea_configure_event), self);
    g_signal_connect(self->drawingarea,
		     "expose-event",
		     G_CALLBACK(on_drawingarea_expose_event), self);
    g_signal_connect(self->eventbox,
		     "button-press-event",
		     G_CALLBACK(on_eventbox_button_press_event), self);

    // drag & drop supports
    dnd_connect(GTK_WIDGET(self->eventbox),
		G_CALLBACK(on_eventbox_drag_data_received), self);
    dnd_enable(GTK_WIDGET(self->eventbox));

    return self;
}

inline void visualizer_set_visible(Visualizer * self, gboolean visible)
{
    if (visible) {
	gtk_widget_show(self->eventbox);
	turn_on(self);
    } else {
	gtk_widget_hide(self->eventbox);
	turn_off(self);
    }
}

inline gboolean visualizer_get_visible(Visualizer * self)
{
    return GTK_WIDGET_VISIBLE(self->eventbox);
}

void visualizer_set_style(Visualizer * self, VisualizerStyle style)
{
    turn_off(self);
    self->style = style;
    turn_on(self);

    init(self);
}

VisualizerStyle visualizer_get_style(Visualizer * self)
{
    return self->style;
}

void visualizer_update(Visualizer * self)
{
    clear(self);
    draw(self);
    invalidate(self);
}

/* boilerplates -----------------------------------------*/

LA_TYPE_BOILERPLATE(Visualizer, visualizer, G_TYPE_OBJECT)

/* visualizer.c */

/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "osd.h"
#include "liteamp.h"

/* private-----------------------------------------------*/

struct _Osd {
    GtkWindow parent;

    GtkWidget *image;
    GdkBitmap *mask;
    PangoLayout *layout;

    GdkColor text_color;
    GdkColor shadow_color;
    GdkColor mask_color;

    gint shadow_offset_x;
    gint shadow_offset_y;

    gint width;
    gint height;
};

struct _OsdClass {
    GtkWindowClass parent_class;
};

static void update_layout(Osd * self, const gchar * text)
{
    PangoRectangle rect;

    pango_layout_set_markup(self->layout, text, -1);
    pango_layout_get_pixel_extents(self->layout, NULL, &rect);
    self->width = rect.width + self->shadow_offset_x;
    self->height = rect.height + self->shadow_offset_y;

    gtk_window_resize(GTK_WINDOW(self), self->width, self->height);
}

static void update_image(Osd * self)
{
    GdkPixmap *shape;
    GdkGC *shape_gc;
    GdkPixbuf *shape_pixbuf;
    GdkPixbuf *pixbuf;

    // make shape pixmap from pango layout
    shape =
	gdk_pixmap_new(GTK_WIDGET(self)->window, self->width,
		       self->height, -1);

    shape_gc = gdk_gc_new(shape);

    gdk_gc_set_rgb_fg_color(shape_gc, &self->mask_color);
    gdk_draw_rectangle(shape, shape_gc, TRUE, 0, 0, self->width + 1,
		       self->height + 1);

    gdk_gc_set_rgb_fg_color(shape_gc, &self->shadow_color);
    gdk_draw_layout(shape, shape_gc, self->shadow_offset_x,
		    self->shadow_offset_y, self->layout);

    gdk_gc_set_rgb_fg_color(shape_gc, &self->text_color);
    gdk_draw_layout(shape, shape_gc, 0, 0, self->layout);

    gtk_image_set_from_pixmap(GTK_IMAGE(self->image), shape, NULL);

    // make shape pixbuf from shape pixmap
    shape_pixbuf = gdk_pixbuf_get_from_drawable(NULL, shape, NULL,
						0, 0, 0, 0,
						self->width, self->height);

    g_object_unref(shape);
    g_object_unref(shape_gc);

    // make alpha enabled pixbuf from shape pixbuf
    pixbuf =
	gdk_pixbuf_add_alpha(shape_pixbuf, TRUE, self->mask_color.red,
			     self->mask_color.green,
			     self->mask_color.blue);

    // finally, i got the mask bitmap!
    // it's needed to update_shape().
    // FIXME: somewhat clumsy :( need better/easy/efficient way.
    gdk_pixbuf_render_pixmap_and_mask(pixbuf, NULL, &self->mask, 1);

    g_object_unref(pixbuf);
    g_object_unref(shape_pixbuf);
}

static void update_shape(Osd * self)
{
    gtk_widget_shape_combine_mask(GTK_WIDGET(self), self->mask, 0, 0);
}

/* public------------------------------------------------*/

Osd *osd_new(void)
{
    Osd *self;

    self = g_object_new(TYPE_OSD, NULL);

    GTK_WINDOW(self)->type = GTK_WINDOW_POPUP;

    self->image = gtk_image_new_from_pixmap(NULL, NULL);
    gtk_widget_show(self->image);

    gtk_container_add(GTK_CONTAINER(self), self->image);

    //FIXME:
    gtk_window_move(GTK_WINDOW(self), 0, 0);
    gtk_window_resize(GTK_WINDOW(self), 1, 1);
    gtk_widget_show(GTK_WIDGET(self));

    self->layout = gtk_widget_create_pango_layout(GTK_WIDGET(self), NULL);

    gtk_widget_hide(GTK_WIDGET(self));

    return self;
}

void osd_set_position(Osd * self, gint x, gint y)
{
    if (x < 0)
	x += gdk_screen_width();
    if (y < 0)
	y += gdk_screen_height();

    gtk_window_move(GTK_WINDOW(self), x, y);
}

void osd_set_text(Osd * self, const gchar * text)
{
    update_layout(self, text);
    update_image(self);
    update_shape(self);
}

void osd_get_size(Osd * self, gint * width, gint * height)
{
    *width = self->width;
    *height = self->height;
}

/* boilerplates -----------------------------------------*/

//LA_TYPE_BOILERPLATE_EX(Osd, osd, GTK_TYPE_WINDOW)

static void osd_class_init(OsdClass * klass);
static void osd_instance_init(Osd * self);
static void osd_instance_finalize(GObject * gobject);

GType osd_get_type(void)
{
    static const GTypeInfo self_info = { sizeof(OsdClass), NULL, NULL,
	(GClassInitFunc) osd_class_init, NULL, NULL, sizeof(Osd),
	0,
	(GInstanceInitFunc) osd_instance_init, 0
    };
    static GType self_type = 0;
    if (self_type)
	return self_type;
    self_type =
	g_type_register_static(GTK_TYPE_WINDOW, "Osd", &self_info, 0);
    return self_type;
}

static void osd_class_init(OsdClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = osd_instance_finalize;
}

static void osd_instance_init(Osd * self)
{
    self->image = NULL;
    self->mask = NULL;
    self->layout = NULL;

    gdk_color_parse("#00ff00", &self->text_color);
    gdk_color_parse("#000000", &self->shadow_color);
    gdk_color_parse("#7f7f7f", &self->mask_color);

    self->shadow_offset_x = 2;
    self->shadow_offset_y = 2;

    self->width = 0;
    self->height = 0;
}

static void osd_instance_finalize(GObject * gobject)
{
    Osd *self = OSD(gobject);

    g_object_unref(self->mask);
    g_object_unref(self->layout);
}

/* osd.c */

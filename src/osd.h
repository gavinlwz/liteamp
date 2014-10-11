/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __OSD_H__
#define __OSD_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define TYPE_OSD		(osd_get_type())
#define OSD(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_OSD, Osd))
#define OSD_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_OSD, OsdClass))
#define IS_OSD(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_OSD))
#define IS_OSD_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_OSD))
typedef struct _Osd Osd;
typedef struct _OsdClass OsdClass;

GType osd_get_type(void) G_GNUC_CONST;
Osd *osd_new(void);

void osd_set_position(Osd * self, gint x, gint y);
void osd_set_text(Osd * self, const gchar * text);
void osd_get_size(Osd * self, gint * width, gint * height);

G_END_DECLS
#endif				/* __OSD_H__ */

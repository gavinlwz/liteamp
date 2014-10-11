/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __DND_H__
#define __DND_H__

G_BEGIN_DECLS
// possible drag and drop target id
    typedef enum _DNDTargetID {
    DND_TARGET_ID_URI_LIST,	// from nautilus
} DNDTargetID;

void dnd_connect(GtkWidget * widget, GCallback callback,
		 gpointer user_data);
void dnd_enable(GtkWidget * widget);
void dnd_disable(GtkWidget * widget);

gboolean is_dnd_mime_type_directory(const gchar * mime_type);
gboolean is_dnd_mime_type_music(const gchar * mime_type);
gboolean is_dnd_mime_type_image(const gchar * mime_type);

G_END_DECLS
#endif				/*dnd.h */

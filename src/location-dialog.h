/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __LOCATION_DIALOG_H__
#define __LOCATION_DIALOG_H__

G_BEGIN_DECLS
#define TYPE_LOCATION_DIALOG		(location_dialog_get_type())
#define LOCATION_DIALOG(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_LOCATION_DIALOG, LocationDialog))
#define LOCATION_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_LOCATION_DIALOG, LocationDialogClass))
#define IS_LOCATION_DIALOG(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_LOCATION_DIALOG))
#define IS_LOCATION_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_LOCATION_DIALOG))
typedef struct _LocationDialog LocationDialog;
typedef struct _LocationDialogClass LocationDialogClass;

GType location_dialog_get_type(void) G_GNUC_CONST;
LocationDialog *location_dialog_new(GtkWindow * parent);

void location_dialog_show(LocationDialog * self, const gchar * location);
const gchar *location_dialog_get_location(LocationDialog * self);

G_END_DECLS
#endif /*__LOCATION_DIALOG_H__ */

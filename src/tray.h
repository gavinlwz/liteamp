/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __TRAY_H__
#define __TRAY_H__

#if GTK_CHECK_VERSION(2,1,0)
#define ENABLE_TRAY 1
#else
#undef ENABLE_TRAY
#endif

G_BEGIN_DECLS
#define TYPE_TRAY		(tray_get_type())
#define TRAY(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_TRAY, Tray))
#define TRAY_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_TRAY, TrayClass))
#define IS_TRAY(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_TRAY))
#define IS_TRAY_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_TRAY))
typedef struct _Tray Tray;
typedef struct _TrayClass TrayClass;

GType tray_get_type(void) G_GNUC_CONST;
Tray *tray_new(void);

void tray_set_image(Tray * self, const gchar * image);

G_END_DECLS
#endif /*__TRAY_H__ */

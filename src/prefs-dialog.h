/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __PREFS_DIALOG_H__
#define __PREFS_DIALOG_H__

#include "prefs.h"

G_BEGIN_DECLS
#define TYPE_PREFS_DIALOG		(prefs_dialog_get_type())
#define PREFS_DIALOG(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_PREFS_DIALOG, PrefsDialog))
#define PREFS_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_PREFS_DIALOG, PrefsDialogClass))
#define IS_PREFS_DIALOG(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_PREFS_DIALOG))
#define IS_PREFS_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_PREFS_DIALOG))
typedef struct _PrefsDialog PrefsDialog;
typedef struct _PrefsDialogClass PrefsDialogClass;

GType prefs_dialog_get_type(void) G_GNUC_CONST;
PrefsDialog *prefs_dialog_new(GtkWindow * parent);

void prefs_dialog_show(PrefsDialog * self, Prefs * prefs);
Prefs *prefs_dialog_get_prefs(PrefsDialog * self);

G_BEGIN_DECLS
#endif				/* __PREFS_DIALOG_H__ */

/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __TRACK_DIALOG_H__
#define __TRACK_DIALOG_H__

#include "la-track.h"

G_BEGIN_DECLS
#define TYPE_TRACK_DIALOG		(track_dialog_get_type())
#define TRACK_DIALOG(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_TRACK_DIALOG, TrackDialog))
#define TRACK_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_TRACK_DIALOG, TrackDialogClass))
#define IS_TRACK_DIALOG(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_TRACK_DIALOG))
#define IS_TRACK_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_TRACK_DIALOG))
typedef struct _TrackDialog TrackDialog;
typedef struct _TrackDialogClass TrackDialogClass;

GType track_dialog_get_type(void) G_GNUC_CONST;
TrackDialog *track_dialog_new(GtkWindow * parent);

void track_dialog_show(TrackDialog * self, GSList *tracks);

G_END_DECLS
#endif /*__TRACK_DIALOG_H__ */

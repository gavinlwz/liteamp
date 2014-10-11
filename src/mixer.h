/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __MIXER_H__
#define __MIXER_H__

#include <glib-object.h>

G_BEGIN_DECLS
#define TYPE_MIXER		(mixer_get_type())
#define MIXER(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_MIXER, Mixer))
#define MIXER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_MIXER, MixerClass))
#define IS_MIXER(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_MIXER))
#define IS_MIXER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_MIXER))
typedef struct _Mixer Mixer;
typedef struct _MixerClass MixerClass;

GType mixer_get_type(void) G_GNUC_CONST;
Mixer *mixer_new(void);

void mixer_set_volume(Mixer * self, gint volume);
void mixer_volume_up(Mixer * self);
void mixer_volume_down(Mixer * self);
gint mixer_get_volume(Mixer * self);
void mixer_set_mute(Mixer * self, gboolean mute);

G_END_DECLS
#endif				/*mixer.h */

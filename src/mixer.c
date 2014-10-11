/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>

#include "liteamp.h"
#include "mixer.h"
#include "playback.h"
#include "util.h"

#include <fcntl.h>

#include <sys/ioctl.h>
#ifdef LINUX
#include <linux/soundcard.h>
#else
#include <sys/soundcard.h>
#endif

// these may be specified with compiler options
// such as -DMIXER_DEV_FILE=/dev/mymixer

// Mixer uses "/dev/mixer" device
#ifndef MIXER_DEV_FILE
#define MIXER_DEV_FILE "/dev/mixer"
#endif

// Mixer controls "Master Volume" or "PCM" channel
#ifndef MIXER_CHANNEL
#define MIXER_CHANNEL SOUND_MIXER_VOLUME
//#define MIXER_CHANNEL SOUND_MIXER_PCM
#endif

// Mixer is synchronized with outer world every "0.5" sec
#ifndef MIXER_SYNC_TIME
#define MIXER_SYNC_TIME 500
#endif

/* private-----------------------------------------------*/

struct _Mixer {
    GObject parent;

    int dev;			// mixer device file descriptor
    gint vol_min;
    gint vol_max;
    gint vol_step;
    gint vol_saved;
    gboolean mute;

    guint timeout_handler_id;
};

struct _MixerClass {
    GObjectClass parent_class;
};

static gboolean on_timeout(Mixer * self)
{
    // sync volume with outer world ;)
    update_ui_volume(app);
    return TRUE;
}

/* public-----------------------------------------------*/

Mixer *mixer_new()
{
    Mixer *self;

    self = g_object_new(TYPE_MIXER, NULL);

    self->dev = open(MIXER_DEV_FILE, O_RDWR);
    if (!self->dev)
	g_message("Can't open mixer\n");

    self->vol_min = 0;
    self->vol_max = 100;//0x7f;
    self->vol_step = (self->vol_max - self->vol_min + 1) / 10;

    self->timeout_handler_id = gtk_timeout_add(MIXER_SYNC_TIME,
					       (GtkFunction)on_timeout,
					       self);

    return self;
}

void mixer_set_volume(Mixer * self, gint vol)
{
    vol = CLAMP(vol, self->vol_min, self->vol_max);

    // set software volume
    playback_volume_set_factor((vol / 64.0) * 2.0);

    // use same volume for left and right channels
    vol = ((vol << 8) & 0x7f00) | (vol & 0x007f);
    //vol = ((right << 8) & 0x7f00) | (left & 0x007f);

    // when using software volume, don't set mixer
    if (playback_volume_get_active())
	return;

    if (!self->dev)
	return;
    if (ioctl(self->dev, MIXER_WRITE(MIXER_CHANNEL), &vol) == -1) {
	g_message("failed to set mixer volume!\n");
    }
}

void mixer_volume_up(Mixer * self)
{
    mixer_set_volume(self, mixer_get_volume(self) + self->vol_step);
}


void mixer_volume_down(Mixer * self)
{
    mixer_set_volume(self, mixer_get_volume(self) - self->vol_step);
}

gint mixer_get_volume(Mixer * self)
{
    gint vol;

    if (!self->dev)
	return 0;
    if (ioctl(self->dev, MIXER_READ(MIXER_CHANNEL), &vol) == -1) {
	g_message("failed to get mixer volume!\n");
	return 0;
    }
    //TODO: support balance tune
    //int left = vol & 0x7f;
    //int right = (vol >> 8) & 0x7f;

    // use left channel
    return vol & 0x7f;
}

void mixer_set_mute(Mixer * self, gboolean mute)
{
    self->mute = mute;

    if (mute) {
	self->vol_saved = mixer_get_volume(self);
	mixer_set_volume(self, 0);
    } else {
	mixer_set_volume(self, self->vol_saved);
    }
}

/* boilerplates -----------------------------------------*/

LA_TYPE_BOILERPLATE_EX(Mixer, mixer, G_TYPE_OBJECT)

static void mixer_instance_init(Mixer * self)
{
    self->dev = 0;
    self->vol_min = 0;
    self->vol_max = 0;
    self->vol_step = 0;
    self->vol_saved = 0;
    self->mute = FALSE;
    self->timeout_handler_id = 0;
}

static void mixer_instance_finalize(GObject * gobject)
{
    Mixer *self = MIXER(gobject);

    if (self->timeout_handler_id)
	gtk_timeout_remove(self->timeout_handler_id);

    if (self->dev) {
	close(self->dev);
	self->dev = 0;
    }
}

/*mixer.c*/

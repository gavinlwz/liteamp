/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __PLAYBACK_H__
#define __PLAYBACK_H__

#include <gnome.h>
#include <ao/ao.h>

G_BEGIN_DECLS typedef struct _Playback {
    int opt;
    char *devicetype;
    char *device;
    ao_device *playdevice;
    GMutex *pause_mutex;
    GCond *pause_condv;
    gboolean paused;
    gboolean effect;
    gboolean volume;
    gfloat vol_factor;
} Playback;

enum {
    AO_USE_OSS = 0x00000010,
    AO_USE_SUN = 0x00000020,
    AO_USE_ALSA = 0x00000040,
    AO_USE_ESD = 0x00000080,
    AO_USE_ARTS = 0x00000100,
    AO_USE_NULL = 0x00000200,
    AO_USE_STDOUT = 0x00000400,
    AO_USE_WAV = 0x00000800,
    AO_USE_AU = 0x00001000,
    AO_USE_CDR = 0x00002000,
    AO_USE_USERDEF = 0x00004000,
    AO_USE_ALSA09 = 0x00008000,
};

/*---------------------------------------------------------------------------*/

/* libao interfacing and general audio-out functions */
void playback_init(void);
void playback_quit(void);
int playback_is_big_endian(void);
void playback_play_it(unsigned char *, int length, int nch, int samplerate,
		      int bitrate);
void playback_open_playdevice(int rate, int nch, int bitrate);
void playback_close_playdevice(void);
int playback_is_live(void);
void playback_set_playdevice(char *devicename);
void playback_pause(void);
void playback_unpause(void);

/* get playback infomations */
ao_device *playback_get_playdevice(void);
char *playback_get_devicetype(void);
GList *playback_get_playable_device(void);

/* effect control function */
void playback_effect_set_active(gboolean value);
gboolean playback_effect_get_active(void);

/* Software volume control functions */
void playback_volume_set_factor(gfloat factor);
void playback_volume_set_active(gboolean val);
gboolean playback_volume_get_active(void);

G_END_DECLS
#endif				/*playback.h */

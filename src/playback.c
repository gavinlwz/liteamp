/* vim: set ai ts=8 sw=4 sts=4 noet: */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "decoder.h"
#include "effect.h"
#include "playback.h"
#include "liteamp.h"
#include "prefs.h"

#define AUDIO_DEFAULT "the libao default"

void playback_check_default_playdevice(void);
void playback_check_ao_default_playdevice(void);
static gint playback_volume_adjust(void *buffer, gint length, gint bitrate,
				   gfloat factor);

Playback playback;

static
gint playback_volume_adjust(void *buffer, gint length, gint bitrate,
			    gfloat factor)
{
    static gfloat fval, maxval;
    gint i, val = 0;

    switch (bitrate) {
    case 8:
	maxval = (gfloat) 0x7f;
	break;
    case 16:
	maxval = (gfloat) 0x7fff;
	break;
    case 32:
	maxval = (gfloat) 0x7fffffff;
	break;
    default:
	g_message("bitrate should be 8,16 or 32\n");
	return length;
    }

    for (i = 0; i < length / (bitrate / 8); i++) {
	switch (bitrate) {
	case 8:
	    val = (gint) (*((gint8*) buffer + i));
	    break;
	case 16:
	    val = (gint) (*((gint16*) buffer + i));
	    break;
	case 32:
	    val = (gint) (*((gint32*) buffer + i));
	    break;
	default:
	    g_assert_not_reached();
	    return length;
	}

	fval = (gfloat) val *factor;
	if (fval > maxval)
	    fval = maxval;
	else if (fval < -maxval)
	    fval = -maxval;

	val = (int) fval;

	switch (bitrate) {
	case 8:
	    *((gint8*) buffer + i) = (gint8) val;
	    break;
	case 16:
	    *((gint16*) buffer + i) = (gint16) val;
	    break;
	case 32:
	    *((gint32*) buffer + i) = (gint32) val;
	    break;
	default:
	    g_assert_not_reached();
	    return length;
	}
    }
    return i;
}


void playback_set_playdevice(char *devicename)
{
    if (strcmp(devicename, "oss") == 0) {
	playback.opt |= AO_USE_OSS;
    }

    else if (strcmp(devicename, "sun") == 0) {
	playback.opt |= AO_USE_SUN;
    }

    else if (strcmp(devicename, "alsa") == 0) {
	playback.opt |= AO_USE_ALSA;
    }

    else if (strcmp(devicename, "esd") == 0) {
	playback.opt |= AO_USE_ESD;
    }

    else if (strcmp(devicename, "arts") == 0) {
	playback.opt |= AO_USE_ARTS;
    }

    else if (strcmp(devicename, "alsa09") == 0) {
	playback.opt |= AO_USE_ALSA09;
    }

    else {
	playback.opt |= AO_USE_USERDEF;
	playback.devicetype = strdup(devicename);
    }
}

#if 0
/* we use this if the default ao device fails....maybe */
void playback_check_ao_default_playdevice()
{
    char *default_device;
    ao_info *default_info;

    int driver_id;

    driver_id = ao_default_driver_id();

    if (driver_id < 0) {
	fprintf(stderr, "No default libao driver available.\n");
	exit(1);
    }


}
#endif

void playback_check_default_playdevice()
{
    /* check that no output devices are currently selected */
    if (!
	(playback.
	 opt & (AO_USE_OSS | AO_USE_STDOUT | AO_USE_ALSA | AO_USE_ESD |
		AO_USE_NULL | AO_USE_WAV | AO_USE_ARTS | AO_USE_AU |
		AO_USE_CDR | AO_USE_ALSA09 | AO_USE_USERDEF))) {
	ao_info *default_info;

	/* set default output device & various other bits. this is here
	   so that the device-specific inits in mad.c: open_ao_playdevice 
	   can do their dirty work all on their own */

	if (strcmp(AUDIO_DEFAULT, "the libao default") == 0) {	/* just use the libao-specified default.
								   This is the default when compiling. */
	    //int unset = 1;
	    int driver_id;

	    /* ESD is spawned when executing the ao_default_driver_id routine.
	       This causes a delay and is rather annoying, so we'll disable it here
	       for now. */

#ifdef HAS_GETENV
	    if (getenv("ESD_NO_SPAWN"))
		unset = 0;	/* only unset it later 
				   if it's not already set */
#endif

#ifdef HAS_PUTENV
	    putenv("ESD_NO_SPAWN=1");
#else
#ifdef HAS_SETENV
	    setenv("ESD_NO_SPAWN", "1", 0);
#endif
#endif
	    driver_id = ao_default_driver_id();

#ifdef HAS_PUTENV
	    if (unset)
		putenv("ESD_NO_SPAWN");
#else
#ifdef HAS_UNSETENV
	    if (unset)
		unsetenv("ESD_NO_SPAWN");
#endif
#endif

	    if (driver_id < 0) {
		fprintf(stderr, "No default libao driver available.\n");
		exit(1);
	    }

	    default_info = ao_driver_info(driver_id);

	    playback_set_playdevice(default_info->short_name);
	}

	else {
	    playback_set_playdevice(AUDIO_DEFAULT);
	}

    }
}

int playback_is_live()
{
    int driver_id = 0;

    if (playback.opt & AO_USE_AU) {
	driver_id = ao_driver_id("au");
    }

    else if (playback.opt & AO_USE_CDR) {
	driver_id = ao_driver_id("raw");
    }

    else if (playback.opt & AO_USE_WAV) {
	driver_id = ao_driver_id("wav");
    }

    else if (playback.opt & AO_USE_NULL) {
	driver_id = ao_driver_id("null");
    }

    else if (playback.opt & AO_USE_STDOUT) {
	driver_id = ao_driver_id("raw");
    }

    else if (playback.opt & AO_USE_ESD) {
	driver_id = ao_driver_id("esd");
    }

    else if (playback.opt & AO_USE_ARTS) {
	driver_id = ao_driver_id("arts");
    }

    else if (playback.opt & AO_USE_ALSA) {
	driver_id = ao_driver_id("alsa");
    }

    else if (playback.opt & AO_USE_OSS) {
	driver_id = ao_driver_id("oss");
    }

    else if (playback.opt & AO_USE_SUN) {
	driver_id = ao_driver_id("sun");
    }

    else if (playback.opt & AO_USE_ALSA09) {
	driver_id = ao_driver_id("alsa09");
    }

    else if (playback.opt & AO_USE_USERDEF) {
	driver_id = ao_driver_id(playback.devicetype);
    }

    return (ao_driver_info(driver_id)->type == AO_TYPE_LIVE);
}

void playback_open_playdevice(int srate, int nch, int bitrate)
{
    ao_sample_format format;

    /* Because these can sometimes block, we stop our custom signal handler,
       and restore it afterwards */
    signal(SIGINT, SIG_DFL);

    format.bits = bitrate;
    format.rate = srate;
    format.channels = nch;

    /* mad gives us little-endian data; we swap it on big-endian targets, to
       big-endian format, because that's what most drivers expect. */
    format.byte_format = AO_FMT_NATIVE;

    if (playback.opt & AO_USE_AU) {
	int driver_id = ao_driver_id("au");
	ao_option *ao_options = NULL;

	/* Don't have to check playback.device here: we only define
	   AO_USE_AU when --au <aufile> is defined, and <aufile>
	   is pointd to by playback.device */
	if ((playback.playdevice =
	     ao_open_file(driver_id, playback.device, 1 /*overwrite */ ,
			  &format, ao_options)) == NULL) {
	    fprintf(stderr,
		    "Error opening libao file output driver to write AU data.\n");
	    exit(1);
	}
    }

    else if (playback.opt & AO_USE_CDR) {
	ao_option *ao_options = NULL;
	int driver_id = ao_driver_id("raw");

	/* because CDR is a special format, i.e. headerless PCM, big endian,
	   this is a special case. */
	ao_append_option(&ao_options, "byteorder", "big");

	if ((playback.playdevice =
	     ao_open_file(driver_id, playback.device, 1 /*overwrite */ ,
			  &format, ao_options)) == NULL) {
	    fprintf(stderr,
		    "Error opening libao file output driver to write CDR data.\n");
	    exit(1);
	}
    }

    /* if the user specifies both au and wave, wav will be prefered, so testing
     * later */
    else if (playback.opt & AO_USE_WAV) {
	int driver_id = ao_driver_id("wav");
	ao_option *ao_options = NULL;

	/* Don't have to check playback.device here: we only define
	   AO_USE_WAV when -w <wavfile> is defined, and <wavfile>
	   is pointd to by playback.device */
	if ((playback.playdevice =
	     ao_open_file(driver_id, playback.device, 1 /*overwrite */ ,
			  &format, ao_options)) == NULL) {
	    fprintf(stderr,
		    "Error opening libao wav file driver. (Do you have write permissions?)\n");
	    exit(1);
	}
    }

    else if (playback.opt & AO_USE_NULL) {
	int driver_id = ao_driver_id("null");
	/* null is dirty, create a proper playback struct later */

	if ((playback.playdevice =
	     ao_open_live(driver_id, &format, NULL)) == NULL) {
	    fprintf(stderr,
		    "Error opening libao null driver. (This shouldn't have happened.)\n");
	    exit(1);
	}
    }

    else if (playback.opt & AO_USE_STDOUT) {
	ao_option *ao_options = NULL;
	int driver_id = ao_driver_id("raw");

	/* stdout output is expected to be little-endian generally */
	ao_append_option(&ao_options, "byteorder", "little");

	if ((playback.playdevice =
	     ao_open_file(driver_id, "-", 1 /*overwrite */ ,
			  &format, ao_options)) == NULL) {
	    fprintf(stderr, "Error opening libao raw output driver.\n");
	    exit(1);
	}
    }

    else if (playback.opt & AO_USE_USERDEF) {
	ao_option *ao_options = NULL;
	int driver_id = ao_driver_id(playback.devicetype);

	if (driver_id < 0) {
	    fprintf(stderr, "Can't open unknown ao driver %s\n",
		    playback.devicetype);
	    exit(1);
	}

	if (playback_is_live()) {
	    if (playback.device) {
		fprintf(stderr,
			"Can't set output device to %s for unknown ao plugin %s",
			playback.device, playback.devicetype);
	    }

	    if ((playback.playdevice =
		 ao_open_live(driver_id, &format, ao_options)) == NULL) {
		fprintf(stderr,
			"Error opening unknown libao %s driver. (Is device in use?)\n",
			playback.devicetype);
		exit(1);
	    }
	}

	else {
	    if (playback.device) {
		/* Just assume that playback.device is a filename. The user can shoot 
		   themselves in the foot all they like... */
		if ((playback.playdevice =
		     ao_open_file(driver_id, playback.device,
				  1 /*overwrite */ ,
				  &format, ao_options)) == NULL) {
		    fprintf(stderr,
			    "Error opening unknown libao %s file driver for file %s. (Do you have write permissions?)\n",
			    playback.devicetype, playback.device);
		    exit(1);
		}
	    }

	    else {
		fprintf(stderr,
			"Filename must be specified (with -a filename) for unknown ao driver %s\n",
			playback.devicetype);
	    }
	}
    } else {
	/* Hack-tacular. This code tries to the device as specified; if it can't, it'll
	   fall through to the other devices, trying each in turn. If the user specified
	   a device to use, though, it won't fall through: principle of least surprise */
	int opened = 0;

	if (!opened && playback.opt & AO_USE_ALSA) {
	    ao_option *ao_options = NULL;
	    int driver_id = ao_driver_id("alsa");
	    char *c;

	    if (playback.device) {
		if ((c = strchr(playback.device, ':')) == NULL
		    || strlen(c + 1) < 1) {
		    fprintf(stderr,
			    "Poorly formed ALSA card:device specification %s",
			    playback.device);
		    exit(1);
		}

		*(c++) = '\0';	/* change the : to a null to create two separate strings */

		ao_append_option(&ao_options, "card", playback.device);
		ao_append_option(&ao_options, "dev", c);
	    }

	    if ((playback.playdevice =
		 ao_open_live(driver_id, &format, ao_options)) == NULL) {
		if (playback.device) {
		    fprintf(stderr,
			    "Can't open libao driver with device %s (is device in use?)\n",
			    playback.device);
		    exit(1);
		} else
		    playback.opt |= AO_USE_ALSA09;
	    } else
		opened++;
	}

	if (!opened && playback.opt & AO_USE_ALSA09) {
	    ao_option *ao_options = NULL;
	    int driver_id = ao_driver_id("alsa09");

	    if (playback.device)
		ao_append_option(&ao_options, "dev", playback.device);

	    if ((playback.playdevice =
		 ao_open_live(driver_id, &format, ao_options)) == NULL) {
		if (playback.device) {
		    fprintf(stderr,
			    "Can't open libao driver with device %s (is device in use?)\n",
			    playback.device);
		    exit(1);
		} else
		    playback.opt |= AO_USE_OSS;
	    } else
		opened++;
	}


	if (!opened && playback.opt & AO_USE_OSS) {
	    ao_option *ao_options = NULL;
	    int driver_id = ao_driver_id("oss");

	    if (playback.device)
		ao_append_option(&ao_options, "dsp", playback.device);

	    if ((playback.playdevice =
		 ao_open_live(driver_id, &format, ao_options)) == NULL) {
		if (playback.device) {
		    fprintf(stderr,
			    "Can't open libao driver with device %s (is device in use?)\n",
			    playback.device);
		    exit(1);
		} else
		    playback.opt |= AO_USE_SUN;
	    } else
		opened++;
	}

	if (!opened && playback.opt & AO_USE_SUN) {
	    ao_option *ao_options = NULL;
	    int driver_id = ao_driver_id("sun");

	    if (playback.device)
		ao_append_option(&ao_options, "dev", playback.device);

	    if ((playback.playdevice =
		 ao_open_live(driver_id, &format, ao_options)) == NULL) {
		if (playback.device) {
		    fprintf(stderr,
			    "Can't open libao driver with device %s (is device in use?)\n",
			    playback.device);
		    exit(1);
		} else
		    playback.opt |= AO_USE_ESD;
	    }
	}

	if (!opened && playback.opt & AO_USE_ESD) {
	    ao_option *ao_options = NULL;
	    int driver_id = ao_driver_id("esd");

	    if (playback.device)
		ao_append_option(&ao_options, "host", playback.device);

	    if ((playback.playdevice =
		 ao_open_live(driver_id, &format, ao_options)) == NULL) {
		if (playback.device) {
		    fprintf(stderr,
			    "Can't open libao driver with device %s (is device in use?)\n",
			    playback.device);
		    exit(1);
		} else
		    playback.opt |= AO_USE_ARTS;
	    } else
		opened++;
	}

	if (!opened && playback.opt & AO_USE_ARTS) {
	    ao_option *ao_options = NULL;
	    int driver_id = ao_driver_id("arts");

	    if ((playback.playdevice =
		 ao_open_live(driver_id, &format, ao_options)) == NULL) {
		fprintf(stderr,
			"Can't find a suitable libao driver. (Is device in use?)\n");
		exit(1);
	    }
	}

    }
}

void playback_init(void)
{
    ao_initialize();
    effect_init();

    playback.opt = 0;
    playback.devicetype = 0;
    playback.device = 0;
    playback.pause_mutex = g_mutex_new();
    playback.pause_condv = g_cond_new();
    playback.playdevice = 0;
    playback.paused = FALSE;
    playback.effect = FALSE;
    playback.volume = FALSE;
    playback.vol_factor = 1.0;
}

void playback_quit(void)
{
    effect_quit();
    g_mutex_free(playback.pause_mutex);
    g_cond_free(playback.pause_condv);
    ao_shutdown();
}

int playback_is_big_endian(void)
{
    return ao_is_big_endian();
}

void playback_play_it(unsigned char *buffer, int length, int nch,
		      int samplerate, int bitrate)
{
    g_mutex_lock(playback.pause_mutex);

    // software volume control
    if (playback.volume) {
	playback_volume_adjust(buffer, length, bitrate,
			       playback.vol_factor);
    }
    // effect loop
    if (playback.effect) {
	effect_play_it((gint16 **) & buffer, length, nch, samplerate,
		       bitrate);
    }

    ao_play(playback.playdevice, buffer, length);

    // update play position slider
    update_ui_position(app);

    if (playback.paused)
	// if paused, waiting resume
	g_cond_wait(playback.pause_condv, playback.pause_mutex);
    g_mutex_unlock(playback.pause_mutex);
}

void playback_close_playdevice()
{
    if (playback_get_playdevice()) {
	ao_close(playback.playdevice);
	playback.playdevice = 0;
    }
}

char *playback_get_devicetype()
{
    return playback.devicetype;
}

ao_device *playback_get_playdevice()
{
    return playback.playdevice;
}

GList *playback_get_playable_device()
{
    ao_info **list;
    GList *retval;
    gint count;

    retval = g_list_alloc();
    list = ao_driver_info_list(&count);

    while (count) {
	count--;
	if (list[count]->type == 1) {
	    retval =
		g_list_append(retval, g_strdup(list[count]->short_name));
	}
    }
    return retval;
}

void playback_pause()
{
    if (!playback.paused) {
	playback.paused = TRUE;
    }
}

void playback_unpause()
{
    if (playback.paused) {
	g_cond_signal(playback.pause_condv);
	playback.paused = FALSE;
    }
}

void playback_effect_set_active(gboolean value)
{
    playback.effect = value;
}

gboolean playback_effect_get_active()
{
    return playback.effect;
}

void playback_volume_set_factor(gfloat factor)
{
    playback.vol_factor = factor;
}

void playback_volume_set_active(gboolean value)
{
    playback.volume = value;
}

gboolean playback_volume_get_active()
{
    return playback.volume;
}

/*playback.c*/

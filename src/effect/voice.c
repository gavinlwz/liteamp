#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include "voice.h"

void voice_init(struct _voice *voice)
{
    memset(voice, 0, sizeof(struct _voice));

    /* read from config file ... if fail, set default */
    gnome_config_push_prefix("/liteamp/voice/");
    voice->on = gnome_config_get_bool("on=FALSE");
    gnome_config_pop_prefix();
}

void voice_quit()
{
}

void voice_apply_change(struct _voice *voice)
{
    voice->on =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(voice->chk));

    gnome_config_push_prefix("/liteamp/voice/");
    gnome_config_set_bool("on", voice->on);
    gnome_config_pop_prefix();
}

gint voice_play_it(gint16 ** d, gint length, struct _voice *voice,
		   gint nch, gint samplerate, gint bitrate)
{
    int x;
    int left, right;
    gint16 *dataptr = (gint16 *) * d;


    for (x = 0; x < length; x += 4) {
	left = dataptr[1] - dataptr[0];
	right = dataptr[0] - dataptr[1];
	if (left < -32768)
	    left = -32768;
	if (left > 32767)
	    left = 32767;
	if (right < -32768)
	    right = -32768;
	if (right > 32767)
	    right = 32767;
	dataptr[0] = left;
	dataptr[1] = right;
	dataptr += 2;
    }

    return length;
}

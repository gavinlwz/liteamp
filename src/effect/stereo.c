#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include "stereo.h"

void stereo_init(struct _stereo *stereo)
{

    memset(stereo, 0, sizeof(struct _stereo));

    /* read from config file ... if fail, set default */
    gnome_config_push_prefix("/liteamp/stereo/");

    stereo->on = gnome_config_get_bool("on=FALSE");
    stereo->level = gnome_config_get_float("level=2.0");

    gnome_config_pop_prefix();
}

void stereo_quit()
{

}

void stereo_apply_change(struct _stereo *stereo)
{
    /* get value from widget and write config file */
    stereo->on =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(stereo->chk));
    stereo->level =
	gtk_range_get_value(GTK_RANGE(stereo->hs_stereo_level));

    gnome_config_push_prefix("/liteamp/stereo/");

    gnome_config_set_bool("on", stereo->on);
    gnome_config_set_float("level", stereo->level);

    gnome_config_pop_prefix();
}

gint stereo_play_it(gint16 ** d, gint length, struct _stereo *stereo,
		    gint nch, gint samplerate, gint bitrate)
{
    gint i;
    gfloat value = stereo->level;
    gdouble avg, ldiff, rdiff, tmp, mul;
    gint16 *data = (gint16 *) * d;

    mul = value;

    for (i = 0; i < length / 2; i += 2) {
	avg = (data[i] + data[i + 1]) / 2;
	ldiff = data[i] - avg;
	rdiff = data[i + 1] - avg;

	tmp = avg + ldiff * mul;
	if (tmp < -32768)
	    tmp = -32768;
	if (tmp > 32767)
	    tmp = 32767;
	data[i] = tmp;

	tmp = avg + rdiff * mul;
	if (tmp < -32768)
	    tmp = -32768;
	if (tmp > 32767)
	    tmp = 32767;
	data[i + 1] = tmp;
    }
    return length;
}

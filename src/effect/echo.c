#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include "echo.h"

#define MAX_SRATE 50000
#define MAX_CHANNELS 2
#define BYTES_PS 2
#define BUFFER_SAMPLES (MAX_SRATE * MAX_DELAY / 1000)
#define BUFFER_SHORTS (BUFFER_SAMPLES * MAX_CHANNELS)
#define BUFFER_BYTES (BUFFER_SHORTS * BYTES_PS)

static gint16 *buffer = NULL;
gboolean echo_surround_enable = TRUE;
static int w_ofs;

void echo_init(struct _echo *echo)
{
    memset(echo, 0, sizeof(struct _echo));

    gnome_config_push_prefix("/liteamp/echo/");

    /* read from config file ... if fail, set default */
    echo->on = gnome_config_get_bool("on=FALSE");
    echo->delay = gnome_config_get_int("delay=200");
    echo->feedback = gnome_config_get_int("feedback=30");
    echo->wet = gnome_config_get_int("wet=25");

    gnome_config_pop_prefix();
}

void echo_quit(void)
{
    g_free(buffer);
    buffer = NULL;
}

void echo_apply_change(struct _echo *echo)
{
    /* get value from widget and write config file */
    echo->on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(echo->chk));
    echo->delay =
	(gint) gtk_range_get_value(GTK_RANGE(echo->hs_echo_delay));
    echo->feedback =
	(gint) gtk_range_get_value(GTK_RANGE(echo->hs_echo_feedback));
    echo->wet = (gint) gtk_range_get_value(GTK_RANGE(echo->hs_echo_wet));

    gnome_config_push_prefix("/liteamp/echo/");

    gnome_config_set_bool("on", echo->on);
    gnome_config_set_int("delay", echo->delay);
    gnome_config_set_int("feedback", echo->feedback);
    gnome_config_set_int("wet", echo->wet);

    gnome_config_pop_prefix();
}

gint echo_play_it(gint16 ** d, gint length, struct _echo *echo, gint nch,
		  gint srate, gint bitrate)
{
    gint echo_delay = echo->delay;
    gint echo_feedback = echo->feedback;
    gint echo_volume = echo->wet;
    gint i, in, out, buf, r_ofs, fb_div;
    gint16 *data = (gint16 *) * d;
    static gint old_srate, old_nch;

    if (!buffer)
	buffer = g_malloc0(BUFFER_BYTES + 2);

    if (nch != old_nch || srate != old_srate) {
	memset(buffer, 0, BUFFER_BYTES);
	w_ofs = 0;
	old_nch = nch;
	old_srate = srate;
    }

    if (echo_surround_enable && nch == 2)
	fb_div = 200;
    else
	fb_div = 100;

    r_ofs = w_ofs - (srate * echo_delay / 1000) * nch;
    if (r_ofs < 0)
	r_ofs += BUFFER_SHORTS;

    for (i = 0; i < length / BYTES_PS; i++) {
	in = data[i];
	buf = buffer[r_ofs];
	if (echo_surround_enable && nch == 2) {
	    if (i & 1)
		buf -= buffer[r_ofs - 1];
	    else
		buf -= buffer[r_ofs + 1];
	}
	out = in + buf * echo_volume / 100;
	buf = in + buf * echo_feedback / fb_div;
	out = CLAMP(out, -32768, 32767);
	buf = CLAMP(buf, -32768, 32767);
	buffer[w_ofs] = buf;
	data[i] = out;
	if (++r_ofs >= BUFFER_SHORTS)
	    r_ofs -= BUFFER_SHORTS;
	if (++w_ofs >= BUFFER_SHORTS)
	    w_ofs -= BUFFER_SHORTS;
    }

    return length;
}

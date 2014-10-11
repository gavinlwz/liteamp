#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include "normal.h"

#define GAINSHIFT 5

static int gainCurrent, gainTarget;
static int *peaks = NULL;

void normal_init(struct _normal *normal)
{
    memset(normal, 0, sizeof(struct _normal));

    gnome_config_push_prefix("/liteamp/normalizer/");
    normal->on = gnome_config_get_bool("on=FALSE");
    normal->target = gnome_config_get_int("target=25000");
    normal->gainmax = gnome_config_get_int("gainmax=20");
    normal->gainsmooth = gnome_config_get_int("gainsmooth=8");
    normal->buckets = gnome_config_get_int("buckets=400");
    gnome_config_pop_prefix();
}

void normal_quit()
{
}

void normal_apply_change(struct _normal *normal)
{
    normal->on =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(normal->chk));
    normal->target =
	(gint) gtk_range_get_value(GTK_RANGE(normal->hs_normal_target));
    normal->gainmax =
	(gint) gtk_range_get_value(GTK_RANGE(normal->hs_normal_gainmax));
    normal->gainsmooth = (gint)
	gtk_range_get_value(GTK_RANGE(normal->hs_normal_gainsmooth));
    normal->buckets = (gint)
	gtk_spin_button_get_value(GTK_SPIN_BUTTON
				  (normal->sp_normal_buckets));

    gnome_config_push_prefix("/liteamp/normalizer/");
    gnome_config_set_bool("on", normal->on);
    gnome_config_set_int("target", normal->target);
    gnome_config_set_int("gainmax", normal->gainmax);
    gnome_config_set_int("gainsmooth", normal->gainsmooth);
    gnome_config_set_int("buckets", normal->buckets);
    gnome_config_pop_prefix();
}

void normalizer(gint16 ** data, gint length, gint target, gint gainmax,
		gint gainsmooth, gint buckets)
{
    gint16 *audio = *data, *ap;
    int peak, pos;
    int i;
    int gr, gf, gn;
    static int pn = -1;
    static int clipped = 0;

    if (!peaks)
	return;

    if (pn == -1) {
	for (i = 0; i < buckets; i++)
	    peaks[i] = 0;
    }
    pn = (pn + 1) % buckets;

    /* Determine peak's value and position */
    peak = 1;
    pos = 0;

    ap = audio;
    for (i = 0; i < length / 2; i++) {
	int val = *ap;
	if (val > peak) {
	    peak = val;
	    pos = i;
	} else if (-val > peak) {
	    peak = -val;
	    pos = i;
	}
	ap++;
    }
    peaks[pn] = peak;

    for (i = 0; i < buckets; i++) {
	if (peaks[i] > peak) {
	    peak = peaks[i];
	    pos = 0;
	}
    }

    /* Determine target gain */
    gn = (1 << GAINSHIFT) * target / peak;

    if (gn < (1 << GAINSHIFT))
	gn = 1 << GAINSHIFT;

    gainTarget = (gainTarget * ((1 << gainsmooth) - 1) + gn)
	>> gainsmooth;

    /* Give it an extra insignifigant nudge to counteract possible
     ** rounding error
     */

    if (gn < gainTarget)
	gainTarget--;
    else if (gn > gainTarget)
	gainTarget++;

    if (gainTarget > gainmax << GAINSHIFT)
	gainTarget = GAINSHIFT << GAINSHIFT;


    /* See if a peak is going to clip */
    gn = (1 << GAINSHIFT) * 32768 / peak;

    if (gn < gainTarget) {
	gainTarget = gn;
	pos = 0;

    } else {
	/* We're ramping up, so draw it out over the whole frame */
	pos = length;
    }

    /* Determine gain rate necessary to make target */
    if (!pos)
	pos = 1;

    gr = ((gainTarget - gainCurrent) << 16) / pos;

    /* Do the shiznit */
    gf = gainCurrent << 16;

    ap = audio;
    for (i = 0; i < length / 2; i++) {
	int sample;

	/* Interpolate the gain */
	gainCurrent = gf >> 16;
	if (i < pos)
	    gf += gr;
	else if (i == pos)
	    gf = gainTarget << 16;

	/* Amplify */
	sample = (*ap) * gainCurrent >> GAINSHIFT;
	if (sample < -32768) {
	    clipped += -32768 - sample;
	    sample = -32768;
	} else if (sample > 32767) {
	    clipped += sample - 32767;
	    sample = 32767;
	}
	*ap++ = sample;
    }
}

gint normal_play_it(gint16 ** data, gint length, struct _normal *normal,
		    gint nch, gint samplerate, gint bitrate)
{
    normalizer(data, length, normal->target, normal->gainmax,
	       normal->gainsmooth, normal->buckets);

    return length;
}

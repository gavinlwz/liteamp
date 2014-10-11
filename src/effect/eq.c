#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <math.h>
#include "eq.h"

#define FLT_LOSHELF 1
#define FLT_HISHELF 2
#define FLT_PEAK 3
#define NUM_BANDS 10

/* 1.4 * */
float cut_freq_band[NUM_BANDS] =
    { 30.0, 62.0, 125.0, 250.5, 500.75, 1024.2,
    2000.2, 4028.2, 6000.7, 16000.6
};

//float cut_freq_band[NUM_BANDS]={60.0, 90.0, 135.0, 202.5, 303.75, 455.6,
//                              683.4, 1025.2, 1537.7, 2306.6};                         
int sampling_frequency = 50000;

struct filter_data_str {
    float a0, a1, a2, b1, b2;
    float x1, x2, y1, y2;
    float cutfreq;
};

static struct filter_data_str bandfilter[NUM_BANDS];

void calc_coeff_flt(int type, float smpfreq, float cutfreq, float gain,
		    float Q, struct filter_data_str *flt)
{
    float A, omega, sn, cs, alpha, beta;
    float norm;

    A = pow(10.0, gain / 40.0);
    omega = 2.0 * M_PI * cutfreq / smpfreq;
    sn = sin(omega);
    cs = cos(omega);
    alpha = sn / (2.0 * Q);
    beta = sqrt((A * A + 1.0) / Q - ((A - 1.0) * (A - 1.0)));

    switch (type) {
    case FLT_HISHELF:
	norm = (A + 1.0) - (A - 1.0) * cs + beta * sn;
	flt->a0 = (A * ((A + 1.0) + (A - 1.0) * cs + beta * sn)) / norm;
	flt->a1 = (-2.0 * A * ((A - 1.0) + (A + 1.0) * cs)) / norm;
	flt->a2 = (A * ((A + 1.0) + (A - 1.0) * cs - beta * sn)) / norm;
	flt->b1 = (2.0 * ((A - 1.0) - (A + 1.0) * cs)) / norm;
	flt->b2 = ((A + 1.0) - (A - 1.0) * cs - beta * sn) / norm;
	break;
    case FLT_LOSHELF:
	norm = (A + 1.0) + (A - 1.0) * cs + beta * sn;
	flt->a0 = (A * ((A + 1.0) - (A - 1.0) * cs + beta * sn)) / norm;
	flt->a1 = (2.0 * A * ((A - 1.0) - (A + 1.0) * cs)) / norm;
	flt->a2 = (A * ((A + 1.0) - (A - 1.0) * cs - beta * sn)) / norm;
	flt->b1 = (-2.0 * ((A - 1.0) + (A + 1.0) * cs)) / norm;
	flt->b2 = ((A + 1.0) + (A - 1.0) * cs - beta * sn) / norm;
	break;

    case FLT_PEAK:
	norm = 1.0 + alpha / A;
	flt->a0 = (1.0 + alpha * A) / norm;
	flt->a1 = (-2.0 * cs) / norm;
	flt->a2 = (1.0 - alpha * A) / norm;
	flt->b1 = (-2.0 * cs) / norm;
	flt->b2 = (1.0 - alpha / A) / norm;
	break;
    }
}


void update_adj_mid_calc(float value, struct filter_data_str *pdata)
{
    calc_coeff_flt(FLT_PEAK, (float) sampling_frequency, pdata->cutfreq,
		   value, 1.0, pdata);
}


inline float filter(struct filter_data_str *fltdata, float smp)
{
    float result;

    result =
	(fltdata->a0 * smp) + (fltdata->a1 * fltdata->x1) +
	(fltdata->a2 * fltdata->x2);
    result =
	result - (fltdata->b1 * fltdata->y1) - (fltdata->b2 * fltdata->y2);
    fltdata->x2 = fltdata->x1;
    fltdata->x1 = smp;
    fltdata->y2 = fltdata->y1;
    fltdata->y1 = result;
    return result;
}

gint eq_play_it(gint16 ** buffer, gint size, struct _eq * eq, gint nch,
		gint smpfreq, gint bitrate)
{
    gint16 *smpdata = *buffer;
    int i, fn;
    float tmpsmp;

    if (sampling_frequency != smpfreq) {
	sampling_frequency = smpfreq;
	for (fn = 0; fn < NUM_BANDS; fn++)
	    calc_coeff_flt(FLT_PEAK, (float) smpfreq,
			   bandfilter[fn].cutfreq, eq->eq[fn], 1.0,
			   &(bandfilter[fn]));
    }

    for (i = 0; i < (size >> 1); i++) {
	tmpsmp = (float) smpdata[i];

	for (fn = 0; fn < NUM_BANDS; fn++) {
	    if (eq->eq[fn] != 0.0)
		tmpsmp = filter(&(bandfilter[fn]), tmpsmp);
	}

	if (tmpsmp > 32767.0)
	    tmpsmp = 32767.0;
	else if (tmpsmp < -32768.0)
	    tmpsmp = -32768.0;

	smpdata[i] = (short int) tmpsmp;
    }

    return size;
}

void eq_init(struct _eq *eq)
{
    gint i;
    gchar key[64];

    memset(eq, 0, sizeof(struct _eq));

    gnome_config_push_prefix("/liteamp/equalizer/");
    eq->on = gnome_config_get_bool("on=FALSE");
    eq->on_auto = gnome_config_get_bool("on_auto=FALSE");;
    for (i = 0; i < NUM_BANDS; i++) {
	g_snprintf(key, sizeof(key), "eq_%d=0", i);
	eq->eq[i] = gnome_config_get_float(key);
    }
    gnome_config_pop_prefix();

    for (i = 0; i < NUM_BANDS; i++) {
	bandfilter[i].cutfreq = cut_freq_band[i];
    }
}

void eq_quit()
{
}

void eq_apply_change(struct _eq *eq)
{
    gint i;
    gchar key[64];

    /* get value from widget and write config file */
    gnome_config_push_prefix("/liteamp/equalizer/");

    eq->on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(eq->chk));
    eq->on_auto =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(eq->chk_auto));
    gnome_config_set_bool("on", eq->on);
    gnome_config_set_bool("on_auto", eq->on_auto);

    for (i = 0; i < NUM_BANDS; i++) {
	g_snprintf(key, sizeof(key), "eq_%d", i);
	eq->eq[i] = (float) gtk_range_get_value(GTK_RANGE(eq->vs_eq[i]));
	gnome_config_set_float(key, eq->eq[i]);
	update_adj_mid_calc(eq->eq[i], &(bandfilter[i]));
    }
    gnome_config_pop_prefix();
}

void eq_set_preset(struct _eq *eq, gfloat preset[])
{
    gint i;
    for (i = 0; i < 10; i++) {
	gtk_range_set_value(GTK_RANGE(eq->vs_eq[i]), preset[i]);
    }
}

void eq_value_changed(struct _eq *eq, gint no)
{
    if (eq->on_auto) {
	eq->eq[no] =
	    (gfloat) gtk_range_get_value(GTK_RANGE(eq->vs_eq[no]));
	update_adj_mid_calc(eq->eq[no], &(bandfilter[no]));
    }
}

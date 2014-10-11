#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include "effect.h"

gboolean on_effect_delete_event(GtkWidget * widget, GdkEvent * event,
				gpointer user_data);
void on_cancelbutton_clicked(GtkButton * button, gpointer user_data);
void on_applybutton_clicked(GtkButton * button, gpointer user_data);
void on_okbutton_clicked(GtkButton * button, gpointer user_data);

void on_btn_eq_flat_clicked(GtkButton * button, gpointer user_data);
void on_btn_eq_rock_clicked(GtkButton * button, gpointer user_data);
void on_btn_eq_pop_clicked(GtkButton * button, gpointer user_data);
void on_btn_eq_laptop_clicked(GtkButton * button, gpointer user_data);
void on_btn_eq_classic_clicked(GtkButton * button, gpointer user_data);
void on_vs_eq_value_changed(GtkRange * range, gpointer user_data);
gboolean on_vs_eq_double_clicked(GtkWidget * widget,
				 GdkEventButton * event,
				 gpointer user_data);

static GtkWidget *effect_dialog = NULL;
static struct _effect effect;

void effect_init()
{
    echo_init(&effect.echo);
    stereo_init(&effect.stereo);
    voice_init(&effect.voice);
    normal_init(&effect.normal);
    eq_init(&effect.eq);
}

void effect_quit()
{
    echo_quit();
    stereo_quit();
    voice_quit();
    normal_quit();
    eq_quit();
}

void effect_apply_change()
{
    echo_apply_change(&effect.echo);
    stereo_apply_change(&effect.stereo);
    normal_apply_change(&effect.normal);
    voice_apply_change(&effect.voice);
    eq_apply_change(&effect.eq);
}

gint effect_play_it(gint16 ** buffer, gint length, gint nch,
		    gint samplerate, gint bitrate)
{
    gint retval = 0;

    if (effect.eq.on) {
	retval =
	    eq_play_it(buffer, length, &effect.eq, nch, samplerate,
		       bitrate);
    }

    if (effect.normal.on) {
	retval =
	    normal_play_it(buffer, length, &effect.normal, nch, samplerate,
			   bitrate);
    }

    if (effect.voice.on) {
	retval =
	    voice_play_it(buffer, length, &effect.voice, nch, samplerate,
			  bitrate);
    }

    if (effect.stereo.on) {
	retval =
	    stereo_play_it(buffer, length, &effect.stereo, nch, samplerate,
			   bitrate);
    }

    if (effect.echo.on) {
	retval =
	    echo_play_it(buffer, length, &effect.echo, nch, samplerate,
			 bitrate);
    }
    return retval;
}


/* Callbacks */
gboolean
on_effect_delete_event(GtkWidget * widget,
		       GdkEvent * event, gpointer user_data)
{
    gtk_widget_destroy(effect_dialog);
    effect_dialog = 0;
    return FALSE;
}


void on_cancelbutton_clicked(GtkButton * button, gpointer user_data)
{
    gtk_widget_destroy(effect_dialog);
    effect_dialog = 0;
}


void on_applybutton_clicked(GtkButton * button, gpointer user_data)
{
    effect_apply_change();
}


void on_okbutton_clicked(GtkButton * button, gpointer user_data)
{
    effect_apply_change();
    gtk_widget_destroy(effect_dialog);
    effect_dialog = 0;
}


void on_btn_eq_flat_clicked(GtkButton * button, gpointer user_data)
{
    gfloat data[] = EQ_FLAT;
    eq_set_preset(&effect.eq, data);
}

void on_btn_eq_rock_clicked(GtkButton * button, gpointer user_data)
{
    gfloat data[] = EQ_ROCK;
    eq_set_preset(&effect.eq, data);
}

void on_btn_eq_pop_clicked(GtkButton * button, gpointer user_data)
{
    gfloat data[] = EQ_POP;
    eq_set_preset(&effect.eq, data);
}

void on_btn_eq_laptop_clicked(GtkButton * button, gpointer user_data)
{
    gfloat data[] = EQ_LAPTOP;
    eq_set_preset(&effect.eq, data);
}

void on_btn_eq_classic_clicked(GtkButton * button, gpointer user_data)
{
    gfloat data[] = EQ_CLASSIC;
    eq_set_preset(&effect.eq, data);
}


void on_vs_eq_value_changed(GtkRange * range, gpointer user_data)
{
    gint no = GPOINTER_TO_INT(user_data);
    eq_value_changed(&effect.eq, no);
}

gboolean on_vs_eq_double_clicked(GtkWidget * widget,
				 GdkEventButton * event,
				 gpointer user_data)
{
    gint no = GPOINTER_TO_INT(user_data);
    if (event->type == GDK_2BUTTON_PRESS) {
	gtk_range_set_value(GTK_RANGE(widget), 0);
    }
    return FALSE;
}

void effect_show_dialog(void)
{

    gint i;

    /* echo widgets */
    GtkWidget *frame_echo;
    GtkWidget *chk_echo;
    GtkWidget *hs_echo_delay, *hs_echo_feedback, *hs_echo_wet;

    /* stereo enhance widgets */
    GtkWidget *frame_stereo;
    GtkWidget *chk_stereo;
    GtkWidget *hs_stereo_level;

    /* voice removal widgets */
    GtkWidget *frame_voice;
    GtkWidget *chk_voice;

    /* normalizer widgets */
    GtkWidget *frame_normal;
    GtkWidget *chk_normal;
    GtkWidget *sp_normal_buckets;
    GtkWidget *hs_normal_target, *hs_normal_gainmax, *hs_normal_gainsmooth;

    /* equalizer widgets */
    GtkWidget *frame_eq;
    GtkWidget *chk_eq;
    GtkWidget *chk_auto;
    GtkWidget *btn_eq_flat, *btn_eq_rock, *btn_eq_pop, *btn_eq_laptop,
	*btn_eq_classic;
    GtkWidget *vs_eq[10];

    /* stock button widgets */
    GtkWidget *cancelbutton;
    GtkWidget *applybutton;
    GtkWidget *okbutton;

    GtkWidget *dialog_vbox1;
    GtkWidget *dialog_action_area1;
    GtkObject *sp_normal_buckets_adj;

    GtkWidget *hseparator1, *hseparator2, *hseparator3, *hseparator4;
    GtkWidget *hbox1, *hbox2, *hbox3, *hbox4, *hbox5, *hbox6, *hbox7,
	*hbox8, *hbox9, *hbox10;
    GtkWidget *hbox11, *hbox12, *hbox13;
    GtkWidget *vbox1, *vbox2, *vbox3, *vbox5, *vbox6, *vbox7, *vbox8,
	*vbox18, *vbox19;
    GtkWidget *label4, *label5, *label6, *label7, *label8, *label9,
	*label10, *label11, *label12;
    GtkWidget *label13, *label14, *label15, *label16, *label17, *label18,
	*label19, *label20;
    GtkWidget *label21, *label22, *label23, *label24, *label25, *label26;
    GtkWidget *hbox_eq1, *hbox_eq2;

    if (effect_dialog != NULL)
	return;

    effect_dialog = gtk_dialog_new();
    gtk_container_set_border_width(GTK_CONTAINER(effect_dialog), 2);
    gtk_window_set_title(GTK_WINDOW(effect_dialog), _("Effect"));
    gtk_window_set_position(GTK_WINDOW(effect_dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(effect_dialog), 500, -1);

    dialog_vbox1 = GTK_DIALOG(effect_dialog)->vbox;
    gtk_widget_show(dialog_vbox1);

    hbox1 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox1);
    gtk_box_pack_start(GTK_BOX(dialog_vbox1), hbox1, TRUE, TRUE, 2);

    vbox6 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox6);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox6, TRUE, TRUE, 0);

    hbox10 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox10);
    gtk_box_pack_start(GTK_BOX(vbox6), hbox10, FALSE, TRUE, 0);

    frame_echo = gtk_frame_new(NULL);
    gtk_widget_show(frame_echo);
    gtk_box_pack_start(GTK_BOX(hbox10), frame_echo, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frame_echo), 2);

    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(frame_echo), vbox1);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), 4);

    chk_echo = gtk_check_button_new_with_mnemonic(_("Enable Echo"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_echo),
				 effect.echo.on);
    gtk_widget_show(chk_echo);
    gtk_box_pack_start(GTK_BOX(vbox1), chk_echo, FALSE, FALSE, 0);

    hseparator3 = gtk_hseparator_new();
    gtk_widget_show(hseparator3);
    gtk_box_pack_start(GTK_BOX(vbox1), hseparator3, FALSE, FALSE, 1);

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox2);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox2, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox2), 1);

    label4 = gtk_label_new(_("Deley(ms)"));
    gtk_widget_show(label4);
    gtk_box_pack_start(GTK_BOX(hbox2), label4, FALSE, FALSE, 2);
    gtk_label_set_justify(GTK_LABEL(label4), GTK_JUSTIFY_LEFT);

    hs_echo_delay =
	gtk_hscale_new(GTK_ADJUSTMENT
		       (gtk_adjustment_new
			(effect.echo.delay, 0, MAX_DELAY, 1, 0, 0)));
    gtk_widget_show(hs_echo_delay);
    gtk_box_pack_start(GTK_BOX(vbox1), hs_echo_delay, FALSE, TRUE, 0);
    gtk_scale_set_value_pos(GTK_SCALE(hs_echo_delay), GTK_POS_RIGHT);
    gtk_scale_set_digits(GTK_SCALE(hs_echo_delay), 0);

    hbox3 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox3);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox3, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox3), 1);

    label5 = gtk_label_new(_("Feedback(%)"));
    gtk_widget_show(label5);
    gtk_box_pack_start(GTK_BOX(hbox3), label5, FALSE, FALSE, 2);
    gtk_label_set_justify(GTK_LABEL(label5), GTK_JUSTIFY_LEFT);

    hs_echo_feedback =
	gtk_hscale_new(GTK_ADJUSTMENT
		       (gtk_adjustment_new
			(effect.echo.feedback, 0, 100, 1, 0, 0)));
    gtk_widget_show(hs_echo_feedback);
    gtk_box_pack_start(GTK_BOX(vbox1), hs_echo_feedback, FALSE, TRUE, 0);
    gtk_scale_set_value_pos(GTK_SCALE(hs_echo_feedback), GTK_POS_RIGHT);
    gtk_scale_set_digits(GTK_SCALE(hs_echo_feedback), 0);

    hbox4 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox4);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox4, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox4), 1);

    label6 = gtk_label_new(_("Wet(%)"));
    gtk_widget_show(label6);
    gtk_box_pack_start(GTK_BOX(hbox4), label6, FALSE, TRUE, 2);
    gtk_label_set_justify(GTK_LABEL(label6), GTK_JUSTIFY_LEFT);

    hs_echo_wet =
	gtk_hscale_new(GTK_ADJUSTMENT
		       (gtk_adjustment_new
			(effect.echo.wet, 0, 100, 1, 0, 0)));
    gtk_widget_show(hs_echo_wet);
    gtk_box_pack_start(GTK_BOX(vbox1), hs_echo_wet, FALSE, TRUE, 0);
    gtk_scale_set_value_pos(GTK_SCALE(hs_echo_wet), GTK_POS_RIGHT);
    gtk_scale_set_digits(GTK_SCALE(hs_echo_wet), 0);

    label8 = gtk_label_new(_("Echo"));
    gtk_widget_show(label8);
    gtk_frame_set_label_widget(GTK_FRAME(frame_echo), label8);
    gtk_label_set_justify(GTK_LABEL(label8), GTK_JUSTIFY_LEFT);

    vbox7 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox7);
    gtk_box_pack_start(GTK_BOX(hbox10), vbox7, TRUE, TRUE, 0);

    frame_stereo = gtk_frame_new(NULL);
    gtk_widget_show(frame_stereo);
    gtk_box_pack_start(GTK_BOX(vbox7), frame_stereo, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frame_stereo), 2);

    vbox2 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox2);
    gtk_container_add(GTK_CONTAINER(frame_stereo), vbox2);
    gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);

    chk_stereo =
	gtk_check_button_new_with_mnemonic(_("Enable Stereo Enhance"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_stereo),
				 effect.stereo.on);
    gtk_widget_show(chk_stereo);
    gtk_box_pack_start(GTK_BOX(vbox2), chk_stereo, FALSE, FALSE, 0);

    hseparator2 = gtk_hseparator_new();
    gtk_widget_show(hseparator2);
    gtk_box_pack_start(GTK_BOX(vbox2), hseparator2, FALSE, FALSE, 1);

    hbox5 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox5);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox5, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox5), 1);

    label7 = gtk_label_new(_("Effect Level"));
    gtk_widget_show(label7);
    gtk_box_pack_start(GTK_BOX(hbox5), label7, FALSE, FALSE, 2);
    gtk_label_set_justify(GTK_LABEL(label7), GTK_JUSTIFY_LEFT);

    hs_stereo_level =
	gtk_hscale_new(GTK_ADJUSTMENT
		       (gtk_adjustment_new
			(effect.stereo.level, 0, 8, 0.1, 0, 0)));
    gtk_widget_show(hs_stereo_level);
    gtk_box_pack_start(GTK_BOX(vbox2), hs_stereo_level, FALSE, TRUE, 0);
    gtk_scale_set_value_pos(GTK_SCALE(hs_stereo_level), GTK_POS_RIGHT);

    label9 = gtk_label_new(_("Stereo Enhance"));
    gtk_widget_show(label9);
    gtk_frame_set_label_widget(GTK_FRAME(frame_stereo), label9);
    gtk_label_set_justify(GTK_LABEL(label9), GTK_JUSTIFY_LEFT);

    frame_voice = gtk_frame_new(NULL);
    gtk_widget_show(frame_voice);
    gtk_box_pack_start(GTK_BOX(vbox7), frame_voice, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frame_voice), 2);

    vbox3 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox3);
    gtk_container_add(GTK_CONTAINER(frame_voice), vbox3);
    gtk_container_set_border_width(GTK_CONTAINER(vbox3), 4);

    chk_voice =
	gtk_check_button_new_with_mnemonic(_("Enable Voice Removal"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_voice),
				 effect.voice.on);
    gtk_widget_show(chk_voice);
    gtk_box_pack_start(GTK_BOX(vbox3), chk_voice, FALSE, FALSE, 0);

    hseparator1 = gtk_hseparator_new();
    gtk_widget_show(hseparator1);
    gtk_box_pack_start(GTK_BOX(vbox3), hseparator1, FALSE, FALSE, 1);

    label10 = gtk_label_new(_("Voice Removal"));
    gtk_widget_show(label10);
    gtk_frame_set_label_widget(GTK_FRAME(frame_voice), label10);
    gtk_label_set_justify(GTK_LABEL(label10), GTK_JUSTIFY_LEFT);

    frame_normal = gtk_frame_new(NULL);
    gtk_widget_show(frame_normal);
    gtk_box_pack_start(GTK_BOX(hbox10), frame_normal, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frame_normal), 2);

    vbox5 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox5);
    gtk_container_add(GTK_CONTAINER(frame_normal), vbox5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox5), 4);

    chk_normal =
	gtk_check_button_new_with_mnemonic(_("Enable Normalizer"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_normal),
				 effect.normal.on);
    gtk_widget_show(chk_normal);
    gtk_box_pack_start(GTK_BOX(vbox5), chk_normal, FALSE, FALSE, 0);

    hseparator4 = gtk_hseparator_new();
    gtk_widget_show(hseparator4);
    gtk_box_pack_start(GTK_BOX(vbox5), hseparator4, TRUE, TRUE, 1);

    hbox9 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox9);
    gtk_box_pack_start(GTK_BOX(vbox5), hbox9, TRUE, TRUE, 0);

    label15 = gtk_label_new(_("Window Size"));
    gtk_widget_show(label15);
    gtk_box_pack_start(GTK_BOX(hbox9), label15, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label15), GTK_JUSTIFY_LEFT);

    sp_normal_buckets_adj =
	gtk_adjustment_new(effect.normal.buckets, 0, 1024, 1, 10, 10);
    sp_normal_buckets =
	gtk_spin_button_new(GTK_ADJUSTMENT(sp_normal_buckets_adj), 1, 0);
    gtk_widget_show(sp_normal_buckets);
    gtk_box_pack_start(GTK_BOX(hbox9), sp_normal_buckets, TRUE, TRUE, 0);

    hbox6 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox6);
    gtk_box_pack_start(GTK_BOX(vbox5), hbox6, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox6), 1);

    label12 = gtk_label_new(_("Target Audio Level"));
    gtk_widget_show(label12);
    gtk_box_pack_start(GTK_BOX(hbox6), label12, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label12), GTK_JUSTIFY_LEFT);

    hs_normal_target =
	gtk_hscale_new(GTK_ADJUSTMENT
		       (gtk_adjustment_new
			(effect.normal.target, 0, 99999, 1, 0, 0)));
    gtk_widget_show(hs_normal_target);
    gtk_box_pack_start(GTK_BOX(vbox5), hs_normal_target, FALSE, TRUE, 0);
    gtk_scale_set_value_pos(GTK_SCALE(hs_normal_target), GTK_POS_RIGHT);
    gtk_scale_set_digits(GTK_SCALE(hs_normal_target), 0);

    hbox7 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox7);
    gtk_box_pack_start(GTK_BOX(vbox5), hbox7, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox7), 1);

    label13 = gtk_label_new(_("Maximum Gain"));
    gtk_widget_show(label13);
    gtk_box_pack_start(GTK_BOX(hbox7), label13, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label13), GTK_JUSTIFY_LEFT);

    hs_normal_gainmax =
	gtk_hscale_new(GTK_ADJUSTMENT
		       (gtk_adjustment_new
			(effect.normal.gainmax, 0, 9999, 1, 0, 0)));
    gtk_widget_show(hs_normal_gainmax);
    gtk_box_pack_start(GTK_BOX(vbox5), hs_normal_gainmax, FALSE, TRUE, 0);
    gtk_scale_set_value_pos(GTK_SCALE(hs_normal_gainmax), GTK_POS_RIGHT);
    gtk_scale_set_digits(GTK_SCALE(hs_normal_gainmax), 0);

    hbox8 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox8);
    gtk_box_pack_start(GTK_BOX(vbox5), hbox8, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox8), 1);

    label14 = gtk_label_new(_("Gain Smooth"));
    gtk_widget_show(label14);
    gtk_box_pack_start(GTK_BOX(hbox8), label14, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label14), GTK_JUSTIFY_LEFT);

    hs_normal_gainsmooth =
	gtk_hscale_new(GTK_ADJUSTMENT
		       (gtk_adjustment_new
			(effect.normal.gainsmooth, 0, 10, 1, 0, 0)));
    gtk_widget_show(hs_normal_gainsmooth);
    gtk_box_pack_start(GTK_BOX(vbox5), hs_normal_gainsmooth, FALSE, TRUE,
		       0);
    gtk_scale_set_value_pos(GTK_SCALE(hs_normal_gainsmooth),
			    GTK_POS_RIGHT);
    gtk_scale_set_digits(GTK_SCALE(hs_normal_gainsmooth), 0);

    label11 = gtk_label_new(_("Normalizer"));
    gtk_widget_show(label11);
    gtk_frame_set_label_widget(GTK_FRAME(frame_normal), label11);
    gtk_label_set_justify(GTK_LABEL(label11), GTK_JUSTIFY_LEFT);

    frame_eq = gtk_frame_new(NULL);
    gtk_widget_show(frame_eq);
    gtk_box_pack_start(GTK_BOX(vbox6), frame_eq, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frame_eq), 2);

    hbox11 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox11);
    gtk_container_add(GTK_CONTAINER(frame_eq), hbox11);
    gtk_container_set_border_width(GTK_CONTAINER(hbox11), 4);

    hbox13 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox13);
    gtk_box_pack_start(GTK_BOX(hbox11), hbox13, TRUE, TRUE, 0);

    vbox19 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox19);
    gtk_box_pack_start(GTK_BOX(hbox13), vbox19, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox19), 4);

    chk_eq = gtk_check_button_new_with_mnemonic(_("Enable Equalizer"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_eq), effect.eq.on);
    gtk_widget_show(chk_eq);
    gtk_box_pack_start(GTK_BOX(vbox19), chk_eq, FALSE, FALSE, 0);

    chk_auto = gtk_check_button_new_with_mnemonic(_("Auto"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_auto),
				 effect.eq.on_auto);
    gtk_widget_show(chk_auto);
    gtk_box_pack_start(GTK_BOX(vbox19), chk_auto, FALSE, FALSE, 0);

    vbox18 = gtk_vbox_new(TRUE, 0);
    gtk_widget_show(vbox18);
    gtk_box_pack_start(GTK_BOX(hbox13), vbox18, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox18), 4);

    btn_eq_flat = gtk_button_new_with_mnemonic(_("Flat"));
    gtk_widget_show(btn_eq_flat);
    gtk_box_pack_start(GTK_BOX(vbox18), btn_eq_flat, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(btn_eq_flat), 2);

    btn_eq_rock = gtk_button_new_with_mnemonic(_("Rock"));
    gtk_widget_show(btn_eq_rock);
    gtk_box_pack_start(GTK_BOX(vbox18), btn_eq_rock, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(btn_eq_rock), 2);

    btn_eq_pop = gtk_button_new_with_mnemonic(_("Pop"));
    gtk_widget_show(btn_eq_pop);
    gtk_box_pack_start(GTK_BOX(vbox18), btn_eq_pop, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(btn_eq_pop), 2);

    btn_eq_laptop = gtk_button_new_with_mnemonic(_("Laptop Speaker"));
    gtk_widget_show(btn_eq_laptop);
    gtk_box_pack_start(GTK_BOX(vbox18), btn_eq_laptop, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(btn_eq_laptop), 2);

    btn_eq_classic = gtk_button_new_with_mnemonic(_("Classic"));
    gtk_widget_show(btn_eq_classic);
    gtk_box_pack_start(GTK_BOX(vbox18), btn_eq_classic, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(btn_eq_classic), 2);

    hbox12 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox12);
    gtk_box_pack_start(GTK_BOX(hbox11), hbox12, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox12), 4);

    vbox8 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox8);
    gtk_box_pack_start(GTK_BOX(hbox12), vbox8, TRUE, TRUE, 0);



    hbox_eq1 = gtk_hbox_new(TRUE, 0);
    gtk_widget_show(hbox_eq1);
    gtk_box_pack_start(GTK_BOX(vbox8), hbox_eq1, TRUE, TRUE, 0);

    hbox_eq2 = gtk_hbox_new(TRUE, 0);
    gtk_widget_show(hbox_eq2);
    gtk_box_pack_start(GTK_BOX(vbox8), hbox_eq2, FALSE, TRUE, 0);

    for (i = 0; i < 10; i++) {
	vs_eq[i] =
	    gtk_vscale_new(GTK_ADJUSTMENT
			   (gtk_adjustment_new
			    (effect.eq.eq[i], -10.0, 10.0, 0.1, 0.5, 0)));
	gtk_widget_show(vs_eq[i]);
	gtk_box_pack_start(GTK_BOX(hbox_eq1), vs_eq[i], TRUE, TRUE, 0);
	gtk_scale_set_draw_value(GTK_SCALE(vs_eq[i]), FALSE);
	gtk_scale_set_digits(GTK_SCALE(vs_eq[i]), 2);
	gtk_range_set_inverted(GTK_RANGE(vs_eq[i]), TRUE);
	gtk_range_set_update_policy(GTK_RANGE(vs_eq[i]),
				    GTK_UPDATE_DISCONTINUOUS);
    }

    label17 = gtk_label_new(_("30"));
    gtk_widget_show(label17);
    gtk_box_pack_start(GTK_BOX(hbox_eq2), label17, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label17), GTK_JUSTIFY_LEFT);

    label18 = gtk_label_new(_("62"));
    gtk_widget_show(label18);
    gtk_box_pack_start(GTK_BOX(hbox_eq2), label18, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label18), GTK_JUSTIFY_LEFT);

    label19 = gtk_label_new(_("125"));
    gtk_widget_show(label19);
    gtk_box_pack_start(GTK_BOX(hbox_eq2), label19, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label19), GTK_JUSTIFY_LEFT);

    label20 = gtk_label_new(_("250"));
    gtk_widget_show(label20);
    gtk_box_pack_start(GTK_BOX(hbox_eq2), label20, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label20), GTK_JUSTIFY_LEFT);

    label21 = gtk_label_new(_("500"));
    gtk_widget_show(label21);
    gtk_box_pack_start(GTK_BOX(hbox_eq2), label21, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label21), GTK_JUSTIFY_LEFT);

    label22 = gtk_label_new(_("1K"));
    gtk_widget_show(label22);
    gtk_box_pack_start(GTK_BOX(hbox_eq2), label22, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label22), GTK_JUSTIFY_LEFT);

    label23 = gtk_label_new(_("2K"));
    gtk_widget_show(label23);
    gtk_box_pack_start(GTK_BOX(hbox_eq2), label23, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label23), GTK_JUSTIFY_LEFT);

    label24 = gtk_label_new(_("4K"));
    gtk_widget_show(label24);
    gtk_box_pack_start(GTK_BOX(hbox_eq2), label24, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label24), GTK_JUSTIFY_LEFT);

    label25 = gtk_label_new(_("6K"));
    gtk_widget_show(label25);
    gtk_box_pack_start(GTK_BOX(hbox_eq2), label25, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label25), GTK_JUSTIFY_LEFT);

    label26 = gtk_label_new(_("16K"));
    gtk_widget_show(label26);
    gtk_box_pack_start(GTK_BOX(hbox_eq2), label26, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label26), GTK_JUSTIFY_LEFT);

    label16 = gtk_label_new(_("Equalizer"));
    gtk_widget_show(label16);
    gtk_frame_set_label_widget(GTK_FRAME(frame_eq), label16);
    gtk_label_set_justify(GTK_LABEL(label16), GTK_JUSTIFY_LEFT);

    dialog_action_area1 = GTK_DIALOG(effect_dialog)->action_area;
    gtk_widget_show(dialog_action_area1);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(dialog_action_area1),
			      GTK_BUTTONBOX_END);

    cancelbutton = gtk_button_new_from_stock("gtk-cancel");
    gtk_widget_show(cancelbutton);
    gtk_dialog_add_action_widget(GTK_DIALOG(effect_dialog), cancelbutton,
				 GTK_RESPONSE_CANCEL);
    GTK_WIDGET_SET_FLAGS(cancelbutton, GTK_CAN_DEFAULT);

    applybutton = gtk_button_new_from_stock("gtk-apply");
    gtk_widget_show(applybutton);
    gtk_dialog_add_action_widget(GTK_DIALOG(effect_dialog), applybutton,
				 GTK_RESPONSE_APPLY);
    GTK_WIDGET_SET_FLAGS(applybutton, GTK_CAN_DEFAULT);

    okbutton = gtk_button_new_from_stock("gtk-ok");
    gtk_widget_show(okbutton);
    gtk_dialog_add_action_widget(GTK_DIALOG(effect_dialog), okbutton,
				 GTK_RESPONSE_OK);
    GTK_WIDGET_SET_FLAGS(okbutton, GTK_CAN_DEFAULT);

    g_signal_connect((gpointer) effect_dialog, "delete_event",
		     G_CALLBACK(on_effect_delete_event), NULL);
    g_signal_connect((gpointer) btn_eq_flat, "clicked",
		     G_CALLBACK(on_btn_eq_flat_clicked), NULL);
    g_signal_connect((gpointer) btn_eq_rock, "clicked",
		     G_CALLBACK(on_btn_eq_rock_clicked), NULL);
    g_signal_connect((gpointer) btn_eq_pop, "clicked",
		     G_CALLBACK(on_btn_eq_pop_clicked), NULL);
    g_signal_connect((gpointer) btn_eq_laptop, "clicked",
		     G_CALLBACK(on_btn_eq_laptop_clicked), NULL);
    g_signal_connect((gpointer) btn_eq_classic, "clicked",
		     G_CALLBACK(on_btn_eq_classic_clicked), NULL);

    for (i = 0; i < 10; i++) {
	g_signal_connect((gpointer) vs_eq[i], "value_changed",
			 G_CALLBACK(on_vs_eq_value_changed),
			 GINT_TO_POINTER(i));

	g_signal_connect(GTK_WIDGET(vs_eq[i]), "button-press-event",
			 G_CALLBACK(on_vs_eq_double_clicked),
			 GINT_TO_POINTER(i));
    }

    g_signal_connect((gpointer) cancelbutton, "clicked",
		     G_CALLBACK(on_cancelbutton_clicked), NULL);
    g_signal_connect((gpointer) applybutton, "clicked",
		     G_CALLBACK(on_applybutton_clicked), NULL);
    g_signal_connect((gpointer) okbutton, "clicked",
		     G_CALLBACK(on_okbutton_clicked), NULL);

    /* echo widgets */
    effect.echo.chk = chk_echo;
    effect.echo.hs_echo_delay = hs_echo_delay;
    effect.echo.hs_echo_feedback = hs_echo_feedback;
    effect.echo.hs_echo_wet = hs_echo_wet;

    /* stereo enhance widgets */
    effect.stereo.chk = chk_stereo;
    effect.stereo.hs_stereo_level = hs_stereo_level;

    /* voice removal widgets */
    effect.voice.chk = chk_voice;

    /* normalizer widgets */
    effect.normal.chk = chk_normal;
    effect.normal.sp_normal_buckets = sp_normal_buckets;
    effect.normal.hs_normal_target = hs_normal_target;
    effect.normal.hs_normal_gainmax = hs_normal_gainmax;
    effect.normal.hs_normal_gainsmooth = hs_normal_gainsmooth;

    /* equalizer widgets */
    effect.eq.chk = chk_eq;
    effect.eq.chk_auto = chk_auto;
    for (i = 0; i < 10; i++) {
	effect.eq.vs_eq[i] = vs_eq[i];
    }

    gtk_widget_show(effect_dialog);
}

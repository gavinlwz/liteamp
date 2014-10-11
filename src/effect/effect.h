#ifndef __EFFECT_H__
#define __EFFECT_H__

#include "echo.h"
#include "stereo.h"
#include "voice.h"
#include "normal.h"
#include "eq.h"

struct _effect {
    struct _echo echo;
    struct _stereo stereo;
    struct _voice voice;
    struct _normal normal;
    struct _eq eq;
};

void effect_init();
void effect_quit();
void effect_apply_change();
void effect_show_dialog();
gint effect_play_it(gint16 ** buffer, gint length, gint nch,
		    gint samplerate, gint bitrate);

#endif				/* effect.h */

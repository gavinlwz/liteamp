#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _stereo {
    gboolean on;
    GtkWidget *chk;
    GtkWidget *hs_stereo_level;
    gfloat level;
};

void stereo_init(struct _stereo *);
void stereo_quit();
void stereo_apply_change(struct _stereo *);
gint stereo_play_it(gint16 ** buffer, gint length, struct _stereo *, gint,
		    gint, gint);

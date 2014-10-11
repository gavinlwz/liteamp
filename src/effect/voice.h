#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _voice {
    gboolean on;
    GtkWidget *chk;
};

void voice_init(struct _voice *);
void voice_quit();
void voice_apply_change(struct _voice *);
gint voice_play_it(gint16 ** buffer, gint length, struct _voice *, gint,
		   gint, gint);

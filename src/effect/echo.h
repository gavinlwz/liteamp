#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define MAX_DELAY 1000

struct _echo {
    gboolean on;
    GtkWidget *chk;
    GtkWidget *hs_echo_delay;
    GtkWidget *hs_echo_feedback;
    GtkWidget *hs_echo_wet;
    gint delay;
    gint feedback;
    gint wet;
};

void echo_init(struct _echo *);
void echo_quit();
void echo_apply_change(struct _echo *);
gint echo_play_it(gint16 ** buffer, gint, struct _echo *, gint, gint,
		  gint);
void echo_cleanup(void);

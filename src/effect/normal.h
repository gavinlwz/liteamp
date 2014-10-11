#ifndef _COMPRESS_H_

struct _normal {
    gboolean on;
    GtkWidget *chk;
    GtkWidget *hs_normal_target;
    GtkWidget *hs_normal_gainmax;
    GtkWidget *hs_normal_gainsmooth;
    GtkWidget *sp_normal_buckets;
    gint target;
    gint gainmax;
    gint gainsmooth;
    gint buckets;
};

void normal_init(struct _normal *);
void normal_quit();
void normal_apply_change(struct _normal *);
gint normal_play_it(gint16 ** data, gint length, struct _normal *, gint,
		    gint, gint);

#endif

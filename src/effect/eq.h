/*
    eq.h: Copyright (C) 2002, 2003 Jacojang
    
    Originally based heavily upon:
	
    
*/

#define EQ_FLAT  {0,0,0,0,0,0,0,0,0,0}
#define EQ_POP  {0.0,2.0,3.5,4,2.5,0,-1,-1,0.0,0.0}
#define EQ_ROCK   {4,2,-2.5,-4.0,-1.5,2.0,4.0,5.5,5.5,5.5}
#define EQ_LAPTOP  {2.0,5.0,2.5,-1.5,-1.0,1.0,2.0,4.0,6.0,7.0}
#define EQ_CLASSIC  {0.0,0.0,0.0,0.0,0.0,0.0,-3.0,-3.0,-3.5,-4.0}

struct _eq {
    gboolean on;
    gboolean on_auto;
    GtkWidget *chk;
    GtkWidget *chk_auto;
    GtkWidget *vs_eq[10];

    gfloat eq[10];
};

void eq_init(struct _eq *);
void eq_quit();
void eq_apply_change(struct _eq *);
gint eq_play_it(gint16 ** buffer, gint length, struct _eq *, gint, gint,
		gint);

void eq_set_preset(struct _eq *, gfloat preset[]);
void eq_value_changed(struct _eq *, gint);

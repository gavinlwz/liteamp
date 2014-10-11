/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __PREFS_H__
#define __PREFS_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS typedef struct _Prefs Prefs;

struct _Prefs {
    /* play menu item */
    gboolean mute;
    gboolean shuffle;
    gboolean repeat;
    gboolean effects;
    /* view menu item */
    gboolean compact;
    gboolean visualizer;
    gboolean sidebar;
    gboolean playlist;
    gboolean statusbar;
    gboolean save_geometry;
    /* save/load layout */
    gint geometry_x;
    gint geometry_y;
    gint geometry_width;
    gint geometry_height;
    gint hpaned_position;
    /* osd */
    gboolean osd;
    gint osd_x;
    gint osd_y;
    gint osd_duration;
    /* playlist */
    GSList *playlist_columns;
    /* tag parser */
    GSList *tag_codesets;
    /* playback */
    gchar *output_device;
    gboolean select_decoder_by_contents;
    gboolean software_volume;

    gint seek_offset;
    gchar *last_dir;
    gchar *last_playlist;
    gchar *last_track;
};

void prefs_read(void);
void prefs_write(void);
void prefs_apply(void);
void prefs_free(void);

// *global* preferences object
extern Prefs prefs;

G_END_DECLS
#endif				/* __PREFS_H__ */

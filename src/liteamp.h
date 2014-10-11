/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __LITEAMP_H__
#define __LITEAMP_H__

#include <gnome.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>

#include "visualizer.h"
#include "player.h"
#include "sidebar.h"
#include "playlist.h"
#include "tray.h"
#include "osd.h"

#include "mixer.h"
#include "decoder.h"
#include "playback.h"

G_BEGIN_DECLS
#define TYPE_LITEAMP			(liteamp_get_type())
#define LITEAMP(object)			(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_LITEAMP, Liteamp))
#define LITEAMP_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_LITEAMP, LiteampClass))
#define IS_LITEAMP(object)		(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_LITEAMP))
#define IS_LITEAMP_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_LITEAMP))
typedef struct _Liteamp Liteamp;
typedef struct _LiteampClass LiteampClass;

GType liteamp_get_type(void) G_GNUC_CONST;
Liteamp *liteamp_new(GnomeProgram * gnome_program,
		     gboolean start, gchar * playlist, gchar * track);

// *global* application object ;)
extern Liteamp *app;

inline GtkWidget *lookup_widget(Liteamp * self, const gchar * name);
inline GtkAction *lookup_action(Liteamp * self, const gchar * name);
inline GtkWidget *lookup_action_widget(Liteamp * self, const gchar * name);

// getters
inline GtkWindow *get_window(Liteamp * self);
inline Visualizer *get_visualizer(Liteamp * self);
inline Player *get_player(Liteamp * self);
inline Playlist *get_playlist(Liteamp * self);
inline Sidebar *get_sidebar(Liteamp * self);
inline Tray *get_tray(Liteamp * self);
inline Osd *get_osd(Liteamp * self);
inline Mixer *get_mixer(Liteamp * self);
inline Decoder *get_decoder(Liteamp * self);
inline Playback *get_playback(Liteamp * self);

// statusbar
inline void statusbar_set_visible(Liteamp * self, gboolean visible);
inline gboolean statusbar_get_visible(Liteamp * self);
void statusbar_set_text(Liteamp * self, const gchar * text);
void statusbar_set_textv(Liteamp * self, const gchar * text_fmt, ...);

// osd
void osd_show(Liteamp * self, const gchar * text);
void osd_hide(Liteamp * self);

// ui update methods
void update_ui_layout(Liteamp * self);
void update_ui_menu(Liteamp * self);
void update_ui_player(Liteamp * self);
void update_ui_information(Liteamp * self);
void update_ui_position(Liteamp * self);
void update_ui_volume(Liteamp * self);
void update_ui_shuffle(Liteamp * self);
void update_ui_repeat(Liteamp * self);
void update_ui_effects(Liteamp * self);
void update_ui_mute(Liteamp * self);

G_END_DECLS
#endif				/* __LITEAMP_H__ */

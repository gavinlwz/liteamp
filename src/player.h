/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <gtk/gtk.h>
#include "la-track.h"

G_BEGIN_DECLS
#define TYPE_PLAYER		(player_get_type())
#define PLAYER(object)		(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_PLAYER, Player))
#define PLAYER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_PLAYER, PlayerClass))
#define IS_PLAYER(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_PLAYER))
#define IS_PLAYER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_PLAYER))
typedef struct _Player Player;
typedef struct _PlayerClass PlayerClass;

GType player_get_type(void) G_GNUC_CONST;
Player *player_new(void);

void player_set_track(Player * self, LaTrack * track);
LaTrack *player_get_track(Player * self);

inline gboolean player_is_stopped(Player * self);
inline gboolean player_is_playing(Player * self);
inline gboolean player_is_paused(Player * self);
inline const gchar* player_get_status_text(Player *self);
inline GdkPixbuf* player_get_status_icon(Player *self);

void player_start(Player * self);
void player_stop(Player * self);
void player_pause(Player * self);
void player_resume(Player * self);
void player_back(Player * self);
void player_forward(Player * self);

void player_locate(Player * self);

G_END_DECLS
#endif				/*player.h */

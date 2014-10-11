/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>

#include "player.h"
#include "liteamp.h"
#include "prefs.h"
#include "playback.h"
#include "util.h"

/* private-----------------------------------------------*/

typedef enum _PlayerStatus {
    PLAYER_STATUS_PLAYING,
    PLAYER_STATUS_PAUSED,
    PLAYER_STATUS_STOPPED,
} PlayerStatus;

struct _Player {
    GObject parent;

    LaTrack *track;

    PlayerStatus status;
};

struct _PlayerClass {
    GObjectClass parent_class;
};

static const gchar *PLAYER_STATUS_TEXT[] = {
    N_("PLAYING"),
    N_("PAUSED"),
    N_("STOPPED"),
};

static const gchar *PLAYER_STATUS_ICON[] = {
    PKGDATADIR "/pixmaps/start.png",
    PKGDATADIR "/pixmaps/pause.png",
    PKGDATADIR "/pixmaps/stop.png",
};

/* public-----------------------------------------------*/

Player *player_new()
{
    Player *self;

    self = g_object_new(TYPE_PLAYER, NULL);

    self->track = NULL;
    self->status = PLAYER_STATUS_STOPPED;

    return self;
}

void player_set_track(Player * self, LaTrack * track)
{
    self->track = track;
    visualizer_update(get_visualizer(app));
    update_ui_information(app);
}

LaTrack *player_get_track(Player * self)
{
    return self->track;
}

inline gboolean player_is_stopped(Player * self)
{
    return self->status == PLAYER_STATUS_STOPPED;
}

inline gboolean player_is_playing(Player * self)
{
    return self->status == PLAYER_STATUS_PLAYING;
}

inline gboolean player_is_paused(Player * self)
{
    return self->status == PLAYER_STATUS_PAUSED;
}

inline const gchar* player_get_status_text(Player *self)
{
    return _(PLAYER_STATUS_TEXT[self->status]);
}

inline GdkPixbuf* player_get_status_icon(Player *self)
{
    return gdk_pixbuf_new_from_file(PLAYER_STATUS_ICON[self->status], NULL);
}

void player_start(Player * self)
{
    // set track to play
    if (!self->track)
	self->track = playlist_get_cursor_track(get_playlist(app));

    if (self->track) {
	decoder_play((gchar *) la_track_get_uri(self->track));
	self->status = PLAYER_STATUS_PLAYING;

	g_free(prefs.last_track);
	prefs.last_track = g_strdup(la_track_get_uri(self->track));
    } else {
	self->status = PLAYER_STATUS_STOPPED;
    }

    update_ui_player(app);
}

void player_stop(Player * self)
{
    decoder_stop();

    self->status = PLAYER_STATUS_STOPPED;

    update_ui_player(app);
}

void player_pause(Player * self)
{
    decoder_pause();

    if (self->status == PLAYER_STATUS_PLAYING) {
	self->status = PLAYER_STATUS_PAUSED;
    } else if (self->status == PLAYER_STATUS_PAUSED) {
	self->status = PLAYER_STATUS_PLAYING;
    }

    update_ui_player(app);
}

void player_back(Player * self)
{
    gint curtime;

    if (!player_is_playing(self))
	return;

    curtime = decoder_get_cur_time();
    if (curtime < 0)
	return;
    decoder_seek(curtime - prefs.seek_offset);

    update_ui_position(app);
}

void player_forward(Player * self)
{
    gint curtime;

    if (!player_is_playing(self))
	return;

    curtime = decoder_get_cur_time();
    if (curtime < 0)
	return;
    decoder_seek(curtime + prefs.seek_offset);

    update_ui_position(app);
}

void player_locate(Player * self)
{
    gchar *dirname;
    gchar *uri_str;
    if (self->track) {
	dirname = g_path_get_dirname(la_track_get_uri(self->track));
	uri_str = gnome_vfs_get_uri_from_local_path(dirname);
	gnome_url_show(uri_str, NULL);
	g_free(uri_str);
	g_free(dirname);
    }
}

/* boilerplates -----------------------------------------*/

LA_TYPE_BOILERPLATE(Player, player, G_TYPE_OBJECT)

/*player.c*/

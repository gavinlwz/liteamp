/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "liteamp.h"
#include "prefs.h"
#include "prefs-dialog.h"
#include "util.h"
#include "la-track-column.h"

// *global* preferences object
Prefs prefs;

//TODO:
//1. reimplement with GConf
//2. make it instance variable of application class(Liteamp)

inline static gchar *prefs_get_string(const gchar * name)
{
    return gnome_config_get_string(name);
}

inline static gint prefs_get_int(const gchar * name)
{
    return gnome_config_get_int(name);
}

inline static gboolean prefs_get_bool(const gchar * name)
{
    return gnome_config_get_bool(name);
}

static gint prefs_get_enum(GType type, const gchar * name,
			   gint default_value)
{
    gchar *enum_name;
    gint value;

    enum_name = gnome_config_get_string(name);
    value = la_str_to_enum_with_fallback(type, enum_name, default_value);
    g_free(enum_name);
    return value;
}

static GSList *prefs_get_string_list(const gchar * name)
{
    GSList *slist;
    gchar *strings;
    gchar **array;
    gchar **item;

    slist = NULL;
    strings = gnome_config_get_string(name);
    array = g_strsplit(strings, ",", 0);
    if (array) {
	item = array;
	while (*item) {
	    if (LA_STR_IS_NOT_EMPTY(*item))
		slist = g_slist_append(slist, *item);
	    ++item;
	}
	//g_strfreev(array);
    }
    g_free(strings);

    return slist;
}

static GSList *prefs_get_enum_list(GType type, const gchar * name)
{
    GSList *slist;
    gchar *enum_names;
    gchar **array;
    gchar **enum_name;
    gint value;

    slist = NULL;
    enum_names = gnome_config_get_string(name);
    array = g_strsplit(enum_names, ",", 0);
    if (array) {
	enum_name = array;
	while (*enum_name) {
	    if (LA_STR_IS_NOT_EMPTY(*enum_name)) {
		value = la_str_to_enum(type, *enum_name);
		if (value >= 0) {
		    slist = g_slist_append(slist, GINT_TO_POINTER(value));
		}
	    }
	    ++enum_name;
	}
	g_strfreev(array);
    }
    g_free(enum_names);

    return slist;
}

inline static void prefs_set_string(const gchar * name,
				    const gchar * value)
{
    gnome_config_set_string(name, value);
}
inline static void prefs_set_int(const gchar * name, gint value)
{
    gnome_config_set_int(name, value);
}

inline static void prefs_set_bool(const gchar * name, gboolean value)
{
    gnome_config_set_bool(name, value);
}

inline static void prefs_set_enum(GType type, const gchar * name,
				  gint value)
{
    prefs_set_string(name, la_enum_to_str(type, value));
}

static void prefs_set_string_list(const gchar * name, GSList * value_list)
{
    GString *str;
    GSList *slist;
    gboolean first;
    gchar *item;

    str = g_string_sized_new(256);
    slist = value_list;
    first = TRUE;
    while (slist) {
	item = slist->data;
	if (LA_STR_IS_NOT_EMPTY(item)) {
	    if (first)
		first = FALSE;
	    else
		g_string_append_c(str, ',');
	    g_string_append(str, item);
	}
	slist = g_slist_next(slist);
    }
    prefs_set_string(name, str->str);
    g_string_free(str, TRUE);
}

static void prefs_set_enum_list(GType type, const gchar * name,
				GSList * value_list)
{
    GString *str;
    GSList *slist;
    gboolean first;
    const gchar *enum_name;

    str = g_string_sized_new(256);
    slist = value_list;
    first = TRUE;
    while (slist) {
	enum_name = la_enum_to_str(type, GPOINTER_TO_INT(slist->data));
	if (enum_name) {
	    if (first)
		first = FALSE;
	    else
		g_string_append_c(str, ',');
	    g_string_append(str, enum_name);
	}
	slist = g_slist_next(slist);
    }
    prefs_set_string(name, str->str);
    g_string_free(str, TRUE);
}

/*-----------------------------------------------------------------*/

void prefs_read()
{
    memset(&prefs, 0, sizeof(prefs));

    gnome_config_push_prefix("/liteamp/prefs/");

    prefs.mute = prefs_get_bool("mute=false");
    prefs.shuffle = prefs_get_bool("shuffle=false");
    prefs.repeat = prefs_get_bool("repeat=false");
    prefs.effects = prefs_get_bool("effects=false");

    prefs.compact = prefs_get_bool("compact=false");
    prefs.visualizer = prefs_get_bool("visualizer=true");
    prefs.sidebar = prefs_get_bool("sidebar=true");
    prefs.playlist = prefs_get_bool("playlist=true");
    prefs.statusbar = prefs_get_bool("statusbar=true");

    prefs.save_geometry = prefs_get_bool("save_geometry=true");
    prefs.geometry_x = prefs_get_int("geometry_x=0");
    prefs.geometry_y = prefs_get_int("geometry_y=0");
    prefs.geometry_width = prefs_get_int("geometry_width=400");
    prefs.geometry_height = prefs_get_int("geometry_height=300");
    prefs.hpaned_position = prefs_get_int("hpaned_position=100");

    prefs.osd = prefs_get_bool("osd=true");
    prefs.osd_x = prefs_get_int("osd_x=0");
    prefs.osd_y = prefs_get_int("osd_y=-50");
    prefs.osd_duration = prefs_get_int("osd_duration=5");

    prefs.playlist_columns =
	prefs_get_enum_list(LA_TYPE_TRACK_COLUMN,
			    "playlist_columns=title,genre,artist,album,time-str,size-str");

    prefs.tag_codesets =
	prefs_get_string_list("tag_codesets=CP949,CP932,LATIN1");

    prefs.output_device = prefs_get_string("output_device=esd");
    prefs.select_decoder_by_contents =
	prefs_get_bool("select_decoder_by_contents=false");
    prefs.software_volume = prefs_get_bool("software_volume=false");

    prefs.seek_offset = prefs_get_int("seek_offset=5");
    prefs.last_dir = prefs_get_string("last_dir=");
    prefs.last_playlist = prefs_get_string("last_playlist=");
    prefs.last_track = prefs_get_string("last_track=");

    gnome_config_pop_prefix();
}


void prefs_write()
{
    gnome_config_push_prefix("/liteamp/prefs/");

    prefs_set_bool("mute", prefs.mute);
    prefs_set_bool("shuffle", prefs.shuffle);
    prefs_set_bool("repeat", prefs.repeat);
    prefs_set_bool("effects", prefs.effects);

    prefs_set_bool("compact", prefs.compact);
    prefs_set_bool("visualizer", prefs.visualizer);
    prefs_set_bool("sidebar", prefs.sidebar);
    prefs_set_bool("playlist", prefs.playlist);
    prefs_set_bool("statusbar", prefs.statusbar);

    prefs_set_bool("save_geometry", prefs.save_geometry);
    prefs_set_int("geometry_x", prefs.geometry_x);
    prefs_set_int("geometry_y", prefs.geometry_y);
    prefs_set_int("geometry_width", prefs.geometry_width);
    prefs_set_int("geometry_height", prefs.geometry_height);
    prefs_set_int("hpaned_position", prefs.hpaned_position);

    prefs_set_bool("osd", prefs.osd);
    prefs_set_int("osd_x", prefs.osd_x);
    prefs_set_int("osd_y", prefs.osd_y);
    prefs_set_int("osd_duration", prefs.osd_duration);

    prefs_set_enum_list(LA_TYPE_TRACK_COLUMN, "playlist_columns",
			prefs.playlist_columns);

    prefs_set_string_list("tag_codesets", prefs.tag_codesets);

    prefs_set_string("output_device", prefs.output_device);
    prefs_set_bool("select_decoder_by_contents",
		   prefs.select_decoder_by_contents);
    prefs_set_bool("software_volume", prefs.software_volume);

    prefs_set_int("seek_offset", prefs.seek_offset);
    prefs_set_string("last_dir", prefs.last_dir);
    prefs_set_string("last_playlist", prefs.last_playlist);
    prefs_set_string("last_track", prefs.last_track);

    gnome_config_pop_prefix();
    gnome_config_sync();
}


void prefs_apply(void)
{
    playlist_update_columns(get_playlist(app));
    playback_set_playdevice(prefs.output_device);
    playback_volume_set_active(prefs.software_volume);
}

void prefs_free(void)
{
    g_slist_free(prefs.playlist_columns);
    g_slist_free(prefs.tag_codesets);
    g_free(prefs.output_device);
    g_free(prefs.last_dir);
    g_free(prefs.last_playlist);
    g_free(prefs.last_track);
}

/*prefs.c*/

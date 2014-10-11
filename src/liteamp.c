/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "liteamp.h"
#include "prefs.h"
#include "stock.h"
#include "action.h"
#include "callbacks.h"
#include "util.h"

// *global* application object ;)
Liteamp *app;

/* private-----------------------------------------------*/

struct _Liteamp {
    GObject parent;

    GnomeProgram *gnome_program;
    GnomeClient *gnome_client;
    GConfClient *gconf_client;

    GladeXML *glade;
    GtkUIManager *ui_manager;
    GtkActionGroup *action_group;
    GtkAccelGroup *accel_group;

    //
    // glade widgets
    //

    GtkWidget *window;
    GtkWidget *information;	//GtkLabel
    GtkWidget *position;	//GtkHScale
    GtkWidget *mute;		//GtkToggleToolButton
    GtkWidget *volume;		//GtkHScale
    GtkWidget *hpaned;		//GtkHPaned
    GtkWidget *statusbar;	//GtkStatusBar

    //
    // managed UI widgets and actions
    //

    GtkAction *add_directory_action;
    GtkAction *add_file_action;
    GtkAction *add_location_action;
    GtkAction *remove_playlist_action;
    GtkAction *cut_action;
    GtkAction *clear_action;
    GtkAction *paste_action;
    GtkAction *start_action;
    GtkAction *pause_action;
    GtkAction *stop_action;
    GtkAction *mute_action;
    GtkAction *shuffle_action;
    GtkAction *repeat_action;
    GtkAction *effects_action;
    GtkAction *compact_action;
    GtkAction *visualizer_action;
    GtkAction *sidebar_action;
    GtkAction *playlist_action;
    GtkAction *statusbar_action;

    //
    // hand made widgets
    //

    Visualizer *visualizer;
    Player *player;
    Sidebar *sidebar;
    Playlist *playlist;
    Tray *tray;
    Osd *osd;

    //
    // non-visual stuff
    //

    Mixer *mixer;
    Decoder *decoder;
    Playback *playback;
};

struct _LiteampClass {
    GObjectClass parent_class;
};

// information label markup
static const gchar *IDLE_MARKUP =
N_
("<span size=\"x-large\">LiteAmp - </span><span size=\"large\">[STOPPED]</span>\nMay the music be with you!");
// must have 3 of %s for title, status, album and artist
static const gchar *PLAY_MARKUP =
N_
("<span size=\"x-large\">%s</span><span size=\"large\"> [%s]</span>\n<span>From %s by %s</span>");
static const gchar *UNKNOWN_ALBUM = N_("Unknown Album");
static const gchar *UNKNOWN_ARTIST = N_("Unknown Artist");

// osd label markup
// must have two %s for title and status
static const gchar *OSD_MARKUP =
N_
("<span weight=\"bold\" size=\"xx-large\">LiteAmp - %s</span><span size=\"x-large\"> [%s]</span>");

/**
 * session management with GnomeClient
 * TODO:
 */
static gint
on_gnome_client_connect(GnomeClient * client, gboolean restarted,
			Liteamp * self)
{
    g_message("GnomeClient: connect\n");
}

static gint
on_gnome_client_save_yourself(GnomeClient * client, gint phase,
			      GnomeSaveStyle save_style, gint is_shutdown,
			      GnomeInteractStyle interact_style,
			      gint is_fast, Liteamp * self)
{
    g_message("GnomeClient: save_yoursef\n");
}

static void on_gnome_client_die(GnomeClient * client, Liteamp * self)
{
    g_message("GnomeClient: die\n");
}

static void init_gnome_client(Liteamp * self)
{
    self->gnome_client = gnome_master_client();

    g_signal_connect(self->gnome_client,
		     "connect", G_CALLBACK(on_gnome_client_connect), self);
    g_signal_connect(self->gnome_client, "save_yourself",
		     G_CALLBACK(on_gnome_client_save_yourself), self);
    g_signal_connect(self->gnome_client, "die",
		     G_CALLBACK(on_gnome_client_die), self);
}

static void on_gconf_client_notify(GConfClient * client, guint cnxn_id,
				   GConfEntry * entry, Liteamp * self)
{
    //GConfValue* value = gconf_entry_get_value(entry);
    //gboolean bool_value = gconf_value_get_bool(value);
}

static void init_gconf_client(Liteamp * self)
{
    self->gconf_client = gconf_client_get_default();

    gconf_client_add_dir(self->gconf_client,
			 "/apps/" PACKAGE,
			 GCONF_CLIENT_PRELOAD_NONE, NULL);
    gconf_client_notify_add(self->gconf_client,
			    "/apps/" PACKAGE, (GConfClientNotifyFunc)
			    on_gconf_client_notify, self, NULL, NULL);
}

static gboolean on_timeout(Liteamp * self)
{
    static gint heartbeat = 0;
    g_message("timeout heartbeat=%d\n", heartbeat++);
    return TRUE;
}

static gboolean on_idle(Liteamp * self)
{
    static gint heartbeat = 0;
    g_message("idle heartbeat=%d\n", heartbeat++);
    return TRUE;
}

static void load_layout(Liteamp * self)
{
    // restore geometry for normal size
    if (prefs.geometry_x >= 0 && prefs.geometry_y >= 0)
	gtk_window_move(GTK_WINDOW(self->window), prefs.geometry_x,
			prefs.geometry_y);

    if (prefs.geometry_width >= 0 && prefs.geometry_height >= 0)
	gtk_window_resize(GTK_WINDOW(self->window),
			  prefs.geometry_width, prefs.geometry_height);
    else
	gtk_widget_set_size_request(self->window, 400, 300);

    gtk_paned_set_position(GTK_PANED(self->hpaned),
			   (prefs.hpaned_position >= 0)
			   ? prefs.hpaned_position : 150);
}

static void save_layout(Liteamp * self)
{
    gint x, y, width, height;

    // save geometry not in compact mode
    if (prefs.save_geometry && !prefs.compact) {
	gtk_window_get_position(GTK_WINDOW(self->window), &x, &y);
	gtk_window_get_size(GTK_WINDOW(self->window), &width, &height);

	prefs.geometry_x = x;
	prefs.geometry_y = y;
	prefs.geometry_width = width;
	prefs.geometry_height = height;

	prefs.hpaned_position =
	    gtk_paned_get_position(GTK_PANED(self->hpaned));
    }

    prefs.visualizer = visualizer_get_visible(self->visualizer);
    prefs.sidebar = sidebar_get_visible(self->sidebar);
    prefs.playlist = playlist_get_visible(self->playlist);
    prefs.statusbar = statusbar_get_visible(self);
}

static gboolean on_window_delete_event(GtkWidget * widget,
				       GdkEvent * event, Liteamp * self)
{
    save_layout(self);

    gtk_main_quit();

    return FALSE;
}

static gboolean on_osd_timeout(Liteamp * self)
{
    osd_hide(self);
    return FALSE;
}

static void init_glade(Liteamp * self)
{
    self->glade =
	glade_xml_new(PKGDATADIR "/glade/liteamp-window.glade", NULL, NULL);

    self->window = lookup_widget(self, "window");
    self->information = lookup_widget(self, "information_label");
    self->position = lookup_widget(self, "position_hscale");
    self->volume = lookup_widget(self, "volume_hscale");
    self->mute = lookup_widget(self, "mute_toolbutton");
    self->hpaned = lookup_widget(self, "hpaned");
    self->statusbar = lookup_widget(self, "statusbar");
}

static void init_window(Liteamp * self)
{
    gtk_window_set_title(GTK_WINDOW(self->window), PACKAGE);
    gtk_window_set_icon_from_file(GTK_WINDOW(self->window),
				  PKGDATADIR "/pixmaps/liteamp.png", NULL);

    load_layout(self);

    g_signal_connect(self->window, "delete-event",
		     G_CALLBACK(on_window_delete_event), self);
}

static void init_ui_manager(Liteamp * self) {
    self->ui_manager = gtk_ui_manager_new();

    self->action_group = init_action_group(self->window);
    gtk_ui_manager_insert_action_group(self->ui_manager, self->action_group, 0);

    self->accel_group = gtk_ui_manager_get_accel_group(self->ui_manager);
    gtk_window_add_accel_group(GTK_WINDOW(self->window), self->accel_group);
    gtk_ui_manager_add_ui_from_file(self->ui_manager, PKGDATADIR "/ui/liteamp-ui.xml", NULL);

    // connect control buttons with actions
    gtk_action_connect_proxy(lookup_action(self, "Previous"), lookup_widget(self, "previous_toolbutton"));
    gtk_action_connect_proxy(lookup_action(self, "Start"), lookup_widget(self, "start_toolbutton"));
    gtk_action_connect_proxy(lookup_action(self, "Pause"), lookup_widget(self, "pause_toolbutton"));
    gtk_action_connect_proxy(lookup_action(self, "Stop"), lookup_widget(self, "stop_toolbutton"));
    gtk_action_connect_proxy(lookup_action(self, "Next"), lookup_widget(self, "next_toolbutton"));
    gtk_action_connect_proxy(lookup_action(self, "Shuffle"), lookup_widget(self, "shuffle_toolbutton"));
    gtk_action_connect_proxy(lookup_action(self, "Repeat"), lookup_widget(self, "repeat_toolbutton"));
    gtk_action_connect_proxy(lookup_action(self, "Mute"), lookup_widget(self, "mute_toolbutton"));

    // cache(?) interactive actions
    self->add_directory_action = lookup_action(self, "AddDirectory");
    self->add_file_action = lookup_action(self, "AddFile");
    self->add_location_action = lookup_action(self, "AddLocation");
    self->remove_playlist_action = lookup_action(self, "RemovePlaylist");
    self->cut_action = lookup_action(self, "Cut");
    self->paste_action = lookup_action(self, "Paste");
    self->clear_action = lookup_action(self, "Clear");
    self->start_action = lookup_action(self, "Start");
    self->pause_action = lookup_action(self, "Pause");
    self->stop_action = lookup_action(self, "Stop");
    self->mute_action = lookup_action(self, "Mute");
    self->shuffle_action = lookup_action(self, "Shuffle");
    self->repeat_action = lookup_action(self, "Repeat");
    self->effects_action = lookup_action(self, "Effects");
    self->compact_action = lookup_action(self, "Compact");
    self->visualizer_action = lookup_action(self, "Visualizer");
    self->sidebar_action = lookup_action(self, "Sidebar");
    self->playlist_action = lookup_action(self, "Playlist");
    self->statusbar_action = lookup_action(self, "Statusbar");
}

static void process_args(Liteamp * self, gboolean start, gchar * playlist,
			 gchar * track)
{
    if (LA_STR_IS_NOT_EMPTY(track)) {
	g_free(prefs.last_track), prefs.last_track = g_strdup(track);
    }

    if (LA_STR_IS_NOT_EMPTY(playlist)) {
	g_free(prefs.last_playlist),
	    prefs.last_playlist = g_strdup(playlist);
    }

    if (LA_STR_IS_NOT_EMPTY(prefs.last_playlist)) {
	playlist_set_playlist(self->playlist,
			      sidebar_get_playlist_by_uri(self->sidebar,
							  prefs.
							  last_playlist));
    }

    if (LA_STR_IS_NOT_EMPTY(prefs.last_track)) {
	player_set_track(self->player,
			 playlist_get_track_by_uri(self->playlist,
						   prefs.last_track));
    }

    if (start)
	player_start(self->player);
}

/* public-----------------------------------------------*/

Liteamp *liteamp_new(GnomeProgram * gnome_program,
		     gboolean start, gchar * playlist, gchar * track)
{
    Liteamp *self;
    self = g_object_new(TYPE_LITEAMP, NULL);

    //-------------------------------------------
    // initialize
    //-------------------------------------------

    app = self;

    self->gnome_program = gnome_program;

    init_gnome_client(self);
    init_gconf_client(self);

    // initialize non-visible stuff
    prefs_read();
    self->mixer = mixer_new();
    decoder_init();
    tag_init(prefs.tag_codesets);
    stock_init();

    // initialize glade-made widgets
    init_glade(self);

    // initialize main window
    init_window(self);

    // initialize ui manager
    init_ui_manager(self);

    gtk_container_add(GTK_CONTAINER(lookup_widget(self, "menubar_alignment")),
		      gtk_ui_manager_get_widget(self->ui_manager, "/MainMenu"));

    // initialize hand-made stuff
    self->visualizer = visualizer_new();
    self->player = player_new();
    self->sidebar = sidebar_new();
    self->playlist = playlist_new();
    self->tray = tray_new();
    self->osd = osd_new();

    // connect signal handlers with libglade
    glade_xml_signal_autoconnect(self->glade);

    gtk_widget_show_all(self->window);

    // restore layout
    update_ui_layout(self);
    update_ui_menu(self);
    update_ui_player(self);

    // process command line arguments
    process_args(self, start, playlist, track);

    //-------------------------------------------
    // main
    //-------------------------------------------

    //@@self->timeout_handler_id = gtk_timeout_add(1000, (GtkFunction)on_timeout, self);
    //@@self->idle_handler_id = gtk_idle_add((GtkFunction)on_idle, self);

    gdk_threads_enter();
    gtk_main();
    gdk_threads_leave();

    //@@gtk_timeout_remove(self->timeout_handler_id);
    //@@gtk_idle_remove(self->idle_handler_id);

    //-------------------------------------------
    // finalize
    //-------------------------------------------

    decoder_quit();
    g_free(self->mixer);
    prefs_write();
    prefs_free();
}

inline GtkWidget *lookup_widget(Liteamp * self, const gchar * name)
{
    return glade_xml_get_widget(self->glade, name);
}

inline GtkAction *lookup_action(Liteamp * self, const gchar * name)
{
    return gtk_action_group_get_action(self->action_group, name);
}

inline GtkWidget *lookup_action_widget(Liteamp * self, const gchar * name)
{
    return gtk_ui_manager_get_widget(self->ui_manager, name);
}


inline GtkWindow *get_window(Liteamp * self)
{
    return GTK_WINDOW(self->window);
}

inline Visualizer *get_visualizer(Liteamp * self)
{
    return self->visualizer;
}

inline Player *get_player(Liteamp * self)
{
    return self->player;
}

inline Playlist *get_playlist(Liteamp * self)
{
    return self->playlist;
}

inline Sidebar *get_sidebar(Liteamp * self)
{
    return self->sidebar;
}

inline Tray *get_tray(Liteamp * self)
{
    return self->tray;
}

inline Osd *get_osd(Liteamp * self)
{
    return self->osd;
}

inline Mixer *get_mixer(Liteamp * self)
{
    return self->mixer;
}

inline Decoder *get_decoder(Liteamp * self)
{
    return self->decoder;
}

inline Playback *get_playback(Liteamp * self)
{
    return self->playback;
}

inline void statusbar_set_visible(Liteamp * self, gboolean visible)
{
    if (visible)
	gtk_widget_show(self->statusbar);
    else
	gtk_widget_hide(self->statusbar);
}

inline gboolean statusbar_get_visible(Liteamp * self)
{
    return GTK_WIDGET_VISIBLE(self->statusbar);
}

void statusbar_set_text(Liteamp * self, const gchar * text)
{
    static guint context_id = -1;
    if (context_id == -1)
	context_id =
	    gtk_statusbar_get_context_id(GTK_STATUSBAR
					 (self->statusbar), "");
    gtk_statusbar_push(GTK_STATUSBAR(self->statusbar), context_id, text);
}

void statusbar_set_textv(Liteamp * self, const gchar * text_fmt, ...)
{
    gchar *text;
    va_list args;

    g_assert(text_fmt);
    va_start(args, text_fmt);
    text = g_strdup_vprintf(text, args);
    va_end(args);
    statusbar_set_text(self, text);
    g_free(text);
}

void osd_show(Liteamp * self, const gchar * text)
{
    gchar *escaped_text;
    gchar *markup;

    if (prefs.osd && LA_STR_IS_NOT_EMPTY(text)) {
	osd_set_position(self->osd, prefs.osd_x, prefs.osd_y);
	osd_set_text(self->osd, text);
	gtk_widget_show(GTK_WIDGET(self->osd));
	gtk_timeout_add(prefs.osd_duration * 1000,
			(GtkFunction) on_osd_timeout, self);
    }
}

void osd_hide(Liteamp * self)
{
    gtk_widget_hide(GTK_WIDGET(self->osd));
}

//
// ui updates methods
// generally, these functions called by callbacks only
//

void update_ui_layout(Liteamp * self)
{
    // restore widgets' visiblilty
    if (prefs.compact) {
	visualizer_set_visible(self->visualizer, FALSE);
	sidebar_set_visible(self->sidebar, FALSE);
	playlist_set_visible(self->playlist, FALSE);
	statusbar_set_visible(self, FALSE);
    } else {
	visualizer_set_visible(self->visualizer, prefs.visualizer);
	sidebar_set_visible(self->sidebar, prefs.sidebar);
	playlist_set_visible(self->playlist, prefs.playlist);
	statusbar_set_visible(self, prefs.statusbar);
    }

    // shrink/expand window size if possible
    if (!sidebar_get_visible(self->sidebar)
	&& !playlist_get_visible(self->playlist)) {
	gtk_widget_hide(self->hpaned);
	// request minimum size
	gtk_window_resize(GTK_WINDOW(self->window), 1, 1);
    } else {
	gtk_widget_show(self->hpaned);
	gtk_window_resize(GTK_WINDOW(self->window),
			  prefs.geometry_width, prefs.geometry_height);
    }
}

/**
  * why there's no gtk_action_set_sensitive? :(
  */
inline void action_set_sensitive(GtkAction *action, gboolean sensitive) {
    g_object_set(action, "sensitive", sensitive, NULL);
}

void update_ui_menu(Liteamp * self)
{
    gboolean writable, clipboard_has_data;

    // prepare conditions
    writable = playlist_is_writable(self->playlist);
    clipboard_has_data = playlist_clipboard_has_data(self->playlist);

    // enable/disable menu items
    action_set_sensitive(self->add_directory_action, writable);
    action_set_sensitive(self->add_file_action, writable);
    action_set_sensitive(self->add_location_action, writable);
    action_set_sensitive(self->remove_playlist_action, writable);
    action_set_sensitive(self->cut_action, writable);
    action_set_sensitive(self->clear_action, writable);
    action_set_sensitive(self->paste_action, writable
			     && clipboard_has_data);

    // (de)activate menu items
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(self->compact_action), prefs.compact);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(self->visualizer_action), prefs.visualizer);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(self->sidebar_action), prefs.sidebar);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(self->playlist_action), prefs.playlist);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(self->statusbar_action), prefs.statusbar);

    update_ui_shuffle(self);
    update_ui_repeat(self);
    update_ui_effects(self);
    update_ui_mute(self);
}

void update_ui_player(Liteamp * self)
{
    gboolean can_start, can_pause, can_stop, is_paused;
    gchar *tray_image;

    if (player_is_stopped(self->player)) {
	can_start = TRUE;
	can_stop = FALSE;
	can_pause = FALSE;
	is_paused = FALSE;
	tray_image = PKGDATADIR "/pixmaps/stop.png";
    } else {
	can_start = FALSE;
	can_stop = TRUE;
	can_pause = TRUE;
	is_paused = player_is_paused(self->player);
	if (is_paused) tray_image = PKGDATADIR "/pixmaps/pause.png";
	else tray_image = PKGDATADIR "/pixmaps/start.png";
    }
    tray_set_image(self->tray, tray_image);

    action_set_sensitive(self->start_action, can_start);
    action_set_sensitive(self->stop_action, can_stop);
    action_set_sensitive(self->pause_action, can_pause);

    g_signal_handlers_block_by_func(self->pause_action, NULL, NULL);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(self->pause_action), is_paused);
    g_signal_handlers_unblock_by_func(self->pause_action, NULL, NULL);

    update_ui_information(self);
    update_ui_position(self);
    //update_ui_volume(self);
    //update_ui_mute(self);
}

void update_ui_information(Liteamp * self)
{
    LaTrack *track;
    gchar *title;
    gchar *album;
    gchar *artist;
    gchar *status;
    gchar *markup;

    track = player_get_track(self->player);

    if (track) {
	title = la_markup_escape(la_track_get_title(track));

	album = (gchar *) la_track_get_album(track);
	if (LA_STR_IS_EMPTY(album))
	    album = _(UNKNOWN_ALBUM);
	album = la_markup_escape(album);

	artist = (gchar *) la_track_get_artist(track);
	if (LA_STR_IS_EMPTY(artist))
	    artist = _(UNKNOWN_ARTIST);
	artist = la_markup_escape(artist);

	status = la_markup_escape(player_get_status_text(self->player));

	// set information text
	markup =
	    g_strdup_printf(_(PLAY_MARKUP), title, status, album, artist);
	gtk_label_set_markup(GTK_LABEL(self->information), markup);
	g_free(markup);

	// set osd text
	if (prefs.osd) {
	    markup = g_strdup_printf(_(OSD_MARKUP), title, status);
	    osd_show(self, markup);
	    g_free(markup);
	}

	g_free(title);
	g_free(album);
	g_free(artist);
	g_free(status);

	LA_BLOCK_CALLBACK(self->position, on_position_hscale_value_changed);
	gtk_range_set_range(GTK_RANGE(self->position), 0,
			    la_track_get_seconds(track));
	gtk_range_set_value(GTK_RANGE(self->position), 0);
	gtk_widget_set_sensitive(self->position, TRUE);
	LA_UNBLOCK_CALLBACK(self->position, on_position_hscale_value_changed);
    } else {
	gtk_label_set_markup(GTK_LABEL(self->information), _(IDLE_MARKUP));

	osd_hide(self);

	LA_BLOCK_CALLBACK(self->position, on_position_hscale_value_changed);
	gtk_range_set_range(GTK_RANGE(self->position), 0, 1);
	gtk_range_set_value(GTK_RANGE(self->position), 0);
	gtk_widget_set_sensitive(self->position, FALSE);
	LA_UNBLOCK_CALLBACK(self->position, on_position_hscale_value_changed);
    }
}

void update_ui_position(Liteamp * self)
{
    static gint prev_time = 0;
    gint cur_time;

    cur_time = decoder_get_cur_time();
    if (cur_time == -1 || cur_time == prev_time)
	return;

    LA_BLOCK_CALLBACK(self->position, on_position_hscale_value_changed);

    gtk_range_set_value(GTK_RANGE(self->position), cur_time);
    prev_time = cur_time;

    LA_UNBLOCK_CALLBACK(self->position, on_position_hscale_value_changed);
}

void update_ui_shuffle(Liteamp * self)
{
    g_signal_handlers_block_by_func(self->shuffle_action, NULL, NULL);

    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(self->shuffle_action), prefs.shuffle);

    g_signal_handlers_unblock_by_func(self->shuffle_action, NULL, NULL);
}

void update_ui_repeat(Liteamp * self)
{
    g_signal_handlers_block_by_func(self->repeat_action, NULL, NULL);

    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(self->repeat_action), prefs.repeat);

    g_signal_handlers_unblock_by_func(self->repeat_action, NULL, NULL);
}

void update_ui_effects(Liteamp * self)
{
    g_signal_handlers_block_by_func(self->effects_action, NULL, NULL);

    playback_effect_set_active(prefs.effects);

    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(self->effects_action), prefs.effects);

    g_signal_handlers_unblock_by_func(self->effects_action, NULL, NULL);
}

void update_ui_mute(Liteamp * self)
{
    g_signal_handlers_block_by_func(self->mute_action, NULL, NULL);

    //@@mixer_set_mute(self->mixer, prefs.mute);

    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(self->mute_action), prefs.mute);

    if (prefs.mute) {
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(self->mute), LA_STOCK_MUTE); 
	gtk_widget_set_sensitive(self->volume, FALSE);
    } else {
	gtk_widget_set_sensitive(self->volume, TRUE);
    }

    g_signal_handlers_unblock_by_func(self->mute_action, NULL, NULL);
}

void update_ui_volume(Liteamp * self)
{
    gint volume;
    const gchar* volume_stock_id;

    LA_BLOCK_CALLBACK(self->volume, on_volume_hscale_value_changed);

    volume = mixer_get_volume(self->mixer);
    gtk_range_set_value(GTK_RANGE(self->volume), volume);

    if (!prefs.mute) {
	if (volume > 80) volume_stock_id = LA_STOCK_VOLUME_HI;
	else if (volume > 60) volume_stock_id = LA_STOCK_VOLUME_UP;
	else if (volume > 40) volume_stock_id = LA_STOCK_VOLUME;
	else if (volume > 20) volume_stock_id = LA_STOCK_VOLUME_DOWN;
	else volume_stock_id = LA_STOCK_VOLUME_LO;

	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(self->mute), volume_stock_id); 
    }

    LA_UNBLOCK_CALLBACK(self->volume, on_volume_hscale_value_changed);
}


/* boilerplates -----------------------------------------*/

LA_TYPE_BOILERPLATE(Liteamp, liteamp, G_TYPE_OBJECT)

/*liteamp.c*/

/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>

#include "playlist.h"
#include "liteamp.h"
#include "la-iter.h"
#include "la-track.h"
#include "la-track-column.h"
#include "la-tracks-store.h"
#include "la-tracks-iter.h"
#include "dnd.h"
#include "mime.h"
#include "callbacks.h"
#include "tag.h"
#include "util.h"
#include "prefs.h"

/* private-----------------------------------------------*/

struct _Playlist {
    GObject parent;

    GtkTreeView *view;
    GtkTreeModel *model;
    GtkTreeSelection *selection;

    GtkWidget *scrolledwindow;

    GtkMenu *popup;

    LaPlaylist *playlist;

    GList *play_order;
    GSList *clipboard;
};

struct _PlaylistClass {
    GObjectClass parent_class;
};

enum {
    COL_TRACK,
    NUM_COLUMNS,
};

static inline void update_status_text(Playlist * self)
{
    statusbar_set_text(app,
		       la_tracks_store_get_summary(LA_TRACKS_STORE
						   (self->model)));
}

static void get_selection_func(GtkTreeModel * model,
			       GtkTreePath * path, GtkTreeIter * iter,
			       GSList ** slist)
{
    LaTrack *track;

    gtk_tree_model_get(model, iter, LA_TRACK_COLUMN_SELF, &track, -1);
    if (track)
	*slist = g_slist_append(*slist, g_strdup(la_track_get_uri(track)));
}

static GSList *get_selection(Playlist * self)
{
    GSList *slist = NULL;
    gtk_tree_selection_selected_foreach(self->selection,
					(GtkTreeSelectionForeachFunc)
					get_selection_func, &slist);
    return slist;
}

static void free_selection_func(gpointer data, gpointer user_data)
{
    g_free(data);
}

static void free_selection(GSList ** slist)
{
    if (*slist) {
	g_slist_foreach(*slist, free_selection_func, NULL);
	g_slist_free(*slist);
	*slist = NULL;
    }
}

static void on_track_added(LaIter * iter, LaTrack * track, Playlist * self)
{
    /*
       gdk_threads_enter();
       update_status_text(self);
       gdk_threads_leave();
     */
}

static void on_track_removed(LaIter * iter, LaTrack * track,
			     Playlist * self)
{
    /*
       gdk_threads_enter();
       update_status_text(self);
       gdk_threads_leave();
     */
}

static void on_track_changed(LaIter * iter, LaTrack * track,
			     Playlist * self)
{
    /*
       gdk_threads_enter();
       update_status_text(self);
       gdk_threads_leave();
     */
}

static gboolean on_view_button_press_event(GtkWidget * widget,
					   GdkEventButton * event,
					   Playlist * self)
{
    if (event->button != 3)
	return FALSE;

    gtk_menu_popup(self->popup, NULL, NULL,
		   NULL, NULL, 3, gtk_get_current_event_time());

    return TRUE;
}

static void on_view_size_request(GtkWidget * widget,
				 GtkRequisition * request, Playlist * self)
{
    /*
       gint full_width, remain_width, cur_width, min_width, new_width;
       GtkTreeView *view;
       GList *columns;
       GtkTreeViewColumn *column;

       full_width = request->width;
       remain_width = full_width;
       g_print("full=%d\n", full_width);

       view = GTK_TREE_VIEW(widget);

       columns = gtk_tree_view_get_columns(view);
       while (columns) {
       column = GTK_TREE_VIEW_COLUMN(columns->data);

       cur_width = gtk_tree_view_column_get_width(column);

       if (gtk_tree_view_column_get_sizing(column) == GTK_TREE_VIEW_COLUMN_FIXED) {
       min_width = gtk_tree_view_column_get_width(column);
       new_width = (remain_width > min_width) ? remain_width * 0.5 : min_width;
       gtk_tree_view_column_set_fixed_width(column, new_width);
       remain_width -= new_width;
       g_print("cur:%d min:%d new:%d remain:%d\n",
       cur_width, min_width, new_width, remain_width);
       } else {
       remain_width -= cur_width;
       }

       columns = g_list_next(columns);
       }
       g_list_free(columns);
     */
}

static void on_view_row_activated(GtkTreeView * view, GtkTreePath * path,
				  GtkTreeViewColumn * column,
				  Playlist * self)
{
    player_set_track(get_player(app), playlist_get_cursor_track(self));
    player_start(get_player(app));
}

static void on_selection_changed(GtkTreeSelection * selection,
				 Playlist * self)
{
    //probably something useful...
}

static void on_checked_renderer_toggled(GtkCellRendererToggle * renderer,
					gchar * path_str, Playlist * self)
{
    GtkTreePath *path;

    path = gtk_tree_path_new_from_string(path_str);
    la_tracks_store_toggle_checked(LA_TRACKS_STORE(self->model), path);
    gtk_tree_path_free(path);

    update_status_text(self);
}

static void on_clipboard_received(GtkClipboard * clipboard,
				  GtkSelectionData * selection_data,
				  Playlist * self)
{
    g_print("playlist:clipboard_received\n");
}

static void on_clipboard_get(GtkClipboard * clipboard,
			     GtkSelectionData * selection_data, guint info,
			     Playlist * self)
{
    g_print("playlist:clipboard_get\n");
}

static void on_clipboard_clear(GtkClipboard * clipboard, Playlist * self)
{
    g_print("playlist:clipboard_clear\n");
}

static void on_view_drag_data_received(GtkWidget * widget,
				       GdkDragContext * context, gint x,
				       gint y, GtkSelectionData * data,
				       guint info, guint time,
				       Playlist * self)
{
    GList *uri_list;
    gchar *uri_str;
    gchar *local_path;
    gchar *mime_type;

    if (info == DND_TARGET_ID_URI_LIST) {

	uri_list = gnome_vfs_uri_list_parse(data->data);

	while (uri_list) {
	    g_assert(uri_list->data);
	    uri_str =
		gnome_vfs_uri_to_string(uri_list->data,
					GNOME_VFS_URI_HIDE_NONE);
	    local_path = gnome_vfs_get_local_path_from_uri(uri_str);
	    mime_type = (gchar *) gnome_vfs_get_mime_type(uri_str);

	    if (is_mime_type_directory(mime_type)) {

		playlist_add_directory(self, local_path, FALSE, TRUE);

	    } else if (is_mime_type_music(mime_type)) {

		playlist_add_file(self, local_path, TRUE);

	    } else if (is_mime_type_image(mime_type)) {

		if (LA_STR_IS_EMPTY(la_playlist_get_icon(self->playlist))) {
		    la_playlist_set_icon(self->playlist, local_path);
		}

	    } else {
		// ignore unknown mime type!
		g_message
		    ("unknown mime type: %s for %s... ignored!\n",
		     mime_type, uri_str);
	    }

	    playlist_write(self);

	    g_free(local_path);
	    g_free(uri_str);

	    uri_list = g_list_next(uri_list);
	}
	gnome_vfs_uri_list_free(uri_list);

	//gtk_drag_finish(context, TRUE, FALSE, time);
    }
    //gtk_drag_finish(context, TRUE, FALSE, time);
}

static void add_column(Playlist * self, LaTrackColumn index)
{
    GtkTreeViewColumn *column;
    GList *list;

    column = la_track_column_new_tree_view_column(index);
    gtk_tree_view_append_column(self->view, column);

    if (index == LA_TRACK_COLUMN_CHECKED) {
	list = gtk_tree_view_column_get_cell_renderers(column);
	if (list) {
	    g_assert(GTK_IS_CELL_RENDERER_TOGGLE(list->data));
	    g_signal_connect(list->data,
			     "toggled",
			     G_CALLBACK
			     (on_checked_renderer_toggled), self);
	    g_list_free(list);
	}
    }
}

/* public-----------------------------------------------*/

Playlist *playlist_new(void)
{
    Playlist *self;

    self = g_object_new(TYPE_PLAYLIST, NULL);

    self->view = GTK_TREE_VIEW(lookup_widget(app, "playlist_treeview"));
    self->scrolledwindow = lookup_widget(app, "playlist_scrolledwindow");

    self->popup = GTK_MENU(lookup_action_widget(app, "/PlaylistPopup"));

    self->playlist = NULL;
    self->model = NULL;

    playlist_update_columns(self);

    self->selection = gtk_tree_view_get_selection(self->view);
    gtk_tree_selection_set_mode(self->selection, GTK_SELECTION_MULTIPLE);

    // signal handlers

    g_signal_connect(self->view,
		     "button-press-event",
		     G_CALLBACK(on_view_button_press_event), self);

    g_signal_connect(self->view,
		     "size-request",
		     G_CALLBACK(on_view_size_request), self);

    g_signal_connect(self->view,
		     "row-activated",
		     G_CALLBACK(on_view_row_activated), self);

    g_signal_connect(self->selection,
		     "changed", G_CALLBACK(on_selection_changed), self);

    // drag and drop supports
    dnd_connect(GTK_WIDGET(self->view),
		G_CALLBACK(on_view_drag_data_received), self);
    dnd_enable(GTK_WIDGET(self->view));

    // clipboard supports
    // TODO: supports generic clipboard with GtkClipboard
    // ...

    return self;
}

inline void playlist_set_visible(Playlist * self, gboolean visible)
{
    if (visible)
	gtk_widget_show(self->scrolledwindow);
    else
	gtk_widget_hide(self->scrolledwindow);
}

inline gboolean playlist_get_visible(Playlist * self)
{
    return GTK_WIDGET_VISIBLE(self->scrolledwindow);
}

void playlist_update_columns(Playlist * self)
{
    GList *list;
    GSList *slist;

    // remove existing column
    list = gtk_tree_view_get_columns(self->view);
    while (list) {
	gtk_tree_view_remove_column(self->view, list->data);
	list = g_list_next(list);
    }
    g_list_free(list);

    // add mandatory columns
    add_column(self, LA_TRACK_COLUMN_ICON);
    add_column(self, LA_TRACK_COLUMN_CHECKED);

    // add specified columns
    slist = prefs.playlist_columns;
    while (slist) {
	add_column(self, GPOINTER_TO_INT(slist->data));
	slist = g_slist_next(slist);
    }
}


void playlist_set_playlist(Playlist * self, LaPlaylist * playlist)
{
    LaIter *tracks;

    if (!playlist)
	return;

    if (playlist == self->playlist)
	return;

    // save the current(old) playlist
    if (self->playlist)
	playlist_write(self);

    self->playlist = playlist;

    // free the current(old) playlist store
    if (self->model) {
	gtk_tree_view_set_model(self->view, NULL);
	g_free(self->model);
	self->model = NULL;
    }

    tracks = la_playlist_get_tracks(self->playlist);

    // create a store for tracks of the playlist
    self->model = GTK_TREE_MODEL(la_tracks_store_new(tracks));
    gtk_tree_view_set_model(self->view, self->model);

    // FIXME: resize all columns to optimal width in "size-request"
    gtk_tree_view_columns_autosize(self->view);

    g_signal_connect(tracks, "added", G_CALLBACK(on_track_added), self);
    g_signal_connect(tracks,
		     "removed", G_CALLBACK(on_track_removed), self);
    g_signal_connect(tracks,
		     "changed", G_CALLBACK(on_track_changed), self);

    playlist_read(self);

    visualizer_update(get_visualizer(app));

    la_tracks_store_update_summary(LA_TRACKS_STORE(self->model));
    update_status_text(self);

    g_free(prefs.last_playlist);
    prefs.last_playlist = g_strdup(la_playlist_get_uri(self->playlist));
}

LaPlaylist *playlist_get_playlist(Playlist * self)
{
    return self->playlist;
}

gboolean playlist_is_writable(Playlist * self)
{
    return TRUE;
}

gboolean playlist_clipboard_has_data(Playlist * self)
{
    return (self->clipboard != NULL);
}

LaTrack *playlist_get_cursor_track(Playlist * self)
{
    GtkTreePath *path;
    GtkTreeIter iter;
    LaTrack *track;

    if (!self->play_order)
	playlist_shuffle(self);

    gtk_tree_view_get_cursor(self->view, &path, NULL);
    if (path) {
	if (gtk_tree_model_get_iter(self->model, &iter, path)) {
	    gtk_tree_model_get(self->model, &iter,
			       LA_TRACK_COLUMN_SELF, &track, -1);
	    gtk_tree_path_free(path);
	    self->play_order = g_list_find(self->play_order, track);
	    return track;
	}
	gtk_tree_path_free(path);
    }
    return NULL;
}

LaTrack *playlist_get_previous_track(Playlist * self)
{
    GList *list;

    if (!self->play_order)
	playlist_shuffle(self);

    list = g_list_previous(self->play_order);
    if (!list && prefs.repeat)
	list = g_list_last(self->play_order);
    if (list) {
	self->play_order = list;
	return list->data;
    }
    return NULL;
}

LaTrack *playlist_get_next_track(Playlist * self)
{
    GList *list;

    if (!self->play_order)
	playlist_shuffle(self);

    list = g_list_next(self->play_order);
    if (!list && prefs.repeat)
	list = g_list_first(self->play_order);
    if (list) {
	self->play_order = list;
	return list->data;
    }
    return NULL;
}

LaTrack *playlist_get_track_by_uri(Playlist * self, const gchar * uri)
{
    if (!self->playlist)
	return NULL;
    return
	la_tracks_iter_lookup(LA_TRACKS_ITER
			      (la_playlist_get_tracks(self->playlist)),
			      uri);
}

void playlist_read(Playlist * self)
{
    playlist_import_pls(self, la_playlist_get_uri(self->playlist));
    playlist_shuffle(self);
}

void playlist_write(Playlist * self)
{
    playlist_export_pls(self, la_playlist_get_uri(self->playlist));
}

void playlist_add_file(Playlist * self, const gchar * filename,
		       gboolean checked)
{
    LaTrack *track;
    gint number;
    gint bytes;
    gint seconds;
    GtkTreeIter iter;
    GtkTreePath *path;

    if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
	g_message("file not found: %s!\n", filename);
	return;
    }

    bytes = la_file_size(filename);

    if (bytes <= 0) {
	g_message("file is corrupted:%s (bytes=0)\n", filename);
	return;
    }

    if (la_str_has_suffix_nocase(filename, ".png") ||
	la_str_has_suffix_nocase(filename, ".jpg") ||
	la_str_has_suffix_nocase(filename, ".gif") ||
	la_str_has_suffix_nocase(filename, ".svg")) {

	if (LA_STR_IS_EMPTY(la_playlist_get_icon(self->playlist))) {
	    la_playlist_set_icon(self->playlist, filename);
	}
	return;
    }

    // FIXME: check the file is valid mp3 or ogg file
    if (!la_str_has_suffix_nocase(filename, ".mp3") &&
	!la_str_has_suffix_nocase(filename, ".ogg")) {
	return;
    }

    // create a track object
    track = la_track_new(filename);

    // parse tag
    tag_read(track);

    // FIXME: check the file is valid mp3 or ogg file
    seconds = la_track_get_seconds(track);
    if (seconds <= 0) {
	g_message("file is corrupted:%s (seconds=0)\n", filename);
	g_free(track);
	return;
    }

    la_track_set_checked(track, checked);

    la_iter_add(la_playlist_get_tracks(self->playlist), track);
}

void playlist_add_directory(Playlist * self, const gchar * dirname,
			    gboolean recursive, gboolean checked)
{
    GDir *dir;
    const gchar *name;
    gchar *filename;

    dir = g_dir_open(dirname, 0, NULL);
    if (!dir)
	return;
    while ((name = g_dir_read_name(dir)) != NULL) {
	filename = g_build_filename(dirname, name, NULL);
	if (g_file_test(filename, G_FILE_TEST_IS_DIR)) {
	    if (recursive) {
		// recursive call
		playlist_add_directory(self, filename, recursive, checked);
	    }
	} else {
	    playlist_add_file(self, filename, checked);
	}

	g_free(filename);
    }
    g_dir_close(dir);
}

void playlist_remove_file(Playlist * self, const gchar * filename)
{
}

/*
 * read .pls file with gnome-config
 * TODO: implemnts without gnome-config
 */
void playlist_import_pls(Playlist * self, const gchar * pls_filename)
{
    gint num, row_count;
    gchar key[256];

    gchar *filename;
    gboolean checked;

    g_snprintf(key, sizeof(key), "=%s=/playlist/", pls_filename);
    gnome_config_push_prefix(key);

    row_count = gnome_config_get_int("NumberOfEntries=0");

    for (num = 1; num <= row_count; num++) {
	g_snprintf(key, sizeof(key), "File%d", num);
	filename = gnome_config_get_string(key);

	// custom extension
	g_snprintf(key, sizeof(key), "Check%d=true", num);
	checked = gnome_config_get_bool(key);

	playlist_add_file(self, filename, checked);

	g_free(filename);
    }

    gnome_config_pop_prefix();
}

/*
 * write .pls file with gnome-config
 * TODO:
 * 1. implemnts without gnome-config
 * 2. save/cache tag info
 */
void playlist_export_pls(Playlist * self, const gchar * pls_filename)
{
    LaIter *tracks;
    gint n, num_tracks;
    LaTrack *track;
    gchar key[256];
    gboolean checked;

    g_snprintf(key, sizeof(key), "=%s=/playlist/", pls_filename);
    gnome_config_clean_section(key);

    gnome_config_push_prefix(key);

    num_tracks = la_playlist_get_num_tracks(self->playlist);
    gnome_config_set_int("NumberOfEntries", num_tracks);

    tracks = la_playlist_get_tracks(self->playlist);
    for (n = 0; n < num_tracks; n++) {
	track = la_iter_nth(tracks, n);

	g_snprintf(key, sizeof(key), "File%d", n + 1);
	gnome_config_set_string(key, la_track_get_uri(track));

	checked = la_track_get_checked(track);
	if (!checked) {
	    g_snprintf(key, sizeof(key), "Check%d", n + 1);
	    gnome_config_set_bool(key, checked);
	}
    }

    gnome_config_pop_prefix();

    gnome_config_sync();
}

void playlist_cut_selection(Playlist * self)
{
    playlist_copy_selection(self);
    playlist_clear_selection(self);
}

void playlist_copy_selection(Playlist * self)
{
    free_selection(&self->clipboard);
    self->clipboard = get_selection(self);
}

void playlist_paste_selection(Playlist * self)
{
    GSList *slist;
    const gchar *track_uri;

    slist = self->clipboard;
    while (slist) {
	track_uri = slist->data;
	if (!track_uri)
	    continue;
	playlist_add_file(self, track_uri, TRUE);
	slist = g_slist_next(slist);
    }

    playlist_write(self);
}

void playlist_clear_selection(Playlist * self)
{
    GSList *slist;
    const gchar *track_uri;

    slist = get_selection(self);
    while (slist) {
	track_uri = slist->data;
	if (!track_uri)
	    continue;
	playlist_remove_file(self, track_uri);
	slist = g_slist_next(slist);
    }
    free_selection(&self->clipboard);

    playlist_write(self);
}

void playlist_select_all(Playlist * self)
{
    gtk_tree_selection_select_all(self->selection);
}

void playlist_select_none(Playlist * self)
{
    gtk_tree_selection_unselect_all(self->selection);
}

void playlist_invert_selection(Playlist * self)
{
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter_first(self->model, &iter)) {
	do {
	    if (gtk_tree_selection_iter_is_selected
		(self->selection, &iter))
		gtk_tree_selection_unselect_iter(self->selection, &iter);
	    else
		gtk_tree_selection_select_iter(self->selection, &iter);
	} while (gtk_tree_model_iter_next(self->model, &iter));
    }
}

void playlist_previous(Playlist * self)
{
    player_set_track(get_player(app), playlist_get_previous_track(self));
    player_start(get_player(app));
}

void playlist_next(Playlist * self)
{
    player_set_track(get_player(app), playlist_get_next_track(self));
    player_start(get_player(app));
}

void playlist_shuffle(Playlist * self)
{
    LaIter *tracks;
    GList *list;
    gint n, num_tracks;
    LaTrack *track;

    g_list_free(self->play_order);

    if (!self->playlist)
	return;

    tracks = la_playlist_get_tracks(self->playlist);
    num_tracks = la_playlist_get_num_tracks(self->playlist);

    list = NULL;

    if (prefs.shuffle) {
	for (n = 0; n < num_tracks; n++) {
	    do {
		track =
		    la_iter_nth(tracks, g_random_int_range(0, num_tracks));
	    } while (g_list_index(list, track) >= 0);
	    list = g_list_append(list, track);
	}
    } else {
	for (n = 0; n < num_tracks; n++) {
	    list = g_list_append(list, la_iter_nth(tracks, n));
	}
    }

    self->play_order = list;
}

void playlist_jump_to(Playlist * self)
{
    LaTrack *track = player_get_track(get_player(app));
    g_print("jump_to %s\n", la_track_get_title(track));
}

void playlist_refresh(Playlist * self)
{
    playlist_update_columns(self);
}

static void get_selected_tracks_func(GtkTreeModel * model,
			       GtkTreePath * path, GtkTreeIter * iter,
			       GSList ** slist)
{
    LaTrack *track;

    gtk_tree_model_get(model, iter, LA_TRACK_COLUMN_SELF, &track, -1);
    if (track)
	*slist = g_slist_append(*slist, track);
}

inline GSList *playlist_get_selected_tracks(Playlist *self)
{
    GSList *slist = NULL;
    gtk_tree_selection_selected_foreach(self->selection,
					(GtkTreeSelectionForeachFunc)
					get_selected_tracks_func, &slist);
    return slist;
}

/* boilerplates -----------------------------------------*/

LA_TYPE_BOILERPLATE(Playlist, playlist, G_TYPE_OBJECT)

/*playlist.c*/

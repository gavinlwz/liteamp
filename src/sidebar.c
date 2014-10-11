/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sidebar.h"
#include "liteamp.h"
#include "dnd.h"
#include "mime.h"
#include "util.h"

/* private-----------------------------------------------*/

struct _Sidebar {
    GObject parent;

    GtkTreeView *view;
    GtkTreeModel *model;

    GtkWidget *scrolledwindow;

    GtkMenu *popup;

    gchar *filename;
};

struct _SidebarClass {
    GObjectClass parent_class;
};

enum {
    COL_PLAYLIST,
    NUM_COLS,
};

static void on_model_row_inserted(GtkTreeModel * treemodel,
				  GtkTreePath * path,
				  GtkTreeIter * iter, Sidebar * self)
{
}

static void on_model_row_deleted(GtkTreeModel * treemodel,
				 GtkTreePath * path, Sidebar * self)
{
}

static gboolean on_view_button_press_event(GtkWidget * widget,
					   GdkEventButton * event,
					   Sidebar * self)
{
    if (event->button != 3)
	return FALSE;

    gtk_menu_popup(self->popup, NULL, NULL,
		   NULL, NULL, 3, gtk_get_current_event_time());

    return TRUE;
}

static void on_view_cursor_changed(GtkTreeView * treeview, Sidebar * self)
{
    LaPlaylist *playlist;

    playlist = sidebar_get_cursor_playlist(self);
    if (playlist)
	playlist_set_playlist(get_playlist(app), playlist);
}

static void on_view_drag_data_received(GtkWidget * widget,
				       GdkDragContext * context, gint x,
				       gint y, GtkSelectionData * data,
				       guint info, guint time,
				       Sidebar * self)
{
    GList *uri_list;
    gchar *uri_str;
    gchar *local_path;
    gchar *mime_type;
    gchar *short_name;
    gchar *playlist_name;
    gchar playlist_uri[PATH_MAX];
    LaPlaylist *playlist;

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
		// create new playlist
		short_name =
		    gnome_vfs_uri_extract_short_name(uri_list->data);
		// titled with basename of dropped directory
		playlist_name = la_filename_to_utf8(short_name);
		// unique playlist file name
		g_snprintf(playlist_uri, sizeof(playlist_uri),
			   "%s/.liteamp/%s-%s.pls",
			   g_get_home_dir(), short_name, la_timestamp());

		playlist = la_playlist_new();
		la_playlist_set_name(playlist, playlist_name);
		la_playlist_set_uri(playlist, playlist_uri);

		g_free(playlist_name);
		g_free(short_name);

		sidebar_add_playlist(self, playlist);

		playlist_set_playlist(get_playlist(app), playlist);

		playlist_add_directory(get_playlist(app),
				       local_path, FALSE, TRUE);

	    } else if (is_mime_type_music(mime_type)) {

		playlist_add_file(get_playlist(app), local_path, TRUE);

	    } else if (is_mime_type_image(mime_type)) {

		playlist = playlist_get_playlist(get_playlist(app));
		if (LA_STR_IS_EMPTY(la_playlist_get_icon(playlist))) {
		    la_playlist_set_icon(playlist, local_path);
		}

	    } else {
		// ignore unknown mime type!
		g_message
		    ("unknown mime type: %s for %s... ignored!\n",
		     mime_type, uri_str);
	    }

	    playlist_write(get_playlist(app));

	    g_free(local_path);
	    g_free(uri_str);

	    uri_list = g_list_next(uri_list);
	}

	sidebar_write(self);

	gnome_vfs_uri_list_free(uri_list);

	//gtk_drag_finish(context, TRUE, FALSE, time);
    }
    //gtk_drag_finish(context, TRUE, FALSE, time);
}


static void playlist_cell_data_func(GtkTreeViewColumn * column,
				    GtkCellRenderer * renderer,
				    GtkTreeModel * model,
				    GtkTreeIter * iter, Sidebar * self)
{
    LaPlaylist *playlist;
    gtk_tree_model_get(model, iter, COL_PLAYLIST, &playlist, -1);
    g_object_set(renderer, "text", la_playlist_get_name(playlist), NULL);
}

gboolean sidebar_lookup_playlist(Sidebar * self, LaPlaylist * playlist,
				 GtkTreeIter * iter)
{
    LaPlaylist *cur_playlist;

    if (gtk_tree_model_get_iter_first(self->model, iter)) {
	do {
	    gtk_tree_model_get(self->model, iter,
			       COL_PLAYLIST, &cur_playlist, -1);
	    if (playlist == cur_playlist)
		return TRUE;
	} while (gtk_tree_model_iter_next(self->model, iter));
    }
    return FALSE;
}

/* public-----------------------------------------------*/

Sidebar *sidebar_new(void)
{
    Sidebar *self;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    self = g_object_new(TYPE_SIDEBAR, NULL);

    self->view = GTK_TREE_VIEW(lookup_widget(app, "sidebar_treeview"));
    self->scrolledwindow = lookup_widget(app, "sidebar_scrolledwindow");

    self->popup = GTK_MENU(lookup_action_widget(app, "/SidebarPopup"));

    // create columns

    column = gtk_tree_view_column_new();
    gtk_tree_view_append_column(self->view, column);
    gtk_tree_view_column_set_title(column, _("Playlist"));
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_set_cell_data_func(column,
					    renderer, (GtkTreeCellDataFunc)
					    playlist_cell_data_func,
					    self, NULL);

    // create model

    self->model =
	GTK_TREE_MODEL(gtk_list_store_new(NUM_COLS, G_TYPE_POINTER));

    gtk_tree_view_set_model(self->view, self->model);

    // read sidebar settings with gnome-config

    self->filename =
	g_build_filename(g_get_home_dir(), ".liteamp/sidebar.conf", NULL);

    sidebar_read(self);

    // signal handlers

    g_signal_connect(self->model,
		     "row-inserted",
		     G_CALLBACK(on_model_row_inserted), self);

    g_signal_connect(self->model,
		     "row-inserted",
		     G_CALLBACK(on_model_row_inserted), self);

    g_signal_connect(self->view,
		     "button-press-event",
		     G_CALLBACK(on_view_button_press_event), self);

    g_signal_connect(self->view,
		     "cursor-changed",
		     G_CALLBACK(on_view_cursor_changed), self);

    // drag and drop supports
    dnd_connect(GTK_WIDGET(self->view),
		G_CALLBACK(on_view_drag_data_received), self);
    dnd_enable(GTK_WIDGET(self->view));

    return self;
}

inline void sidebar_set_visible(Sidebar * self, gboolean visible)
{
    if (visible)
	gtk_widget_show(self->scrolledwindow);
    else
	gtk_widget_hide(self->scrolledwindow);
}

inline gboolean sidebar_get_visible(Sidebar * self)
{
    return GTK_WIDGET_VISIBLE(self->scrolledwindow);
}

inline gboolean sidebar_has_focus(Sidebar * self)
{
    return GTK_WIDGET_HAS_FOCUS(self->view);
}

LaPlaylist *sidebar_get_cursor_playlist(Sidebar * self)
{
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    LaPlaylist *playlist;

    path = gtk_tree_path_new();
    gtk_tree_view_get_cursor(self->view, &path, NULL);

    model = gtk_tree_view_get_model(self->view);

    if (path != NULL && gtk_tree_model_get_iter(self->model, &iter, path)) {
	gtk_tree_model_get(self->model, &iter, COL_PLAYLIST,
			   &playlist, -1);
	gtk_tree_path_free(path);
	return playlist;
    }
    return NULL;
}

LaPlaylist *sidebar_get_playlist_by_uri(Sidebar * self, const gchar * uri)
{
    GtkTreeIter iter;
    LaPlaylist *playlist;

    if (gtk_tree_model_get_iter_first(self->model, &iter)) {
	do {
	    gtk_tree_model_get(self->model, &iter,
			       COL_PLAYLIST, &playlist, -1);
	    if (!strcmp(la_playlist_get_uri(playlist), uri))
		return playlist;
	} while (gtk_tree_model_iter_next(self->model, &iter));
    }
    return NULL;
}

/**
 * read sidebar settings using gnome-config
 * TODO: implements without gnome-config
 */
void sidebar_read(Sidebar * self)
{
    gint num, count;
    gchar key[256];
    gchar *title;
    gchar *icon;
    gchar *filename;
    LaPlaylist *playlist;
    GtkTreeIter iter;

    g_snprintf(key, sizeof(key), "=%s=/sidebar/", self->filename);
    gnome_config_push_prefix(key);

    count = gnome_config_get_int("NumberOfPlaylists=0");

    for (num = 1; num <= count; num++) {
	g_snprintf(key, sizeof(key), "Title%d", num);
	title = gnome_config_get_string(key);

	g_snprintf(key, sizeof(key), "Icon%d", num);
	icon = gnome_config_get_string(key);

	g_snprintf(key, sizeof(key), "Filename%d", num);
	filename = gnome_config_get_string(key);

	playlist = la_playlist_new();
	la_playlist_set_name(playlist, title);
	la_playlist_set_icon(playlist, icon);
	la_playlist_set_uri(playlist, filename);

	//sidebar_add_playlist(self, playlist);

	gtk_list_store_append(GTK_LIST_STORE(self->model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(self->model), &iter,
			   COL_PLAYLIST, playlist, -1);

	g_free(title);
	g_free(icon);
	g_free(filename);
    }

    gnome_config_pop_prefix();
}


/**
 * write sidebar settings using gnome-config
 * TODO: implements without gnome-config
 */
void sidebar_write(Sidebar * self)
{
    gint num;
    gchar key[256];
    LaPlaylist *playlist;
    GtkTreeIter iter;
    num = 0;

    g_snprintf(key, sizeof(key), "=%s=/sidebar/", self->filename);
    gnome_config_clean_section(key);

    gnome_config_push_prefix(key);

    if (gtk_tree_model_get_iter_first(self->model, &iter)) {
	do {
	    num++;
	    gtk_tree_model_get(self->model, &iter,
			       COL_PLAYLIST, &playlist, -1);

	    g_snprintf(key, sizeof(key), "Title%d", num);
	    gnome_config_set_string(key, la_playlist_get_name(playlist));

	    g_snprintf(key, sizeof(key), "Icon%d", num);
	    gnome_config_set_string(key, la_playlist_get_icon(playlist));

	    g_snprintf(key, sizeof(key), "Filename%d", num);
	    gnome_config_set_string(key, la_playlist_get_uri(playlist));

	} while (gtk_tree_model_iter_next(self->model, &iter));
    }

    gnome_config_set_int("NumberOfPlaylists", num);

    gnome_config_pop_prefix();

    gnome_config_sync();
}

void sidebar_add_playlist(Sidebar * self, LaPlaylist * playlist)
{
    GtkTreeIter iter;

    gtk_list_store_append(GTK_LIST_STORE(self->model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(self->model), &iter,
		       COL_PLAYLIST, playlist, -1);
    sidebar_write(self);
}

void sidebar_remove_playlist(Sidebar * self, LaPlaylist * playlist)
{
    GtkTreeIter iter;

    if (sidebar_lookup_playlist(self, playlist, &iter))
	gtk_list_store_remove(GTK_LIST_STORE(self->model), &iter);
    sidebar_write(self);
}

void sidebar_update_playlist(Sidebar * self, LaPlaylist * playlist)
{
    GtkTreeIter iter;
    LaPlaylist *current;

    if (gtk_tree_model_get_iter_first(self->model, &iter)) {
	do {
	    gtk_tree_model_get(self->model, &iter,
			       COL_PLAYLIST, &current, -1);
	    if (current == playlist) {
		gtk_tree_model_row_changed(self->model, NULL, &iter);
		break;
	    }

	} while (gtk_tree_model_iter_next(self->model, &iter));
    }
    sidebar_write(self);
}

/* boilerplates -----------------------------------------*/

LA_TYPE_BOILERPLATE(Sidebar, sidebar, G_TYPE_OBJECT)

/* sidebar.c */

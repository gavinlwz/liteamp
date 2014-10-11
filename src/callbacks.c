/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "liteamp.h"
#include "playlist.h"

#include "playlist-dialog.h"
#include "location-dialog.h"
#include "track-dialog.h"
#include "prefs-dialog.h"
#include "util.h"

void on_add_playlist_activate(GtkAction * action, gpointer user_data)
{
    PlaylistDialog *dialog;
    dialog = playlist_dialog_new(get_window(app));
    playlist_dialog_show(dialog, NULL);
}


void on_remove_playlist_activate(GtkAction * action, gpointer user_data)
{
    LaPlaylist *playlist;
    GtkResponseType response;

    playlist = playlist_get_playlist(get_playlist(app));
    if (!playlist) {
	la_message_box(get_window(app), _("Choose a playlist to delete!"));
	return;
    }

    response = la_prompt_boxv(get_window(app),
			      GTK_MESSAGE_QUESTION,
			      GTK_BUTTONS_YES_NO,
			      _("Remove the playlist \"%s\"?"),
			      la_playlist_get_name(playlist));

    if (response = GTK_RESPONSE_YES) {
	sidebar_remove_playlist(get_sidebar(app), playlist);
    }
}


void on_add_file_activate(GtkAction * action, gpointer user_data)
{
    GtkWidget *dialog;
    GtkResponseType response;
    gchar last_dir[PATH_MAX];
    GSList *filenames;
    gchar *filename;

    g_strlcpy(last_dir, prefs.last_dir, sizeof(last_dir));

    dialog = gtk_file_chooser_dialog_new(_("Add Files"),
					  get_window(app),
					  GTK_FILE_CHOOSER_ACTION_OPEN,
					  GTK_STOCK_ADD, 1,
					  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					  NULL);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), last_dir);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
    gtk_widget_show_all(dialog);

    while((response = gtk_dialog_run(GTK_DIALOG(dialog))) == 1) {

	filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
	while(filenames) {
	    filename = (gchar*)filenames->data;
	    if (filename) {
		playlist_add_file(get_playlist(app), filename, TRUE);
		g_free(filename);
	    }
	    filenames = g_slist_next(filenames);
	}
	g_slist_free(filenames);

	playlist_write(get_playlist(app));
    }

    gtk_widget_destroy(dialog);
}


void on_add_directory_activate(GtkAction * action, gpointer user_data)
{
    GtkWidget *dialog;
    GtkResponseType response;
    gchar last_dir[PATH_MAX];
    const gchar *filename;

    g_strlcpy(last_dir, prefs.last_dir, sizeof(last_dir));

    dialog = gtk_file_chooser_dialog_new(_("Add Directories"),
					  get_window(app),
					  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					  GTK_STOCK_ADD, 1,
					  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					  NULL);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), last_dir);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
    gtk_widget_show_all(dialog);

    while ((response = gtk_dialog_run(GTK_DIALOG(dialog))) == 1) {

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	playlist_add_directory(get_playlist(app), filename, TRUE, TRUE);

	g_free(prefs.last_dir);
	prefs.last_dir = g_path_get_dirname(filename);

	playlist_write(get_playlist(app));
    }

    gtk_widget_destroy(dialog);
}


void on_add_location_activate(GtkAction * action, gpointer user_data)
{
    LocationDialog *dialog = location_dialog_new(get_window(app));
    location_dialog_show(dialog, "");
}


void on_import_activate(GtkAction * action, gpointer user_data)
{
    GtkFileSelection *fs;
    GtkResponseType response;
    const gchar *filename;
    gchar last_dir[PATH_MAX];

    g_strlcpy(last_dir, prefs.last_dir, sizeof(last_dir));
    if (!la_str_has_suffix_case(prefs.last_dir, G_DIR_SEPARATOR_S))
	g_strlcat(last_dir, "/", sizeof(last_dir));

    fs = GTK_FILE_SELECTION(gtk_file_selection_new
			    (_("Select .pls Files to Import...")));
    gtk_file_selection_set_select_multiple(fs, FALSE);
    gtk_file_selection_set_filename(fs, last_dir);
    gtk_widget_show_all(GTK_WIDGET(fs));

    response = gtk_dialog_run(GTK_DIALOG(fs));

    if (response == GTK_RESPONSE_OK) {
	filename = gtk_file_selection_get_filename(fs);
	playlist_import_pls(get_playlist(app), filename);

	g_free(prefs.last_dir);
	prefs.last_dir = g_path_get_dirname(filename);
    }

    gtk_widget_destroy(GTK_WIDGET(fs));
}


void on_export_activate(GtkAction * action, gpointer user_data)
{
    LaPlaylist *playlist;
    gchar *title;
    GtkFileSelection *fs;
    GtkResponseType response;
    const gchar *filename;

    playlist = playlist_get_playlist(get_playlist(app));
    if (playlist) {
	title =
	    g_strdup_printf(_("Export the playlist \"%s\" as..."),
			    la_playlist_get_name(playlist));
	filename =
	    g_strdup_printf("%s.pls", la_playlist_get_name(playlist));
    } else {
	title = g_strdup(_("Export the library as..."));
	filename = g_strdup("liteamp-library.pls");
    }

    fs = GTK_FILE_SELECTION(gtk_file_selection_new(title));
    gtk_file_selection_set_filename(fs, filename);
    gtk_file_selection_set_select_multiple(fs, FALSE);
    gtk_widget_show_all(GTK_WIDGET(fs));

    response = gtk_dialog_run(GTK_DIALOG(fs));

    if (response == GTK_RESPONSE_OK) {
	filename = gtk_file_selection_get_filename(fs);
	playlist_export_pls(get_playlist(app), filename);

	g_free(prefs.last_dir);
	prefs.last_dir = g_path_get_dirname(filename);
    }

    gtk_widget_destroy(GTK_WIDGET(fs));
    g_free(title);
}


void on_quit_activate(GtkAction * action, gpointer user_data)
{
    gtk_main_quit();
}


void on_cut_activate(GtkAction * action, gpointer user_data)
{
    playlist_cut_selection(get_playlist(app));
}


void on_copy_activate(GtkAction * action, gpointer user_data)
{
    playlist_copy_selection(get_playlist(app));
}


void on_paste_activate(GtkAction * action, gpointer user_data)
{
    playlist_paste_selection(get_playlist(app));
}


void on_clear_activate(GtkAction * action, gpointer user_data)
{
    playlist_clear_selection(get_playlist(app));
}


void on_select_all_activate(GtkAction * action, gpointer user_data)
{
    playlist_select_all(get_playlist(app));
}


void on_select_none_activate(GtkAction * action, gpointer user_data)
{
    playlist_select_none(get_playlist(app));
}


void on_invert_selection_activate(GtkAction * action, gpointer user_data)
{
    playlist_invert_selection(get_playlist(app));
}


void on_properties_activate(GtkAction * action, gpointer user_data)
{
    if (sidebar_has_focus(get_sidebar(app))) {
	LaPlaylist *playlist;
	PlaylistDialog *dialog;
	playlist = sidebar_get_cursor_playlist(get_sidebar(app));
	if (playlist) {
	    dialog = playlist_dialog_new(get_window(app));
	    playlist_dialog_show(dialog, playlist);
	}
    } else {
	/*
	LaTrack *track;
	TrackDialog *dialog;
	track = playlist_get_cursor_track(get_playlist(app));
	if (track) {
	    dialog = track_dialog_new(get_window(app));
	    track_dialog_show(dialog, track);
	}
	*/
	TrackDialog *dialog;
	GSList *tracks = playlist_get_selected_tracks(get_playlist(app));
	if (tracks) {
	    dialog = track_dialog_new(get_window(app));
	    track_dialog_show(dialog, tracks);
	}
    }
}

void on_preferences_activate(GtkAction * action, gpointer user_data)
{
    PrefsDialog *dialog;
    dialog = prefs_dialog_new(get_window(app));
    prefs_dialog_show(dialog, &prefs);
}


void on_configure_effects_activate(GtkAction * action, gpointer user_data)
{
    //TODO:extracts equalizer from effect dialog
    effect_show_dialog(app);
}


void on_previous_activate(GtkAction * action, gpointer user_data)
{
    playlist_previous(get_playlist(app));
}


void on_start_activate(GtkAction * action, gpointer user_data)
{
    player_start(get_player(app));
}


void on_pause_activate(GtkToggleAction * action, gpointer user_data)
{
    gboolean pause = gtk_toggle_action_get_active(action);
    if (player_is_paused(get_player(app)) == pause)
	return;
    player_pause(get_player(app));
}


void on_stop_activate(GtkAction * action, gpointer user_data)
{
    player_stop(get_player(app));
}


void on_next_activate(GtkAction * action, gpointer user_data)
{
    playlist_next(get_playlist(app));
}


void on_back_activate(GtkAction * action, gpointer user_data)
{
    player_back(get_player(app));
}


void on_forward_activate(GtkAction * action, gpointer user_data)
{
    player_forward(get_player(app));
}


void on_mute_activate(GtkToggleAction * action, gpointer user_data)
{
    prefs.mute = gtk_toggle_action_get_active(action);
    update_ui_mute(app);
}


void on_volume_down_activate(GtkAction * action, gpointer user_data)
{
    mixer_volume_down(get_mixer(app));
    update_ui_volume(app);
}


void on_volume_up_activate(GtkAction * action, gpointer user_data)
{
    mixer_volume_up(get_mixer(app));
    update_ui_volume(app);
}


void on_shuffle_activate(GtkToggleAction * action, gpointer user_data)
{
    prefs.shuffle = gtk_toggle_action_get_active(action);
    update_ui_shuffle(app);
    playlist_shuffle(get_playlist(app));
}


void on_repeat_activate(GtkToggleAction * action, gpointer user_data)
{
    prefs.repeat = gtk_toggle_action_get_active(action);
    update_ui_repeat(app);
}


void on_effects_activate(GtkToggleAction * action, gpointer user_data)
{
    prefs.effects = gtk_toggle_action_get_active(action);
    update_ui_effects(app);
}


void on_compact_activate(GtkToggleAction * action, gpointer user_data)
{
    prefs.compact = gtk_toggle_action_get_active(action);
    update_ui_layout(app);
}


void on_visualizer_activate(GtkToggleAction * action, gpointer user_data)
{
    prefs.visualizer = gtk_toggle_action_get_active(action);
    update_ui_layout(app);
}


void on_sidebar_activate(GtkToggleAction * action, gpointer user_data)
{
    prefs.sidebar = gtk_toggle_action_get_active(action);
    update_ui_layout(app);
}


void on_playlist_activate(GtkToggleAction * action, gpointer user_data)
{
    prefs.playlist = gtk_toggle_action_get_active(action);
    update_ui_layout(app);
}


void on_statusbar_activate(GtkToggleAction * action, gpointer user_data)
{
    prefs.statusbar = gtk_toggle_action_get_active(action);
    update_ui_layout(app);
}


void on_jump_to_activate(GtkAction * action, gpointer user_data)
{
    playlist_jump_to(get_playlist(app));
}


void on_locate_activate(GtkAction * action, gpointer user_data)
{
    player_locate(get_player(app));
}


void on_refresh_activate(GtkAction * action, gpointer user_data)
{
    playlist_refresh(get_playlist(app));
}


void on_about_activate(GtkAction * action, gpointer user_data)
{
    about_dialog_show();
}


gchar *on_position_hscale_format_value(GtkScale * scale,
				       gdouble value, gpointer user_data)
{
    return la_format_time_str_new((gulong) value);
}


void on_position_hscale_value_changed(GtkRange * range, gpointer user_data)
{
    decoder_seek(gtk_range_get_value(range));
    update_ui_position(app);
}


void on_volume_hscale_value_changed(GtkRange * range, gpointer user_data)
{
    mixer_set_volume(get_mixer(app), gtk_range_get_value(range));
    update_ui_volume(app);
}


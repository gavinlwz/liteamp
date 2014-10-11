/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

void on_add_playlist_activate(GtkAction * action, gpointer user_data);
void on_remove_playlist_activate(GtkAction * action, gpointer user_data);
void on_add_file_activate(GtkAction * action, gpointer user_data);
void on_add_directory_activate(GtkAction * action, gpointer user_data);
void on_add_location_activate(GtkAction * action, gpointer user_data);
void on_import_activate(GtkAction * action, gpointer user_data);
void on_export_activate(GtkAction * action, gpointer user_data);
void on_quit_activate(GtkAction * action, gpointer user_data);
void on_cut_activate(GtkAction * action, gpointer user_data);
void on_copy_activate(GtkAction * action, gpointer user_data);
void on_paste_activate(GtkAction * action, gpointer user_data);
void on_clear_activate(GtkAction * action, gpointer user_data);
void on_select_all_activate(GtkAction * action, gpointer user_data);
void on_select_none_activate(GtkAction * action, gpointer user_data);
void on_invert_selection_activate(GtkAction * action, gpointer user_data);
void on_properties_activate(GtkAction * action, gpointer user_data);
void on_preferences_activate(GtkAction * action, gpointer user_data);
void on_configure_effects_activate(GtkAction * action, gpointer user_data);
void on_previous_activate(GtkAction * action, gpointer user_data);
void on_start_activate(GtkAction * action, gpointer user_data);
void on_pause_activate(GtkToggleAction * action, gpointer user_data);
void on_stop_activate(GtkAction * action, gpointer user_data);
void on_next_activate(GtkAction * action, gpointer user_data);
void on_back_activate(GtkAction * action, gpointer user_data);
void on_forward_activate(GtkAction * action, gpointer user_data);
void on_mute_activate(GtkToggleAction * action, gpointer user_data);
void on_volume_down_activate(GtkAction * action, gpointer user_data);
void on_volume_up_activate(GtkAction * action, gpointer user_data);
void on_shuffle_activate(GtkToggleAction * action, gpointer user_data);
void on_repeat_activate(GtkToggleAction * action, gpointer user_data);
void on_effects_activate(GtkToggleAction * action, gpointer user_data);
void on_compact_activate(GtkToggleAction * action, gpointer user_data);
void on_visualizer_activate(GtkToggleAction * action, gpointer user_data);
void on_sidebar_activate(GtkToggleAction * action, gpointer user_data);
void on_playlist_activate(GtkToggleAction * action, gpointer user_data);
void on_statusbar_activate(GtkToggleAction * action, gpointer user_data);
void on_jump_to_activate(GtkAction * action, gpointer user_data);
void on_locate_activate(GtkAction * action, gpointer user_data);
void on_refresh_activate(GtkAction * action, gpointer user_data);
void on_about_activate(GtkAction * action, gpointer user_data);

gchar *on_position_hscale_format_value(GtkScale * scale, gdouble value, gpointer user_data);
void on_position_hscale_value_changed(GtkRange * range, gpointer user_data);
void on_volume_hscale_value_changed(GtkRange * range, gpointer user_data);

G_END_DECLS
#endif

/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgnomeui/libgnomeui.h>

#include "action.h"
#include "stock.h"
#include "callbacks.h"

static GtkActionEntry entries[] = {
    { "FileMenu", NULL, "_File" },
    { "EditMenu", NULL, "_Edit" },
    { "PlayMenu", NULL, "_Play" },
    { "ViewMenu", NULL, "_View" },
    { "HelpMenu", NULL, "_Help" },
    { "AddPlaylist", LA_STOCK_ADD_PLAYLIST, "_Add Playlist", "<control>N", NULL, G_CALLBACK(on_add_playlist_activate) },
    { "RemovePlaylist", LA_STOCK_REMOVE_PLAYLIST, "_Remove Playlist", NULL, NULL, G_CALLBACK(on_remove_playlist_activate) },
    { "AddFile", LA_STOCK_ADD_FILE, "Add _File", "<control>F", NULL, G_CALLBACK(on_add_file_activate) },
    { "AddDirectory", LA_STOCK_ADD_DIRECTORY, "Add _Directory", "<control>D", NULL, G_CALLBACK(on_add_directory_activate) },
    { "AddLocation", LA_STOCK_ADD_LOCATION, "Add _Location", "<control>L", NULL, G_CALLBACK(on_add_location_activate) },
    { "Import", NULL, "_Import", NULL, NULL, G_CALLBACK(on_import_activate) },
    { "Export", NULL, "_Export", NULL, NULL, G_CALLBACK(on_export_activate) },
    { "Quit", GTK_STOCK_QUIT, "_Quit", NULL, NULL, G_CALLBACK(on_quit_activate) },
    { "Cut", GTK_STOCK_CUT, "Cu_t", "<control>X", NULL, G_CALLBACK(on_cut_activate) },
    { "Copy", GTK_STOCK_COPY, "_Copy", "<control>C", NULL, G_CALLBACK(on_copy_activate) },
    { "Paste", GTK_STOCK_PASTE, "_Paste", "<control>V", NULL, G_CALLBACK(on_paste_activate) },
    { "Clear", GTK_STOCK_CLEAR, "C_lear", NULL, NULL, G_CALLBACK(on_clear_activate) },
    { "SelectAll", NULL, "Select _All", "<control>A", NULL, G_CALLBACK(on_select_all_activate) },
    { "SelectNone", NULL, "Select _None", NULL, NULL, G_CALLBACK(on_select_none_activate) },
    { "InvertSelection", NULL, "Invert Selection", NULL, NULL, G_CALLBACK(on_invert_selection_activate) },
    { "Properties", GTK_STOCK_PROPERTIES, "_Properties", "<control>I", NULL, G_CALLBACK(on_properties_activate) },
    { "Preferences", GTK_STOCK_PREFERENCES, "Prefere_nces", NULL, NULL, G_CALLBACK(on_preferences_activate) },
    { "ConfigureEffects", NULL, "Configure E_ffects", "<control>E", NULL, G_CALLBACK(on_configure_effects_activate) },
    { "Previous", LA_STOCK_PREVIOUS, "_Previous", "Z", NULL, G_CALLBACK(on_previous_activate) },
    { "Start", LA_STOCK_START, "_Start", "X", NULL, G_CALLBACK(on_start_activate) },
    { "Stop", LA_STOCK_STOP, "S_top", "V", NULL, G_CALLBACK(on_stop_activate) },
    { "Next", LA_STOCK_NEXT, "_Next", "B", NULL, G_CALLBACK(on_next_activate) },
    { "Back", LA_STOCK_BACK, "_Back", "N", NULL, G_CALLBACK(on_back_activate) },
    { "Forward", LA_STOCK_FORWARD, "_Forward", "M", NULL, G_CALLBACK(on_forward_activate) },
    { "VolumeDown", LA_STOCK_VOLUME_DOWN, "Volume _Down", "comma", NULL, G_CALLBACK(on_volume_down_activate) },
    { "VolumeUp", LA_STOCK_VOLUME_UP, "Volume _Up", "period", NULL, G_CALLBACK(on_volume_up_activate) },
    { "JumpTo", GTK_STOCK_JUMP_TO, "_Jump To", "<control>J", NULL, G_CALLBACK(on_jump_to_activate) },
    { "Locate", NULL, "_Locate", "<control>L", NULL, G_CALLBACK(on_locate_activate) },
    { "Refresh", GTK_STOCK_REFRESH, "_Refresh", "<control>R", NULL, G_CALLBACK(on_refresh_activate) },
    { "About", GNOME_STOCK_ABOUT, "_About", NULL, NULL, G_CALLBACK(on_about_activate) },
};

static GtkToggleActionEntry toggle_entries[] = {
    { "Pause", LA_STOCK_PAUSE, "P_ause", "C", NULL, G_CALLBACK(on_pause_activate), FALSE },
    { "Mute", LA_STOCK_MUTE, "_Mute", "slash", NULL, G_CALLBACK(on_mute_activate), FALSE },
    { "Shuffle", LA_STOCK_SHUFFLE, "S_huffle", "S", NULL, G_CALLBACK(on_shuffle_activate), FALSE },
    { "Repeat", LA_STOCK_REPEAT, "_Repeat", "R", NULL, G_CALLBACK(on_repeat_activate), FALSE },
    { "Effects", NULL, "Effects", "E", NULL, G_CALLBACK(on_effects_activate), FALSE },
    { "Compact", NULL, "_Compact", "F8", NULL, G_CALLBACK(on_compact_activate), FALSE },
    { "Visualizer", NULL, "_Visualizer", NULL, NULL, G_CALLBACK(on_visualizer_activate), FALSE },
    { "Sidebar", NULL, "_Sidebar", "F9", NULL, G_CALLBACK(on_sidebar_activate), FALSE },
    { "Playlist", NULL, "_Playlist", NULL, NULL, G_CALLBACK(on_playlist_activate), FALSE },
    { "Statusbar", NULL, "S_tatusbar", NULL, NULL, G_CALLBACK(on_statusbar_activate), FALSE },
};

//static GtkToggleActionEntry radio_entries[] = { };

GtkActionGroup *init_action_group(gpointer user_data)
{
    GtkActionGroup * action_group = gtk_action_group_new ("MainGroup");
    gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), user_data);
    gtk_action_group_add_toggle_actions (action_group, toggle_entries, G_N_ELEMENTS (toggle_entries), user_data);
    //gtk_action_group_add_radio_actions (action_group, radio_entries, G_N_ELEMENTS (radio_entries), 0, radio_action_callback, user_data);
    return action_group;
}

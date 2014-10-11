/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "liteamp.h"
#include "prefs-dialog.h"
#include "util.h"
#include "la-track-column.h"
#include "la-codeset.h"

/* private-----------------------------------------------*/

struct _PrefsDialog {
    GObject parent;

    GladeXML *gladexml;
    GtkWidget *dialog;
    // general tab
    GtkWidget *plc_treeview;
    GtkWidget *plc_avail_treeview;
    GtkWidget *plc_up_button;
    GtkWidget *plc_down_button;
    GtkWidget *plc_add_button;
    GtkWidget *plc_remove_button;
    GtkWidget *osd_check;
    GtkWidget *save_geometry_check;
    // input tab
    GtkWidget *select_decoder_by_contents_check;
    GtkWidget *codeset_treeview;
    GtkWidget *codeset_avail_treeview;
    GtkWidget *codeset_up_button;
    GtkWidget *codeset_down_button;
    GtkWidget *codeset_add_button;
    GtkWidget *codeset_remove_button;
    // output tab
    GtkWidget *output_device_combo;
    GtkWidget *software_volume_check;
    GtkWidget *effects_check;

    GtkTreeModel *plc_model;
    GtkTreeSelection *plc_selection;
    GtkTreeModel *plc_avail_model;
    GtkTreeSelection *plc_avail_selection;
    GtkTreeModel *codeset_model;
    GtkTreeSelection *codeset_selection;
    GtkTreeModel *codeset_avail_model;
    GtkTreeSelection *codeset_avail_selection;

    Prefs *prefs;
};

struct _PrefsDialogClass {
    GObjectClass parent_class;
};

// available playback devices
static const gchar *output_devices[] = {
    "esd",
    "oss",
    "arts",
    "alsa",
    "alsa09",
    "sun",
    NULL,
};

// available playlist columns
static const gint plc_avail_columns[] = {
    //LA_TRACK_COLUMN_URI,
    LA_TRACK_COLUMN_TITLE,
    LA_TRACK_COLUMN_GENRE,
    LA_TRACK_COLUMN_ARTIST,
    LA_TRACK_COLUMN_ALBUM,
    LA_TRACK_COLUMN_TRACK,
    LA_TRACK_COLUMN_YEAR,
    LA_TRACK_COLUMN_COMMENT,
    //LA_TRACK_COLUMN_BITRATE,
    //LA_TRACK_COLUMN_SAMPLERATE,
    //LA_TRACK_COLUMN_FRAMES,
    //LA_TRACK_COLUMN_CHANNELS,
    //LA_TRACK_COLUMN_SECONDS,
    //LA_TRACK_COLUMN_BYTES,
    //LA_TRACK_COLUMN_ICON,
    //LA_TRACK_COLUMN_RATING,
    //LA_TRACK_COLUMN_CHECKED,
    //LA_TRACK_COLUMN_TIMESTAMP,
    LA_TRACK_COLUMN_TIME_STR,
    LA_TRACK_COLUMN_SIZE_STR,
    LA_TRACK_COLUMN_DATE_STR,
    LA_TRACK_COLUMN_RATING_STR,
    LA_TRACK_COLUMN_QUALITY_STR,
    //LA_TRACK_COLUMN_SUMMARY_STR,
    //LA_TRACK_COLUMN_SELF,
};

// column list for playlist columns treeview
enum _PLCColumns {
    PLC_COLUMN_TITLE,
    PLC_COLUMN_INDEX,		//invisible
    N_PLC_COLUMNS,
};

// column list for tag codeset treeview
enum _CodesetColumns {
    CODESET_COLUMN_NAME,
    CODESET_COLUMN_CODESET,	//invisible
    N_CODESET_COLUMNS,
};

static void update_data(PrefsDialog * self, gboolean save)
{
    GtkTreeIter iter;
    gboolean valid;
    gint plc_index, codeset, i;
    GSList *slist;

    if (save) {
	self->prefs->osd =
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					 (self->osd_check));
	self->prefs->save_geometry =
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					 (self->save_geometry_check));
	self->prefs->select_decoder_by_contents =
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					 (self->
					  select_decoder_by_contents_check));
	/*
	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(self->output_device_combo),
				      &iter);
	gtk_tree_model_get(gtk_combo_box_get_model(self->output_device_combo),
			   &iter,
			   gtk_combo_box_entry_get_text_column(self->output_device_combo), &self->prefs->output_device, -1);
			   */
	self->prefs->software_volume =
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					 (self->software_volume_check));
	self->prefs->effects =
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					 (self->effects_check));

	slist = NULL;
	valid = gtk_tree_model_get_iter_first(self->plc_model, &iter);
	while (valid) {
	    gtk_tree_model_get(self->plc_model,
			       &iter, PLC_COLUMN_INDEX, &plc_index, -1);
	    slist = g_slist_append(slist, GINT_TO_POINTER(plc_index));
	    valid = gtk_tree_model_iter_next(self->plc_model, &iter);
	}
	g_slist_free(self->prefs->playlist_columns);
	self->prefs->playlist_columns = slist;

	slist = NULL;
	valid = gtk_tree_model_get_iter_first(self->codeset_model, &iter);
	while (valid) {
	    gtk_tree_model_get(self->codeset_model,
			       &iter,
			       CODESET_COLUMN_CODESET, &codeset, -1);
	    slist =
		g_slist_append(slist,
			       (gpointer) la_codeset_get_iconv(codeset));
	    valid = gtk_tree_model_iter_next(self->codeset_model, &iter);
	}
	g_slist_free(self->prefs->tag_codesets);
	self->prefs->tag_codesets = slist;
    } else {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->osd_check),
				     self->prefs->osd);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				     (self->save_geometry_check),
				     self->prefs->save_geometry);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				     (self->
				      select_decoder_by_contents_check),
				     self->prefs->
				     select_decoder_by_contents);
	//@@gtk_entry_set_text(GTK_ENTRY(self->output_device_entry), self->prefs->output_device);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				     (self->software_volume_check),
				     self->prefs->software_volume);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
				     (self->effects_check),
				     self->prefs->effects);

	// fill playlist columns tree model
	gtk_list_store_clear(GTK_LIST_STORE(self->plc_model));
	slist = self->prefs->playlist_columns;
	while (slist) {
	    plc_index = GPOINTER_TO_INT(slist->data);
	    if (plc_index >= 0) {
		gtk_list_store_append(GTK_LIST_STORE
				      (self->plc_model), &iter);
		gtk_list_store_set(GTK_LIST_STORE
				   (self->plc_model),
				   &iter,
				   PLC_COLUMN_TITLE,
				   la_track_column_get_column_title
				   (plc_index), PLC_COLUMN_INDEX,
				   plc_index, -1);
	    }
	    slist = g_slist_next(slist);
	}

	// fill available playlist columns tree model
	gtk_list_store_clear(GTK_LIST_STORE(self->plc_avail_model));
	for (i = 0; i < G_N_ELEMENTS(plc_avail_columns); i++) {
	    plc_index = plc_avail_columns[i];
	    if (plc_index >= 0) {
		gtk_list_store_append(GTK_LIST_STORE
				      (self->plc_avail_model), &iter);
		gtk_list_store_set(GTK_LIST_STORE
				   (self->plc_avail_model),
				   &iter,
				   PLC_COLUMN_TITLE,
				   la_track_column_get_column_title
				   (plc_index), PLC_COLUMN_INDEX,
				   plc_index, -1);
	    }
	}

	// fill tag codesets tree model
	gtk_list_store_clear(GTK_LIST_STORE(self->codeset_model));
	slist = self->prefs->tag_codesets;
	while (slist) {
	    codeset = la_codeset_from_iconv(slist->data);
	    if (codeset >= 0) {
		gtk_list_store_append(GTK_LIST_STORE
				      (self->codeset_model), &iter);
		gtk_list_store_set(GTK_LIST_STORE
				   (self->codeset_model),
				   &iter,
				   CODESET_COLUMN_NAME,
				   la_codeset_get_name(codeset),
				   CODESET_COLUMN_CODESET, codeset, -1);
	    }
	    slist = g_slist_next(slist);
	}

	// fill available tag codesets tree model
	gtk_list_store_clear(GTK_LIST_STORE(self->codeset_avail_model));
	for (codeset = 0; codeset < LA_N_CODESETS; codeset++) {
	    gtk_list_store_append(GTK_LIST_STORE
				  (self->codeset_avail_model), &iter);
	    gtk_list_store_set(GTK_LIST_STORE
			       (self->codeset_avail_model),
			       &iter,
			       CODESET_COLUMN_NAME,
			       la_codeset_get_name(codeset),
			       CODESET_COLUMN_CODESET, codeset, -1);
	}
    }
}

static void response_cb(GtkDialog * dialog, gint response,
			PrefsDialog * self)
{
    if (response == GTK_RESPONSE_OK) {
	update_data(self, TRUE);
	prefs_apply();
	prefs_write();
    }

    gtk_widget_destroy(GTK_WIDGET(self->dialog));
}
static gboolean delete_event_cb(GtkWidget * widget, GdkEvent * event,
				PrefsDialog * self)
{
    g_object_unref(self);
    return FALSE;
}

static void plc_up_button_clicked_cb(GtkButton * button,
				     PrefsDialog * self)
{
    GtkTreeIter iter;
    GtkTreeIter iter2;
    GtkTreePath *path;
    gint plc_index, plc_index2;

    if (gtk_tree_selection_get_selected(self->plc_selection, NULL, &iter)) {
	path = gtk_tree_model_get_path(self->plc_model, &iter);
	if (path) {
	    if (gtk_tree_path_prev(path)) {
		gtk_tree_model_get_iter(self->plc_model, &iter2, path);

		gtk_list_store_swap(GTK_LIST_STORE
				    (self->plc_model), &iter, &iter2);
	    }
	    gtk_tree_path_free(path);
	}
    }
}

static void plc_down_button_clicked_cb(GtkButton * button,
				       PrefsDialog * self)
{
    GtkTreeIter iter;
    GtkTreeIter iter2;
    gint plc_index, plc_index2;

    if (gtk_tree_selection_get_selected(self->plc_selection, NULL, &iter)) {
	iter2 = iter;
	if (gtk_tree_model_iter_next(self->plc_model, &iter2)) {
	    gtk_list_store_swap(GTK_LIST_STORE
				(self->plc_model), &iter, &iter2);
	}
    }
}

static void plc_add_button_clicked_cb(GtkButton * button,
				      PrefsDialog * self)
{
    GtkTreeIter iter;
    gchar *plc_column;
    gint plc_index;

    if (gtk_tree_selection_get_selected
	(self->plc_avail_selection, NULL, &iter)) {

	gtk_tree_model_get(self->plc_avail_model, &iter,
			   PLC_COLUMN_TITLE, &plc_column,
			   PLC_COLUMN_INDEX, &plc_index, -1);

	gtk_list_store_append(GTK_LIST_STORE(self->plc_model), &iter);
	gtk_list_store_set(GTK_LIST_STORE
			   (self->plc_model), &iter,
			   PLC_COLUMN_TITLE, plc_column,
			   PLC_COLUMN_INDEX, plc_index, -1);
	gtk_tree_selection_select_iter(self->plc_selection, &iter);

	g_free(plc_column);
    }
}

static void plc_remove_button_clicked_cb(GtkButton * button,
					 PrefsDialog * self)
{
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(self->plc_selection, NULL, &iter)) {

	gtk_list_store_remove(GTK_LIST_STORE(self->plc_model), &iter);
    }
}

static void codeset_up_button_clicked_cb(GtkButton * button,
					 PrefsDialog * self)
{
    GtkTreeIter iter;
    GtkTreeIter iter2;
    GtkTreePath *path;
    gint codeset, codeset2;

    if (gtk_tree_selection_get_selected
	(self->codeset_selection, NULL, &iter)) {
	path = gtk_tree_model_get_path(self->codeset_model, &iter);
	if (path) {
	    if (gtk_tree_path_prev(path)) {
		gtk_tree_model_get_iter(self->codeset_model, &iter2, path);

		gtk_list_store_swap(GTK_LIST_STORE
				    (self->codeset_model), &iter, &iter2);
	    }
	    gtk_tree_path_free(path);
	}
    }
}


static void codeset_down_button_clicked_cb(GtkButton * button,
					   PrefsDialog * self)
{
    GtkTreeIter iter;
    GtkTreeIter iter2;
    gint codeset, codeset2;

    if (gtk_tree_selection_get_selected
	(self->codeset_selection, NULL, &iter)) {
	iter2 = iter;
	if (gtk_tree_model_iter_next(self->codeset_model, &iter2)) {
	    gtk_list_store_swap(GTK_LIST_STORE
				(self->codeset_model), &iter, &iter2);
	}
    }
}

static void codeset_add_button_clicked_cb(GtkButton * button,
					  PrefsDialog * self)
{
    GtkTreeIter iter;
    gchar *codeset_column;
    gint codeset;

    if (gtk_tree_selection_get_selected
	(self->codeset_avail_selection, NULL, &iter)) {

	gtk_tree_model_get(self->codeset_avail_model, &iter,
			   CODESET_COLUMN_NAME, &codeset_column,
			   CODESET_COLUMN_CODESET, &codeset, -1);

	gtk_list_store_append(GTK_LIST_STORE(self->codeset_model), &iter);
	gtk_list_store_set(GTK_LIST_STORE
			   (self->codeset_model), &iter,
			   CODESET_COLUMN_NAME, codeset_column,
			   CODESET_COLUMN_CODESET, codeset, -1);
	gtk_tree_selection_select_iter(self->codeset_selection, &iter);

	g_free(codeset_column);
    }
}

static void codeset_remove_button_clicked_cb(GtkButton * button,
					     PrefsDialog * self)
{
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected
	(self->codeset_selection, NULL, &iter)) {

	gtk_list_store_remove(GTK_LIST_STORE(self->codeset_model), &iter);
    }
}

/* public-----------------------------------------------*/

PrefsDialog *prefs_dialog_new(GtkWindow * parent)
{
    PrefsDialog *self;

    self = g_object_new(TYPE_PREFS_DIALOG, NULL);

    self->gladexml =
	glade_xml_new(PKGDATADIR "/glade/prefs-dialog.glade", NULL, NULL);

    self->dialog = glade_xml_get_widget(self->gladexml, "prefs_dialog");

    // general tab
    self->plc_treeview =
	glade_xml_get_widget(self->gladexml, "plc_treeview");
    self->plc_avail_treeview =
	glade_xml_get_widget(self->gladexml, "plc_avail_treeview");
    self->plc_up_button =
	glade_xml_get_widget(self->gladexml, "plc_up_button");
    self->plc_down_button =
	glade_xml_get_widget(self->gladexml, "plc_down_button");
    self->plc_add_button =
	glade_xml_get_widget(self->gladexml, "plc_add_button");
    self->plc_remove_button =
	glade_xml_get_widget(self->gladexml, "plc_remove_button");
    self->osd_check = glade_xml_get_widget(self->gladexml, "osd_check");
    self->save_geometry_check =
	glade_xml_get_widget(self->gladexml, "save_geometry_check");

    // input tab
    self->select_decoder_by_contents_check =
	glade_xml_get_widget(self->gladexml,
			     "select_decoder_by_contents_check");
    self->codeset_treeview =
	glade_xml_get_widget(self->gladexml, "codeset_treeview");
    self->codeset_avail_treeview =
	glade_xml_get_widget(self->gladexml, "codeset_avail_treeview");
    self->codeset_up_button =
	glade_xml_get_widget(self->gladexml, "codeset_up_button");
    self->codeset_down_button =
	glade_xml_get_widget(self->gladexml, "codeset_down_button");
    self->codeset_add_button =
	glade_xml_get_widget(self->gladexml, "codeset_add_button");
    self->codeset_remove_button =
	glade_xml_get_widget(self->gladexml, "codeset_remove_button");

    // output tab
    self->output_device_combo =
	glade_xml_get_widget(self->gladexml, "output_device_combo");
    self->software_volume_check =
	glade_xml_get_widget(self->gladexml, "software_volume_check");
    self->effects_check =
	glade_xml_get_widget(self->gladexml, "effects_check");

    self->plc_model =
	GTK_TREE_MODEL(gtk_list_store_new
		       (N_PLC_COLUMNS, G_TYPE_STRING, G_TYPE_INT));
    gtk_tree_view_set_model(GTK_TREE_VIEW(self->plc_treeview),
			    self->plc_model);
    self->plc_selection =
	gtk_tree_view_get_selection(GTK_TREE_VIEW(self->plc_treeview));
    gtk_tree_selection_set_mode(self->plc_selection, GTK_SELECTION_SINGLE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self->plc_treeview),
				gtk_tree_view_column_new_with_attributes
				(_("Column"),
				 gtk_cell_renderer_text_new(), "text",
				 PLC_COLUMN_TITLE, NULL));

    self->plc_avail_model =
	GTK_TREE_MODEL(gtk_list_store_new
		       (N_PLC_COLUMNS, G_TYPE_STRING, G_TYPE_INT));
    gtk_tree_view_set_model(GTK_TREE_VIEW(self->plc_avail_treeview),
			    self->plc_avail_model);
    self->plc_avail_selection =
	gtk_tree_view_get_selection(GTK_TREE_VIEW
				    (self->plc_avail_treeview));
    gtk_tree_selection_set_mode(self->plc_avail_selection,
				GTK_SELECTION_SINGLE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self->plc_avail_treeview),
				gtk_tree_view_column_new_with_attributes(_
									 ("Column"),
									 gtk_cell_renderer_text_new
									 (),
									 "text",
									 PLC_COLUMN_TITLE,
									 NULL));

    self->codeset_model =
	GTK_TREE_MODEL(gtk_list_store_new
		       (N_CODESET_COLUMNS, G_TYPE_STRING, G_TYPE_INT));
    gtk_tree_view_set_model(GTK_TREE_VIEW(self->codeset_treeview),
			    self->codeset_model);
    self->codeset_selection =
	gtk_tree_view_get_selection(GTK_TREE_VIEW(self->codeset_treeview));
    gtk_tree_selection_set_mode(self->codeset_selection,
				GTK_SELECTION_SINGLE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self->codeset_treeview),
				gtk_tree_view_column_new_with_attributes(_
									 ("Codeset"),
									 gtk_cell_renderer_text_new
									 (),
									 "text",
									 CODESET_COLUMN_NAME,
									 NULL));

    self->codeset_avail_model =
	GTK_TREE_MODEL(gtk_list_store_new
		       (N_CODESET_COLUMNS, G_TYPE_STRING, G_TYPE_INT));
    gtk_tree_view_set_model(GTK_TREE_VIEW(self->codeset_avail_treeview),
			    self->codeset_avail_model);
    self->codeset_avail_selection =
	gtk_tree_view_get_selection(GTK_TREE_VIEW
				    (self->codeset_avail_treeview));
    gtk_tree_selection_set_mode(self->codeset_avail_selection,
				GTK_SELECTION_SINGLE);
    gtk_tree_view_append_column(GTK_TREE_VIEW
				(self->codeset_avail_treeview),
				gtk_tree_view_column_new_with_attributes(_
									 ("Codeset"),
									 gtk_cell_renderer_text_new
									 (),
									 "text",
									 CODESET_COLUMN_NAME,
									 NULL));

    /*
    gtk_combo_set_popdown_strings(GTK_COMBO(self->output_device_combo),
				  la_array_to_list((gpointer *)
						   output_devices));
						   */

    gtk_window_set_transient_for(GTK_WINDOW(self->dialog), parent);
    gtk_dialog_set_default_response(GTK_DIALOG(self->dialog),
				    GTK_RESPONSE_OK);

    g_signal_connect(self->dialog, "response",
		     G_CALLBACK(response_cb), self);
    g_signal_connect(self->dialog, "delete-event",
		     G_CALLBACK(delete_event_cb), self);

    g_signal_connect(self->plc_up_button, "clicked",
		     G_CALLBACK(plc_up_button_clicked_cb), self);
    g_signal_connect(self->plc_down_button, "clicked",
		     G_CALLBACK(plc_down_button_clicked_cb), self);
    g_signal_connect(self->plc_remove_button, "clicked",
		     G_CALLBACK(plc_remove_button_clicked_cb), self);
    g_signal_connect(self->plc_add_button, "clicked",
		     G_CALLBACK(plc_add_button_clicked_cb), self);

    g_signal_connect(self->codeset_up_button, "clicked",
		     G_CALLBACK(codeset_up_button_clicked_cb), self);
    g_signal_connect(self->codeset_down_button, "clicked",
		     G_CALLBACK(codeset_down_button_clicked_cb), self);
    g_signal_connect(self->codeset_remove_button, "clicked",
		     G_CALLBACK(codeset_remove_button_clicked_cb), self);
    g_signal_connect(self->codeset_add_button, "clicked",
		     G_CALLBACK(codeset_add_button_clicked_cb), self);

    return self;
}

void prefs_dialog_show(PrefsDialog * self, Prefs * prefs)
{
    self->prefs = prefs;

    update_data(self, FALSE);

    gtk_widget_show_all(GTK_WIDGET(self->dialog));
}

Prefs *prefs_dialog_get_prefs(PrefsDialog * self)
{
    return self->prefs;
}

/* boilerplates -----------------------------------------*/

LA_TYPE_BOILERPLATE(PrefsDialog, prefs_dialog, G_TYPE_OBJECT)

/* prefs.c */

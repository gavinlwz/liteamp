/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __UTIL_H__
#define __UTIL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
/* boilerplate for simplest gobject */
#define LA_TYPE_BOILERPLATE(type, type_as_func, parent_type_macro)	\
GType type_as_func##_get_type (void)					\
{									\
	static const GTypeInfo self_info = {				\
	    sizeof (type##Class),					\
	    NULL,		/* base_init */				\
	    NULL,		/* base_finalize */			\
	    NULL,		/* class_init */			\
	    NULL,		/* class_finalize */			\
	    NULL,               /* class_data */			\
	    sizeof (type),						\
	    0,                  /* n_preallocs */			\
	    NULL,		/* instance_init */			\
	    0                   /* value_table */			\
	};								\
	static GType self_type = 0;					\
	if (self_type) return self_type;				\
	self_type = g_type_register_static(parent_type_macro, #type,	\
					   &self_info, 0);		\
	return self_type;						\
}
/* boilderplate for gobject has init & finalize functions */
#define LA_TYPE_BOILERPLATE_EX(type, type_as_func, parent_type_macro)	\
static void type_as_func##_class_init(type##Class *klass);		\
static void type_as_func##_instance_init(type *self);			\
static void type_as_func##_instance_finalize(GObject *gobject);		\
									\
GType type_as_func##_get_type (void)					\
{									\
	static const GTypeInfo self_info = {				\
	    sizeof (type##Class),					\
	    NULL,		/* base_init */				\
	    NULL,		/* base_finalize */			\
	    (GClassInitFunc) type_as_func##_class_init,			\
	    NULL,		/* class_finalize */			\
	    NULL,               /* class_data */			\
	    sizeof (type),						\
	    0,                  /* n_preallocs */			\
	    (GInstanceInitFunc) type_as_func##_instance_init,		\
	    0                   /* value_table */			\
	};								\
	static GType self_type = 0;					\
	if (self_type) return self_type;				\
	self_type = g_type_register_static(parent_type_macro, #type,	\
					   &self_info, 0);		\
	return self_type;						\
}									\
									\
static void type_as_func##_class_init(type##Class *klass)		\
{									\
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);		\
	gobject_class->finalize = type_as_func ## _instance_finalize;	\
}
/* charset */
    gchar * la_utf8_regularize(const gchar * str);
gchar *la_utf8_recover(const gchar * str, const gchar * codeset,
		       gint endurance);
gchar *la_str_to_utf8_with_fallback(const gchar * str,
				    GSList * fallback_codesets,
				    gchar ** used_codeset);
gchar *la_str_to_utf8(const gchar * str);

gchar *la_filename_to_utf8(const gchar * str);
gchar *la_utf8_to_filename(const gchar * str);
gchar *la_locale_to_utf8(const gchar * str);
gchar *la_utf8_to_locale(const gchar * str);

gchar *la_uri_to_local(const gchar * str);
gchar *la_local_to_uri(const gchar * str);

gchar *la_markup_escape(const gchar * str);
gchar *la_markup_unescape(const gchar * str);

/* string */

gboolean la_str_has_suffix(const gchar * str, const gchar * suffix,
			   gboolean case_sensitive);
gboolean la_str_has_suffix_nocase(const gchar * str, const gchar * suffix);
gboolean la_str_has_suffix_case(const gchar * str, const gchar * suffix);
gint la_strfindv(const gchar * str, const gchar ** str_array,
		 gboolean case_sensitive);
gboolean la_strfindv_nocase(const gchar * str, const gchar ** array);
gboolean la_strfindv_case(const gchar * str, const gchar ** array);

gchar *la_str_truncate(const gchar * str, gint length, gint dots);

gchar *la_format_size_str(gchar * buf, gint length, gulong size);
gchar *la_format_size_str_new(gulong size);
gchar *la_format_time_str(gchar * buf, gint length, gulong seconds);
gchar *la_format_time_str_new(gulong seconds);
gchar *la_format_date_str(gchar * buf, gint length, gulong timestamp);
gchar *la_format_date_str_new(gulong timestamp);

gchar *la_ltostr(gchar * buf, gint length, glong l);
gchar *la_ltostr_new(glong l);
glong la_strtol(const gchar * str, glong fallback);
gchar *la_btostr(gchar * buf, gint length, gboolean b);
gchar *la_btostr_new(gboolean b);
gboolean la_strtob(const gchar * str, gboolean fallback);

#define LA_STR_IS_EMPTY(str)	(((str) == NULL) || ((*(str)) == '\0'))
#define LA_STR_IS_NOT_EMPTY(str)	(((str) != NULL) && ((*(str)) != '\0'))

/* widget */

GdkPixbuf *la_pixbuf_new(const gchar * name, gint width, gint height);
gboolean la_image_set(GtkImage * image, const gchar * name,
		      GtkIconSize size, const gchar * fallback);
GtkImage *la_image_new(const gchar * name, GtkIconSize size,
		       const gchar * fallback);

void la_message_box(GtkWindow * parent, const gchar * message);
GtkResponseType la_prompt_box(GtkWindow * parent, GtkMessageType type,
			      GtkButtonsType buttons,
			      const gchar * message);
GtkResponseType la_prompt_boxv(GtkWindow * parent,
			       GtkMessageType type,
			       GtkButtonsType buttons,
			       const gchar * message_fmt, ...);

#define LA_SET_TOGGLE_BUTTON(widget, active) \
    (gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), active))

#define LA_GET_TOGGLE_BUTTON(widget) \
    (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))

#define LA_BLOCK_CALLBACK(object, callback) \
    (g_signal_handlers_block_by_func(G_OBJECT(object), G_CALLBACK(callback), NULL))

#define LA_UNBLOCK_CALLBACK(object, callback) \
    (g_signal_handlers_unblock_by_func(G_OBJECT(object), G_CALLBACK(callback), NULL))

/* misc */

gint la_str_to_enum_with_fallback(GType type, const gchar * str,
				  gint fallback);
gint la_str_to_enum(GType type, const gchar * str);
const gchar *la_enum_to_str_with_fallback(GType type, gint value,
					  gchar * fallback);
const gchar *la_enum_to_str(GType type, gint value);

gulong la_file_timestamp(const gchar * filename);
gulong la_file_size(const gchar * filename);

const gchar *la_timestamp();

GSList *la_array_to_slist(const gpointer * array);
gpointer *la_slist_to_array(GSList * slist);
GList *la_array_to_list(const gpointer * array);
gpointer *la_list_to_array(GList * slist);

void la_print_dump(const gpointer buf, gint length);

G_END_DECLS
#endif				/* __UTIL_H__ */

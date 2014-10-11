/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include "util.h"

gchar *la_utf8_regularize(const gchar * str)
{
    gchar *ret;
    gchar *ptr;
    ret = g_strdup(str);
    ptr = ret;
    // replaces all invalid characters with '?'
    while (!g_utf8_validate(ptr, -1, (const gchar **) &ptr))
	*ptr = '?';
    return ret;
}

gchar *la_utf8_recover(const gchar * str, const gchar * codeset,
		       gint endurance)
{
    gchar *ptr;
    gchar *ret;
    gint len;
    gint bytes_read;
    gint bytes_written;
    GError *error;

    ptr = g_strdup(str);

    // it's a already valid utf-8 string! :D
    if (g_utf8_validate(str, -1, NULL))
	return ptr;

    len = strlen(ptr);

    do {
	error = NULL;
	ret = g_convert(ptr, -1, "UTF-8", codeset, &bytes_read,
			&bytes_written, &error);

	// ok! we recover it :D
	if (error == NULL) {
	    g_free(ptr);
	    return ret;
	}
	// skip the character makes illegal sequence error
	if (error->code == G_CONVERT_ERROR_ILLEGAL_SEQUENCE) {
	    memmove(ptr + bytes_read, ptr + bytes_read + 1,
		    --len - bytes_read);
	}
	// try again :(
	if (ret)
	    g_free(ret);
	g_error_free(error);
    } while (--endurance);

    g_free(ptr);

    return la_utf8_regularize(str);
}

gchar *la_str_to_utf8_with_fallback(const gchar * str,
				    GSList * fallback_codesets,
				    gchar ** used_codeset)
{
    gchar *ret;
    gchar *ptr;
    gint len;
    gint i;
    const gchar *codeset;
    gint bytes_read;
    gint bytes_written;
    GError *error = NULL;
    gint retry_chance;

    if (used_codeset)
	*used_codeset = NULL;

    // it's a already valid utf-8 string! :D
    if (g_utf8_validate(str, -1, NULL))
	return g_strdup(str);

    // try to convert with current locale
    error = NULL;
    ret = g_locale_to_utf8(str, -1, NULL, NULL, &error);
    if (error == NULL)
	return ret;
    if (ret)
	g_free(ret);

    // no fallback codesets specified.
    if (!fallback_codesets)
	return g_strdup(str);

    // try to convert whole string
    // with speficifed fallback codesets
    while (fallback_codesets) {
	codeset = fallback_codesets->data;
	if (!codeset)
	    continue;

	error = NULL;
	ret = g_convert(str, -1, "UTF-8", codeset, &bytes_read,
			&bytes_written, &error);

	if (error == NULL) {
	    if (used_codeset)
		*used_codeset = (gchar *) codeset;
	    return ret;
	}

	g_error_free(error);

	fallback_codesets = g_slist_next(fallback_codesets);
    }

    // give up :(
    if (used_codeset)
	*used_codeset = NULL;
    return la_utf8_regularize(str);
}

inline gchar *la_str_to_utf8(const gchar * str)
{
    return la_str_to_utf8_with_fallback(str, NULL, NULL);
}


gchar *la_filename_to_utf8(const gchar * str)
{
    gchar *ret = g_filename_to_utf8(str, -1, NULL, NULL, NULL);
    if (!ret) {
	ret = g_locale_to_utf8(str, -1, NULL, NULL, NULL);
	if (!ret)
	    ret = g_strdup(str);
    }
    return ret;
}

gchar *la_utf8_to_filename(const gchar * str)
{
    gchar *ret = g_filename_from_utf8(str, -1, NULL, NULL, NULL);
    if (!ret) {
	ret = g_locale_to_utf8(str, -1, NULL, NULL, NULL);
	if (!ret)
	    ret = g_strdup(str);
    }
    return ret;
}

gchar *la_locale_to_utf8(const gchar * str)
{
    gchar *ret = g_locale_to_utf8(str, -1, NULL, NULL, NULL);
    if (!ret) {
	ret = g_strdup(str);
    }
    return ret;
}

gchar *la_utf8_to_locale(const gchar * str)
{
    gchar *ret = g_locale_from_utf8(str, -1, NULL, NULL, NULL);
    if (!ret) {
	ret = g_strdup(str);
    }
    return ret;
}

gchar *la_uri_to_local(const gchar * str)
{
    gchar *ret = gnome_vfs_get_local_path_from_uri(str);
    if (!ret) {
	ret = g_strdup(str);
    }
    return ret;
}

gchar *la_local_to_uri(const gchar * str)
{
    gchar *ret = gnome_vfs_get_uri_from_local_path(str);
    if (!ret) {
	ret = g_strdup(str);
    }
    return ret;
}

gchar *la_markup_escape(const gchar * str)
{
    gchar *ret = g_markup_escape_text(str, -1);
    if (!ret) {
	ret = g_strdup(str);
    }
    return g_strstrip(ret);
}

gchar *la_markup_unescape(const gchar * str)
{
    GString *ret_str = g_string_sized_new(512);
    gchar *ret;

    while (*str) {
	if (*str == '&') {
	    if (!strncmp(str, "amp;", 4)) {
		g_string_append_c(ret_str, '&');
		str += 4;
	    } else if (!strncmp(str, "lt;", 3)) {
		g_string_append_c(ret_str, '<');
		str += 3;
	    } else if (!strncmp(str, "gt;", 3)) {
		g_string_append_c(ret_str, '>');
		str += 3;
	    } else if (!strncmp(str, "quot;", 5)) {
		g_string_append_c(ret_str, '\"');
		str += 5;
	    }
	} else {
	    g_string_append_c(ret_str, *str);
	    ++str;
	}
    }


    ret = g_strdup(ret_str->str);
    g_string_free(ret_str, TRUE);
    return ret;
}

gboolean la_str_has_suffix(const gchar * str, const gchar * suffix,
			   gboolean case_sensitive)
{
    int len, suffix_len;
    if ((len = strlen(str)) < (suffix_len = strlen(suffix))) {
	return FALSE;
    }
    return (case_sensitive)
	? !strncmp(str + len - suffix_len, suffix, suffix_len)
	: !strncasecmp(str + len - suffix_len, suffix, suffix_len);
}

inline gboolean la_str_has_suffix_nocase(const gchar * str,
					 const gchar * suffix)
{
    return la_str_has_suffix(str, suffix, FALSE);
}

inline gboolean la_str_has_suffix_case(const gchar * str,
				       const gchar * suffix)
{
    return la_str_has_suffix(str, suffix, TRUE);
}

gint la_strfindv(const gchar * str, const gchar ** array,
		 gboolean case_sensitive)
{
    int i;
    for (i = 0; array[i]; i++) {
	if (case_sensitive) {
	    if (!strcmp(str, array[i]))
		return i;
	} else {
	    if (!strcasecmp(str, array[i]))
		return i;
	}
    }
    return -1;
}

inline gboolean la_strfindv_nocase(const gchar * str, const gchar ** array)
{
    return la_strfindv(str, array, FALSE);
}

inline gboolean la_strfindv_case(const gchar * str, const gchar ** array)
{
    return la_strfindv(str, array, TRUE);
}

gchar *la_str_truncate(const gchar * str, gint length, gint dots)
{
    gchar *res;
    gchar *p;

    if (str == NULL)
	return NULL;

    if (length <= 0 || strlen(str) <= (guint) length)
	return g_strdup(str);

    res = g_strndup(str, length);

    p = res + length - dots;
    while (--dots >= 0)
	*p++ = '.';

    return res;
}


gchar *la_format_size_str(gchar * buf, gint length, gulong size)
{
    if (size > 1024 * 1024 * 1024)
	g_snprintf(buf, length, "%.1fG",
		   (gdouble) size / (1024 * 1024 * 1024));
    else if (size > 1024 * 1024)
	g_snprintf(buf, length, "%.1fM", (gdouble) size / (1024 * 1024));
    else if (size > 1024)
	g_snprintf(buf, length, "%.1fK", (gdouble) size / 1024);
    else
	g_snprintf(buf, length, "%u", size);
    return buf;
}

inline gchar *la_format_size_str_new(gulong size)
{
    gchar buf[32];
    return g_strdup(la_format_size_str(buf, sizeof(buf), size));
}

gchar *la_format_time_str(gchar * buf, gint length, gulong sec)
{
    if (sec > 3600)
	g_snprintf(buf, length, "%u:%02u:%02u",
		   sec / 3600, (sec % 3600) / 60, (sec % 3600) % 60);
    else
	g_snprintf(buf, length, "%02u:%02u", sec / 60, sec % 60);
    return buf;
}

inline gchar *la_format_time_str_new(gulong seconds)
{
    gchar buf[32];
    return g_strdup(la_format_time_str(buf, sizeof(buf), seconds));
}

gchar *la_format_date_str(gchar * buf, gint length, gulong timestamp)
{
    GDate *gdate = g_date_new();
    g_date_set_time(gdate, timestamp);
    g_date_strftime(buf, length, "%c", gdate);
    g_date_free(gdate);
    return buf;
}

inline gchar *la_format_date_str_new(gulong timestamp)
{
    gchar buf[64];
    return g_strdup(la_format_date_str(buf, sizeof(buf), timestamp));
}

inline gchar *la_ltostr(gchar * buf, gint length, glong l)
{
    g_snprintf(buf, length, "%d", l);
    return buf;
}

inline gchar *la_ltostr_new(glong l)
{
    return g_strdup_printf("%d", l);
}

glong la_strtol(const gchar * str, glong fallback)
{
    glong l;
    gchar *tail;
    if (LA_STR_IS_EMPTY(str))
	return fallback;
    l = strtol(str, &tail, 10);
    return (str != tail) ? l : fallback;
}

inline gchar *la_btostr(gchar * buf, gint length, gboolean b)
{
    g_strlcpy(buf, (b) ? "true" : "false", length);
    return buf;
}

inline gchar *la_btostr_new(gboolean b)
{
    return g_strdup((b) ? "true" : "false");
}

gboolean la_strtob(const gchar * str, gboolean fallback)
{
    if (LA_STR_IS_EMPTY(str))
	return fallback;
    if (strcasecmp(str, "false") && strcasecmp(str, "no")
	&& strcmp(str, "off") && strcmp(str, "0"))
	return TRUE;
    return fallback;
}

GdkPixbuf *la_pixbuf_new(const gchar * name, gint width, gint height)
{
    GdkPixbuf *pixbuf;
    GdkPixbuf *scaled_pixbuf;

    if (LA_STR_IS_EMPTY(name))
	return NULL;

    pixbuf = gdk_pixbuf_new_from_file(name, NULL);
    if (pixbuf) {
	scaled_pixbuf =
	    gdk_pixbuf_scale_simple(pixbuf, width, height,
				    GDK_INTERP_BILINEAR);
	if (scaled_pixbuf) {
	    g_object_unref(pixbuf);
	    return scaled_pixbuf;
	}
	return pixbuf;
    }
    return NULL;
}

gboolean la_image_set(GtkImage * image, const gchar * name,
		      GtkIconSize size, const gchar * fallback)
{
    GdkPixbuf *pixbuf;
    GdkPixbuf *scaled_pixbuf;
    gint width, height;

    if (LA_STR_IS_EMPTY(name))
	goto LOAD_FALLBACK_IMAGE;

    // gtk_lookup_stock() doesn't search on user-defined stock. :(
    // And, gtk_image_new_from_stock() always returns a valid GtkImage*,
    // even if there's no stock specified. :(
    // so, i made some ugly workaround.

    if (!gtk_icon_size_lookup(size, &width, &height))
	gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);

    pixbuf = la_pixbuf_new(name, width, height);
    if (pixbuf) {
	gtk_image_set_from_pixbuf(image, pixbuf);
	g_object_unref(pixbuf);
    } else {
	// somewhat huristic ;)
	if (strchr(name, '.') || strchr(name, '/'))
	    goto LOAD_FALLBACK_IMAGE;
	gtk_image_set_from_stock(image, name, size);
    }
    return TRUE;

  LOAD_FALLBACK_IMAGE:
    if (!fallback)
	fallback = GTK_STOCK_MISSING_IMAGE;
    gtk_image_set_from_stock(image, fallback, size);
    return FALSE;
}

inline GtkImage *la_image_new(const gchar * name, GtkIconSize size,
			      const gchar * fallback)
{
    GtkImage *image = GTK_IMAGE(gtk_image_new());
    la_image_set(image, name, size, fallback);
    return image;
}

void la_message_box(GtkWindow * parent, const gchar * message)
{
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
				    GTK_DIALOG_MODAL |
				    GTK_DIALOG_DESTROY_WITH_PARENT,
				    GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
				    message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

GtkResponseType la_prompt_box(GtkWindow * parent, GtkMessageType type,
			      GtkButtonsType buttons,
			      const gchar * message)
{
    GtkWidget *dialog;
    GtkResponseType result;

    dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
				    GTK_DIALOG_DESTROY_WITH_PARENT,
				    type, buttons, message);

    result = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);

    return result;
}

GtkResponseType la_prompt_boxv(GtkWindow * parent,
			       GtkMessageType type,
			       GtkButtonsType buttons,
			       const gchar * message_fmt, ...)
{
    GtkResponseType result;
    gchar *message;
    va_list args;

    g_assert(message_fmt);
    va_start(args, message_fmt);
    message = g_strdup_vprintf(message_fmt, args);
    va_end(args);
    result = la_prompt_box(parent, type, buttons, message);
    g_free(message);
    return result;
}

gint la_str_to_enum_with_fallback(GType type, const gchar * str,
				  gint fallback)
{
    GEnumClass *enum_class;
    GEnumValue *enum_value;
    enum_class = g_type_class_ref(type);
    enum_value = g_enum_get_value_by_nick(enum_class, str);
    if (!enum_value)
	enum_value = g_enum_get_value_by_name(enum_class, str);
    g_type_class_unref(enum_class);
    return (enum_value) ? enum_value->value : fallback;
}

inline gint la_str_to_enum(GType type, const gchar * str)
{
    return la_str_to_enum_with_fallback(type, str, -1);
}

const gchar *la_enum_to_str_with_fallback(GType type, gint value,
					  gchar * fallback)
{
    GEnumClass *enum_class;
    GEnumValue *enum_value;
    const gchar *ret;
    enum_class = g_type_class_ref(type);
    enum_value = g_enum_get_value(enum_class, value);
    ret = (enum_value) ? enum_value->value_nick : fallback;
    g_type_class_unref(enum_class);
    return ret;
}

inline const gchar *la_enum_to_str(GType type, gint value)
{
    return la_enum_to_str_with_fallback(type, value, NULL);
}

inline gulong la_file_timestamp(const gchar * filename)
{
    struct stat st;
    stat(filename, &st);
    return (gulong) st.st_mtime;
}

inline gulong la_file_size(const gchar * filename)
{
    struct stat st;
    stat(filename, &st);
    return (gulong) st.st_size;
}

const gchar *la_timestamp()
{
    static gchar timestamp[20];
    GTimeVal time;
    g_get_current_time(&time);
    g_snprintf(timestamp, sizeof(timestamp), "%08x-%08x",
	       time.tv_sec, time.tv_usec);
    return timestamp;
}

GSList *la_array_to_slist(const gpointer * array)
{
    GSList *slist;
    const gpointer *ptr;

    slist = NULL;
    ptr = array;

    while (*ptr)
	slist = g_slist_append(slist, *ptr++);

    return slist;
}

gpointer *la_slist_to_array(GSList * slist)
{
    gpointer *array;
    gpointer *ptr;

    array = g_new0(gpointer, g_slist_length(slist));
    ptr = array;

    while (slist) {
	*ptr++ = slist->data;
	slist = g_slist_next(slist);
    }

    return array;
}

GList *la_array_to_list(const gpointer * array)
{
    GList *list;
    const gpointer *ptr;

    list = NULL;
    ptr = array;

    while (*ptr)
	list = g_list_append(list, *ptr++);

    return list;
}

gpointer *la_list_to_array(GList * list)
{
    gpointer *array;
    gpointer *ptr;

    array = g_new0(gpointer, g_list_length(list));
    ptr = array;

    while (list) {
	*ptr++ = list->data;
	list = g_list_next(list);
    }

    return array;
}

void la_print_dump(const gpointer buf, gint length)
{
    guchar *ptr = buf;
    while (length-- >= 0) {
	g_print("%02x ", *ptr++);
    }
    g_print("\n");
}


/* la-util.c */

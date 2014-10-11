/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include "mime.h"

inline gboolean is_mime_type_directory(const gchar * mime_type)
{
    return !strcasecmp(mime_type, MIME_TYPE_DIRECTORY);
}

inline gboolean is_mime_type_ogg(const gchar * mime_type)
{
    return (!strcasecmp(mime_type, MIME_TYPE_OGG)
	    || !strcasecmp(mime_type, MIME_TYPE_OGG_BROKEN));
}

inline gboolean is_mime_type_music(const gchar * mime_type)
{
    return (is_mime_type_mp3(mime_type)
	    || is_mime_type_mp3(mime_type));
}

inline gboolean is_mime_type_mp3(const gchar * mime_type)
{
    return (!strcasecmp(mime_type, MIME_TYPE_MP3)
	    || !strcasecmp(mime_type, MIME_TYPE_MP3_BROKEN));
}

inline gboolean is_mime_type_image(const gchar * mime_type)
{
    return (!strcasecmp(mime_type, MIME_TYPE_PNG)
	    || !strcasecmp(mime_type, MIME_TYPE_JPG)
	    || !strcasecmp(mime_type, MIME_TYPE_GIF)
	    || !strcasecmp(mime_type, MIME_TYPE_SVG));
}

/* mime.c */

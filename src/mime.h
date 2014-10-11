/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __MIME_H__
#define __MIME_H__

G_BEGIN_DECLS

// used mime type
#define MIME_TYPE_DIRECTORY "x-directory/normal"
#define MIME_TYPE_URI_LIST "text/uri-list"
#define MIME_TYPE_OGG "application/ogg"
#define MIME_TYPE_OGG_BROKEN "application/x-ogg"
#define MIME_TYPE_MP3 "audio/mpeg"
#define MIME_TYPE_MP3_BROKEN "audio/x-mp3"
#define MIME_TYPE_PNG "image/png"
#define MIME_TYPE_JPG "image/jpeg"
#define MIME_TYPE_GIF "image/gif"
#define MIME_TYPE_SVG "image/svg"

gboolean is_mime_type_directory(const gchar * mime_type);
gboolean is_mime_type_ogg(const gchar * mime_type);
gboolean is_mime_type_mp3(const gchar * mime_type);
gboolean is_mime_type_music(const gchar * mime_type);
gboolean is_mime_type_image(const gchar * mime_type);

G_END_DECLS
#endif				/* mime.h */

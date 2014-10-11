/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __INPUT_H__
#define __INPUT_H__

#include <libgnomevfs/gnome-vfs.h>

G_BEGIN_DECLS typedef struct _input_info {
    /* is this localfile ? */
    gboolean local;
    /* local or remote file uri */
    //GnomeVFSURI *uri;
    gchar *url;
    /* gnome vfs handler */
    GnomeVFSHandle *vfs_handle;
} input_info;

input_info *input_init(gchar * url);
gint input_quit(input_info *);
gint input_get_data(input_info *, gchar *, gint, gint);
gint input_seek(input_info *, GnomeVFSSeekPosition, GnomeVFSFileOffset);
gulong input_tell(input_info *);
gboolean input_is_local(input_info *);
G_END_DECLS
#endif				/* input.h */

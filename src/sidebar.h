/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __SIDEBAR_H__
#define __SIDEBAR_H__

#include "la-playlist.h"

G_BEGIN_DECLS
#define TYPE_SIDEBAR		(sidebar_get_type())
#define SIDEBAR(object)	(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_SIDEBAR, Sidebar))
#define SIDEBAR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_SIDEBAR, SidebarClass))
#define IS_SIDEBAR(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_SIDEBAR))
#define IS_SIDEBAR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_SIDEBAR))
typedef struct _Sidebar Sidebar;
typedef struct _SidebarClass SidebarClass;

GType sidebar_get_type(void) G_GNUC_CONST;
Sidebar *sidebar_new(void);

inline void sidebar_set_visible(Sidebar * self, gboolean visible);
inline gboolean sidebar_get_visible(Sidebar * self);
inline gboolean sidebar_has_focus(Sidebar * self);

LaPlaylist *sidebar_get_cursor_playlist(Sidebar * self);
LaPlaylist *sidebar_get_playlist_by_uri(Sidebar * self, const gchar * uri);

void sidebar_read(Sidebar * self);
void sidebar_write(Sidebar * self);

void sidebar_add_playlist(Sidebar * self, LaPlaylist * playlist);
void sidebar_remove_playlist(Sidebar * self, LaPlaylist * playlist);
void sidebar_update_playlist(Sidebar * self, LaPlaylist * playlist);

G_END_DECLS
#endif /*__SIDEBAR_H__ */

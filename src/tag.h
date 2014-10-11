/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __TAG_H__
#define __TAG_H__

G_BEGIN_DECLS void tag_init(GSList * fallback_list);

gboolean tag_read(LaTrack * track);
gboolean tag_write(LaTrack * track);
gboolean tag_remove(LaTrack * track);

G_END_DECLS
#endif				/* __TAG_H__ */

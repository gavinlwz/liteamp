/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __OGG_DECODER_H__
#define __OGG_DECODER_H__

G_BEGIN_DECLS void ogg_init(void);
void ogg_play(char *filename);
void ogg_stop(void);
void ogg_pause(gboolean);
void ogg_seek(gint time);
void ogg_next(char *filename);
gboolean ogg_is_our_file(char *filename);
gint ogg_get_cur_time();
gint ogg_get_cur_frame();
gint ogg_get_total_time_frame(char *filename, gint * time, gint * frame);

G_END_DECLS
#endif				/*ogg-decoder.h */

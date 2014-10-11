/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __MAD_DECODER_H__
#define __MAD_DECODER_H__

G_BEGIN_DECLS
/* mad control functions */
void mad_init();
void mad_play(char *filename);
void mad_stop();
void mad_pause(gboolean);
void mad_seek(gint time);
void mad_next(char *filename);
gboolean mad_is_our_file(char *filename);
gint mad_get_cur_time();
gint mad_get_cur_frame();
gint mad_get_total_time_frame(char *filename, gint * time, gint * frame);


G_END_DECLS
#endif				/*mad-decoder.h */

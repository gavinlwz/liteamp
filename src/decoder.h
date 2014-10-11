/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __DECODER_H__
#define __DECODER_H__

#include "gnome.h"

G_BEGIN_DECLS
#define BUF_SIZE 8192
    typedef struct _Decoder {
    gchar *filename;

    void (*init) (void);
    void (*play) (gchar * filename);
    void (*stop) (void);
    void (*pause) (gboolean);
    void (*seek) (gint time);
    void (*next) (gchar * filename);
     gboolean(*is_our_file) (gchar * filename);
     gint(*get_cur_time) (void);
     gint(*get_cur_frame) (void);
     gint(*get_total_time_frame) (gchar * filename, gint * time,
				  gint * frame);
} Decoder;

typedef enum _DecoderStatus {
    STOPPED = 0x0001,
    PLAYING = 0x0002,
    SEEKING = 0x0004
} DecoderStatus;

typedef enum _DecoderType {
    DECODER_TYPE_MAD,
    DECODER_TYPE_OGG,
    DECODER_TYPE_UNKNOWN,
} DecoderType;


typedef struct _decoder_data {
    gboolean playing;
    gboolean paused;
    Decoder *current_decoder;
} decoder_data;

typedef struct _queue_data {
    char *command;
    void *data;
} queue_data;

void decoder_quit(void);
void decoder_play(gchar * filename);
void decoder_pause(void);
void decoder_stop(void);
void decoder_seek(gint time);
gint decoder_get_cur_time();
gint decoder_get_cur_frame();
gint decoder_get_total_time(gchar * filename, gint * time, gint * frame);

G_END_DECLS
#endif				/*decoder.h */

/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include "liteamp.h"
#include "decoder.h"
#include "mad-decoder.h"
#include "ogg-decoder.h"
#include "playback.h"
#include "callbacks.h"
#include "util.h"
#include "prefs.h"

Decoder mad_decoder = {
    0,
    mad_init,
    mad_play,
    mad_stop,
    mad_pause,
    mad_seek,
    mad_next,
    mad_is_our_file,
    mad_get_cur_time,
    mad_get_cur_frame,
    mad_get_total_time_frame
};

Decoder ogg_decoder = {
    0,
    ogg_init,
    ogg_play,
    ogg_stop,
    ogg_pause,
    ogg_seek,
    ogg_next,
    ogg_is_our_file,
    ogg_get_cur_time,
    ogg_get_cur_frame,
    ogg_get_total_time_frame
};

decoder_data d_data = {
    0,
    0,
    NULL
};

Decoder *decoders[] = {
    &mad_decoder,
    &ogg_decoder,
    NULL
};

GAsyncQueue *queue = NULL;
pthread_t tid;

/*-----------------------------------------------------------------*/

DecoderType get_decoder_type(const gchar * filename);
Decoder *get_current_decoder(void);
gboolean get_decoder_playing(void);
gboolean get_decoder_paused(void);
void free_queue_data(queue_data * data);
void set_current_decoder(Decoder * decoder);
void d_play(gchar * filename);
void d_pause(void);
void d_stop(void);
void d_seek(gint time);

/*-----------------------------------------------------------------*/

void d_init()
{
    playback_set_playdevice(prefs.output_device);
    playback_volume_set_active(prefs.software_volume);
}

Decoder *decoder_new_from_filename(const gchar * filename)
{
    Decoder *decoder;

    switch (get_decoder_type(filename)) {
    case DECODER_TYPE_OGG:
	decoder = (Decoder *) & ogg_decoder;
	break;
    case DECODER_TYPE_MAD:
	decoder = (Decoder *) & mad_decoder;
	break;
    default:
	return NULL;
    }
    if (decoder->filename)
	free(decoder->filename);
    decoder->filename = g_strdup(filename);

    return decoder;
}


Decoder *get_current_decoder()
{
    return d_data.current_decoder;
}

void set_current_decoder(Decoder * decoder)
{
    d_data.current_decoder = decoder;
}

gboolean get_decoder_playing()
{
    return d_data.playing;
}

gboolean get_decoder_paused()
{
    return d_data.paused;
}

DecoderType get_decoder_type(const gchar * filename)
{
    if (prefs.select_decoder_by_contents) {
	/* select decoder by contents */
	if (ogg_decoder.is_our_file((char *) filename))
	    return DECODER_TYPE_OGG;
	else if (mad_decoder.is_our_file((char *) filename))
	    return DECODER_TYPE_MAD;
    } else {
	/* accept file suffix without reserve */
	if (la_str_has_suffix_nocase(filename, ".ogg")) {
	    return DECODER_TYPE_OGG;
	} else if (la_str_has_suffix_nocase(filename, ".mp3")) {
	    return DECODER_TYPE_MAD;
	}

	/* guess it */
	if (ogg_decoder.is_our_file((char *) filename))
	    return DECODER_TYPE_OGG;
	else if (mad_decoder.is_our_file((char *) filename))
	    return DECODER_TYPE_MAD;
    }
    /* give up */
    g_message("failed to get decoder type for: %s\n", filename);
    return DECODER_TYPE_UNKNOWN;
}

void d_play(gchar * filename)
{
    Decoder *decoder = NULL;

    decoder = decoder_new_from_filename(filename);
    if (!decoder) {
	/* failed to get decoder , goto nextsong */
	playlist_next(get_playlist(app));
    }
    if (decoder) {
	if (get_decoder_playing() && get_current_decoder()) {
	    if (decoder == get_current_decoder()) {
		get_current_decoder()->
		    next(get_current_decoder()->filename);
		d_data.playing = TRUE;
		d_data.paused = FALSE;
	    } else {
		d_init();
		get_current_decoder()->stop();
		set_current_decoder(decoder);
		get_current_decoder()->
		    play(get_current_decoder()->filename);
		d_data.playing = TRUE;
		d_data.paused = FALSE;
	    }
	} else {
	    d_init();
	    set_current_decoder(decoder);
	    get_current_decoder()->play(get_current_decoder()->filename);
	    d_data.playing = TRUE;
	    d_data.paused = FALSE;
	}
    }
}

void d_pause()
{
    if (get_decoder_playing() && get_current_decoder()) {
	d_data.paused = !d_data.paused;
	get_current_decoder()->pause(d_data.paused);
    }
}


void d_stop()
{
    if (get_decoder_playing() && get_current_decoder()) {
	d_data.playing = FALSE;
	d_data.paused = FALSE;
	get_current_decoder()->stop();
    }
}


void d_seek(gint time)
{
    if (get_decoder_playing() && get_current_decoder()) {
	get_current_decoder()->seek(time);
    }
}

void free_queue_data(queue_data * data)
{
    if (data) {
	if (data->command)
	    free(data->command);
	if (data->data)
	    free(data->data);
	free(data);
    }
}

gpointer decoder_loop(gpointer data)
{
    GAsyncQueue *d_queue = (GAsyncQueue *) data;
    queue_data *q_data;

    playback_init();
    while (1) {
	q_data = (queue_data *) g_async_queue_pop(d_queue);
	g_print("%s\n", q_data->command);
	if (q_data) {
	    if (!g_strncasecmp(q_data->command, "PLAY", 4)) {
		d_play((gchar *) (q_data->data));
		free_queue_data(q_data);
	    } else if (!g_strncasecmp(q_data->command, "STOP", 4)) {
		d_stop();
		free_queue_data(q_data);
	    } else if (!g_strncasecmp(q_data->command, "PAUSE", 5)) {
		d_pause();
		free_queue_data(q_data);
	    } else if (!g_strncasecmp(q_data->command, "SEEK", 4)) {
		d_seek(*(gint *) (q_data->data));
		free_queue_data(q_data);
	    } else if (!g_strncasecmp(q_data->command, "EXIT", 4)) {
		playback_unpause();
		d_stop();
		playback_quit();
		free_queue_data(q_data);
		pthread_exit(NULL);
	    } else {
		g_message("Unknown Command Type\n");
	    }
	}
    }
}


/*-----------------------------------------------------------------*/

void decoder_quit()
{
    if (queue) {
	queue_data *data = g_malloc0(sizeof(queue_data));
	data->command = g_strdup("EXIT");
	g_async_queue_push(queue, (gpointer) data);
	pthread_join(tid, NULL);
    }
}

void decoder_init()
{
    queue = g_async_queue_new();
    pthread_create(&tid, NULL, decoder_loop, queue);
}

void decoder_play(gchar * filename)
{
    if (queue && filename) {
	queue_data *data = g_malloc0(sizeof(queue_data));
	data->command = g_strdup("PLAY");
	data->data = g_strdup(filename);
	g_async_queue_push(queue, (gpointer) data);
    }
}


void decoder_pause()
{
    if (queue) {
	queue_data *data = g_malloc0(sizeof(queue_data));
	data->command = g_strdup("PAUSE");
	g_async_queue_push(queue, (gpointer) data);
    }
}


void decoder_stop()
{
    if (queue) {
	queue_data *data = g_malloc0(sizeof(queue_data));
	data->command = g_strdup("STOP");
	g_async_queue_push(queue, (gpointer) data);
    }
}


void decoder_seek(gint time)
{
    if (queue) {
	queue_data *data = g_malloc0(sizeof(queue_data));
	gint *value = g_malloc(sizeof(gint));
	*value = time;
	data->command = g_strdup("SEEK");
	data->data = value;
	g_async_queue_push(queue, (gpointer) data);
    }
}


gint decoder_get_cur_time()
{
    if (get_decoder_playing() && get_current_decoder()) {
	return get_current_decoder()->get_cur_time();
    } else {
	return -1;
    }
}

gint decoder_get_total_time_frame(gchar * filename, gint * time,
				  gint * frame)
{
    Decoder *decoder;
    switch (get_decoder_type(filename)) {
    case DECODER_TYPE_OGG:
	decoder = (Decoder *) & ogg_decoder;
	break;
    case DECODER_TYPE_MAD:
	decoder = (Decoder *) & mad_decoder;
	break;
    default:
	return -1;
    }
    if (!decoder)
	return -1;
    return decoder->get_total_time_frame(filename, time, frame);
}

gint decoder_get_cur_frame()
{
    if (get_decoder_playing() && get_current_decoder()) {
	return get_current_decoder()->get_cur_frame();
    } else {
	return -1;
    }
}

/*decoder.c*/

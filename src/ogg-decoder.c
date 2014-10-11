/* vim: set ai et ts=4 sw=4: */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include "mad.h"

#include "input.h"
#include "playback.h"
#include "decoder.h"
#include "ogg-decoder.h"
#include "liteamp.h"


typedef struct ogg_info {
    char buffer[BUF_SIZE];

    ogg_int64_t total_pcm;
    double total_time;

    OggVorbis_File *vf;
    vorbis_info *vi;

    pthread_t tid;
    pthread_mutex_t mutex;

    DecoderStatus status;

    void *input_info;
    gboolean init_done;
} ogg_info;

static size_t ovcb_read(void *ptr, size_t size, size_t nmemb,
			void *datasource);
static int ovcb_seek(void *datasource, int64_t offset, int whence);
static int ovcb_close(void *datasource);
static long ovcb_tell(void *datasource);

ogg_info info;
ov_callbacks ovcb = { ovcb_read, ovcb_seek, ovcb_close, ovcb_tell };


void vi_clear(vorbis_info * vi)
{
    g_print("vorbis_info : %d", vi);
}

static size_t ovcb_read(void *ptr, size_t size, size_t nmemb,
			void *datasource)
{
    return (size_t) input_get_data(datasource, ptr, size, nmemb);
}

static int ovcb_seek(void *datasource, int64_t offset, int whence)
{
    if (!input_is_local(datasource))
	return -1;
    return (int) input_seek(datasource, whence, offset);
}

static int ovcb_close(void *datasource)
{
    return (int) input_quit(datasource);
}

static long ovcb_tell(void *datasource)
{
    return (long) input_tell(datasource);
}


void *thread_play(void *filename)
{
    int eof = 0;
    int current_section = 0;
    int bitrate = 16;
    int rate = 0, channels = 0;
    long ret;

    info.vf = (struct OggVorbis_File *) malloc(sizeof(OggVorbis_File));

    pthread_mutex_lock(&(info.mutex));

    /* open file */
    info.input_info = input_init(filename);
    if (!info.input_info) {
	return NULL;
    }

    /* open vorbisfile */
    if ((ret =
	 ov_open_callbacks(info.input_info, info.vf, NULL, 0,
			   ovcb)) != 0) {
	pthread_mutex_unlock(&(info.mutex));
	g_print("ov_open_callacks Error - %d \n", ret);
	input_quit(info.input_info);
	pthread_exit(NULL);
    }

    /* get song infomation */
    info.vi = ov_info(info.vf, -1);	/* get vorbis infomation */
    if (input_is_local(info.input_info)) {
	info.total_pcm = ov_pcm_total(info.vf, -1);	/* get total pcmsample count */
	info.total_time = ov_time_total(info.vf, -1);	/* get total playtime */
    } else {
	info.total_pcm = -1;
	info.total_time = -1;
    }

    pthread_mutex_unlock(&(info.mutex));

    if (filename) {
	g_free(filename);
    }
    info.init_done = TRUE;

    /* play loop */
    while (!eof) {
	if (info.status & STOPPED) {
	    goto stop;
	}

	pthread_mutex_lock(&(info.mutex));
	if (playback_is_big_endian())
	    ret =
		ov_read(info.vf, info.buffer, sizeof(info.buffer), 1,
			bitrate / 8, 1, &current_section);
	else
	    ret =
		ov_read(info.vf, info.buffer, sizeof(info.buffer), 0,
			bitrate / 8, 1, &current_section);
	pthread_mutex_unlock(&(info.mutex));

	if (ret == 0) {
	    /* EOF */
	    eof = 1;
	} else if (ret < 0) {
	    /* error in the stream.  Not a problem, just reporting it in
	     *             *    case we (the app) cares.  In this case, we don't. */
	} else {
	    /* we don't bother dealing with sample rate changes, etc, but
	     *             *    you'll have to*/
	    //ret = volume_adjust(buffer,ret);      

	    /* audio out device (re)open */
	    if (!playback_get_playdevice()) {
		rate = info.vi->rate;
		channels = info.vi->channels;
		playback_open_playdevice(info.vi->rate, info.vi->channels,
					 bitrate);
	    } else if (info.vi->channels != channels
		       || info.vi->rate != rate) {
		playback_close_playdevice();
		rate = info.vi->rate;
		channels = info.vi->channels;
		playback_open_playdevice(info.vi->rate, info.vi->channels,
					 bitrate);
	    }

	    playback_play_it(info.buffer, ret, channels, rate, bitrate);
	}
    }

  stop:
    pthread_mutex_lock(&(info.mutex));
    ov_clear(info.vf);
    free(info.vf);
    pthread_mutex_unlock(&(info.mutex));

    playback_close_playdevice();

    if (info.status & PLAYING) {
	//pthread_detach(pthread_self());
	info.status = STOPPED;
	/* temporary auto next function */
	gdk_threads_enter();
	playlist_next(get_playlist(app));
	gdk_threads_leave();
    }
    pthread_exit(NULL);
}

/*---------------------------------------------------------------------------*/

void ogg_init(void)
{
    info.total_pcm = 0;
    info.total_time = 0;
    info.init_done = FALSE;

    info.vf = NULL;

    info.input_info = NULL;
    pthread_mutex_init(&info.mutex, NULL);
}

void ogg_play(char *filename)
{
    gchar *file = g_strdup(filename);
    info.status = PLAYING;
    playback_unpause();		/* ensure audio out  */
    ogg_init();
    pthread_create(&(info.tid), 0, thread_play, (void *) file);
}

void ogg_stop(void)
{
    playback_unpause();
    info.status = STOPPED;
    pthread_join(info.tid, 0);
}

void ogg_pause(pause)
{
    if (pause)
	playback_pause();
    else
	playback_unpause();
}

void ogg_seek(gint time)
{
    ogg_int64_t wf;
    if (input_is_local(info.input_info)) {
	if (time >= info.total_time) {
	    time = info.total_time - 1;
	} else if (time < 0) {
	    time = 0;
	}

	pthread_mutex_lock(&(info.mutex));
	ov_time_seek(info.vf, (double) time);
	pthread_mutex_unlock(&(info.mutex));
    }
}

void ogg_next(char *filename)
{
    int ret = 0;

    playback_unpause();		/* ensure audio out  */

    /* auto next */
    if (info.status & STOPPED) {
	pthread_join(info.tid, NULL);
	ogg_play(filename);
	return;
    }


    while (!info.init_done) {
	usleep(1000);
    }
    pthread_mutex_lock(&(info.mutex));

    /* re open file */
    if (info.vf) {
	ov_clear(info.vf);
	free(info.vf);
    }
    info.vf = (struct OggVorbis_File *) malloc(sizeof(OggVorbis_File));
    info.input_info = input_init(filename);
    if (!info.input_info) {
	pthread_mutex_unlock(&(info.mutex));
	return;
    }

    /* open vorbisfile */
    if ((ret =
	 ov_open_callbacks(info.input_info, info.vf, NULL, 0,
			   ovcb)) != 0) {
	pthread_mutex_unlock(&(info.mutex));
	g_print("ov_open_callacks Error\n");
	input_quit(info.input_info);
	return;
    }

    /* get song infomation */
    info.vi = ov_info(info.vf, -1);	/* get vorbis infomation */
    if (input_is_local(info.input_info)) {
	info.total_pcm = ov_pcm_total(info.vf, -1);	/* get total pcmsample count */
	info.total_time = ov_time_total(info.vf, -1);	/* get total playtime */
    } else {
	info.total_pcm = -1;
	info.total_time = -1;
    }

    pthread_mutex_unlock(&(info.mutex));
}


gboolean ogg_is_our_file(char *filename)
{
    FILE *test_fd;
    OggVorbis_File test_vf;
    input_info *input;

    input = input_init(filename);
    if (input == NULL) {
	return FALSE;
    }

    if (!ov_test_callbacks(input, &test_vf, NULL, 0, ovcb)) {
	ov_clear(&test_vf);
	return TRUE;
    }
    ov_clear(&test_vf);
    return FALSE;
}


gint ogg_get_cur_time()
{
    gint value;
    pthread_mutex_lock(&(info.mutex));
    value = (gint) ov_time_tell(info.vf);
    pthread_mutex_unlock(&(info.mutex));
    if (value == OV_EINVAL)
	return -1;
    return value;
}

gint ogg_get_cur_frame()
{
    gint value;
    pthread_mutex_lock(&(info.mutex));
    value = (gint) ov_pcm_tell(info.vf);
    pthread_mutex_unlock(&(info.mutex));
    if (value == OV_EINVAL)
	return -1;
    return value;
}

gint ogg_get_total_time_frame(char *filename, gint * time, gint * frame)
{
    FILE *test_fd;
    OggVorbis_File *test_vf;

    if (time == NULL || frame == NULL)
	return -1;

    test_vf = (OggVorbis_File *) malloc(sizeof(OggVorbis_File));

    test_fd = fopen(filename, "r");
    if (test_fd == 0) {
	return -1;
    }

    if (ov_open(test_fd, test_vf, NULL, 0) < 0) {
	ov_clear(test_vf);
	return -1;
    }

    *time = (gint) ov_time_total(test_vf, -1);
    *frame = (gint) ov_pcm_total(test_vf, -1);

    ov_clear(test_vf);

    return 0;
}


/*---------------------------------------------------------------------------*/

/*ogg-decoder.c*/

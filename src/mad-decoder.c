/* vim: set ai et ts=4 sw=4: */
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdlib.h>
#include <pthread.h>
#include <mad.h>

#include "input.h"
#include "decoder.h"
#include "playback.h"
#include "mad-decoder.h"
#include "liteamp.h"

typedef struct mad_info {
    /* The buffer of raw mpeg data for libmad to decode */
    //unsigned char* buf[BUF_SIZE];
    void *buf;

    /* length of the current stream, corrected for id3 tags */
    ssize_t length;

    /* total number of frames */
    unsigned long num_frames;

    /* number of frames to play */
    unsigned long max_frames;

    /* total duration of the file */
    mad_timer_t duration;

    /* current positon infomaion */
    unsigned long cur_frame;
    mad_timer_t cur_time;

    /* variable for threading */
    pthread_t tid;
    pthread_mutex_t mutex;

    /* paused positon */
    unsigned long paused_frame;

    /* status of mad-decoder */
    DecoderStatus status;

    /* input information struct */
    void *input_info;
    gboolean init_done;
} mad_info;

mad_info info;

/* XING parsing is from the MAD winamp input plugin */

struct xing {
    int flags;
    unsigned long frames;
    unsigned long bytes;
    unsigned char toc[100];
    long scale;
};

enum {
    XING_FRAMES = 0x0001,
    XING_BYTES = 0x0002,
    XING_TOC = 0x0004,
    XING_SCALE = 0x0008
};

# define XING_MAGIC     (('X' << 24) | ('i' << 16) | ('n' << 8) | 'g')

static
int parse_xing(struct xing *xing, struct mad_bitptr ptr,
	       unsigned int bitlen)
{
    if (bitlen < 64 || mad_bit_read(&ptr, 32) != XING_MAGIC)
	goto fail;

    xing->flags = mad_bit_read(&ptr, 32);
    bitlen -= 64;

    if (xing->flags & XING_FRAMES) {
	if (bitlen < 32)
	    goto fail;

	xing->frames = mad_bit_read(&ptr, 32);
	bitlen -= 32;
    }

    if (xing->flags & XING_BYTES) {
	if (bitlen < 32)
	    goto fail;

	xing->bytes = mad_bit_read(&ptr, 32);
	bitlen -= 32;
    }

    if (xing->flags & XING_TOC) {
	int i;

	if (bitlen < 800)
	    goto fail;

	for (i = 0; i < 100; ++i)
	    xing->toc[i] = mad_bit_read(&ptr, 8);

	bitlen -= 800;
    }

    if (xing->flags & XING_SCALE) {
	if (bitlen < 32)
	    goto fail;

	xing->scale = mad_bit_read(&ptr, 32);
	bitlen -= 32;
    }

    return 1;

  fail:
    xing->flags = 0;
    return 0;
}


/* Following two functions are adapted from mad_timer, from the 
   libmad distribution */
void scan(void const *ptr, ssize_t len, mad_info * buf)
{
    struct mad_stream stream;
    struct mad_header header;
    struct xing xing;

    unsigned long bitrate = 0;
    int has_xing = 0;
    int is_vbr = 0;

    mad_stream_init(&stream);
    mad_header_init(&header);

    mad_stream_buffer(&stream, ptr, len);

    buf->num_frames = 0;

    /* There are three ways of calculating the length of an mp3:
       1) Constant bitrate: One frame can provide the information
       needed: # of frames and duration. Just see how long it
       is and do the division.
       2) Variable bitrate: Xing tag. It provides the number of 
       frames. Each frame has the same number of samples, so
       just use that.
       3) All: Count up the frames and duration of each frames
       by decoding each one. We do this if we've no other
       choice, i.e. if it's a VBR file with no Xing tag.
     */

    while (1) {
	if (mad_header_decode(&header, &stream) == -1) {
	    if (MAD_RECOVERABLE(stream.error))
		continue;
	    else
		break;
	}

	/* Limit xing testing to the first frame header */
	if (!buf->num_frames++) {
	    if (parse_xing(&xing, stream.anc_ptr, stream.anc_bitlen)) {
		is_vbr = 1;

		if (xing.flags & XING_FRAMES) {
		    /* We use the Xing tag only for frames. If it doesn't have that
		       information, it's useless to us and we have to treat it as a
		       normal VBR file */
		    has_xing = 1;
		    buf->num_frames = xing.frames;
		    break;
		}
	    }
	}

	/* Test the first n frames to see if this is a VBR file */
	if (!is_vbr && !(buf->num_frames > 20)) {
	    if (bitrate && header.bitrate != bitrate) {
		is_vbr = 1;
	    }

	    else {
		bitrate = header.bitrate;
	    }
	}

	/* We have to assume it's not a VBR file if it hasn't already been
	   marked as one and we've checked n frames for different bitrates */
	else if (!is_vbr) {
	    break;
	}
	mad_timer_add(&buf->duration, header.duration);
    }

    if (!is_vbr) {
	double time = (len * 8.0) / (header.bitrate);	/* time in seconds */
	double timefrac = (double) time - ((long) (time));
	long nsamples = 32 * MAD_NSBSAMPLES(&header);	/* samples per frame */

	/* samplerate is a constant */
	buf->num_frames = (long) (time * header.samplerate / nsamples);

	mad_timer_set(&buf->duration, (long) time, (long) (timefrac * 100),
		      100);
    }

    else if (has_xing) {
	/* modify header.duration since we don't need it anymore */
	mad_timer_multiply(&header.duration, buf->num_frames);
	buf->duration = header.duration;
    }

    else {
	/* the durations have been added up, and the number of frames
	   counted. We do nothing here. */
    }

    mad_header_finish(&header);
    mad_stream_finish(&stream);
}

int calc_length(char *file, mad_info * buf)
{
    int f;
    struct stat filestat;
    void *fdm;
    char buffer[3];
    input_info *input;

    f = open(file, O_RDONLY);
    if (f < 0) {
	if (input = input_init(file)) {
	    if (!input_is_local(input)) {
		buf->length = 0;
		mad_timer_reset(&buf->duration);
		buf->num_frames = 0;
		return 0;
	    }
	}
	return -1;
    }

    if (fstat(f, &filestat) < 0) {
	perror("calc_length");
	close(f);
	return -1;
    }

    if (!S_ISREG(filestat.st_mode)) {
	fprintf(stderr, "%s: Not a regular file\n", file);
	close(f);
	return -1;
    }

    /* TAG checking is adapted from XMMS */
    buf->length = filestat.st_size;

    if (lseek(f, -128, SEEK_END) < 0) {
	/* File must be very short or empty. Forget it. */
	close(f);
	return -1;
    }

    if (read(f, buffer, 3) != 3) {
	close(f);
	return -1;
    }

    if (!strncmp(buffer, "TAG", 3)) {
	buf->length -= 128;	/* Correct for id3 tags */
    }

    fdm = mmap(0, buf->length, PROT_READ, MAP_SHARED, f, 0);
    if (fdm == MAP_FAILED) {
	perror("calc_length");
	close(f);
	return -1;
    }

    /* Scan the file for a XING header, or calculate the length,
       or just scan the whole file and add everything up. */
    scan(fdm, buf->length, buf);

    if (munmap(fdm, buf->length) == -1) {
	perror("calc_length");
	close(f);
	return -1;
    }

    if (close(f) < 0) {
	perror("calc_length");
	return -1;
    }

    return 0;
}

/* The following two routines and data structure are from the ever-brilliant
   Rob Leslie.
*/

struct audio_dither {
    mad_fixed_t error[3];
    mad_fixed_t random;
};

/*
* NAME:        prng()
* DESCRIPTION: 32-bit pseudo-random number generator
*/
static inline unsigned long prng(unsigned long state)
{
    return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

/*
* NAME:        audio_linear_dither()
* DESCRIPTION: generic linear sample quantize and dither routine
*/
inline
    signed long audio_linear_dither(unsigned int bits, mad_fixed_t sample,
				    struct audio_dither *dither)
{
    unsigned int scalebits;
    mad_fixed_t output, mask, random;

    enum {
	MIN = -MAD_F_ONE,
	MAX = MAD_F_ONE - 1
    };

    /* noise shape */
    sample += dither->error[0] - dither->error[1] + dither->error[2];

    dither->error[2] = dither->error[1];
    dither->error[1] = dither->error[0] / 2;

    /* bias */
    output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

    scalebits = MAD_F_FRACBITS + 1 - bits;
    mask = (1L << scalebits) - 1;

    /* dither */
    random = prng(dither->random);
    output += (random & mask) - (dither->random & mask);

    dither->random = random;

    /* clip */
    if (output > MAX) {
	output = MAX;

	if (sample > MAX)
	    sample = MAX;
    } else if (output < MIN) {
	output = MIN;

	if (sample < MIN)
	    sample = MIN;
    }

    /* quantize */
    output &= ~mask;

    /* error feedback */
    dither->error[0] = sample - output;

    /* scale */
    return output >> scalebits;
}


/*----------------------------------------------------------------------------*/

enum mad_flow input(void *data, struct mad_stream *stream)
{
    mad_info *info = (mad_info *) data;

    gint remain = stream->bufend - stream->next_frame;
    gint len;

    if (remain) {
	g_memmove(info->buf, stream->this_frame, remain);
    }

    pthread_mutex_lock(&(info->mutex));
    len =
	input_get_data(info->input_info, info->buf + remain, 1,
		       BUF_SIZE - remain);

    if (len == 0) {
	pthread_mutex_unlock(&(info->mutex));
	return MAD_FLOW_STOP;
    }

    mad_stream_buffer(stream, info->buf, len + remain);
    pthread_mutex_unlock(&(info->mutex));

    return MAD_FLOW_CONTINUE;
}

enum mad_flow header(void *data, struct mad_header const *header)
{
    mad_info *info = (mad_info *) data;

    pthread_mutex_lock(&(info->mutex));

    info->cur_frame++;
    mad_timer_add(&info->cur_time, header->duration);

    if (info->status & STOPPED) {
	pthread_mutex_unlock(&(info->mutex));
	return MAD_FLOW_STOP;
    }

    if (info->status & SEEKING && info->paused_frame) {
	if (!--(info->paused_frame))
	    info->status = PLAYING;
	pthread_mutex_unlock(&(info->mutex));
	return MAD_FLOW_IGNORE;
    } else {
	info->status = PLAYING;
    }

    pthread_mutex_unlock(&(info->mutex));

    return MAD_FLOW_CONTINUE;
}

enum mad_flow output(void *data,
		     struct mad_header const *header, struct mad_pcm *pcm)
{
    register int nsamples = pcm->length;
    mad_fixed_t const *left_ch = pcm->samples[0];
    mad_fixed_t const *right_ch = pcm->samples[1];
    static unsigned char stream[1152 * 4];	/* 1152 because that's what mad has as a max; *4 because
						 * there are 4 distinct bytes per sample (in 2 channel case) */
    static unsigned int rate = 0;
    static int channels = 0;
    static struct audio_dither dither;
    register char *ptr = stream;
    register signed int sample;
    int count = 0;

    mad_info *info = (mad_info *) data;

    /* We need to know information about the file before we can open the playdevice
     * In some cases. So, we do it here. */
    if (!playback_get_playdevice()) {
	channels = MAD_NCHANNELS(header);
	rate = header->samplerate;
	playback_open_playdevice(rate, channels, 16);
    } else if ((channels != MAD_NCHANNELS(header)
		|| rate != header->samplerate) && playback_is_live()) {
	playback_close_playdevice();
	channels = MAD_NCHANNELS(header);
	rate = header->samplerate;
	playback_open_playdevice(rate, channels, 16);
    }


    while (nsamples--) {
	sample = (signed int) audio_linear_dither(16, *left_ch++, &dither);
	if (playback_is_big_endian()) {
	    *(ptr + count++) = (unsigned char) (sample >> 8);
	    *(ptr + count++) = (unsigned char) (sample >> 0);
	} else {
	    *(ptr + count++) = (unsigned char) (sample >> 0);
	    *(ptr + count++) = (unsigned char) (sample >> 8);
	}

	if (pcm->channels == 2) {
	    sample =
		(signed int) audio_linear_dither(16, *right_ch++, &dither);
	    if (playback_is_big_endian()) {
		*(ptr + count++) = (unsigned char) (sample >> 8);
		*(ptr + count++) = (unsigned char) (sample >> 0);
	    } else {
		*(ptr + count++) = (unsigned char) (sample >> 0);
		*(ptr + count++) = (unsigned char) (sample >> 8);
	    }
	}
    }

    pthread_mutex_lock(&(info->mutex));
    if (info->status & SEEKING) {
	pthread_mutex_unlock(&(info->mutex));
	return MAD_FLOW_CONTINUE;
    }
    pthread_mutex_unlock(&(info->mutex));

    playback_play_it(stream, count, channels, rate, 16);

    return MAD_FLOW_CONTINUE;
}

void *mp3_play(void *filename)
{
    struct mad_decoder decoder;

    info.buf = g_malloc0(BUF_SIZE);

    /* initialize */
    info.input_info = input_init(filename);
    if (input_is_local(info.input_info)) {
	calc_length(filename, &info);
    } else {
	info.num_frames = 0;
	mad_timer_reset(&info.duration);
    }
    mad_decoder_init(&decoder, &info, input, header, NULL, output, NULL,
		     NULL);
    if (filename) {
	g_free(filename);
    }
    info.init_done = TRUE;

    /* play */
    mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

    /* finish & cleanup buffer */
    input_quit(info.input_info);
    mad_decoder_finish(&decoder);
    playback_close_playdevice();
    g_free(info.buf);

    if (info.status & PLAYING) {
	info.status = STOPPED;
	/* goto next song */
	gdk_threads_enter();
	playlist_next(get_playlist(app));
	gdk_threads_leave();
    }
    pthread_exit(NULL);
}


/*---------------------------------------------------------------------------*/

/* Control Functions */

void mad_init()
{
    info.cur_frame = 0;
    info.paused_frame = 0;
    info.init_done = FALSE;
    mad_timer_reset(&info.cur_time);
    pthread_mutex_init(&info.mutex, NULL);
}

void mad_play(char *filename)
{
    gchar *file;
    file = g_strdup(filename);
    playback_unpause();		/* ensure audio out  */
    info.status = PLAYING;
    mad_init();
    pthread_create(&info.tid, NULL, mp3_play, (void *) file);
}

void mad_stop()
{
    playback_unpause();		/* ensure audio out  */
    info.status = STOPPED;
    pthread_join(info.tid, NULL);
}

void mad_pause(pause)
{
    if (pause)
	playback_pause();
    else
	playback_unpause();
}

void mad_seek(gint time)
{
    unsigned long frame;
    if (input_is_local(info.input_info)) {
	if (info.status & SEEKING)
	    return;

	if (time < 0) {
	    time = 0;
	} else if (time >= info.duration.seconds) {
	    time = info.duration.seconds - 1;
	}

	info.status = SEEKING;

	pthread_mutex_lock(&(info.mutex));
	frame = info.num_frames * time / info.duration.seconds;
	if ((long) frame > info.cur_frame) {
	    info.paused_frame = (long) frame - info.cur_frame;
	} else if ((long) frame < info.cur_frame) {
	    info.paused_frame = (long) frame;
	    mad_timer_reset(&info.cur_time);
	    info.cur_frame = 0;
	    input_seek(info.input_info, 0, 0);
	}
	pthread_mutex_unlock(&(info.mutex));
    }
}


void mad_next(char *filename)
{
    playback_unpause();		/* ensure audio out  */

    if (info.status & STOPPED) {
	pthread_join(info.tid, NULL);
	mad_play(filename);
	return;
    }

    while (!info.init_done) {
	usleep(1000);		/* if init_done flag is FALSE , wait */
    }

    pthread_mutex_lock(&(info.mutex));

    if (info.input_info) {
	input_quit(info.input_info);
    }
    info.input_info = input_init(filename);
    if (input_is_local(info.input_info)) {
	calc_length(filename, &info);
    } else {
	info.num_frames = 0;
	mad_timer_reset(&info.duration);
    }
    info.cur_frame = 0;
    info.paused_frame = 0;
    mad_timer_reset(&info.cur_time);

    pthread_mutex_unlock(&(info.mutex));
}


gint mad_get_cur_time()
{
    if (info.status & PLAYING) {
	return (gint) mad_timer_count(info.cur_time, MAD_UNITS_SECONDS);
    }
    return -1;
}

gint mad_get_cur_frame()
{
    if (info.status & PLAYING) {
	return (gint) info.cur_frame;
    }
    return -1;
}


gint mad_get_total_time_frame(char *filename, gint * time, gint * frame)
{
    mad_info temp;
    mad_timer_reset(&temp.duration);
    temp.num_frames = 0;

    if (time == NULL || frame == NULL)
	return -1;

    if (calc_length(filename, &temp) == 0) {
	*time = (gint) mad_timer_count(temp.duration, MAD_UNITS_SECONDS);
	*frame = (gint) temp.num_frames;
	return 0;
    } else {
	return -1;
    }
}

gboolean mad_is_our_file(char *filename)
{
    struct mad_stream stream;
    struct mad_header header;
    unsigned char buf[BUF_SIZE];
    input_info *input;
    int len, remain, count = 0;
    gboolean retval = FALSE;

    mad_stream_init(&stream);
    mad_header_init(&header);
    input = input_init(filename);

    while (1) {
	remain = stream.bufend - stream.next_frame;
	len = input_get_data(input, buf + remain, 1, BUF_SIZE - remain);
	if (len == 0) {
	    goto end;
	}
	mad_stream_buffer(&stream, buf, len + remain);

	while (1) {
	    if (mad_header_decode(&header, &stream) != -1) {
		if (count == 1) {
		    retval = TRUE;
		    goto end;
		}
		count++;
	    } else {
		if (!MAD_RECOVERABLE(stream.error)) {
		    break;
		}
		if (stream.error == MAD_ERROR_LOSTSYNC) {
		    continue;
		}
		goto end;
	    }
	    mad_stream_sync(&stream);
	}
    }

  end:
    mad_header_finish(&header);
    mad_stream_finish(&stream);
    input_quit(input);
    return retval;
}

/*---------------------------------------------------------------------------*/

/*mad-decoder.c*/

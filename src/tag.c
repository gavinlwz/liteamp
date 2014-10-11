/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <id3tag.h>
#include "liteamp.h"
#include "la-track.h"
#include "util.h"
#include "mime.h"
// copied from vorbis-tools
#include "vcedit.h"

// vorbis comment field name
// see http://www.xiph.org/ogg/vorbis/doc/v-comment.html
#define VC_FIELD_TITLE		("TITLE")
#define VC_FIELD_ARTIST		("ARTIST")
#define VC_FIELD_ALBUM		("ALBUM")
#define VC_FIELD_DATE		("DATE")
#define VC_FIELD_TRACKNUMBER	("TRACKNUMBER")
#define VC_FIELD_GENRE		("GENRE")
#define VC_FIELD_DESCRIPTION	("DESCRIPTION")

// libid3tag version 0.15 or higher is required to write id3 tag
#define DISABLE_ID3_WRITE	((ID3_VERSION_MAJOR == 0) && (ID3_VERSION_MINOR <= 14))

/* private------------------------------------------------*/

static GSList *fallback_codesets = NULL;
static gchar *used_codeset = NULL;

//
// id3 supports
//

static gchar *id3_get_field(struct id3_tag *tag, char const *name)
{
    struct id3_frame *frame;
    id3_ucs4_t const *ucs4;
    id3_latin1_t *latin1;
    gchar *value;

    frame = id3_tag_findframe(tag, name, 0);
    if (!frame)
	return NULL;

    if (!strcmp(name, ID3_FRAME_COMMENT))
	ucs4 = id3_field_getfullstring(&frame->fields[3]);
    else
	ucs4 = id3_field_getstrings(&frame->fields[1], 0);
    if (!ucs4)
	return NULL;

    // FIXME: why id3_ucs4_utf8duplicate() doesn't work correctly?
    if (!strcmp(name, ID3_FRAME_GENRE))
	//TODO: provide translated genre name
	latin1 = id3_ucs4_latin1duplicate(id3_genre_name(ucs4));
    else
	latin1 = id3_ucs4_latin1duplicate(ucs4);
    if (!latin1)
	return NULL;

    value =
	la_str_to_utf8_with_fallback(latin1, fallback_codesets,
				     &used_codeset);
    free(latin1);
    return value;;
}

static void id3_set_field(struct id3_tag *tag, char const *name,
			  const gchar * value)
{
#if DISABLE_ID3_WRITE
    g_message("libid3tag version 0.15 or higher is required to write id3 tag!\n");
    return;
#else
    struct id3_frame *frame;
    union id3_field *field;
    id3_ucs4_t *ucs4;
    gint genre_num;
    gchar *genre_value;

    // remove all frames for empty value
    if (LA_STR_IS_EMPTY(value)) {
	while ((frame == id3_tag_findframe(tag, name, 0)))
	    id3_tag_detachframe(tag, frame);
	return;
    }

    frame = id3_tag_findframe(tag, name, 0);

    if (!frame) {
	frame = id3_frame_new(name);
	id3_tag_attachframe(tag, frame);
    }

    // FIXME: why id3_utf8_ucs4duplicate() doesn't work correctly?
    ucs4 = id3_latin1_ucs4duplicate(value);

    if (!strcmp(name, ID3_FRAME_GENRE)) {
	genre_num = id3_genre_number(ucs4);
	free(ucs4);
	genre_value = la_ltostr_new(genre_num);
	ucs4 = id3_latin1_ucs4duplicate(genre_value);
	g_free(genre_value);
    }

    if (!strcmp(name, ID3_FRAME_COMMENT)) {
	//field = &frame->fields[3];
	field = id3_frame_field(frame, 3);
	id3_field_setfullstring(field, ucs4);
    } else {
	//field = &frame->fields[1];
	field = id3_frame_field(frame, 1);
	id3_field_setstrings(field, 1, &ucs4);
    }

    free(ucs4);
#endif
}

static gboolean id3_read(LaTrack * track, const gchar * filename)
{
    struct id3_file *file;
    struct id3_tag *tag;
    struct id3_frame *frame;
    id3_ucs4_t *ucs4;
    gchar *utf8;

    gint seconds;
    gint frames;

    la_track_set_channels(track, 2);
    la_track_set_samplerate(track, 0);
    la_track_set_bitrate(track, 0);

    if ((decoder_get_total_time_frame(filename, &seconds, &frames)) == -1) {
	la_track_set_seconds(track, 0);
	la_track_set_frames(track, 0);
	g_message("mp3 file corruped:%s\n", filename);
	return FALSE;
    }
    la_track_set_seconds(track, seconds);
    la_track_set_frames(track, frames);

    file = id3_file_open(filename, ID3_FILE_MODE_READONLY);
    if (!file)
	return FALSE;

    tag = id3_file_tag(file);
    if (!tag)
	return FALSE;

    //tag->options &= (~ID3_TAG_OPTION_ID3V1);
    tag->options |= ID3_TAG_OPTION_ID3V1;

    utf8 = id3_get_field(tag, ID3_FRAME_TITLE);
    if (utf8) {
	la_track_set_title(track, utf8);
	free(utf8);
    }
    utf8 = id3_get_field(tag, ID3_FRAME_ARTIST);
    if (utf8) {
	la_track_set_artist(track, utf8);
	free(utf8);
    }
    utf8 = id3_get_field(tag, ID3_FRAME_ALBUM);
    if (utf8) {
	la_track_set_album(track, utf8);
	free(utf8);
    }
    utf8 = id3_get_field(tag, ID3_FRAME_TRACK);
    if (utf8) {
	la_track_set_track(track, utf8);
	free(utf8);
    }
    utf8 = id3_get_field(tag, ID3_FRAME_GENRE);
    if (utf8) {
	la_track_set_genre(track, utf8);
	free(utf8);
    }
    utf8 = id3_get_field(tag, ID3_FRAME_COMMENT);
    if (utf8) {
	la_track_set_comment(track, utf8);
	free(utf8);
    }

    //id3_tag_delete(tag);
    id3_file_close(file);

    return TRUE;
}

static gboolean id3_write(LaTrack * track, const gchar * filename)
{
#if DISABLE_ID3_WRITE
    g_message("libid3tag version 0.15 or higher is required to write id3 tag!\n");
    return FALSE;
#else
    struct id3_file *file;
    struct id3_tag *tag;
    file = id3_file_open(filename, ID3_FILE_MODE_READWRITE);
    if (!file)
	return FALSE;

    tag = id3_file_tag(file);
    if (!tag)
	return FALSE; //tag = id3_tag_new();?

    id3_tag_clearframes(tag);
    tag->options |= ID3_TAG_OPTION_ID3V1;

    // tricky...
    // this function used to remove tag, too!
    if (track) {
	id3_set_field(tag, ID3_FRAME_TITLE, la_track_get_title(track));
	id3_set_field(tag, ID3_FRAME_ARTIST, la_track_get_artist(track));
	id3_set_field(tag, ID3_FRAME_ALBUM, la_track_get_album(track));
	id3_set_field(tag, ID3_FRAME_TRACK, la_track_get_track(track));
	id3_set_field(tag, ID3_FRAME_GENRE, la_track_get_genre(track));
	id3_set_field(tag, ID3_FRAME_COMMENT, la_track_get_comment(track));
    }

    if (id3_file_update(file) < 0)
	return FALSE;

    //id3_tag_delete(tag);
    id3_file_close(file);

    return TRUE;
#endif
}

static gboolean id3_remove(LaTrack * track, const gchar * filename)
{
    return id3_write(NULL, filename);
}

//
// ogg-vorbis supports
//

static gchar *vorbis_get_field(vorbis_comment * vc, const gchar * name)
{
    gint count;
    gint i;
    GString *str;
    gchar *value;

    count = vorbis_comment_query_count(vc, (char *) name);
    if (count <= 0)
	value = NULL;
    else if (count ==  1) {
	value = vorbis_comment_query(vc, (char *)name, 0);
	value =
	    la_str_to_utf8_with_fallback(value,
					 fallback_codesets,
					 &used_codeset);
    } else {
	str = g_string_sized_new(256);
	for (i = 0; i < count; i++) {
	    value = vorbis_comment_query(vc, (char *)name, i);
	    g_string_append(str, value);
	}

	value =
	    la_str_to_utf8_with_fallback(str->str, fallback_codesets,
					 &used_codeset);

        g_string_free(str, TRUE);
    }

    return value;
}

static void vorbis_set_field(vorbis_comment * vc, const gchar * name,
			     const gchar * value)
{
    if (!value)
	return;
    //TODO: verify value
    vorbis_comment_add_tag(vc, (char *) name, (char *) value);
}

static gboolean vorbis_read(LaTrack * track, const gchar * filename)
{
    FILE *fp;
    OggVorbis_File vf;
    vorbis_info *vi;
    vorbis_comment *vc;

    g_return_val_if_fail(track && filename, FALSE);

    fp = fopen(filename, "rb");
    if (!fp)
	return FALSE;

    if (ov_open(fp, &vf, NULL, 0) < 0)
	return FALSE;

    vi = ov_info(&vf, -1);
    if (vi) {
	la_track_set_bitrate(track, vi->bitrate_nominal);
	la_track_set_samplerate(track, vi->rate);
	la_track_set_frames(track, ov_pcm_total(&vf, -1));
	la_track_set_channels(track, vi->channels);
	la_track_set_seconds(track, ov_time_total(&vf, -1));
    }

    vc = ov_comment(&vf, -1);
    if (vc) {
	la_track_set_title(track, vorbis_get_field(vc, VC_FIELD_TITLE));
	la_track_set_artist(track, vorbis_get_field(vc, VC_FIELD_ARTIST));
	la_track_set_album(track, vorbis_get_field(vc, VC_FIELD_ALBUM));
	la_track_set_year(track, vorbis_get_field(vc, VC_FIELD_DATE));
	la_track_set_track(track,
			   vorbis_get_field(vc, VC_FIELD_TRACKNUMBER));
	la_track_set_genre(track, vorbis_get_field(vc, VC_FIELD_GENRE));
	la_track_set_comment(track,
			     vorbis_get_field(vc, VC_FIELD_DESCRIPTION));
    }

    ov_clear(&vf);

    return TRUE;
}

static gboolean vorbis_write(LaTrack * track, const gchar * filename)
{
    vcedit_state *state;
    vorbis_comment *vc;
    FILE *in, *out;
    gchar *tmp_name;

    g_return_val_if_fail(filename, FALSE);

    state = vcedit_new_state();

    in = fopen(filename, "rb");
    if (in < 0 || vcedit_open(state, in) < 0) {
	g_message("Failed to open: %s (%s)", filename,
		  vcedit_error(state));
	fclose(in);
	return FALSE;
    }

    vc = vcedit_comments(state);
    vorbis_comment_clear(vc);
    vorbis_comment_init(vc);

    // tricky...
    // this function used to remove tag, too!
    if (track) {
	vorbis_set_field(vc, VC_FIELD_TITLE, la_track_get_title(track));
	vorbis_set_field(vc, VC_FIELD_ARTIST, la_track_get_artist(track));
	vorbis_set_field(vc, VC_FIELD_ALBUM, la_track_get_album(track));
	vorbis_set_field(vc, VC_FIELD_DATE, la_track_get_year(track));
	vorbis_set_field(vc, VC_FIELD_TRACKNUMBER,
			 la_track_get_track(track));
	vorbis_set_field(vc, VC_FIELD_GENRE, la_track_get_genre(track));
	vorbis_set_field(vc, VC_FIELD_DESCRIPTION,
			 la_track_get_comment(track));
    }

    tmp_name = g_strconcat(filename, ".tmp", NULL);

    out = fopen(tmp_name, "wb");
    if (out < 0 || vcedit_write(state, out) < 0) {
	g_message("Failed to write comments to: %s (%s)", filename,
		  vcedit_error(state));
	vcedit_clear(state);
	fclose(in);
	return FALSE;
    }

    vcedit_clear(state);
    fclose(in);
    fclose(out);

    // swap filename with tmp_name :(
    // how can i do this safe and elegance?
    if (remove(filename) == 0) {
	if (rename(tmp_name, filename) == 0) {
	    g_free(tmp_name);
	    return TRUE;
	}
    }

    g_free(tmp_name);
    return FALSE;
}

static gboolean vorbis_remove(LaTrack * track, const gchar * filename)
{
    return vorbis_write(NULL, filename);
}

//
// common stuffs
//

static const gchar *get_mime_type(LaTrack * track)
{
    GnomeVFSFileInfo *info;
    const gchar *mime_type;

    info = la_track_get_vfs_info(track);

    mime_type = gnome_vfs_file_info_get_mime_type(info);

    gnome_vfs_file_info_unref(info);

    return mime_type;
}

static gchar *get_local_path(LaTrack * track)
{
    GnomeVFSFileInfo *info;
    gchar *local_path;

    info = la_track_get_vfs_info(track);

    local_path = NULL;
    if (GNOME_VFS_FILE_INFO_LOCAL(info)) {
	local_path = la_uri_to_local(la_track_get_uri(track));
    }

    gnome_vfs_file_info_unref(info);

    return local_path;
}

/* public-------------------------------------------------*/

void tag_init(GSList * _fallback_codesets)
{
    fallback_codesets = _fallback_codesets;
}

gboolean tag_read(LaTrack * track)
{
    GnomeVFSURI *uri;
    GnomeVFSFileInfo *info;
    const gchar *mime_type;
    gchar *local_path;
    gchar *pseudo_title;

    uri = la_track_get_vfs_uri(track);
    info = la_track_get_vfs_info(track);

    // read physical file info
    la_track_set_bytes(track, info->size);
    la_track_set_timestamp(track, info->mtime);

    local_path = get_local_path(track);

    if (local_path) {

	mime_type = get_mime_type(track);

	if (is_mime_type_ogg(mime_type)) {
	    vorbis_read(track, local_path);
	} else if (is_mime_type_mp3(mime_type)) {
	    id3_read(track, local_path);
	} else {
	    //TODO: some fallback parsing needed. what's that?
	    g_message("unknown file type %s for %s\n", mime_type,
		      local_path);
	}

	g_free(local_path);
    } else {
	//TODO: what can i do for remote file?
    }

    // provides fallback title from uri
    if (LA_STR_IS_EMPTY(la_track_get_title(track))) {
	pseudo_title = gnome_vfs_uri_extract_short_name(uri);
	la_track_set_title(track, pseudo_title);
	g_free(pseudo_title);
    }

    gnome_vfs_uri_unref(uri);
    gnome_vfs_file_info_unref(info);

    return TRUE;
}

gboolean tag_write(LaTrack * track)
{
    const gchar *mime_type;
    gchar *local_path;

    local_path = get_local_path(track);

    if (local_path) {

	mime_type = get_mime_type(track);

	if (is_mime_type_ogg(mime_type)) {
	    vorbis_write(track, local_path);
	} else if (is_mime_type_mp3(mime_type)) {
	    id3_write(track, local_path);
	} else {
	    //TODO: some fallback parsing needed. what's that?
	    g_message("unknown file type %s for %s\n", mime_type,
		      local_path);
	}

	g_free(local_path);
    } else {
	//TODO: what can i do for remote file?
    }

    return TRUE;
}

gboolean tag_remove(LaTrack * track)
{
    const gchar *mime_type;
    gchar *local_path;

    local_path = get_local_path(track);

    if (local_path) {

	mime_type = get_mime_type(track);

	if (is_mime_type_ogg(mime_type)) {
	    vorbis_remove(track, local_path);
	} else if (is_mime_type_mp3(mime_type)) {
	    id3_remove(track, local_path);
	} else {
	    //TODO: some fallback parsing needed. what's that?
	    g_message("unknown file type %s for %s\n", mime_type,
		      local_path);
	}

	g_free(local_path);
    } else {
	//TODO: what can i do for remote file?
    }

    return TRUE;
}

/* la-tag.c */

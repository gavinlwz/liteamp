/* vim: set ai ts=8 sw=4 sts=4 noet: */
#include <gnome.h>

#include "input.h"

/*
* NAME:        parse_url()
* DESCRIPTION: parse a url infomation
* RETURN:      NULL
*/
void parse_uri(input_info * vfs_data)
{
    //vfs_data->local = gnome_vfs_uri_is_local(vfs_data->uri);
    if (!g_strncasecmp("http://", vfs_data->url, 7 ||
		       !g_strncasecmp("ftp://", vfs_data->url, 6))) {
	vfs_data->local = FALSE;
	return;
    } else {
	vfs_data->local = TRUE;
    }
}


/*
* NAME:        input_init()
* DESCRIPTION: create a gnome-vfs descriptor, initialize variable
* RETURN:      Returns the input_info struct pointer. Otherwise, return NULL
*/
input_info *input_init(gchar * url)
{
    GnomeVFSResult retval;

    // allocate dynamic memory
    input_info *vfs_data = g_malloc(sizeof(input_info));

    // init
    //vfs_data->uri = NULL;
    vfs_data->url = NULL;
    vfs_data->vfs_handle = NULL;

    //vfs_data->uri = gnome_vfs_uri_new(url);
    vfs_data->url = gnome_vfs_get_uri_from_local_path(url);
    parse_uri(vfs_data);

    //retval = gnome_vfs_open_uri(&(vfs_data->vfs_handle),vfs_data->uri,GNOME_VFS_OPEN_READ);
    retval =
	gnome_vfs_open(&(vfs_data->vfs_handle), vfs_data->url,
		       GNOME_VFS_OPEN_READ);

    if (retval == GNOME_VFS_OK) {
	return vfs_data;
    } else {
	g_message("INPUT_VFS : open fail : %s",
		  gnome_vfs_result_to_string(retval));
	return NULL;
    }
}

/*
* NAME:        input_quit()
* DESCRIPTION: free dynamic memory, close a gnome-vfs descriptor
* RETURN:      retruns the gnome_vfs_close return code. Otherwise, return -1;
*/
gint input_quit(input_info * vfs_data)
{
    GnomeVFSResult retval;

    if (vfs_data) {
	//      if(vfs_data->uri){ 
	if (vfs_data->url) {
	    //g_free(vfs_data->uri);
	    g_free(vfs_data->url);
	}

	retval = gnome_vfs_close(vfs_data->vfs_handle);
	if (retval != GNOME_VFS_OK) {
	    g_message("INPUT_VFS : close fail : %s",
		      gnome_vfs_result_to_string(retval));
	}

	g_free(vfs_data);
	return retval;
    }
    return -1;
}

/*
* NAME:        input_get_data()
* DESCRIPTION: read data from local file or remote host 
* RETURN:      On success, the number of bytes read is returned. On error or EOF, return 0
*/
gint input_get_data(input_info * vfs_data, gchar * buffer, gint size,
		    gint count)
{
    GnomeVFSFileSize bytes_read;
    GnomeVFSResult retval;

    if (vfs_data) {
	retval =
	    gnome_vfs_read(vfs_data->vfs_handle, buffer, size * count,
			   &bytes_read);
	if (retval == GNOME_VFS_OK) {
	    return (gint) bytes_read / size;
	} else if (retval == GNOME_VFS_ERROR_EOF) {
	    return 0;
	} else {
	    g_message("INPUT_VFS : read fail : %s",
		      gnome_vfs_result_to_string(retval));
	    return 0;
	}
    }
    return 0;
}

/*
* NAME:        input_seek()
* DESCRIPTION: reposition a gnome-vfs stream
* RETURN:      On success, return 0. On error ..... error number
*/
gint input_seek(input_info * vfs_data, GnomeVFSSeekPosition whence,
		GnomeVFSFileOffset offset)
{
    GnomeVFSResult retval;

    if (vfs_data) {
	if (vfs_data->local) {
	    retval = gnome_vfs_seek(vfs_data->vfs_handle, whence, offset);
	    return retval;
	}
	g_message("INPUT_VFS : seek fail : input is stream");
	return -1;
    }
    return -1;
}

/*
* NAME:        input_tell()
* DESCRIPTION: Obtains the current value of the file position indicator
* RETURN:      returns the current offset.Otherwise, -1
*/
gulong input_tell(input_info * vfs_data)
{
    GnomeVFSResult retval;
    GnomeVFSFileSize offset_return;

    if (vfs_data) {
	retval = gnome_vfs_tell(vfs_data->vfs_handle, &offset_return);
	if (retval == GNOME_VFS_OK) {
	    return offset_return;
	} else {
	    g_message("INPUT_VFS : tell fail : %s",
		      gnome_vfs_result_to_string(retval));
	    return -1;
	}
    }
    return -1;
}

/*
* NAME:        input_is_local()
* DESCRIPTION: whether local or not
* RETURN:      returns the current remote status
*/
gboolean input_is_local(input_info * vfs_data)
{
    if (vfs_data) {
	return vfs_data->local;
    }
    g_message("INPUT_VFS : is_local fail : input_info is NULL");
    return FALSE;
}

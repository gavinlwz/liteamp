/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "liteamp.h"

int main(int argc, char *argv[])
{
    static gboolean start = FALSE;
    static gchar *playlist = NULL;
    static gchar *track = NULL;
    static const struct poptOption options[] = {
	{"start", 's', POPT_ARG_NONE, &start, 0, N_("start to play track"),
	 NULL},
	{"playlist", 'p', POPT_ARG_STRING, &playlist, 0,
	 N_("uri of playlist to load"), N_("PLAYLIST")},
	{"track", 't', POPT_ARG_STRING, &track, 0,
	 N_("uri of track to load"), N_("TRACK")},
	{NULL, '\0', 0, NULL, 0, NULL, NULL}
    };
    static GnomeProgram *program;

#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif

    g_thread_init(NULL);
    gdk_threads_init();
    gnome_vfs_init();

    program = gnome_program_init(PACKAGE, VERSION, LIBGNOMEUI_MODULE,
				 argc, argv,
				 GNOME_PARAM_POPT_TABLE, options,
				 GNOME_PARAM_APP_DATADIR, DATADIR,
				 GNOME_PARAM_NONE);

    // ok! here we go!
    liteamp_new(program, start, playlist, track);

    gnome_vfs_shutdown();

    return 0;
}

/* main.c */

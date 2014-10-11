SUBDIRS = effect

INCLUDES = \
	-I$(top_srcdir)/src/effect \
	$(LITEAMP_CFLAGS) \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DPKGDATADIR=\""$(datadir)/$(PACKAGE)"\" \
	-DG_DISABLE_DEPRECATED \
	-DGDK_DISABLE_DEPRECATED \
	-DGTK_DISABLE_DEPRECATED \
	-DGNOME_DISABLE_DEPRECATED

LIBS = \
	$(VORBIS_LIBS) \
	$(AO_LIBS) \
	$(MAD_LIBS) \
	$(ID3TAG_LIBS) \
	$(LITEAMP_LIBS)

bin_PROGRAMS = liteamp

liteamp_SOURCES = \
	main.c \
	liteamp.c liteamp.h \
	stock.c stock.h \
	action.c action.h \
	callbacks.c callbacks.h \
	visualizer.c visualizer.h \
	player.c player.h \
	sidebar.c sidebar.h \
	playlist.c playlist.h \
	dnd.c dnd.h \
	prefs.c prefs.h \
	prefs-dialog.c prefs-dialog.h \
	input.c input.h \
	decoder.c decoder.h \
	mad-decoder.c mad-decoder.h \
	ogg-decoder.c ogg-decoder.h \
	playback.c playback.h \
	mixer.c mixer.h \
	playlist-dialog.c playlist-dialog.h \
	location-dialog.c location-dialog.h \
	track-dialog.c track-dialog.h \
	about-dialog.c about-dialog.h \
	tray.c tray.h \
	osd.c osd.h \
	tag.c tag.h \
	mime.c mime.h \
	vcedit.c vcedit.h \
	util.c util.h \
	la-track.c la-track.h \
	la-playlist.c la-playlist.h \
	la-track-column.c la-track-column.h \
	la-tracks-iter.c la-tracks-iter.h \
	la-tracks-store.c la-tracks-store.h \
	la-iter.c la-iter.h \
	la-codeset.c la-codeset.h \
	eggtrayicon.c eggtrayicon.h

liteamp_LDADD = \
	$(top_builddir)/src/effect/libeffect.a \
	$(LITEAMP_LIBS)


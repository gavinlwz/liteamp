NULL =

SUBDIRS = pixmaps glade ui

DESKTOP_IN_FILES= liteamp.desktop.in
DESKTOP_FILES= $(DESKTOP_IN_FILES:.desktop.in=.desktop)

desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

appicon_DATA = liteamp.png
appicondir = $(datadir)/pixmaps

applicationsdir = $(datadir)/application-registry
applications_DATA = liteamp.applications

EXTRA_DIST = \
	$(desktop_DATA) \
	$(appicon_DATA) \
	$(applications_DATA) \
	$(NULL)

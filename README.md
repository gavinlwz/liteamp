liteamp - Yet Another Light-Weight OGG & MP3 Player for Gnome
=============================================================

Requirements
------------

- gtk 2.0 / libgnome 2.0 / libgnomui 2.0 / gnome-vfs 1.0
	http://www.gtk.org
	http://www.gnome.org

- libmad 0.14
	http://mad.sf.net/

- libogg 1.0 / libvoribis 1.0 / libao 0.8
	http://www.xiph.org/ogg/index.html

In short, you need Gnome 2 Desktop, libmad and ogg-vorbis libraries.
If you want build it from source, you may need developer packages too.


Build & Install
---------------

To build:
```
$ ./configure <some configuration options>
$ make
```

To install:
when you have write permission for installation prefix
```
$ make install
```
otherwise,
```
$ sudo make install
```

To uninstall:
when you have write permission for installation prefix
```
$ make uninstall
```
otherwise,
```
$ sudo make uninstall
```

To run:
```
$ liteamp
```
(installation prefix must be in PATH)

That's all.

Installed Files
---------------

* prefix/share/applications/liteamp.desktop - desktop entry file
* prefix/share/pixmaps/liteamp.png - desktop entry icon
* prefix/share/pixmaps/liteamp/*.png - directory and image files
* prefix/share/man/man1/liteamp.1 - manpage file
* ~/.liteamp - directory and playlist files
* ~/.gnome2/liteamp - configuration file

Known Bugs
----------

* id3 and ogg-vorbis tag can't be modified.
* The status icon(play/pause) in the playlist disappear
  by changing playlist while playing.

ToDo
----

* streaming(internet radio) support
* audio cd supoort
* fully customizable playlist
* id3 & ogg-vorbis tag editor
* more fancy gui with fully drag&drop and clipboard support
* less cpu & memory load
* gnome panel applet support
* and so on...

Contacts
--------

Visit our homepage at http://liteamp.kldp.net
or project pages at http://kldp.net/projects/liteamp

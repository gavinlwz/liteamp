/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __STOCK_H__
#define __STOCK_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LA_STOCK_ADD_DIRECTORY	"la-add-directory"
#define LA_STOCK_ADD_FILE	"la-add-file"
#define LA_STOCK_ADD_LOCATION	"la-add-location"
#define LA_STOCK_ADD_PLAYLIST	"la-add-playlist"
#define LA_STOCK_BACK		"la-back"
#define LA_STOCK_FORWARD	"la-forward"
#define LA_STOCK_LITEAMP_LOGO	"la-liteamp-logo"
#define LA_STOCK_LITEAMP	"la-liteamp"
#define LA_STOCK_MUTE		"la-mute"
#define LA_STOCK_NEXT		"la-next"
#define LA_STOCK_PAUSE		"la-pause"
#define LA_STOCK_PLAYLIST	"la-playlist"
#define LA_STOCK_PREVIOUS	"la-previous"
#define LA_STOCK_REMOVE_PLAYLIST	"la-remove-playlist"
#define LA_STOCK_REPEAT		"la-repeat"
#define LA_STOCK_SHUFFLE	"la-shuffle"
#define LA_STOCK_START		"la-start"
#define LA_STOCK_STOP		"la-stop"
#define LA_STOCK_VOLUME_DOWN	"la-volume-down"
#define LA_STOCK_VOLUME_HI	"la-volume-hi"
#define LA_STOCK_VOLUME_LO	"la-volume-lo"
#define LA_STOCK_VOLUME_UP	"la-volume-up"
#define LA_STOCK_VOLUME		"la-volume"

void stock_init(void);

G_END_DECLS
#endif


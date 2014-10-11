/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __LA_CODESET_H__
#define __LA_CODESET_H__

#include <glib-object.h>

G_BEGIN_DECLS
#define LA_TYPE_CODESET	(la_codeset_get_type())
typedef enum _LaCodeset LaCodeset;

enum _LaCodeset {
    LA_CODESET_WEST_EUROPE,
    LA_CODESET_EAST_EUROPE,
    LA_CODESET_SOUTH_EUROPE,
    LA_CODESET_NORTH_EUROPE,
    LA_CODESET_CYRILLIC,
    LA_CODESET_ARABIC,
    LA_CODESET_GREEK,
    LA_CODESET_HEBREW,
    LA_CODESET_TURKISH,
    LA_CODESET_NORDIC,
    LA_CODESET_THAI,
    LA_CODESET_BALTIC,
    LA_CODESET_CELTIC,
    LA_CODESET_WEST_EUROPE_EURO,
    LA_CODESET_RUSSIAN,
    LA_CODESET_UKRAINE,
    LA_CODESET_CHINESE,
    LA_CODESET_JAPANESE,
    LA_CODESET_KOREAN,
    LA_N_CODESETS
};

GType la_codeset_get_type(void) G_GNUC_CONST;

const gchar *la_codeset_get_name(LaCodeset codeset);
LaCodeset la_codeset_from_name(const gchar * name);

const gchar *la_codeset_get_iconv(LaCodeset codeset);
LaCodeset la_codeset_from_iconv(const gchar * name);

G_END_DECLS
#endif				/* __LA_CODESET_H__ */

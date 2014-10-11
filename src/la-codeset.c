/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "la-codeset.h"

/* private-----------------------------------------------*/

static const GEnumValue self_values[] = {
    {LA_CODESET_WEST_EUROPE, "WestEurope", "west-europe"},
    {LA_CODESET_EAST_EUROPE, "EastEurope", "east-europe"},
    {LA_CODESET_SOUTH_EUROPE, "SouthEurope", "south-europe"},
    {LA_CODESET_NORTH_EUROPE, "NorthEurope", "north-europe"},
    {LA_CODESET_CYRILLIC, "Cyrillic", "cyrillic"},
    {LA_CODESET_ARABIC, "Arabic", "arabic"},
    {LA_CODESET_GREEK, "Greek", "greek"},
    {LA_CODESET_HEBREW, "Hebrew", "hebrew"},
    {LA_CODESET_TURKISH, "Turkish", "turkish"},
    {LA_CODESET_NORDIC, "Nordic", "nordic"},
    {LA_CODESET_THAI, "Thai", "thai"},
    {LA_CODESET_BALTIC, "Baltic", "baltic"},
    {LA_CODESET_CELTIC, "Celtic", "celtic"},
    {LA_CODESET_WEST_EUROPE_EURO, "WestEuropeEuro", "west-europe-euro"},
    {LA_CODESET_RUSSIAN, "Russian", "russian"},
    {LA_CODESET_UKRAINE, "Ukraine", "ukraine"},
    {LA_CODESET_CHINESE, "Chinese", "chinese"},
    {LA_CODESET_JAPANESE, "Japanese", "japanese"},
    {LA_CODESET_KOREAN, "Korean", "korean"},
    {0, NULL, NULL}
};

static const gchar *iconv_codesets[LA_N_CODESETS] = {
    "ISO-8859-1", "ISO-8859-2", "ISO-8859-3", "ISO-8859-4",
    "ISO-8859-5", "ISO-8859-6", "ISO-8859-7", "ISO-8859-8",
    "ISO-8859-9", "ISO-8859-10", "ISO-8859-11", "ISO-8859-13",
    "ISO-8859-14", "ISO-8859-15", "KOI-8R", "KOI-8U",
    "GB18030", "SJIS", "CP949"
};

/* public------------------------------------------------*/

const gchar *la_codeset_get_name(LaCodeset codeset)
{
    g_return_val_if_fail(codeset >= 0 && codeset < LA_N_CODESETS, NULL);
    return self_values[codeset].value_nick;

}

LaCodeset la_codeset_from_name(const gchar * name)
{
    LaCodeset codeset;
    for (codeset = 0; codeset < LA_N_CODESETS; codeset++) {
	if (!strcmp(name, self_values[codeset].value_nick)
	    || !strcmp(name, self_values[codeset].value_name))
	    return codeset;
    }
    return -1;
}

const gchar *la_codeset_get_iconv(LaCodeset codeset)
{
    g_return_val_if_fail(codeset >= 0 && codeset < LA_N_CODESETS, NULL);
    return iconv_codesets[codeset];
}

LaCodeset la_codeset_from_iconv(const gchar * name)
{
    LaCodeset codeset;
    for (codeset = 0; codeset < LA_N_CODESETS; codeset++) {
	if (!strcasecmp(name, iconv_codesets[codeset]))
	    return codeset;
    }
    return -1;
}

/* boilerplates -----------------------------------------*/

GType la_codeset_get_type(void)
{
    static GType self_type = 0;

    if (self_type)
	return self_type;

    self_type = g_enum_register_static("LaCodeset", self_values);
    return self_type;
}

/* la-codeset.c */

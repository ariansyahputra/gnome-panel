#include "clock-zonetable.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>

#include "clock-country.h"
#include "clock-zoneinfo.h"

G_DEFINE_TYPE (ClockZoneTable, clock_zonetable, G_TYPE_OBJECT)

typedef struct {
        gchar *zonetab;
        gchar *iso3166;

        GList *list;
        GHashTable *table;
        GHashTable *l10n_table;

        GHashTable *zone_whitelist;

        GList *country_list;
        GHashTable *country_table;
} ClockZoneTablePrivate;

/* Seeded with the list from Nat's Blackberry */
char *available_zones[] = {
        /* Eniwetok (-12) */
        "Pacific/Midway",
        "Pacific/Honolulu",
        "America/Anchorage",
        "America/Los_Angeles",
        "America/Phoenix",
        "America/Denver",
        "America/Chihuahua",
        /* Central America (-6) */
        "America/Regina",
        "America/Chicago",
        "America/Mexico_City",
        "America/Bogota",
        /* Indiana (-5) */
        "America/New_York",
        "America/Caracas",
        "America/Santiago",
        "America/Halifax",
        "America/St_Johns",
        "America/Argentina/Buenos_Aires",
        "America/Sao_Paulo",
        /* Greenland (-3) */
        /* Mid-Atlantic (-2) */
        "Atlantic/Cape_Verde",
        "Atlantic/Azores",
        "Africa/Casablanca",
        "Europe/London",
        /* W Central Africa */
        "Europe/Berlin",
        "Europe/Belgrade",
        "Europe/Brussels",
        "Europe/Sarajevo",
        "Africa/Harare",
        "Asia/Jerusalem",
        "Europe/Bucharest",
        "Africa/Cairo",
        "Europe/Athens",
        "Europe/Helsinki",
        "Asia/Kuwait",
        "Africa/Nairobi",
        "Asia/Baghdad",
        "Europe/Moscow",
        "Asia/Tehran",
        /* Abu Dhabi (+4) */
        "Asia/Baku",
        "Asia/Kabul",
        /* Islamabad (+5) */
        "Asia/Yekaterinburg",
        "Asia/Calcutta",
        /* Kathmandu (+5.75) */
        /* Astana (+6) */
        /* Sri Lanka (+6) */
        "Asia/Almaty",
        "Asia/Rangoon",
        "Asia/Bangkok",
        "Asia/Krasnoyarsk",
        "Asia/Beijing",
        "Asia/Kuala_Lumpur",
        "Australia/Perth",
        "Asia/Taipei",
        "Asia/Irkutsk",
        "Asia/Tokyo",
        "Asia/Seoul",
        "Asia/Yakutsk",
        "Australia/Darwin",
        "Australia/Adelaide",
        "Australia/Brisbane",
        "Pacific/Guam",
        "Australia/Sydney",
        "Australia/Hobart",
        "Asia/Vladivostok",
        "Asia/Magadan",
        "Pacific/Fiji",
        "Pacific/Auckland",
        /* Nuku'alofa (+13) */
        NULL
};

static void clock_zonetable_finalize (GObject *);
static void clock_zonetable_load_zonetab (ClockZoneTable *this);
static void clock_zonetable_load_iso3166 (ClockZoneTable *this);

#define PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), CLOCK_ZONETABLE_TYPE, ClockZoneTablePrivate))

ClockZoneTable *
clock_zonetable_new (gchar *zoneinfodir)
{
        ClockZoneTable *this;
        ClockZoneTablePrivate *priv;

        this = g_object_new (CLOCK_ZONETABLE_TYPE, NULL);
        priv = PRIVATE (this);

        priv->zonetab = g_build_filename (zoneinfodir, "zone.tab", NULL);
        priv->iso3166 = g_build_filename (zoneinfodir, "iso3166.tab", NULL);

#ifdef CLOCK_TEXTDOMAIN
        /* this is used when clock is embedded in the gnome-panel
           package */
        textdomain (CLOCK_TEXTDOMAIN);
        bindtextdomain (CLOCK_TEXTDOMAIN, GNOMELOCALEDIR);
#endif
	/* FMQ: this sucks; we are using Evolution's gettext domain for our own purposes */
        bindtextdomain (EVOLUTION_TEXTDOMAIN, GNOMELOCALEDIR);
        bind_textdomain_codeset (EVOLUTION_TEXTDOMAIN, "UTF-8");

#ifdef USE_CRIPPLED_ZONELIST
        int i;
        priv->zone_whitelist = g_hash_table_new (g_str_hash, g_str_equal);
        for (i=0; available_zones[i]; i++) {
                g_hash_table_replace
                        (priv->zone_whitelist, available_zones[i], this);
        }
#endif

        clock_zonetable_load_zonetab (this);
        clock_zonetable_load_iso3166 (this);

        return this;
}

static void
clock_zonetable_class_init (ClockZoneTableClass *this_class)
{
        GObjectClass *g_obj_class = G_OBJECT_CLASS (this_class);

        g_obj_class->finalize = clock_zonetable_finalize;

        g_type_class_add_private (this_class, sizeof (ClockZoneTablePrivate));
}

static void
clock_zonetable_init (ClockZoneTable *this)
{
        ClockZoneTablePrivate *priv = PRIVATE (this);

        priv->zonetab = NULL;
        priv->iso3166 = NULL;
        priv->list = NULL;
        priv->table = NULL;
        priv->l10n_table = NULL;

        priv->zone_whitelist = NULL;

        priv->country_list = NULL;
        priv->country_table = NULL;
}

static void
clock_zonetable_finalize (GObject *g_obj)
{
        ClockZoneTablePrivate *priv = PRIVATE (g_obj);

        if (priv->zonetab) {
                g_free (priv->zonetab);
                priv->zonetab = NULL;
        }

        if (priv->iso3166) {
                g_free (priv->iso3166);
                priv->iso3166 = NULL;
        }

        if (priv->list) {
                g_list_free (priv->list);
                priv->list = NULL;
        }

        if (priv->table) {
                g_hash_table_destroy (priv->table);
                priv->table = NULL;
        }

        if (priv->l10n_table) {
                g_hash_table_destroy (priv->l10n_table);
                priv->l10n_table = NULL;
        }

        if (priv->zone_whitelist) {
                g_hash_table_destroy (priv->zone_whitelist);
                priv->zone_whitelist = NULL;
        }

        if (priv->country_list) {
                g_list_free (priv->country_list);
                priv->country_list = NULL;
        }

        if (priv->country_table) {
                g_hash_table_destroy (priv->country_table);
                priv->country_table = NULL;
        }

        G_OBJECT_CLASS (clock_zonetable_parent_class)->finalize (g_obj);
}

static gfloat
clock_zonetable_parse_coord (gchar *coord)
{
        gfloat ret = 0.0;

        gfloat deg = 0;
        gfloat min = 0;
        gfloat sec = 0;

        gchar *num = coord + 1;
        int len = strlen (num);

        if (len == 4) {
                /* DDMM */
                sscanf (num, "%2f%2f", &deg, &min);
        } else if (len == 5) {
                /* DDDMM */
                sscanf (num, "%3f%2f", &deg, &min);
        } else if (len == 6) {
                /* DDMMSS */
                sscanf (num, "%2f%2f%2f", &deg, &min, &sec);
        } else if (len == 7) {
                /* DDDMMSS */
                sscanf (num, "%3f%2f%2f", &deg, &min, &sec);
        }

        ret = deg + min / 60 + sec / 3600;

        if (coord[0] == '-') {
                ret = -ret;
        }

        return ret;
}


static void
clock_zonetable_parse_location (gchar *location, gfloat *lat, gfloat *lon)
{
        int i;

        gchar *lat_str;
        gchar *lon_str;

        for (i = 1; location[i] != '\0'; i++) {
                if (location[i] == '+' || location[i] == '-') {
                        lat_str = g_strndup (location, i);
                        lon_str = location + i;
                        break;
                }
        }

        *lat = clock_zonetable_parse_coord (lat_str);
        *lon = clock_zonetable_parse_coord (lon_str);

        g_free (lat_str);
        /* lon_str wasn't copied */
}

static ClockZoneInfo *
clock_zonetable_parse_info_line (gchar *line)
{
        ClockZoneInfo *ret;

        gchar *country = NULL;
        gchar *location = NULL;
        gchar *zone = NULL;
        gchar *comment = NULL;

        gfloat lat;
        gfloat lon;

        gchar **split = g_strsplit (line, "\t", 0);

        country = split[0];
        location = split[1];
        zone = split[2];

        if (split[3] != '\0') {
                comment = split[3];
        }

        clock_zonetable_parse_location (location, &lat, &lon);

        ret = clock_zoneinfo_new (zone, country, comment, lat, lon);

        g_strfreev (split);
        return ret;
}

static ClockCountry *
clock_zonetable_parse_iso3166_line (gchar *line)
{
        ClockCountry *ret;

        gchar *code = NULL;
        gchar *name = NULL;

        gchar **split = g_strsplit (line, "\t", 0);

        code = split[0];
        name = split[1];

        ret = clock_country_new (code, name);

        g_free (split);
        return ret;
}

static void
clock_zonetable_load_zonetab (ClockZoneTable *this)
{
        ClockZoneTablePrivate *priv = PRIVATE (this);
        gchar *line;

        priv->table = g_hash_table_new (g_str_hash, g_str_equal);
        priv->l10n_table = g_hash_table_new (g_str_hash, g_str_equal);

        GIOChannel *channel = g_io_channel_new_file (priv->zonetab, "r", NULL);

        g_assert (channel != NULL);

        while (g_io_channel_read_line (channel, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
                g_strstrip (line);

                if (line[0] != '#') {
                        ClockZoneInfo *info =
                                clock_zonetable_parse_info_line (line);

                        if (priv->zone_whitelist &&
                            !g_hash_table_lookup (priv->zone_whitelist, clock_zoneinfo_get_name (info))) {
                                g_object_unref (info);
                                g_free (line);
                                continue;
                        }

                        priv->list = g_list_prepend (priv->list, info);
                        g_hash_table_replace (priv->table, clock_zoneinfo_get_name (info), info);
                        g_hash_table_replace (priv->l10n_table, clock_zoneinfo_get_l10n_name (info), info);
                }

                g_free (line);
        }

        g_io_channel_unref (channel);
}

static void
clock_zonetable_load_iso3166 (ClockZoneTable *this)
{
        ClockZoneTablePrivate *priv = PRIVATE (this);
        gchar *line;

        priv->country_table = g_hash_table_new (g_str_hash, g_str_equal);

        GIOChannel *channel = g_io_channel_new_file (priv->iso3166, "r", NULL);

        g_assert (channel != NULL);

        while (g_io_channel_read_line (channel, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
                g_strstrip (line);

                if (line[0] != '#') {
                        ClockCountry *info =
                                clock_zonetable_parse_iso3166_line (line);
                        priv->country_list = g_list_prepend (priv->country_list, info);
                        g_hash_table_replace (priv->country_table, clock_country_get_code (info), info);
                }

                g_free (line);
        }

        g_io_channel_unref (channel);
}

ClockZoneInfo *
clock_zonetable_get_zone (ClockZoneTable *this, gchar *name)
{
        ClockZoneTablePrivate *priv = PRIVATE (this);

        return CLOCK_ZONEINFO (g_hash_table_lookup (priv->table, name));
}

ClockZoneInfo *
clock_zonetable_get_l10n_zone (ClockZoneTable *this, gchar *l10n_name)
{
        ClockZoneTablePrivate *priv = PRIVATE (this);

        return CLOCK_ZONEINFO (g_hash_table_lookup (priv->l10n_table, l10n_name));
}

GList *
clock_zonetable_get_zones (ClockZoneTable *this)
{
        ClockZoneTablePrivate *priv = PRIVATE (this);

        return priv->list;
}

ClockCountry *
clock_zonetable_get_country (ClockZoneTable *this, gchar *code)
{
        ClockZoneTablePrivate *priv = PRIVATE (this);

        return CLOCK_COUNTRY (g_hash_table_lookup (priv->country_table, code));
}
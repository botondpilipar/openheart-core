#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-kernel"

#include <kernel_data.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <glib.h>
#include <func.h>
#include <math.h>

#define _DAYS_OF_WEEK 7

/* Example verbose date format string
    Wed Apr 17 19:21:08 UTC 2024
*/
#define _VERBOSE_FMT_SEGMENTS 6

#define _VERBOSE_FMT_TIME_SEGMENTS 3

/* Kernel Dates */

static GHashTable* verbose_date_fmt_day_keywords = NULL;
static const char day_keywords[7][4] = {"Mon", "Tue", "Wed", 
                                        "Thu", "Fri", "Sat", "Sun"};
static const GDateDay gdays[7] = {G_DATE_MONDAY, G_DATE_TUESDAY, G_DATE_WEDNESDAY,
                                  G_DATE_THURSDAY, G_DATE_FRIDAY, G_DATE_SATURDAY, G_DATE_SUNDAY};

static GHashTable* verbose_date_fmt_month_keywords = NULL;
static const char month_keywords[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static const GDateMonth gmonths[12] = {G_DATE_JANUARY, G_DATE_FEBRUARY, G_DATE_MARCH, G_DATE_APRIL, G_DATE_MAY, G_DATE_JUNE, G_DATE_JULY,
                                       G_DATE_AUGUST, G_DATE_SEPTEMBER, G_DATE_OCTOBER, G_DATE_NOVEMBER, G_DATE_DECEMBER};

static GDateDay date_parse_verbose_day(gchar* buffer)
{
    if(verbose_date_fmt_day_keywords == NULL) {
        verbose_date_fmt_day_keywords = g_hash_table_new_full(g_str_hash,  g_str_equal, g_free, g_free);

        for(short i = 0; i < _DAYS_OF_WEEK; i++) {
            g_hash_table_insert(verbose_date_fmt_day_keywords,
                                q_alloc(day_keywords[i]),
                                q_alloc(gdays[i]));
        }
    }

    g_return_val_if_fail(g_hash_table_contains(verbose_date_fmt_day_keywords, buffer), G_DATE_BAD_DAY);
    return *(GDateDay*)g_hash_table_lookup(verbose_date_fmt_day_keywords, buffer);
}

static GDateMonth date_parse_verbose_month(gchar* buffer)
{
    if(verbose_date_fmt_month_keywords == NULL) {
        verbose_date_fmt_month_keywords = g_hash_table_new_full(g_str_hash,  g_str_equal, g_free, g_free);

        for(short i = 0; i < _DAYS_OF_WEEK; i++) {
            g_hash_table_insert(verbose_date_fmt_month_keywords,
                                q_alloc(month_keywords[i]),
                                q_alloc(gmonths[i]));
        }
    }

    g_return_val_if_fail(g_hash_table_contains(verbose_date_fmt_month_keywords, buffer), G_DATE_BAD_MONTH);
    return *(GDateMonth*)g_hash_table_lookup(verbose_date_fmt_month_keywords, buffer);
}

static GDateTime* date_parse_verbose_fmt(gchar* buffer) 
{
    g_autofree gchar** buffer_segments = g_strsplit(buffer, " ", _VERBOSE_FMT_SEGMENTS);
    guint date_n_segmetns = g_strv_length(buffer_segments);
    if(date_n_segmetns != _VERBOSE_FMT_SEGMENTS) {
        return NULL;
    }

    // GDateDay _day = date_parse_verbose_day(buffer_segments[0]);
    GDateMonth month = date_parse_verbose_month(buffer_segments[1]);
    
    gint day_num = atoi(buffer_segments[2]);
    gint year_num = atoi(buffer_segments[5]);

    const gchar* time_seg_view = buffer_segments[3];
    g_autofree char** time_segments = g_strsplit(time_seg_view, ":", _VERBOSE_FMT_TIME_SEGMENTS);
    guint time_n_segments = g_strv_length(time_segments);
    if(time_n_segments != _VERBOSE_FMT_TIME_SEGMENTS) {
        return NULL;
    }


    gint hour_num = atoi(time_segments[0]);
    gint minute_num = atoi(time_segments[1]);
    gint sec_num = atoi(time_segments[2]);

    const bool is_utc = g_strcmp0(buffer_segments[4], "UTC") == 0;
    GDateTime* (*dt_constructor)(gint, gint, gint, gint, gint, gdouble) = 
        is_utc ? g_date_time_new_utc : g_date_time_new_local;

    return dt_constructor(year_num, (gint)month, day_num, hour_num, minute_num, (gdouble)sec_num);
}

/* Kernel Data helpers */
bool kd_parse_dates(kernel_data* data, gchar* buf)
{
    g_autoptr(GError) builddate_comp_reg = NULL;
    g_autoptr(GMatchInfo) builddate_match_info = NULL;
    const char* const release_pattern = "(?<release>([A-Z][a-z][a-z] .* $))";
    g_autoptr(GRegex) build_date_reg = g_regex_new(release_pattern, G_REGEX_DOLLAR_ENDONLY | G_REGEX_EXTENDED, 0, &builddate_comp_reg);

    if(builddate_comp_reg) {
        g_error("Error compiling build date matcher: %s", builddate_comp_reg->message);
        return false;
    }

    const bool matched = g_regex_match(build_date_reg, buf, G_REGEX_MATCH_DEFAULT, &builddate_match_info);

    if(!matched) {
        g_debug("Could not match '%s' buffer with '%s' pattern", buf, release_pattern);
        return false;
    }

    GDateTime* dt = date_parse_verbose_fmt(g_match_info_fetch_named(builddate_match_info, "release"));
    if(dt == NULL) {
        g_error("Could not parse kernel build date properly");
    }

    data->build_date = dt;

    return true;
}

// Not the ideal solution, but C compiler is playing tricks on me with 'sqrt'
#define KERNEL_FEATURES_LEN 12

static GHashTable* kernel_features_table = NULL;
static const gchar kernel_features_kw[KERNEL_FEATURES_LEN][16] = { "SMP", "PREEMPT_DYNAMIC", "PREEMPT_RT", "VIRT", "PAE", "AGP", "HZ",
                                            "EFI", "CPU_3DNOW", "CPU_3DNOWPLUS", "MEM_NUMA", "MEM_SLAVE_HELPER" };
static const kernel_features kernel_features_v[KERNEL_FEATURES_LEN] = { SMP, PREEMPT_DYNAMIC, PREEMPT_RT, VIRT, PAE, AGP, HZ,
                                   EFI, CPU_3DNOW, CPU_3DNOWPLUS, MEM_NUMA, MEM_SLAVE_HELPER };

static gint kd_extract_kernel_features(kernel_data* data, const gchar* buf)
{
    if(kernel_features_table == NULL) {
        kernel_features_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
        for(size_t i = 0; i < KERNEL_FEATURES_LEN; i++) {
            g_hash_table_insert(kernel_features_table, q_alloc(kernel_features_kw[i]), q_alloc(kernel_features_v[i]));
        }
    }

    g_autofree gchar** feature_words = g_strsplit(buf, " ", 16);
    gint r = KERNEL_FEATURES_MAX; // Set last digit

    for(guint i = 0; i < g_strv_length(feature_words); i++) {
        if(!g_hash_table_contains(kernel_features_table, feature_words[i])) {
            g_warning("Unrecognized word for kernel feature: %s", feature_words[i]);
            return KERNEL_FEATURES_MAX;
        }

        r |= *(kernel_features*)g_hash_table_lookup(kernel_features_table, feature_words[i]);
    }

    return r ^ KERNEL_FEATURES_MAX; // Unset last digit
}


bool kd_parse_features(kernel_data* data, gchar* buf)
{
    g_autoptr(GError) features_comp_reg = NULL;
    g_autoptr(GMatchInfo) features_match_info = NULL;
    const char* const features_pattern = "\\# (?<buildnum>\\S+) \\s (?<features>([A-Z_][A-Z_]+ \\s )+)";
    g_autoptr(GRegex) features_reg = g_regex_new(features_pattern, G_REGEX_EXTENDED, 0, &features_comp_reg);

    if(features_comp_reg) {
        g_error("Error compiling feature matcher: %s", features_comp_reg->message);
        return false;
    }

    const bool matched = g_regex_match(features_reg, buf, G_REGEX_MATCH_DEFAULT, &features_match_info);
    g_autofree gchar* buildnum = g_match_info_fetch_named(features_match_info, "buildnum");
    g_autofree gchar* features = g_match_info_fetch_named(features_match_info, "features");

    if(!matched || buildnum == NULL || features == NULL) {
        g_debug("Could not match '%s' buffer with '%s' pattern", buf, features_pattern);
        return false;
    }

    data->build_number = (uint16_t)atoi(g_strstrip(buildnum));
    data->features = (uint32_t)kd_extract_kernel_features(data, g_strstrip(features));

    return true;
}

bool kd_parse_from_proc_version(kernel_data* data, gchar* buffer)
{
    if(!kd_parse_dates(data, buffer)) {
        g_error("Could not initialize kernel build date");
        return false;
    }
    if(!kd_parse_features(data, buffer)) {
        g_error("Could not initiialize kernel features");
        return false;
    }

    return true;
}

/* Kernel Data (aggregate) */

void kernel_data_init(kernel_data* data)
{
    if(!kd_init_natives(data)) {
        g_error("Could not initialize natives");
        return;
    }

    if(!kd_parse_from_proc_version(data, data->attributes->version)) {
        g_error("Error while parsing kv_content=%s", data->attributes->version);
    }
}

kernel_data* kernel_data_new(void)
{
    kernel_data* data = g_new(kernel_data, 1);
    kernel_data_init(data);
    return data;
}

bool kd_init_natives(kernel_data* data)
{
    system_info* sys_info = g_new(system_info, 1);
    int sysinfo_rc = sysinfo(sys_info);
    if(sysinfo_rc == -1) {
        g_warning("Failure to execute sysinfo(): code %d, message %s", errno, strerror(errno));
        g_free_sized(sys_info, sizeof(system_info));
    } else {
        data->sys_info = sys_info;
    }

    kernel_attributes* u_name = g_new(kernel_attributes, 1);
    int uname_rc = uname(u_name);
    if(uname_rc == -1) {
        g_warning("Failure to execute uname(): code %d, message %s", errno, strerror(errno));
        g_free_sized(u_name, sizeof(kernel_attributes));
    } else {
        data->attributes = u_name;
    }

    release_version release;
    bool release_parsed = release_version_parse(&release, u_name);
    if(!release_parsed) {
        g_warning("Could not parse release from kernel attributes");
    } else {
        data->release = release;
    }

    return sysinfo_rc != -1 && uname_rc != -1 && release_parsed;
}

void kernel_data_free(kernel_data* data)
{
    if(data->build_date != NULL) {
        g_date_time_unref(data->build_date);
    }
}

release_version* release_version_new(void)
{
    kernel_attributes attr;
    int uname_rc = uname(&attr);
    if(uname_rc == -1) {
        return NULL;
    }

    release_version* version = g_new(release_version, 1);
    bool parsed = release_version_parse(version, &attr);
    if(!parsed) {
        g_free_sized(version, sizeof(release_version));
        return NULL;
    }

    return version;
}

bool release_version_parse(release_version* version, kernel_attributes* attr)
{
    g_autoptr(GError) comp_errors = NULL;
    g_autoptr(GMatchInfo) release_match = NULL;
    const char* const release_pattern = "(\\d+)\\.(\\d+)\\.(\\d+)-(\\d+)";
    g_autoptr(GRegex) release_reg = g_regex_new(release_pattern, G_REGEX_ANCHORED, 0, &comp_errors);

    if(comp_errors) {
        g_error("Regex compilation issue: %s", comp_errors->message);
        return false;
    }

    bool matched = g_regex_match(release_reg, attr->release, 0, &release_match);

    if(!matched || release_match == NULL) {
        g_error("Match failure of pattern: %s on input: %s", release_pattern, attr->release);
        return false;
    }

    version->major = (uint32_t)atoi(g_match_info_fetch(release_match, 1));
    version->minor = (uint32_t)atoi(g_match_info_fetch(release_match, 2));
    version->micro = (uint32_t)atoi(g_match_info_fetch(release_match, 3));
    version->patch = (uint32_t)atoi(g_match_info_fetch(release_match, 4));

    return true;
}


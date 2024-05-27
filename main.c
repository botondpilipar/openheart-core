#include <glib.h>
#include <gio/gio.h>
#include <stdbool.h>
#include <main_process.h>

const gchar *const debug_comains = "all";

int main(int argc, char** argv)
{
    // g_log_set_handler(NULL, G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, g_log_default_handler, NULL);
    g_log_set_debug_enabled(true);
    return openheart_core_main(argc, argv);
}

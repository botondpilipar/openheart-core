#include <glib.h>
#include <stdbool.h>
#include <main_process.h>

const gchar *const debug_comains = "all";

int main(int argc, char** argv)
{
    g_log_set_debug_enabled(true);
    g_log_writer_default_set_debug_domains(&debug_comains);
    return openheart_core_main(argc, argv);
}

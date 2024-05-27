#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "openheart-core-main"

#include "main_process.h"
#include <fs_notify.h>
#include <fs_notify_handler.h>
#include <signal_handlers.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <glib-unix.h>

GOptionContext* openheart_core_get_args(int argc, char** argv)
{
    /* Not yet utilized */
    return g_option_context_new(argc > 1 ? argv[argc] : "");
}

gint openheart_core_main(int argc, char** argv)
{
    __attribute((unused)) GOptionContext* _args = openheart_core_get_args(argc, argv);
    GMainContext* main_context = g_main_context_default();
    GMainLoop* main_loop = g_main_loop_new(main_context, true);


    g_unix_signal_add(SIGTERM, on_user_terminate, main_loop);
    g_unix_signal_add(SIGINT, on_user_interrupt, main_loop);

    // Information about who runs the application
    g_debug("Running with user ID: %u", getuid());

    g_auto(fs_notify_device) nt_device;
    fs_notify_device_init(&nt_device);

    g_autoptr(GPtrArray) leafs = fs_notify_leaf_new_recursive("/proc/sys/fs", false);

    g_main_loop_run(main_loop);
    return 0;
}

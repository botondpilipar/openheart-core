#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "openheart-core-main"

#include "main_process.h"
#include <kp_notify.h>
#include <kp_notify_handler.h>
#include <stdbool.h>

int on_kernel_parameter_access(const char* name, int /* descriptor */, unsigned int /* bitmask */, unsigned int /* cookie */)
{
    g_debug("Kernel parameter accessed: %s", name);
    return true;
}

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

    kp_notify_device nt_device;
    kp_notify_device_init(&nt_device);

    GPtrArray* leafs = kp_notify_leaf_new_recursive("/proc/sys", false);
    kp_notify_leaf root;
    kp_notify_leaf_init(&root, "/proc/sys", false);
    kp_notify_device_add_watch_recursive(&nt_device, leafs, &root, IN_ALL_EVENTS);

    kp_notify_device_set_gio_handler(&nt_device, kp_notify_handle_default);
    kp_notify_device_add_callback(&nt_device, &on_kernel_parameter_access, IN_ACCESS);

    g_main_loop_run(main_loop);
    return 0;
}

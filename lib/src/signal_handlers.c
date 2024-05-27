#define G_LOG_DOMAIN "libopenheart-kernel"

#include <signal_handlers.h>

int on_user_terminate(gpointer data) {
    GMainLoop* main_loop = (GMainLoop*) data;
    g_info("Handling SIGTERM");

    if(!g_main_loop_is_running(main_loop)) {
        g_info("Main event loop is not running. Nothing to do.");
        return true;
    }

    return true;
}

int on_user_interrupt(gpointer data)
{
    GMainLoop* main_loop = (GMainLoop*) data;
    g_info("Handling SIGINT");

    g_main_loop_quit(main_loop);
    return true;
}

int on_user_quit(gpointer data) {
    GMainLoop* main_loop = (GMainLoop*) data;
    g_info("Handling SIGQUIT");

    g_main_loop_quit(main_loop);
    return true;
}

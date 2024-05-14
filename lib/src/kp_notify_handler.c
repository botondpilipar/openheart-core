#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-kernel"

#include <kp_notify_handler.h>

static const uint32_t DISPATCHED_EVENTS[] = {
        IN_ACCESS, IN_MODIFY, IN_ATTRIB, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_OPEN,
        IN_MOVED_FROM, IN_MOVED_TO, IN_DELETE, IN_CREATE, IN_DELETE_SELF, IN_UNMOUNT, IN_Q_OVERFLOW,
        IN_IGNORED, IN_ISDIR
};

INotifyCallback kp_notify_select(kp_notify_device* device, uint32_t bitmask)
{
    for(uint32_t i = 0; i<sizeof(DISPATCHED_EVENTS); i++) {
        if(bitmask & DISPATCHED_EVENTS[i]) {
            INotifyCallback callback = (INotifyCallback)g_hash_table_lookup(device->gio_watch_callbacks, &DISPATCHED_EVENTS[i]);
            return callback;
        }
    }
    return NULL;
}

int kp_notify_handle_default(GIOChannel *gio, GIOCondition /* unused */, gpointer data)
{
    gchar buffer[KP_NOTIFY_BUF_SIZE];
    size_t chars_read = 0;
    GError* errors = NULL;

    g_io_channel_read_chars(gio, buffer, KP_NOTIFY_BUF_SIZE, &chars_read, &errors);
    if (errors != NULL) {
        g_warning ("Error reading the inotify device: %s\n", errors->message);
        return false;
    }

    size_t event_number = chars_read / sizeof(struct inotify_event);
    struct inotify_event** events = (struct inotify_event**)buffer;
    kp_notify_device* device = (kp_notify_device*)data;

    for(size_t i = 0; i<event_number; i++) {
        struct inotify_event* next_event = events[i];
        uint32_t bitmask = next_event->mask;
        INotifyCallback callback = kp_notify_select(device, bitmask);

        if(callback == NULL) {
            g_info("No callback found for inotify event bitmask: %u", bitmask);
            continue;
        }

        if(!callback(next_event->name, next_event->wd, next_event->mask, next_event->cookie)) {
            return false;
        }
    }
    return true;
}

#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-kernel"

#include <kp_notify_handler.h>

static const uint32_t DISPATCHED_EVENTS[] = {
        IN_ACCESS, IN_MODIFY, IN_ATTRIB, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_OPEN,
        IN_MOVED_FROM, IN_MOVED_TO, IN_DELETE, IN_CREATE, IN_DELETE_SELF, IN_UNMOUNT, IN_Q_OVERFLOW,
        IN_IGNORED, IN_ISDIR
};

void kp_notify_invoke_callbacks(kp_notify_device* device, uint32_t bitmask, const struct inotify_event* event)
{
    GPtrArray* keys = g_hash_table_get_keys_as_ptr_array(device->gio_watch_callbacks);

    for(size_t i = 0; i<keys->len; i++) {
        uint32_t watch_mask = *(uint32_t*)g_ptr_array_index(keys, i);

        if(bitmask & watch_mask) {
            INotifyCallback cb = (INotifyCallback)g_hash_table_lookup(device->gio_watch_callbacks, &watch_mask);
            cb(event->name, event->wd, event->mask, event->cookie);
        }
    }
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
    g_autoptr(GArray) events = g_array_new_take(buffer, chars_read, true, sizeof(struct inotify_event));
    kp_notify_device* device = (kp_notify_device*)data;

    for(size_t i = 0; i<events->len; i++) {
        struct inotify_event* event = &g_array_index(events, struct inotify_event, i);
        kp_notify_invoke_callbacks(device, event->mask, event);
    }
    return true;
}

void on_kp_notify_leaf_destoryed_default(int watch_desc)
{
    int notification_device = inotify_init();
    inotify_rm_watch(notification_device, watch_desc);
}

void on_kp_notify_leaf_destroyed(kp_notify_device* device, int fd)
{
    g_hash_table_remove(device->notify_watches, &fd);
}

#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-kernel"

#include <fs_notify_handler.h>

static const uint32_t DISPATCHED_EVENTS[] = {
        IN_ACCESS, IN_MODIFY, IN_ATTRIB, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_OPEN,
        IN_MOVED_FROM, IN_MOVED_TO, IN_DELETE, IN_CREATE, IN_DELETE_SELF, IN_UNMOUNT, IN_Q_OVERFLOW,
        IN_IGNORED, IN_ISDIR
};

void fs_notify_invoke_callbacks(fs_notify_device* device, uint32_t bitmask, const struct inotify_event* event)
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

int fs_notify_handle_default(GIOChannel *channel, GIOCondition condition, gpointer data)
{
    if(condition != G_IO_IN) {
        g_warning("Got unexpected GIO condition: %u", condition);
    }

    gchar buffer[KP_NOTIFY_BUF_SIZE];
    size_t chars_read = 0;
    GError* errors = NULL;

    GIOFlags flags = g_io_channel_get_flags(channel);
    if(!(flags & G_IO_FLAG_IS_READABLE)) {
        g_error("GIO handler called, yet gio channel is not readable");
        return (int)flags;
    }

    g_io_channel_read_chars(channel, buffer, KP_NOTIFY_BUF_SIZE, &chars_read, &errors);

    if (errors != NULL) {
        g_warning ("Error reading the inotify device: %s\n", errors->message);
        return false;
    }

    if(chars_read < sizeof(struct inotify_event)) {
        g_info("Could not read enough characters (total %zu) from notification device %d",
                chars_read, g_io_channel_unix_get_fd(channel));
        return 0;
    }

    g_autoptr(GArray) events = g_array_new_take(buffer, chars_read, true, sizeof(struct inotify_event));
    fs_notify_device* device = (fs_notify_device*)data;

    if(events->len == 0) {
        g_warning("Failed to read inotify events, got 0 'struct inotify_event' objects");
    }

    for(size_t i = 0; i<events->len; i++) {
        struct inotify_event* event = &g_array_index(events, struct inotify_event, i);
        fs_notify_invoke_callbacks(device, event->mask, event);
    }
    return true;
}

void on_fs_notify_leaf_destoryed_default(int watch_desc)
{
    int notification_device = inotify_init();
    inotify_rm_watch(notification_device, watch_desc);
}

void on_fs_notify_leaf_destroyed(fs_notify_device* device, int fd)
{
    g_hash_table_remove(device->notify_watches, &fd);
}

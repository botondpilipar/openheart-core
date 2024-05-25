#pragma once
#include <glib.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define KP_FILE_PATH_MAX_LEN 256
#define KP_FILE_LINE_MAX_LENGTH 128
#define KP_MAX_WATCHERS 512
#define KP_MASK_DIGITS 32

typedef gboolean (*INotifyCallback) (const char *, int, unsigned int, unsigned int);
typedef void(*KPNotifyCallback)(int);


typedef enum node_type
{
    DIRECTORY,
    REGULAR_FILE,
    SOFTLINK,
    OTHER,
} node_type;

typedef struct kp_notify_device
{
    GIOChannel* device_channel; /* Main gio device wrapping the inotify device descriptor */
    GHashTable* notify_watches; /* All file descriptors being watched by the device, hashed by descriptor value */
    GHashTable* gio_watch_callbacks; /* All event loop callbacks hashed by the event bitmasks */
} kp_notify_device;

typedef struct kp_notify_leaf
{
    node_type nodetype;
    bool follow_symlinks;
    int watch_descriptor;
    gulong last_modified;
    gchar* content;
    gchar* filepath;
} kp_notify_leaf;

G_END_DECLS

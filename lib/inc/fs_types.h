#pragma once
#include <glib.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define FS_FILE_PATH_MAX_LEN 256
#define FS_FILE_LINE_MAX_LENGTH 128
#define FS_MAX_WATCHERS 512
#define FS_MASK_DIGITS 32

typedef gboolean (*INotifyCallback) (const char *, int, unsigned int, unsigned int);
typedef void(*FSNotifyCallback)(int);


typedef enum node_type
{
    DIRECTORY,
    REGULAR_FILE,
    SOFTLINK,
    OTHER,
} node_type;

typedef struct fs_notify_device
{
    GIOChannel* device_channel; /* Main gio device wrapping the inotify device descriptor */
    GHashTable* notify_watches; /* All file descriptors being watched by the device, hashed by descriptor value */
    GHashTable* gio_watch_callbacks; /* All event loop callbacks hashed by the event bitmasks */
} fs_notify_device;

typedef struct fs_notify_leaf
{
    node_type nodetype;
    bool follow_symlinks;
    int watch_descriptor;
    gulong last_modified;
    gchar* content;
    gchar* filepath;
} fs_notify_leaf;

G_END_DECLS

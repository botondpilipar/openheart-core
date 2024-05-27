#pragma once
#include <glib.h>
#include <fs_types.h>
#include <stdint.h>
#include <sys/inotify.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define KP_NOTIFY_BUF_LEN 512
#define KP_NOTIFY_BUF_SIZE KP_NOTIFY_BUF_LEN * sizeof(struct inotify_event)

void fs_notify_invoke_callbacks(fs_notify_device* device, uint32_t bitmask, const struct inotify_event* event);

int fs_notify_handle_default(GIOChannel *gio, GIOCondition condition, gpointer data);

void on_fs_notify_leaf_destoryed_default(int fd);

void on_fs_notify_leaf_destroyed(fs_notify_device* device, int fd);

G_END_DECLS

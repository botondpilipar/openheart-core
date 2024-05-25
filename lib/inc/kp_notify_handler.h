#pragma once
#include <glib.h>
#include <kp_types.h>
#include <stdint.h>
#include <sys/inotify.h>
#include <stdbool.h>

#define KP_NOTIFY_BUF_LEN 512
#define KP_NOTIFY_BUF_SIZE KP_NOTIFY_BUF_LEN * sizeof(struct inotify_event)

void kp_notify_invoke_callbacks(kp_notify_device* device, uint32_t bitmask, const struct inotify_event* event);

int kp_notify_handle_default(GIOChannel *gio, GIOCondition condition, gpointer data);

void on_kp_notify_leaf_destoryed_default(int fd);

void on_kp_notify_leaf_destroyed(kp_notify_device* device, int fd);

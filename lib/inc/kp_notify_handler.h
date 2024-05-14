#pragma once
#include <glib.h>
#include <kp_types.h>
#include <stdint.h>
#include <sys/inotify.h>
#include <stdbool.h>

#define KP_NOTIFY_BUF_LEN 512
static const uint32_t KP_NOTIFY_BUF_SIZE = KP_NOTIFY_BUF_LEN * sizeof(struct inotify_event);

INotifyCallback kp_notify_select(kp_notify_device* device, uint32_t bitmask);

int kp_notify_handle_default(GIOChannel *gio, GIOCondition condition, gpointer data);

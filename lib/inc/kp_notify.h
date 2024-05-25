#pragma once
#include <kp_types.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <glib.h>
#include <stdint.h>

G_BEGIN_DECLS


/* node_type functions */
node_type get_node_type(const struct stat* stat_entry);

/* kp_notify_device functions */
void kp_notify_device_init(kp_notify_device* device);
void kp_notify_device_free(kp_notify_device* device);

bool kp_notify_device_add_watch(kp_notify_device* device, kp_notify_leaf* leaf, uint32_t mask);
bool kp_notify_device_add_watch_recursive(kp_notify_device* device, GPtrArray* all_leafs, kp_notify_leaf* root_leaf, uint32_t bitmask);
bool kp_notify_device_rm_watch(kp_notify_device* device, kp_notify_leaf* leaf);
bool kp_notify_device_rm_watch_recursive(kp_notify_device* device, GPtrArray* all_leafs, kp_notify_leaf* root_leaf);

bool kp_notify_device_add_callback(kp_notify_device* device, INotifyCallback callback, uint32_t bitmask);
bool kp_notify_device_rm_callback(kp_notify_device* device, uint32_t bitmask);

/* kp_notify_leaf_functions */
GPtrArray* kp_notify_leaf_children(kp_notify_leaf* node);
void kp_notify_leaf_set_follow(kp_notify_leaf* node, bool follow);
void kp_notify_leaf_init(kp_notify_leaf* node, const gchar* path, bool follow_symlinks);
kp_notify_leaf* kp_notify_leaf_new(const gchar* path, bool follow_symlinks);
GPtrArray* kp_notify_leaf_new_recursive(const gchar* path, bool follow_symlinks);
void kp_notify_leaf_free(kp_notify_leaf* node);
void kp_notify_leaf_cleanup(gpointer obj); /* Private function. For automatic cleanup only*/

bool kp_notify_leaf_watch(kp_notify_leaf* node, kp_notify_device* device, uint32_t bitmask, GIOFunc callback);
bool kp_notify_leaf_rm_watch(kp_notify_leaf* node, kp_notify_device* device);
size_t kp_notify_leaf_watch_recursive(kp_notify_leaf* root_node, kp_notify_device* device);
size_t kp_notify_leaf_rm_watch_recursive(kp_notify_leaf* root_node, kp_notify_device* device);
bool kp_notify_device_set_gio_handler(kp_notify_device* device, GIOFunc handler);

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC(kp_notify_leaf, kp_notify_leaf_cleanup)

/* Utility functions */
bool kp_notify_leaf_read_content(kp_notify_leaf* leaf, int descriptor);
bool kp_notify_leaf_is_watched(const kp_notify_leaf* leaf);

G_END_DECLS

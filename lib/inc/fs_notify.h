#pragma once
#include <fs_types.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <glib.h>
#include <stdint.h>

G_BEGIN_DECLS

/* node_type functions */
node_type get_node_type(const struct stat* stat_entry);

/* fs_notify_device functions */

void fs_notify_device_init(fs_notify_device* device);
void fs_notify_device_free(fs_notify_device* device);

bool fs_notify_device_add_watch(fs_notify_device* device, fs_notify_leaf* leaf, uint32_t mask);
bool fs_notify_device_add_watch_recursive(fs_notify_device* device, GPtrArray* all_leafs, fs_notify_leaf* root_leaf, uint32_t bitmask);
bool fs_notify_device_rm_watch(fs_notify_device* device, fs_notify_leaf* leaf);
bool fs_notify_device_rm_watch_recursive(fs_notify_device* device, GPtrArray* all_leafs, fs_notify_leaf* root_leaf);

bool fs_notify_device_add_callback(fs_notify_device* device, INotifyCallback callback, uint32_t bitmask);
bool fs_notify_device_rm_callback(fs_notify_device* device, uint32_t bitmask);

void fs_notify_device_clenaup(gpointer obj); /* Private function. For automatic cleanup only*/
G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC(fs_notify_device, fs_notify_device_clenaup)

/* fs_notify_leaf_functions */

GPtrArray* fs_notify_leaf_children(fs_notify_leaf* node);
GPtrArray* fs_notify_leaf_children_recursive(fs_notify_leaf* node);

void _fs_notify_leaf_children_r_impl(GPtrArray* results, GHashTable* resolved);

void fs_notify_leaf_set_follow(fs_notify_leaf* node, bool follow);
void fs_notify_leaf_init(fs_notify_leaf* node, const gchar* path, bool follow_symlinks);
fs_notify_leaf* fs_notify_leaf_new(const gchar* path, bool follow_symlinks);
GPtrArray* fs_notify_leaf_new_recursive(const gchar* path, bool follow_symlinks);
void fs_notify_leaf_free(fs_notify_leaf* node);
void fs_notify_leaf_cleanup(gpointer obj); /* Private function. For automatic cleanup only*/

bool fs_notify_leaf_watch(fs_notify_leaf* node, fs_notify_device* device, uint32_t bitmask, GIOFunc callback);
bool fs_notify_leaf_rm_watch(fs_notify_leaf* node, fs_notify_device* device);
size_t fs_notify_leaf_watch_recursive(fs_notify_leaf* root_node, fs_notify_device* device);
size_t fs_notify_leaf_rm_watch_recursive(fs_notify_leaf* root_node, fs_notify_device* device);
bool fs_notify_device_set_gio_handler(fs_notify_device* device, GIOFunc handler);

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC(fs_notify_leaf, fs_notify_leaf_cleanup)

/* File System Notify Utility functions */
bool fs_notify_leaf_read_content(fs_notify_leaf* leaf, int descriptor);
bool fs_notify_leaf_is_watched(const fs_notify_leaf* leaf);

G_END_DECLS

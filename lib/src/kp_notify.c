#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-kernel"

#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <func.h>
#include <array_alg.h>
#include <kp_notify.h>
#include <kp_notify_handler.h>

#define FILE_CONTENT_MAX_LINES 1

/* Kernel Parameter Leaf Functions */

void follow_leaf_link(kp_notify_leaf* node)
{
    if(node->nodetype != SOFTLINK) {
        return;
    }

    g_autoptr(GError) errors = NULL;
    g_autofree gchar* next_name = g_file_read_link(node->filepath, &errors);
    if(errors) {
        g_error("Error reaching symbolic link: %s", errors->message);
    }

    if(g_strcmp0(next_name, node->filepath) != 0) {
        node->filepath = strcpy(next_name, node->filepath);
        node->nodetype = get_node_type(node->filepath);
    }
}

node_type get_node_type(const gchar* path)
{
    GFileTest file_test = G_FILE_TEST_IS_REGULAR;
    GFileTest dir_test = G_FILE_TEST_IS_DIR;
    GFileTest link_test = G_FILE_TEST_IS_SYMLINK;

    if(g_file_test(path, file_test)) {
        return FILE;
    } else if(g_file_test(path, dir_test)) {
        return DIRECTORY;
    } else if(g_file_test(path, link_test)) {
        return SOFTLINK;
    }
    
    return OTHER;
}

node_type get_followed_node_type(const gchar* path, gchar* last_pointed)
{
    GFileTest link_test = G_FILE_TEST_IS_SYMLINK;
    g_autofree gchar* pointed_name = g_strdup(path);
    while(g_file_test(pointed_name, link_test)) {
        g_autofree gchar* filename = pointed_name;
        pointed_name = g_file_read_link(filename, NULL);
    }
    if(last_pointed != NULL) {
        strcpy(last_pointed, pointed_name);
    }
    return get_node_type(pointed_name);
}

node_type get_posix_node_type(const gchar* path)
{
    
}

void kp_notify_leaf_init(kp_notify_leaf* node, const gchar* path, bool follow_symlinks)
{
    struct stat path_stat;

    node_type nodetype = get_node_type(path);
    node->filepath = g_strdup(path);
    node->nodetype = nodetype;

    if(nodetype == SOFTLINK && !follow_symlinks) {
        lstat(path, &path_stat);
    } else if(nodetype == SOFTLINK && follow_symlinks) {
        follow_leaf_link(node);
    }

    int path_descriptor = open(node->filepath, O_RDONLY | O_NONBLOCK);
    fstat(path_descriptor, &path_stat);

    if(nodetype != DIRECTORY) {
        int file_descriptor = open(node->filepath, O_RDONLY);
        g_autoptr(GIOChannel)file_channel = g_io_channel_unix_new(file_descriptor);
        gsize chars_read = 0;
        
        // Kernel Parameter file is expected to have a single line of NT_FILE_LINE_MAX_LENGTH chars at max
        gchar* content = g_new(gchar, KP_FILE_LINE_MAX_LENGTH);
        g_autoptr(GError) file_errors = NULL;


        GIOStatus status = g_io_channel_read_chars(file_channel,
                                                content,
                                                KP_FILE_LINE_MAX_LENGTH,
                                                &chars_read,
                                                &file_errors);
        if(status != G_IO_STATUS_NORMAL || chars_read == 0)
        {
            g_free(content);
            g_error("Error while reading file %s - %s", path, file_errors->message);
            g_error_free(file_errors);
            return;
        }
        node->content = content;
    }


    node->follow_symlinks = follow_symlinks;
    node->active_watch = false;
    node->last_modified = path_stat.st_mtim.tv_nsec;
    node->watch_descriptor = -1;
}

void kp_notify_leaf_set_follow(kp_notify_leaf* node, bool follow_symlinks)
{
    bool previously_followed= node->follow_symlinks;
    node->follow_symlinks = follow_symlinks;
    if(!previously_followed && follow_symlinks)
    {
        follow_leaf_link(node);
    }
}


kp_notify_leaf* kp_notify_leaf_new(const gchar* path, bool follow_symlinks)
{
    kp_notify_leaf* leaf = g_new0(kp_notify_leaf, 1);
    kp_notify_leaf_init(leaf, path, follow_symlinks);
    kp_notify_leaf_set_follow(leaf, follow_symlinks);
    return leaf;
}

GPtrArray* kp_notify_leaf_children(kp_notify_leaf* node)
{
    g_autoptr(GError) errors = NULL;
    GPtrArray* children = g_ptr_array_new();
    if(node->nodetype != DIRECTORY) {
        return NULL;
    }

    GDir* dir = g_dir_open(node->filepath, 0, &errors);
    const gchar* next_child = NULL;
    while((next_child = g_dir_read_name(dir))) {
        g_ptr_array_add(children, kp_notify_leaf_new(next_child, node->follow_symlinks));
    }
    g_dir_close(dir);
    if(children->len == 0) {
        g_ptr_array_free(children, true);
        return NULL;
    }
    return children;
}

GPtrArray* kp_notify_leaf_new_recursive(const gchar* path, bool follow_symlinks)
{
    GPtrArray* all_leafs = g_ptr_array_sized_new(1);
    kp_notify_leaf* root = kp_notify_leaf_new(path, follow_symlinks);
    g_ptr_array_add(all_leafs, root);

    if(root->nodetype == DIRECTORY) {
        g_autoptr(GPtrArray) root_children = kp_notify_leaf_children(root);

        for(size_t i = 1; i<root_children->len; i++) {
            kp_notify_leaf* child = g_ptr_array_index(root_children, i);

            if(child->nodetype != DIRECTORY) {
                g_ptr_array_add(all_leafs, 
                                g_ptr_array_steal_index_fast(root_children, i));

            } else {
                g_ptr_array_extend_and_steal(all_leafs, kp_notify_leaf_new_recursive(child->filepath, follow_symlinks));
            }
        }
    }
    return all_leafs;
}

void kp_notify_leaf_free(kp_notify_leaf* node)
{
    if(node->active_watch) {
        // TODO: Remove all watch descriptors from the associated device
    }
    g_free_sized(node->content, KP_FILE_LINE_MAX_LENGTH);
    g_free(node->filepath);
}

void kp_notify_leaf_cleanup(gpointer obj)
{
    kp_notify_leaf_free((kp_notify_leaf*)obj);
}

/* Kernel Paramter Device functions */

void kp_notify_device_init(kp_notify_device* device)
{
    device->device_channel = g_io_channel_unix_new(inotify_init());
    device->watchers = g_list_alloc();
    device->gio_watch_callbacks = g_hash_table_new_full(g_int64_hash, g_int64_equal, g_free, nothing);
}

void kp_notify_device_free(kp_notify_device* device)
{
    GList* watcher = g_list_next(device->watchers);
    while(watcher != NULL) {
        inotify_rm_watch(g_io_channel_unix_get_fd(device->device_channel), *(int*)watcher->data);
        gpointer ptr_free = watcher;
        watcher = g_list_next(ptr_free);
        g_list_free(ptr_free);
    }
    g_hash_table_destroy(device->gio_watch_callbacks);
    g_io_channel_shutdown(device->device_channel, false, NULL);
}

bool kp_notify_device_add_watch(kp_notify_device* device, kp_notify_leaf* leaf, uint32_t mask)
{
    if(g_list_length(device->watchers) == KP_MAX_WATCHERS) {
        g_warning("Cannot add watchers, watch list is full (of size %d)", KP_MAX_WATCHERS);
        return false;
    }

    int fd_device = g_io_channel_unix_get_fd(device->device_channel);
    int wd = inotify_add_watch(fd_device, leaf->filepath, mask);
    leaf->watch_descriptor = wd;
    device->watchers = g_list_prepend(device->watchers, &leaf->watch_descriptor);

    return true;
}



bool kp_notify_device_add_watch_recursive(kp_notify_device* device, GPtrArray* all_leafs, kp_notify_leaf* root_leaf, uint32_t bitmask)
{
    GPtrArray* sub_of_root = g_ptr_array_new();
    array_collection_res result = { sub_of_root, root_leaf };
    g_ptr_array_foreach(all_leafs, find_all_sub_nodes, &result);

    if(sub_of_root->len == 0) {
        g_warning("No match of subnodes for path: %s", root_leaf->filepath);
        return false;
    }

    for(guint s = 0; s < sub_of_root->len; ++s) {
        kp_notify_device_add_watch(device, g_ptr_array_index(sub_of_root, s), bitmask);
    }
    return true;
}

bool kp_notify_device_rm_watch(kp_notify_device* device, kp_notify_leaf* leaf)
{
    if(g_list_length(device->watchers) == 0) {
        g_warning("Cannot remove watchers, watch list is empty");
        return false;
    }

    int fd_device = g_io_channel_unix_get_fd(device->device_channel);
    device->watchers = g_list_remove(device->watchers, &leaf->watch_descriptor);
    inotify_rm_watch(fd_device, leaf->watch_descriptor);

    return true;
}

bool kp_notify_device_rm_watch_recursive(kp_notify_device* device, GPtrArray* all_leafs, kp_notify_leaf* root_leaf)
{
    GPtrArray* sub_of_root = g_ptr_array_new();
    array_collection_res result = { sub_of_root, root_leaf };
    g_ptr_array_foreach(all_leafs, find_all_sub_nodes, &result);

    if(sub_of_root->len == 0) {
        g_warning("No match of subnodes for path: %s", root_leaf->filepath);
        return false;
    }

    for(guint s = 0; s < sub_of_root->len; ++s) {
        kp_notify_device_rm_watch(device, g_ptr_array_index(sub_of_root, s));
    }
    return true;
}


bool kp_notify_device_add_callback(kp_notify_device* device, INotifyCallback callback, uint32_t bitmask)
{
    if(g_hash_table_contains(device->gio_watch_callbacks, &bitmask)) {
        g_error("Error: Cannot have multiple subscribers to a single inotify bitmask");
        return false;
    } else {
        g_hash_table_insert(device->gio_watch_callbacks, &bitmask, callback);
        return true;
    }
}

bool kp_notify_device_rm_callback(kp_notify_device* device, uint32_t bitmask)
{
    return g_hash_table_remove(device->gio_watch_callbacks, &bitmask);
}

bool kp_notify_device_set_gio_handler(kp_notify_device* device, GIOFunc handler)
{
    return g_io_add_watch(device->device_channel, G_IO_IN, handler, device);
}

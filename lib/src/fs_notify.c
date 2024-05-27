#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-kernel"

#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <func.h>
#include <array_alg.h>
#include <fs_notify.h>
#include <fs_notify_handler.h>

#define FILE_CONTENT_MAX_LINES 1

FSNotifyCallback kp_notify_leaf_destroyed_cb = on_fs_notify_leaf_destoryed_default;

void fs_notify_leaf_destroyed_generic(gpointer data) {
    int fd = *(int*)data;
    kp_notify_leaf_destroyed_cb(fd);
}

/* File System Notify Leaf Functions */

void follow_leaf_link(fs_notify_leaf* node)
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
        struct stat path_stat; 
        stat(node->filepath, &path_stat);
        node->nodetype = get_node_type(&path_stat);
    }
}

node_type get_node_type(const struct stat* stat_entry)
{
    char stat_text[64];
    char rule[64];
    node_type nodetype = OTHER;

    switch (stat_entry->st_mode & S_IFMT) {
        case S_IFLNK:  
            strcpy(stat_text, "symlink");
            strcpy(rule, "classifying as SOFTLINK");
            nodetype = SOFTLINK;                
            break;
        case S_IFREG:  
            strcpy(stat_text, "regular file");
            strcpy(rule, "classifying as FILE");
            nodetype = REGULAR_FILE;
            break;
        case S_IFDIR:
            strcpy(stat_text, "directory");
            strcpy(rule, "classifying as DIRECTORY");
            nodetype = DIRECTORY;
            break;
        case S_IFBLK:  
            strcpy(stat_text, "block device");
            strcpy(rule, "ignoring");
            break;
        case S_IFCHR:  
            strcpy(stat_text, "character device");
            strcpy(rule, "ignoring");    
            break;
        case S_IFIFO:  
            strcpy(stat_text, "FIFO/pipe");
            strcpy(rule, "ignoring");            
            break;
        case S_IFSOCK: 
            strcpy(stat_text, "socket");
            strcpy(rule, "ignoring");
            break;
        default:       
            sprintf(stat_text, "unknown stat %u", stat_entry->st_mode & S_IFMT); 
            strcpy(rule, "classifying as OTHER");
            nodetype = OTHER;               
            break;
    }

    g_debug("Encountered file system entry of type '%s'. Resulting action: %s", stat_text, rule);
    return nodetype;
}

bool fs_notify_leaf_read_content(fs_notify_leaf* node, int descriptor)
{
    g_autoptr(GIOChannel)file_channel = g_io_channel_unix_new(descriptor);
    gsize chars_read = 0;
    
    // Kernel Parameter file is expected to have a single line of NT_FILE_LINE_MAX_LENGTH chars at max
    gchar* content = g_new(gchar, FILE_CONTENT_MAX_LINES * FS_FILE_LINE_MAX_LENGTH);
    g_autoptr(GError) file_errors = NULL;


    GIOStatus status = g_io_channel_read_chars(file_channel,
                                            content,
                                            FS_FILE_LINE_MAX_LENGTH,
                                            &chars_read,
                                            &file_errors);
    if(status != G_IO_STATUS_NORMAL || chars_read == 0)
    {
        g_free(content);
        g_error("Error while reading file %s - %s", node->filepath, file_errors->message);
        g_error_free(file_errors);
        return false;
    }
    g_strstrip(content);
    node->content = content;
    return true;
}

__attribute__((__pure__))
bool fs_notify_leaf_is_watched(const fs_notify_leaf* leaf)
{
    return leaf->watch_descriptor != -1;
}

void fs_notify_leaf_init(fs_notify_leaf* node, const gchar* path, bool follow_symlinks)
{
    node->filepath = g_strdup(path);
    node->follow_symlinks = follow_symlinks;
    node->watch_descriptor = -1;

    struct stat path_stat;
    int stat_success;
    int descriptor = open(node->filepath, O_RDONLY);

    if(descriptor == -1) {
        g_warning("Cannot open file %s. Returned with errno: '%d', and message '%s'", path, errno, strerror(errno));
    }

    // We assume, if 'open' succeeded, stat should also succeed
    if(follow_symlinks) {
        stat_success = fstat(descriptor, &path_stat);
    } else {
        stat_success = lstat(path, &path_stat);
    }

    node->last_modified = path_stat.st_mtime;

    node_type nodetype = get_node_type(&path_stat);
    node->nodetype = nodetype;

    if(nodetype == SOFTLINK && follow_symlinks) {
        follow_leaf_link(node);
    }

    if(nodetype == REGULAR_FILE || nodetype == SOFTLINK) {
        fs_notify_leaf_read_content(node, descriptor);
    }
    close(descriptor);

}

void fs_notify_leaf_set_follow(fs_notify_leaf* node, bool follow_symlinks)
{
    bool previously_followed= node->follow_symlinks;
    node->follow_symlinks = follow_symlinks;
    if(!previously_followed && follow_symlinks)
    {
        follow_leaf_link(node);
    }
}


fs_notify_leaf* fs_notify_leaf_new(const gchar* path, bool follow_symlinks)
{
    fs_notify_leaf* leaf = g_new0(fs_notify_leaf, 1);
    fs_notify_leaf_init(leaf, path, follow_symlinks);
    fs_notify_leaf_set_follow(leaf, follow_symlinks);
    return leaf;
}

GPtrArray* fs_notify_leaf_children(fs_notify_leaf* node)
{
    g_autoptr(GError) errors = NULL;
    GPtrArray* children = g_ptr_array_new();
    if(node->nodetype != DIRECTORY) {
        return NULL;
    }

    GDir* dir = g_dir_open(node->filepath, 0, &errors);
    const gchar* next_child = NULL;
    while((next_child = g_dir_read_name(dir))) {
        g_auto(GPathBuf) child_path;
        g_path_buf_init_from_path(&child_path, node->filepath);
        g_path_buf_push(&child_path, next_child);
        g_autofree gchar* child_full_name = g_path_buf_to_path(&child_path);

        g_ptr_array_add(children, fs_notify_leaf_new(child_full_name, node->follow_symlinks));
    }
    g_dir_close(dir);
    if(children->len == 0) {
        g_ptr_array_free(children, true);
        return NULL;
    }
    return children;
}

GPtrArray* fs_notify_leaf_children_recursive(fs_notify_leaf* node)
{
    GPtrArray* results = g_ptr_array_new();
    g_ptr_array_add(results, node);

    if(node->nodetype != DIRECTORY) {
        return results;
    }

    g_autoptr(GHashTable) resolved = g_hash_table_new(g_str_hash, g_str_equal);
    _fs_notify_leaf_children_r_impl(results, resolved);
    return results;
}

void _fs_notify_leaf_children_r_impl(GPtrArray* results, GHashTable* resolved)
{
    size_t target_index = 0;
    fs_notify_leaf* resolution_target = g_ptr_array_index(results, target_index);

    while(resolution_target != NULL) {
        fs_notify_leaf* current_target = resolution_target;
        g_ptr_array_extend_and_steal(results, fs_notify_leaf_children(current_target));
        g_hash_table_insert(resolved, resolution_target->filepath, current_target);

        for(gsize i = target_index; i < results->len; i++) {
            fs_notify_leaf* next_target = (fs_notify_leaf*)g_ptr_array_index(results, i);
            if(next_target->nodetype == DIRECTORY && !g_hash_table_contains(resolved, next_target)) {
                resolution_target = next_target;
                target_index = i;
                break;
            }
        }

        if(resolution_target == current_target) {
            resolution_target = NULL;
        }
    }
}


GPtrArray* fs_notify_leaf_new_recursive(const gchar* path, bool follow_symlinks)
{
    fs_notify_leaf* root = fs_notify_leaf_new(path, follow_symlinks);
    return fs_notify_leaf_children_recursive(root);
}

void fs_notify_leaf_free(fs_notify_leaf* node)
{
    if(fs_notify_leaf_is_watched(node)) {
        // TODO: Remove all watch descriptors from the associated leaf
    }
    g_free_sized(node->content, FILE_CONTENT_MAX_LINES * FS_FILE_LINE_MAX_LENGTH);
    g_free(node->filepath);
}

void fs_notify_leaf_cleanup(gpointer obj)
{
    fs_notify_leaf_free((fs_notify_leaf*)obj);
}

/* File System Notify Device functions */

void fs_notify_device_init(fs_notify_device* device)
{
    int notification_descriptor = inotify_init1(IN_NONBLOCK);

    if(notification_descriptor < 0) {
        perror("Could not initialize inotify device");
        return;
    }

    device->device_channel = g_io_channel_unix_new(notification_descriptor);
    g_return_if_fail(g_io_channel_set_flags(device->device_channel, G_IO_FLAG_NONBLOCK, NULL));
    device->notify_watches = g_hash_table_new_full(g_int64_hash, g_int64_equal, fs_notify_leaf_destroyed_generic, nothing);
    device->gio_watch_callbacks = g_hash_table_new_full(g_int64_hash, g_int64_equal, g_free, nothing);
}

void fs_notify_device_free(fs_notify_device* device)
{
    g_autoptr(GError) gio_err = NULL;
    g_hash_table_destroy(device->notify_watches);
    g_hash_table_destroy(device->gio_watch_callbacks);
    GIOStatus status = g_io_channel_shutdown(device->device_channel, false, &gio_err);
    
    if(status == G_IO_STATUS_ERROR) {
        g_warning("Error when shutting down GIOChannel: %s", gio_err->message);
    }

    g_free_sized(device, sizeof(fs_notify_device));
}

bool fs_notify_device_add_watch(fs_notify_device* device, fs_notify_leaf* leaf, uint32_t mask)
{
    
    if(g_hash_table_size(device->notify_watches) == FS_MAX_WATCHERS) {
        g_warning("Cannot add watchers, watch list is full (of size %d)", FS_MAX_WATCHERS);
        return false;
    }

    int fd_device = g_io_channel_unix_get_fd(device->device_channel);
    int wd = inotify_add_watch(fd_device, leaf->filepath, mask);

    if(wd < 0) {
        perror("inotify_add_watch cannot complete");
        return false;
    }

    leaf->watch_descriptor = wd;
    g_hash_table_insert(device->notify_watches, &leaf->watch_descriptor, leaf);

    return true;
}

bool fs_notify_device_add_watch_recursive(fs_notify_device* device, GPtrArray* all_leafs, fs_notify_leaf* root_leaf, uint32_t bitmask)
{
    GPtrArray* sub_of_root = g_ptr_array_new();
    array_collection_res result = { sub_of_root, root_leaf };
    g_ptr_array_foreach(all_leafs, find_all_sub_nodes, &result);

    if(sub_of_root->len == 0) {
        g_warning("No match of subnodes for path: %s", root_leaf->filepath);
        return false;
    }

    for(guint s = 0; s < sub_of_root->len; ++s) {
        fs_notify_leaf* watch_target = (fs_notify_leaf*)g_ptr_array_index(sub_of_root, s);
        bool device_added = fs_notify_device_add_watch(device, watch_target, bitmask);

        if(!device_added) {
            g_warning("Failed to add kp_notify leaf: %s", watch_target->filepath);
            return false;
        }
    }
    return true;
}

bool fs_notify_device_rm_watch(fs_notify_device* device, fs_notify_leaf* leaf)
{
    if(g_hash_table_size(device->notify_watches) == 0) {
        g_warning("Cannot remove watchers, watch list is empty");
        return false;
    }

    int fd_device = g_io_channel_unix_get_fd(device->device_channel);
    int wd_desc = leaf->watch_descriptor;

    g_hash_table_remove(device->notify_watches, &wd_desc);

    if(wd_desc < 0) {
        g_debug("Skip removing with inotify_rm_watch, descriptor is invalid");
        return true;
    }

    inotify_rm_watch(fd_device, wd_desc);
    leaf->watch_descriptor = -1;

    return true;
}

bool fs_notify_device_rm_watch_recursive(fs_notify_device* device, GPtrArray* all_leafs, fs_notify_leaf* root_leaf)
{
    GPtrArray* sub_of_root = g_ptr_array_new();
    array_collection_res result = { sub_of_root, root_leaf };
    g_ptr_array_foreach(all_leafs, find_all_sub_nodes, &result);

    if(sub_of_root->len == 0) {
        g_warning("No match of subnodes for path: %s", root_leaf->filepath);
        return false;
    }

    for(guint s = 0; s < sub_of_root->len; ++s) {
        fs_notify_leaf* watch_target = (fs_notify_leaf*)g_ptr_array_index(sub_of_root, s);
        
        if(!fs_notify_device_rm_watch(device, watch_target)) {
            g_warning("Failure when removing %s from device", watch_target->filepath);
            return false;
        }
    }
    return true;
}


bool fs_notify_device_add_callback(fs_notify_device* device, INotifyCallback callback, uint32_t bitmask)
{
    if(g_hash_table_contains(device->gio_watch_callbacks, &bitmask)) {
        g_warning("Cannot have multiple subscribers to a single inotify bitmask");
        return false;
    }

    g_hash_table_insert(device->gio_watch_callbacks, &bitmask, callback);
    return true;
}

bool fs_notify_device_rm_callback(fs_notify_device* device, uint32_t bitmask)
{
    if(!g_hash_table_contains(device->gio_watch_callbacks, &bitmask))
    {
        g_warning("No GIO callback found for bitmask: %u", bitmask);
        return false;
    }
    return g_hash_table_remove(device->gio_watch_callbacks, &bitmask);
}

bool fs_notify_device_set_gio_handler(fs_notify_device* device, GIOFunc handler)
{
    return g_io_add_watch(device->device_channel, G_IO_IN, handler, device);
}

void fs_notify_device_clenaup(gpointer obj)
{
    fs_notify_device* device = (fs_notify_device*)obj;
    fs_notify_device_free(device);
}


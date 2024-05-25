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
#include <kp_notify.h>
#include <kp_notify_handler.h>

#define FILE_CONTENT_MAX_LINES 1

KPNotifyCallback kp_notify_leaf_destroyed_cb = on_kp_notify_leaf_destoryed_default;

void kp_notify_leaf_destroyed_generic(gpointer data) {
    int fd = *(int*)data;
    kp_notify_leaf_destroyed_cb(fd);
}

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

    g_info("Encountered file system entry of type '%s'. Resulting action: %s", stat_text, rule);
    return nodetype;
}

bool kp_notify_leaf_read_content(kp_notify_leaf* node, int descriptor)
{
    g_autoptr(GIOChannel)file_channel = g_io_channel_unix_new(descriptor);
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
        g_error("Error while reading file %s - %s", node->filepath, file_errors->message);
        g_error_free(file_errors);
        return false;
    }
    g_strstrip(content);
    node->content = content;
    return true;
}

bool kp_notify_leaf_is_watched(const kp_notify_leaf* leaf)
{
    return leaf->watch_descriptor != -1;
}

void kp_notify_leaf_init(kp_notify_leaf* node, const gchar* path, bool follow_symlinks)
{
    node->filepath = g_strdup(path);
    node->follow_symlinks = follow_symlinks;
    node->watch_descriptor = -1;

    if(!access(path, F_OK | R_OK | X_OK)) {
        g_warning("Path %s does not exist, or openheart has no READ and EXECUTE rights. Returning.");
        return;
    }

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
        kp_notify_leaf_read_content(node, descriptor);
    }
    close(descriptor);

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

        // Do not iterate first element, as it is already present as "root"
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
    if(kp_notify_leaf_is_watched(node)) {
        // TODO: Remove all watch descriptors from the associated leaf
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
    device->notify_watches = g_hash_table_new_full(g_int64_hash, g_int64_equal, kp_notify_leaf_destroyed_generic, nothing);
    device->gio_watch_callbacks = g_hash_table_new_full(g_int64_hash, g_int64_equal, g_free, nothing);
}

void kp_notify_device_free(kp_notify_device* device)
{
    g_hash_table_destroy(device->notify_watches);
    g_hash_table_destroy(device->gio_watch_callbacks);
    g_io_channel_shutdown(device->device_channel, false, NULL);
}

bool kp_notify_device_add_watch(kp_notify_device* device, kp_notify_leaf* leaf, uint32_t mask)
{
    
    if(g_hash_table_size(device->notify_watches) == KP_MAX_WATCHERS) {
        g_warning("Cannot add watchers, watch list is full (of size %d)", KP_MAX_WATCHERS);
        return false;
    }

    int fd_device = g_io_channel_unix_get_fd(device->device_channel);
    int wd = inotify_add_watch(fd_device, leaf->filepath, mask);
    leaf->watch_descriptor = wd;
    g_hash_table_insert(device->notify_watches, &leaf->watch_descriptor, leaf);

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
    if(g_hash_table_size(device->notify_watches) == 0) {
        g_warning("Cannot remove watchers, watch list is empty");
        return false;
    }

    int fd_device = g_io_channel_unix_get_fd(device->device_channel);
    int wd_desc = leaf->watch_descriptor;
    g_hash_table_remove(device->notify_watches, &wd_desc);
    inotify_rm_watch(fd_device, wd_desc);
    leaf->watch_descriptor = -1;

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
        g_warning("Cannot have multiple subscribers to a single inotify bitmask");
        return false;
    }

    g_hash_table_insert(device->gio_watch_callbacks, &bitmask, callback);
    return true;
}

bool kp_notify_device_rm_callback(kp_notify_device* device, uint32_t bitmask)
{
    if(!g_hash_table_contains(device->gio_watch_callbacks, &bitmask))
    {
        g_warning("No GIO callback found for bitmask: %u", bitmask);
        return false;
    }
    return g_hash_table_remove(device->gio_watch_callbacks, &bitmask);
}

bool kp_notify_device_set_gio_handler(kp_notify_device* device, GIOFunc handler)
{
    return g_io_add_watch(device->device_channel, G_IO_IN, handler, device);
}


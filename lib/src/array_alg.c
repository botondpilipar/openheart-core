#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-misc"

#include <array_alg.h>

bool is_sub_of(gchar* parent, gchar* path_tested) {
    g_autofree gchar* test_substr = g_utf8_substring(path_tested, 0, strlen(parent));
    return g_strcmp0(test_substr, parent) == 0;
}

void find_all_sub_nodes(gpointer ptr_array_elem, gpointer result)
{
    array_collection_res* res = (array_collection_res*)result;
    kp_notify_leaf* target = (kp_notify_leaf*)res->alg_target;
    kp_notify_leaf* tested = (kp_notify_leaf*)ptr_array_elem;
    if(is_sub_of(target->filepath, tested->filepath)) {
        GPtrArray* output = (GPtrArray*)res->result_array;
        g_ptr_array_add(output, ptr_array_elem);
    }
}

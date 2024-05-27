#pragma once
#include <stdbool.h>
#include <fs_types.h>
#include <glib.h>


typedef struct array_collection_res
{
    gpointer result_array;
    gpointer alg_target;
} array_collection_res;

bool is_sub_of(gchar* parent, gchar* path_tested);

void find_all_sub_nodes(gpointer ptr_array_elem, gpointer result);

#pragma once
#include <glib.h>

typedef struct g_lib_parse_res_t
{
    gchar* input_str;
    GDate* result_date;
} g_lib_parse_res_t;

void perform_set_parse_test(gchar* str);
g_lib_parse_res_t make_set_parse_test(gchar* str);
void print_set_parse_test(g_lib_parse_res_t* result);
void free_g_lib_parse_res(g_lib_parse_res_t* result);

#include "g_lib_exp.h"
#include <stdbool.h>

g_lib_parse_res_t make_set_parse_test(gchar* str)
{
    GDate* date  = g_new(GDate, 1);
    g_date_set_parse(date, str);
    g_lib_parse_res_t r = { .input_str = str, .result_date = date};
    return r;
}

void print_set_parse_test(g_lib_parse_res_t* result)
{
    const bool test_success = result->result_date != NULL &&
                                (result->result_date->day || result->result_date->month || result->result_date->year);
    const char* test_indicator = test_success ? "SUCCESS" : "FAILURE";
    g_print("%s #### g_lib_set_parse(%s) = {year: %u, month: %u, day: %u} ####\n",
            test_indicator,
            result->input_str, 
            result->result_date->year, 
            result->result_date->month,
            result->result_date->day);
}

void free_g_lib_parse_res(g_lib_parse_res_t* result)
{
    g_date_free(result->result_date);
}

void perform_set_parse_test(gchar* str)
{
    g_lib_parse_res_t result = make_set_parse_test(str);
    print_set_parse_test(&result);
    free_g_lib_parse_res(&result);
}


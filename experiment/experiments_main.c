#include <stdio.h>
#include "g_lib_exp.h"
#include "kernel_exp.h"

int main(int argc, char** argv) {
    printf("Welcome to OpenHeart Core Experimental executable!\n");
    g_print("\n");

    perform_set_parse_test("2024.05.04");
    perform_set_parse_test("2001 May 04");
    perform_set_parse_test("Wed Apr 17 19:21:08 UTC 2024");

    print_uname();

    return 0;
}

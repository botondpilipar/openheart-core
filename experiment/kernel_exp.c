#include "kernel_exp.h"
#include <kernel_attributes.h>
#include <stdio.h>

void print_uname(void)
{
    kernel_attributes attr;
    init_kernel_attributes(&attr, true);
    g_autoptr(GString) buf = g_string_new("");
    print_kernel_attributes(&attr, buf);

    puts(buf->str);
}

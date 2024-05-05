#include <glib.h>
#include <stdio.h>
#include <curl/curl.h>
#include <kernel_attributes.h>

int main(int argc, char** argv)
{
    KernelAttributes attr;
    init_kernel_attributes(&attr, true);
    g_autoptr(GString) output = g_string_new(NULL);

    print_kernel_attributes(&attr, output);
    fputs(output->str, stdout);
    return 0;
}

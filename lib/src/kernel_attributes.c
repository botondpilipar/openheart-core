#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-kernel"

#include <kernel_attributes.h>
#include <stdio.h>
#include <glib.h>

void
init_kernel_attributes(kernel_attributes* attr,
                       bool /*Not used*/)
{
    int res = uname(attr);
    if(!res) {
        perror("uname query failed");
    }
}

kernel_attributes*
new_kernel_attributes(bool /* not yet used */)
{
    kernel_attributes* attr =  g_new0(kernel_attributes, 1);
    init_kernel_attributes(attr, true);
    return attr;
}

void 
cleanup_kernel_attributes(kernel_attributes* attr)
{
    g_free_sized(attr, sizeof(kernel_attributes));
}

void
print_kernel_attributes(kernel_attributes* attr, GString* buffer)
{
    g_string_printf(buffer, kernel_attr_fmt_str, attr->machine, attr->nodename, attr->release, attr->sysname, attr->version);
}
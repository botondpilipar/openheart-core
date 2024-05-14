#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-kernel"

#include <kernel_attributes.h>
#include <glib.h>

void
init_kernel_attributes(KernelAttributes* attr,
                       bool /*Not used*/)
{
    uname(attr);
}

KernelAttributes*
new_kernel_attributes(bool /* not yet used */)
{
    return g_new0(KernelAttributes, 1);
}


void 
cleanup_kernel_attributes(KernelAttributes* attr)
{
    g_free_sized(attr, sizeof(KernelAttributes));
}

void
print_kernel_attributes(KernelAttributes* attr, GString* buffer)
{
    g_string_printf(buffer, kernel_attr_fmt_str, attr->machine, attr->nodename, attr->release, attr->sysname, attr->version);
}
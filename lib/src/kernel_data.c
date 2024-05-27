#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-kernel"

#include <kernel_data.h>

void kernel_data_init(kernel_data* data)
{
    return;
}

kernel_data* kernel_data_new()
{
    return NULL;
}

bool kernel_data_parse(kernel_attributes* attr, system_info* info)
{
    return false;
}

void kernel_data_free(kernel_data* data)
{
    return;
}

#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-kernel"

#include <kernel_data.h>

void kernel_data_init(KernelData* data)
{
    return;
}

KernelData* kernel_data_new()
{
    return NULL;
}

bool kernel_data_parse(KernelAttributes* attr, SystemInfo* info)
{
    return false;
}

void kernel_data_free(KernelData* data)
{
    return;
}

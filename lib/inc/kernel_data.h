#include <glib.h>
#include <stdint.h>
#include <sys/sysinfo.h>

#include "kernel_attributes.h"

G_BEGIN_DECLS

#define KD_ARCH_SIZE 16

static char example[] = "Linux version 6.8.7-300.fc40.x86_64 (mockbuild@fcbf90a6ba6d49db950d41ba55dc124b) (gcc (GCC) 14.0.1 20240411 (Red Hat 14.0.1-0), GNU ld version 2.41-34.fc40) #1 SMP PREEMPT_DYNAMIC Wed Apr 17 19:21:08 UTC 2024";
static gchar kernel_version[] = "proc/version";

typedef struct release_version
{
    uint32_t major;
    uint32_t minor;
    uint32_t micro;
    uint32_t patch;
} release_version;

release_version* release_version_new(void);
bool release_version_parse(kernel_attributes* attr);


typedef struct kernel_data
{
    GTimeSpan uptime;
    bool is_active;
    GDateTime* build_date;
    uint16_t build_number;
    release_version release;

} kernel_data;

void kernel_data_init(kernel_data* data);
kernel_data* kernel_data_new();
bool kernel_data_parse(kernel_attributes* attr, system_info* info);
void kernel_data_free(kernel_data* data);

typedef enum kernel_features
{
    SYMMETRIC_MULTI_PROCESSING = (1 << 0),
    PREEMPTIBLE = (1 << 1),
    REALTIME = (1 << 2),
    GENERIC = (1 << 3),
    KERNEL_FEATURES_MAX = (1 << 4)
} kernel_features;

G_END_DECLS


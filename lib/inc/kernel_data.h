#include <glib.h>
#include <stdint.h>
#include <sys/sysinfo.h>

#include "kernel_attributes.h"

G_BEGIN_DECLS

#define KERNEL_DATA_ARCH_SIZE 16

static char example[] = "Linux version 6.8.7-300.fc40.x86_64 (mockbuild@fcbf90a6ba6d49db950d41ba55dc124b) (gcc (GCC) 14.0.1 20240411 (Red Hat 14.0.1-0), GNU ld version 2.41-34.fc40) #1 SMP PREEMPT_DYNAMIC Wed Apr 17 19:21:08 UTC 2024";
static gchar kernel_version[] = "proc/version";

typedef struct ReleaseVersion
{
    uint32_t major;
    uint32_t minor;
    uint32_t micro;
    uint32_t patch;
} ReleaseVersion;

ReleaseVersion* release_version_new(void);
bool release_version_parse(KernelAttributes* attr);


typedef struct KernelData
{
    GTimeSpan uptime;
    bool is_active;
    GDateTime* build_date;
    uint16_t build_number;
    ReleaseVersion release;

} KernelData;

void kernel_data_init(KernelData* data);
KernelData* kernel_data_new(void);
bool kernel_data_parse(KernelAttributes* attr, SystemInfo* info)

typedef enum KernelFeatures
{
    SYMMETRIC_MULTI_PROCESSING = (1 << 0),
    PREEMPTIBLE = (1 << 1),
    REALTIME = (1 << 2),
    GENERIC = (1 << 3),
    KERNEL_FEATURES_MAX
} KernelFeatures;

G_END_DECLS


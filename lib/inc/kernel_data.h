#include <glib.h>
#include <stdint.h>
#include <sys/sysinfo.h>

#include "kernel_attributes.h"

G_BEGIN_DECLS

#define KD_ARCH_SIZE 16
#define KD_VERSION_BUF 512

static char example[] = "Linux version 6.8.7-300.fc40.x86_64 (mockbuild@fcbf90a6ba6d49db950d41ba55dc124b) (gcc (GCC) 14.0.1 20240411 (Red Hat 14.0.1-0), GNU ld version 2.41-34.fc40) #1 SMP PREEMPT_DYNAMIC Wed Apr 17 19:21:08 UTC 2024";

typedef struct release_version
{
    uint32_t major;
    uint32_t minor;
    uint32_t micro;
    uint32_t patch;
} release_version;

release_version* release_version_new(void);
bool release_version_parse(release_version* version, kernel_attributes* attr);

typedef struct date_v_fmt_table {
    GHashTable* str_to_gday;
    GHashTable* str_to_gmonth;
} date_v_fmt_table;

typedef enum kernel_features
{
    SMP = (1 << 0), // Symmetric Multiprocessing
    PREEMPT_DYNAMIC = (1 << 1), // Dynamic Preemption
    PREEMPT_RT = (1 << 2), // Preemption RealTime
    VIRT = (1 << 3), // Hardware Virtualization
    PAE = (1 << 4), // Physical Address Extension
    AGP = (1 << 5), // Accelerated Graphics Port
    HZ = (1 << 6), // indicated timer ticks
    EFI = (1 << 7), // Extensible Firmware Interface
    CPU_3DNOW = (1 << 8), // 3DNOW AMD Instruction Set
    CPU_3DNOWPLUS = (1 << 9), // 3DNOWPLUS AMD Instruction Set
    MEM_NUMA = (1 << 11), // Non-Uniform Memory Access
    MEM_SLAVE_HELPER = (1 << 12), // Advanced Memory Management
    KERNEL_FEATURES_MAX = (1 << 13)
} kernel_features;

typedef struct kernel_data
{
    bool is_active;
    GDateTime* build_date;
    uint16_t build_number;
    uint32_t features;
    system_info* sys_info;
    kernel_attributes* attributes;
    release_version release;

} kernel_data;

void kernel_data_init(kernel_data* data);

kernel_data* kernel_data_new(void);
void kernel_data_free(kernel_data* data);

/* Kernel Data helpers */

/* Natives are functions, that has a native C binding.
 * For now, this is 'sysinfo' and 'uname' C structures returned by sysinfo() and uname()
 */
bool kd_init_natives(kernel_data* data);
bool kd_parse_from_proc_version(kernel_data* data, gchar* buffer);

/* kd_parse_from_proc_version helpers */
bool kd_parse_dates(kernel_data* data, gchar* buf);
bool kd_parse_features(kernel_data* data, gchar* buf);

G_END_DECLS


#pragma once
#include <stdbool.h>
#include <sys/utsname.h>
#include <glib.h>

/*
* KernelAttributes is meant to be the barebone of kernel
* query methods. By default, this is just what "uname" syscall
* returns. There exists more advanced structures in this
* repository.
*/

G_BEGIN_DECLS
/**
 * @brief Basic UNIX-supplied struct for kernel information
 * @note KernelAttributes is fully stack allocated
 */
typedef struct utsname KernelAttributes;
typedef struct sysinfo SystemInfo;


static gchar kernel_attr_fmt_str[] = 
    "{\n\tmachine: %s\n\tnodename: %s\n\trelease: %s\n\t"
    "sysname: %s\n\tversion: %s\n}\n";

void init_kernel_attributes(KernelAttributes* attr, bool active);
KernelAttributes* new_kernel_attributes(bool active);
void cleanup_kernel_attributes(KernelAttributes* attributes);
void print_kernel_attributes(KernelAttributes* attributes, GString* buffer);

void init_system_info(SystemInfo* info);
SystemInfo* new_system_info(void);

G_END_DECLS

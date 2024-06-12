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
typedef struct utsname kernel_attributes;
typedef struct sysinfo system_info;


void init_kernel_attributes(kernel_attributes* attr, bool active);
kernel_attributes* new_kernel_attributes(bool active);
void cleanup_kernel_attributes(kernel_attributes* attributes);
void print_kernel_attributes(kernel_attributes* attributes, GString* buffer);

void init_system_info(system_info* info);
system_info* new_system_info(void);

G_END_DECLS

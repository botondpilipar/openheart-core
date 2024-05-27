#pragma once
#include <unistd.h>
#include <sys/capability.h>
#include <linux/capability.h>
#include <stdbool.h>

#define USER_MAX_NAME 64
#define LINUX_CAPABILITY_TYPES 32

typedef struct proc_cap
{
    cap_t cap_user;
    int cap_present[LINUX_CAPABILITY_TYPES];
} proc_cap;

void cap_init_this_proc(proc_cap* cap_obj);
proc_cap* cap_new_this_proc(void);
void proc_cap_free(proc_cap*);

/* Process Capability query methods */
bool can_chown(const proc_cap* cap_obj);
bool can_uni_socket_bind(const proc_cap* cap_obj);
bool can_set_uid(const proc_cap* cap_obj);

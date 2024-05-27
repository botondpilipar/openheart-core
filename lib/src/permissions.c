#include <permissions.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <linux/capability.h>
#include <sys/capability.h>

void proc_cap_free(proc_cap* cap_obj)
{
    cap_free(cap_obj->cap_user);
}

void cap_init_this_proc(proc_cap* cap_obj)
{
    cap_obj->cap_user = cap_get_proc();
    cap_mode_t mode = cap_get_mode();
}

proc_cap* cap_new_this_proc()
{
    proc_cap* cap = calloc(sizeof(proc_cap), 1);
    cap_init_this_proc(cap);
    return cap;
}

/* Process Capability query methods */

bool can_chown(const proc_cap* cap_obj)
{
    cap_value_t value;
    return false;
}
bool can_uni_socket_bind(const proc_cap* cap_obj)
{
    return false;
}
bool can_set_uid(const proc_cap* cap_obj)
{
    return false;
}


#include <stdlib.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_SYSCTL_H
#include <sys/sysctl.h>
#endif

#if HAVE_SYSINFO && HAVE_SYS_SYSTEMINFO_H
# include <sys/systeminfo.h>
#endif

#include <getopt.h>
#include <stdbool.h>
#include <glib.h>
#include <curl/curl.h>
#include <json.h>
#include <log.h>
#include <stdint.h>


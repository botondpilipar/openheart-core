#include <glib.h>

typedef struct KernelAttr {
    GString* abspath;
    GString* version;
    gboolean realtime;

} KernelAttr;

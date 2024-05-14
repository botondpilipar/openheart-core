#pragma once
#include <glib.h>

GOptionContext* openheart_core_get_args(int argc, char** argv);

gint openheart_core_main(int argc, char** argv);

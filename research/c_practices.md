# C practices used throughout codebases

## Input and Output handling

Closing channels when programs exit
- C Standard Library: `atexit`
- GLib: `g_atexit`

## Execution options

Getting and setting program names.
Usually, the default programname is `char** argv` first string.
However, to set an alternative program name recognized by the
system, you would use

- C Standard Library: UNKNOWN?
- GLib: `g_get_prgname`
- GLib: `g_set_prgname`

Program argument parsing - similarly has a GLib and Non-Glib

- C Standard: `<getopt.h>`
- GLib: all functions that start with `g_option_`

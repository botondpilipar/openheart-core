#pragma once
#include <stdint.h>
#include <glib.h>

uint32_t identity_u32(uint32_t);
int32_t identity_i32(int32_t);
gpointer identity_ptr(gpointer);
gchar identity_char(gchar);
float identity_float(float);
double identity_double(double);

void nothing(gpointer);

#define identity(X) _Generic((X), \
                    uint32_t: identity_u32, \
                    int32_t: identity_i32, \
                    gpointer: identity_ptr, \
                    gchar: identity_char, \
                    float: identity_float, \
                    double: identity_double \
                )(X)

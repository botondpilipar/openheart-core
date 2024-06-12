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

gint* q_alloc_int(gint v);
guint* q_alloc_uint(guint v);
gint64* q_alloc_int64(gint64 v);
gchar* q_alloc_str(const gchar* v);
unsigned char* q_alloc_uchar(unsigned char v);

#define q_alloc(X) _Generic((X), \
                    gint: q_alloc_int, \
                    guint: q_alloc_uint, \
                    gint64: q_alloc_int64, \
                    const gchar*: q_alloc_str, \
                    gchar*: q_alloc_str, \
                    unsigned char: q_alloc_uchar \
                )(X)


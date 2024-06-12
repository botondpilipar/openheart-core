#define G_LOG_USE_STRUCTURED
#define G_LOG_DOMAIN "libopenheart-misc"
#include <func.h>

uint32_t identity_u32(uint32_t p)
{
	return p;
}
int32_t identity_i32(int32_t p)
{
	return p;
}
gpointer identity_ptr(gpointer p)
{
	return p;
}
gchar identity_char(gchar p)
{
	return p;
}
float identity_float(float p)
{
	return p;
}
double identity_double(double p)
{
	return p;
}

void nothing(gpointer /* unused */)
{
    return;
}

gint* q_alloc_int(gint v)
{
	gint* r =  g_new(int, 1);
	*r = v;
	return r;
}

guint* q_alloc_uint(guint v)
{
	guint* r =  g_new(uint, 1);
	*r = v;
	return r;
}

gint64* q_alloc_int64(gint64 v)
{
	gint64* r =  g_new(gint64, 1);
	*r = v;
	return r;
}

gchar* q_alloc_str(const gchar* v)
{
	gchar* r =  g_strdup(v);
	return r;
}

unsigned char* q_alloc_uchar(unsigned char v)
{
	unsigned char* r = g_new(unsigned char, 1);
	*r = v;
	return r;
}

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

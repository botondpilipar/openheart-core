#pragma once
#include <sys/types.h>
#include <stdbool.h>
#include <glib.h>

G_BEGIN_DECLS

int on_user_terminate(gpointer data);
int on_user_interrupt(gpointer data);
int on_user_quit(gpointer data);

G_END_DECLS

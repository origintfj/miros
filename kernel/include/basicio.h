#ifndef BASICIO_H
#define BASICIO_H

#include <stdarg.h>
#include <cqueue.h>

cqueue_t oqueue;
cqueue_t iqueue;

void vfprintf(void (*putc)(char const), char const* const fmt, va_list args);

#endif

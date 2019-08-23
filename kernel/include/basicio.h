#ifndef BASICIO_H
#define BASICIO_H

#include <stdarg.h>
void vfprintf(void (*putc)(char const), char const* const fmt, va_list args);

#endif

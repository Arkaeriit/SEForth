#ifndef SEF_DEBUG_H
#define SEF_DEBUG_H

#include "SEForth.h"

#include "stdio.h"
#include "string.h"
#include "stdarg.h"
#include "sef_io.h"
static void __attribute__((unused)) sef_log_error_msg(int ANSI_color, const char* tag, const char* msg, ...) {
    char buff[150];
    snprintf(buff, 40, "\033[%im%s ", ANSI_color, tag);
    va_list arg;
    va_start(arg, msg);
    vsnprintf(buff + strlen(buff), 100, msg, arg);
    va_end(arg);
    snprintf(buff + strlen(buff), 9, "\033[0m");
#if SEF_LOG_OVER_STDERR
    fprintf(stderr, "%s", buff);
#else
    sef_print_string(buff);
#endif
}

#if SEF_LOG_LEVEL > 0
#define error_msg(msg, ...) sef_log_error_msg(31, "[ERROR]", msg, ##__VA_ARGS__);
#else
#define error_msg(msg, ...)
#endif

#if SEF_LOG_LEVEL > 1
#define warn_msg(msg, ...) sef_log_error_msg(33, "[WARNING]", msg, ##__VA_ARGS__);
#else
#define warn_msg(msg, ...)
#endif

#if SEF_LOG_LEVEL > 2
#define debug_msg(msg, ...) sef_log_error_msg(36, "[DEBUG]", msg, ##__VA_ARGS__);
#else
#define debug_msg(msg, ...)
#endif

#endif


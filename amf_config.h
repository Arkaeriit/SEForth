#ifndef AMF_CONFIG_H
#define AMF_CONFIG_H

#include "inttypes.h"
typedef int64_t amf_int_t;
#define AMF_INT_PRINT PRId64

//Optional fonctionalites and words. Set to 1 to enable and to 0 to disble
//Enable reaading source files
#define AMF_USE_SOURCE_FILE 1

//Depth of the two stacks
#define CODE_STACK_SIZE 100
#define DATA_STACK_SIZE 100

//Dummy values to put in the state's code position to indicate that
//the interpreter is idling and not inside a function
#define IDLE_CURRENT_WORD ~0
#define IDLE_POS_IN_WORD  ~0

//Parser configuration
#define PARSER_BUFFER_SIZE 700
#define PARSER_CUSTOM_NAME_SIZE 25

//Case-sensitivity
#define AMF_CASE_INSENSITIVE

//Log messages
#define AMF_LOG 0

#if AMF_LOG
#include "stdio.h"
#include "stdarg.h"
static void __attribute__ ((unused)) error_msg(const char* msg){
    fprintf(stderr, "\033[31m%s\033[0m", msg);
}
static void __attribute__ ((unused)) debug_msg(const char* msg, ...){
    fprintf(stderr, "\033[36m");
    va_list arg;
    va_start(arg, msg);
    vfprintf(stderr, msg, arg);
    va_end(arg); 
    fprintf(stderr, "\033[0m");
}
#else
#define error_msg(X,...)
#define debug_msg(X,...)
#endif


#endif

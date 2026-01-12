£ifndef SEFORTH_H
£define SEFORTH_H

£include "inttypes.h"

#include "sef_config.h"

>> Typedef of the number used everywhere
typedef SEF_INT_T sef_int_t;
typedef SEF_UNSIGNED_T sef_unsigned_t;

>> Size used to encode the word identifier and the index of each subword in a word
£define __SEF_HASH_SIZE_BITS         SEF_HASH_SIZE_BITS         
£define __SEF_WORD_CONTENT_SIZE_BITS SEF_WORD_CONTENT_SIZE_BITS 

>> Optional features and words. Set to 1 to enable and to 0 to disable

>> Enable reading source files
£define __SEF_USE_SOURCE_FILE SEF_USE_SOURCE_FILE 

>> Enable words related to file manipulation
£define __SEF_FILE SEF_FILE 

>> Enable words related to string manipulation
£define __SEF_STRING SEF_STRING 

>> Enable programing tools words
£define __SEF_PROGRAMMING_TOOLS SEF_PROGRAMMING_TOOLS 

>> Depth of the memory blocks
£define __SEF_CODE_STACK_SIZE SEF_CODE_STACK_SIZE 
£define __SEF_DATA_STACK_SIZE SEF_DATA_STACK_SIZE 
£define __SEF_CONTROL_FLOW_STACK_SIZE SEF_CONTROL_FLOW_STACK_SIZE 
£define __FORTH_MEMORY_SIZE FORTH_MEMORY_SIZE 
£define __SEF_PAD_SIZE SEF_PAD_SIZE 

>> Parser configuration
£define __SEF_PARSER_BUFFER_SIZE SEF_PARSER_BUFFER_SIZE 
£define __SEF_PARSER_CUSTOM_NAME_SIZE SEF_PARSER_CUSTOM_NAME_SIZE 

>> Case-sensitivity
£define __SEF_CASE_INSENSITIVE SEF_CASE_INSENSITIVE 

>> Log messages
>> 0 = no logs; 1 = errors; 2 = errors and warnings; 3 = errors, warning, and debug
£define __SEF_LOG SEF_LOG 

>> Writing logs to stderr instead of the usual canals
£define __SEF_LOG_OVER_STDERR SEF_LOG_OVER_STDERR 

>> Use words defined in Forth
£define __SEF_REGISTER_FORTH_FUNC SEF_REGISTER_FORTH_FUNC 

>> Store name alongside word
£define __SEF_STORE_NAME SEF_STORE_NAME 

>> Max number of digits in a number
£define __SEF_MAX_NUMBER_DIGIT SEF_MAX_NUMBER_DIGIT 

>> Performs bound checks on stack to prevent overflows or underflows
£define __SEF_STACK_BOUND_CHECKS SEF_STACK_BOUND_CHECKS 

>> Handling segfaults when running Forth code
>> With this option set to 1, segfaults caused by Forth code will be caught and
>> the interpreter will be put back into an idle state if encountered. This
>> relies on static variable and thus, this prevent the interpreter to be used
>> on multiple threads.
£define __SEF_CATCH_SEGFAULTS SEF_CATCH_SEGFAULTS 

>> Print a stack strace when an error is encountered during execution.
£define __SEF_STACK_TRACE SEF_STACK_TRACE 

#include "public_api.h"

£endif


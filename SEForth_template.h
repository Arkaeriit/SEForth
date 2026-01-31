£ifndef SEFORTH_H
£define SEFORTH_H

£include "inttypes.h"

#include "sef_config.h"

>> Typedef of the number used everywhere
typedef SEF_INT_T sef_int_t;
typedef SEF_UNSIGNED_T sef_unsigned_t;

>> Optional features and words. Set to 1 to enable and to 0 to disable

>> Enable words related to file manipulation
£define __SEF_FILE SEF_FILE 

>> Enable words related to string manipulation
£define __SEF_STRING SEF_STRING 

>> Enable programing tools words
£define __SEF_PROGRAMMING_TOOLS SEF_PROGRAMMING_TOOLS 

>> Enable memory allocation words
£define __SEF_MEMORY_ALLOCATION SEF_MEMORY_ALLOCATION

>> Enable Forth getting cli argument and setting exit code
£define __SEF_ARG_AND_EXIT_CODE SEF_ARG_AND_EXIT_CODE

>> Depth of the memory blocks
£define __SEF_CODE_STACK_SIZE SEF_CODE_STACK_SIZE 
£define __SEF_DATA_STACK_SIZE SEF_DATA_STACK_SIZE 
£define __SEF_CONTROL_FLOW_STACK_SIZE SEF_CONTROL_FLOW_STACK_SIZE 
£define __SEF_FORTH_MEMORY_SIZE SEF_FORTH_MEMORY_SIZE 
£define __SEF_PAD_SIZE SEF_PAD_SIZE 

>> Case-sensitivity
£define __SEF_CASE_INSENSITIVE SEF_CASE_INSENSITIVE 

>> Log messages
>> 0 = no logs; 1 = errors; 2 = errors and warnings; 3 = errors, warning, and debug
£define __SEF_LOG SEF_LOG 

>> Writing logs to stderr instead of the usual canals
£define __SEF_LOG_OVER_STDERR SEF_LOG_OVER_STDERR 

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

>> Size of the forth state
£define SEF_STATE_SIZE_INT (1 + ((SEF_FORTH_MEMORY_SIZE / sizeof(sef_int_t)) + (SEF_PAD_SIZE / sizeof(sef_int_t)) + SEF_DATA_STACK_SIZE + SEF_CODE_STACK_SIZE + SEF_CONTROL_FLOW_STACK_SIZE + 17))

#include "public_api.h"

£endif


// Typedef of the number used everywhere
#ifndef SEF_INT_T
#define SEF_INT_T intptr_t
#endif
#ifndef SEF_UNSIGNED_T
#define SEF_UNSIGNED_T uintptr_t
#endif

// Optional features and words. Set to 1 to enable and to 0 to disable

// Enable words related to file manipulation
#ifndef SEF_FILE_ACCESS
#define SEF_FILE_ACCESS 0
#endif

// Enable words related to string manipulation
#ifndef SEF_STRING
#define SEF_STRING 1
#endif

// Enable programing tools words
#ifndef SEF_PROGRAMMING_TOOLS
#define SEF_PROGRAMMING_TOOLS 1
#endif

// Enable memory allocation words
#ifndef SEF_MEMORY_ALLOCATION
#define SEF_MEMORY_ALLOCATION 1
#endif

// Enable Forth getting cli argument and setting exit code
#ifndef SEF_ARG_AND_EXIT_CODE
#define SEF_ARG_AND_EXIT_CODE 1
#endif

// Set to 1 for ABORT to stop Forth and 0 for ABORT to start the shell
#ifndef SEF_ABORT_STOP_FORTH
#define SEF_ABORT_STOP_FORTH 1
#endif

// Depth of the memory blocks
#ifndef SEF_CODE_STACK_SIZE
#define SEF_CODE_STACK_SIZE 1000
#endif
#ifndef SEF_DATA_STACK_SIZE
#define SEF_DATA_STACK_SIZE 1000
#endif
#ifndef SEF_CONTROL_FLOW_STACK_SIZE
#define SEF_CONTROL_FLOW_STACK_SIZE 100
#endif
#ifndef SEF_FORTH_MEMORY_SIZE
#define SEF_FORTH_MEMORY_SIZE 1000000
#endif
#ifndef SEF_PAD_SIZE
#define SEF_PAD_SIZE 100
#endif

// Case-sensitivity
#ifndef SEF_CASE_INSENSITIVE
#define SEF_CASE_INSENSITIVE 1
#endif

// Log messages
// 0 = no logs; 1 = errors; 2 = errors and warnings; 3 = errors, warning, and debug
#ifndef SEF_LOG_LEVEL
#define SEF_LOG_LEVEL 2
#endif

// Writing logs to stderr instead of the usual canals
#ifndef SEF_LOG_OVER_STDERR
#define SEF_LOG_OVER_STDERR 0
#endif

// Performs bound checks on stack to prevent overflows or underflows
#ifndef SEF_STACK_BOUND_CHECKS
#define SEF_STACK_BOUND_CHECKS 1
#endif

// Handling segfaults when running Forth code
// With this option set to 1, segfaults caused by Forth code will be caught and
// the interpreter will be put back into an idle state if encountered. This
// relies on static variable and thus, this prevent the interpreter to be used
// on multiple threads.
#ifndef SEF_CATCH_SEGFAULTS
#define SEF_CATCH_SEGFAULTS 1
#endif


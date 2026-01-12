// Typedef of the number used everywhere
#ifndef SEF_INT_T
#define SEF_INT_T intptr_t
#endif
#ifndef SEF_UNSIGNED_T
#define SEF_UNSIGNED_T uintptr_t
#endif

// Size used to encode the word identifier and the index of each subword in a word
#ifndef SEF_HASH_SIZE_BITS
#define SEF_HASH_SIZE_BITS         24
#endif
#ifndef SEF_WORD_CONTENT_SIZE_BITS
#define SEF_WORD_CONTENT_SIZE_BITS 24
#endif

// Optional features and words. Set to 1 to enable and to 0 to disable

// Enable reading source files
#ifndef SEF_USE_SOURCE_FILE
#define SEF_USE_SOURCE_FILE 1
#endif

// Enable words related to file manipulation
#ifndef SEF_FILE
#define SEF_FILE 1
#endif

// Enable words related to string manipulation
#ifndef SEF_STRING
#define SEF_STRING 1
#endif

// Enable programing tools words
#ifndef SEF_PROGRAMMING_TOOLS
#define SEF_PROGRAMMING_TOOLS 1
#endif

// Depth of the memory blocks
#ifndef SEF_CODE_STACK_SIZE
#define SEF_CODE_STACK_SIZE 1000
#endif
#ifndef SEF_DATA_STACK_SIZE
#define SEF_DATA_STACK_SIZE 1000
#endif
#ifndef SEF_COMPILATION_STACK_SIZE
#define SEF_COMPILATION_STACK_SIZE 100
#endif
#ifndef FORTH_MEMORY_SIZE
#define FORTH_MEMORY_SIZE 1000000
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
#ifndef SEF_LOG
#define SEF_LOG 2
#endif

// Writing logs to stderr instead of the usual canals
#ifndef SEF_LOG_OVER_STDERR
#define SEF_LOG_OVER_STDERR 0
#endif

// Use words defined in Forth
#ifndef SEF_REGISTER_FORTH_FUNC
#define SEF_REGISTER_FORTH_FUNC 1
#endif

// Max number of digits in a number
#ifndef SEF_MAX_NUMBER_DIGIT
#define SEF_MAX_NUMBER_DIGIT 64
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

// Print a stack strace when an error is encountered during execution.
#ifndef SEF_STACK_TRACE
#define SEF_STACK_TRACE 1
#endif


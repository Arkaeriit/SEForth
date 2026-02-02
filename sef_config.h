// ------------------------------ Numeric types ----------------------------- //

// Type used to represent signed one-cell numbers. It must be big enough to
// store a pointer, but can be bigger if needed.
#ifndef SEF_INT_T
#define SEF_INT_T intptr_t
#endif

// Type used to represent unsigned one-cell numbers. It must have the same size
// as `SEF_INT_T`.
//
#ifndef SEF_UNSIGNED_T
#define SEF_UNSIGNED_T uintptr_t
#endif

// --------------------------- Optional word sets --------------------------- //

#ifndef SEF_FILE_ACCESS
#define SEF_FILE_ACCESS 0
#endif

#ifndef SEF_STRING
#define SEF_STRING 1
#endif

#ifndef SEF_PROGRAMMING_TOOLS
#define SEF_PROGRAMMING_TOOLS 1
#endif

#ifndef SEF_MEMORY_ALLOCATION
#define SEF_MEMORY_ALLOCATION 1
#endif

#ifndef SEF_ARG_AND_EXIT_CODE
#define SEF_ARG_AND_EXIT_CODE 1
#endif

// ------------------------------- Memory used ------------------------------ //

// Number of cells in the return stack.
#ifndef SEF_RETURN_STACK_SIZE
#define SEF_RETURN_STACK_SIZE 1000
#endif

// Number of cells in the data stack.
#ifndef SEF_DATA_STACK_SIZE
#define SEF_DATA_STACK_SIZE 1000
#endif

// Number of cells in the control flow stack.
#ifndef SEF_CONTROL_FLOW_STACK_SIZE
#define SEF_CONTROL_FLOW_STACK_SIZE 100
#endif

// Size in bytes of the memory region addressed by HERE. You might need around
// 30 kB on a system with `SEF_INT_T` set to `int64_t`.
#ifndef SEF_FORTH_MEMORY_SIZE
#define SEF_FORTH_MEMORY_SIZE 1000000
#endif

// Size in bytes of the pad region.
#ifndef SEF_PAD_SIZE
#define SEF_PAD_SIZE 100
#endif

// ---------------------------- Optional features --------------------------- //


// If set to 0, the words `abort` and `abort"` will put the user in a Forth
// prompt as the standard suggests. If set to 1, those words will stop the Forth
// system, which is more convenient to run programs.
#ifndef SEF_ABORT_STOP_FORTH
#define SEF_ABORT_STOP_FORTH 1
#endif

// If set to 1, all dictionary searches will be case-insensitive. If set to 0,
// dictionary searches will be case-sensitive for user-defined words and
// case-insensitive for system words.
#ifndef SEF_CASE_INSENSITIVE
#define SEF_CASE_INSENSITIVE 1
#endif

// This control the amount of internal logs outputted by SEForth. If set to 0,
// no internal logs will be outputted; this is only suitable if SEForth runs in
// an environment with very limited resources and where the logs will never be
// read. If set to 1, error message will be outputted; this is the most sensible
// configuration. If set to 2, some additional warnings like for word definition
// will be outputted; this is also a sensible configuration. If set to 3, debug
// messages will be outputted; as this is **extremely** verbose and noisy, it's
// only suitable to debug the SEForth interpreter itself.
#ifndef SEF_LOG_LEVEL
#define SEF_LOG_LEVEL 2
#endif

// If set to 1, the logs controlled by `SEF_LOG_LEVEL` will be printed to
// `stderr`. If set to 0, they will be emitted with the `EMIT` word.
#ifndef SEF_LOG_OVER_STDERR
#define SEF_LOG_OVER_STDERR 0
#endif

// If set to 1, there will be checks to ensure that none of the stacks can
// overflow and underflow, and that the memory space addressed by HERE doesn't
// overflow. If set to 0, those checks are disabled. The checks have some
// performance impact, but they are very convenient.
#ifndef SEF_STACK_BOUND_CHECKS
#define SEF_STACK_BOUND_CHECKS 1
#endif

// With this option set to 1, segfaults caused by Forth code will be caught and
// the interpreter will be put back into an idle state if encountered. This
// relies on static variable and thus, this prevent the interpreter to be used
// on multiple threads. Furthermore, the system running SEForth needs to
// support POSIX signals.
#ifndef SEF_CATCH_SEGFAULTS
#define SEF_CATCH_SEGFAULTS 1
#endif


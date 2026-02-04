£ifndef SEFORTH_H
£define SEFORTH_H

£include "stdint.h"
£include "stddef.h"

#include "sef_config.h"

>> ------------------------------ Numeric types ----------------------------- >>

>> Type used to represent signed one-cell numbers. It must be big enough to
>> store a pointer, but can be bigger if needed.
typedef SEF_INT_T sef_int_t;

>> Type used to represent unsigned one-cell numbers. It must have the same size
>> as `SEF_INT_T`.
typedef SEF_UNSIGNED_T sef_unsigned_t;

>> --------------------------- Optional word sets --------------------------- >>

£define ___SEF_FILE_ACCESS SEF_FILE_ACCESS

£define ___SEF_STRING SEF_STRING

£define ___SEF_PROGRAMMING_TOOLS SEF_PROGRAMMING_TOOLS

£define ___SEF_MEMORY_ALLOCATION SEF_MEMORY_ALLOCATION

£define ___SEF_ARG_AND_EXIT_CODE SEF_ARG_AND_EXIT_CODE

>> ------------------------------- Memory used ------------------------------ >>

>> Number of cells in the return stack.
£define ___SEF_RETURN_STACK_SIZE SEF_RETURN_STACK_SIZE

>> Number of cells in the data stack.
£define ___SEF_DATA_STACK_SIZE SEF_DATA_STACK_SIZE

>> Number of cells in the control flow stack.
£define ___SEF_CONTROL_FLOW_STACK_SIZE SEF_CONTROL_FLOW_STACK_SIZE

>> Size in bytes of the memory region addressed by HERE. You might need around
>> 30 kB on a system with `SEF_INT_T` set to `int64_t`.
£define ___SEF_FORTH_MEMORY_SIZE SEF_FORTH_MEMORY_SIZE

>> Size in bytes of the pad region.
£define ___SEF_PAD_SIZE SEF_PAD_SIZE

>> ---------------------------- Optional features --------------------------- >>

>> If set to 1, all dictionary searches will be case-insensitive. If set to 0,
>> dictionary searches will be case-sensitive for user-defined words and
>> case-insensitive for system words.
£define ___SEF_CASE_INSENSITIVE SEF_CASE_INSENSITIVE

>> This controls the amount of internal logs outputted by SEForth. If set to 0,
>> no internal logs will be outputted; this is only suitable if SEForth runs in
>> an environment with very limited resources and where the logs will never be
>> read. If set to 1, error message will be outputted; this is the most sensible
>> configuration. If set to 2, some additional warnings like for word definition
>> will be outputted; this is also a sensible configuration. If set to 3, debug
>> messages will be outputted; as this is **extremely** verbose and noisy, it's
>> only suitable to debug the SEForth interpreter itself.
£define ___SEF_LOG_LEVEL SEF_LOG_LEVEL

>> If set to 1, the logs controlled by `___SEF_LOG_LEVEL` will be printed to
>> `stderr`. If set to 0, they will be emitted with the `EMIT` word.
£define ___SEF_LOG_OVER_STDERR SEF_LOG_OVER_STDERR

>> If set to 1, there will be checks to ensure that none of the stacks can
>> overflow and underflow, and that the memory space addressed by HERE doesn't
>> overflow. If set to 0, those checks are disabled. The checks have some
>> performance impact, but they are very convenient.
£define ___SEF_STACK_BOUND_CHECKS SEF_STACK_BOUND_CHECKS

>> With this option set to 1, segfaults caused by Forth code will be caught and
>> the interpreter will be put back into an idle state if encountered. This
>> relies on static variable and thus, this prevent the interpreter to be used
>> on multiple threads. Furthermore, the system running SEForth needs to
>> support POSIX signals.
£define ___SEF_CATCH_SEGFAULTS SEF_CATCH_SEGFAULTS

>> Size of the forth state
£define SEF_STATE_SIZE_INT (1 + ((SEF_FORTH_MEMORY_SIZE / sizeof(sef_int_t)) + (SEF_PAD_SIZE / sizeof(sef_int_t)) + SEF_DATA_STACK_SIZE + SEF_RETURN_STACK_SIZE + SEF_CONTROL_FLOW_STACK_SIZE + 17))

#include "public_api.h"

£endif


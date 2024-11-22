#ifndef ERROR_H
#define ERROR_H

#include "assert.h"

// Basic checks on the current configuration

#if SEF_CLI_ARGS
#if !SEF_USE_SOURCE_FILE
#error CLI arguments are only used when interpreting a file. It makes no sense to enable CLI arguments while not enabling source files.
#endif
#endif

static_assert(sizeof(sef_int_t) == sizeof(sef_unsigned_t), "Unsigned and signed values should have the same size.");
static_assert((sizeof(sef_int_t) * 8) >= (SEF_HASH_SIZE_BITS + SEF_WORD_CONTENT_SIZE_BITS), "You should be able to store word position and hash in a sef_int_t.");

#endif


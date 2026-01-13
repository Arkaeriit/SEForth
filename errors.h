#ifndef ERROR_H
#define ERROR_H

#include "assert.h"

// Basic checks on the current configuration

static_assert(sizeof(sef_int_t) == sizeof(sef_unsigned_t), "Unsigned and signed values should have the same size.");

#endif


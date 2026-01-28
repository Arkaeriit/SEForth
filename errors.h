#ifndef ERROR_H
#define ERROR_H

#include "assert.h"

// Basic checks on the current configuration

static_assert(sizeof(sef_int_t) == sizeof(sef_unsigned_t), "Unsigned and signed values should have the same size.");

static_assert(sizeof(sef_int_t) * SEF_STATE_SIZE_INT >= sizeof(forth_state_t), "Exported state size should be at least as large as true state size");

#endif


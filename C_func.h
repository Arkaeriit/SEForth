#include "private_api.h"
#ifndef C_FUNC_H
#define C_FUNC_H

// TODO: This should be public
typedef void (*C_callback_t)(forth_state_t*);

// Add a new word defined in C into the dictionary. name must be NULL terminated.
void sef_register_cfunc(forth_state_t* fs, const char* name, C_callback_t func, bool is_imediate);

// Register run-time system words defined in C.
void sef_register_default_cfunc(forth_state_t* fs);

#endif


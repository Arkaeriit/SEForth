#include "private_api.h"
#ifndef C_FUNC_H
#define C_FUNC_H

// Add a new word defined in C into the dictionary. name must be NULL terminated.
void sef_register_cfunc(forth_state_t* fs, const char* name, void (*func)(forth_state_t*), bool is_imediate);

// Register run-time system words defined in C.
void sef_register_default_cfunc(forth_state_t* fs);

#endif


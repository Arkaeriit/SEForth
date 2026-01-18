#include "private_api.h"
#ifndef SEF_PARSER_H
#define SEF_PARSER_H

// Assume we are at the haven't started to read the input source.
// Parse and run the input source until it's empty.
void sef_inter_compil_run(forth_state_t* fs);

// Register parser's compile time words writtens in C.
void sef_register_parser_cfunc(forth_state_t* fs);

// Set a C string as the input source.
void sef_set_c_string_as_input_source(forth_state_t* fs, const char* str);

// Reset the input source as before the last set.
void sef_pop_input_source(forth_state_t* fs);

#endif


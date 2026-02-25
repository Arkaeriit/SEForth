/*-----------------------------------------------------------------\
|This file is used to include the various headers for the project. |
|The order in which they are included is pretty important so I want|
|to fix this order in a single file.                               |
\-----------------------------------------------------------------*/

#ifndef PRIVATE_API_H
#define PRIVATE_API_H

#define UNUSED(x) (void)(x)

#include "SEForth.h"

struct forth_state_s;
typedef struct forth_state_s forth_state_t;
typedef sef_int_t* dictionary_entry_t;

#include "word_cache.h"
#include "forth_state.h"
#include "stdlib.h"
#include "sef_io.h"
#include "sef_debug.h"
#include "errors.h"
#include "C_func.h"
#include "dictionary.h"
#include "parser.h"
#include "block_c_func.h"

#endif


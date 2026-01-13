#include "private_api.h"
#include <string.h>

void sef_init(forth_state_t* state) {
    sef_state_init(state);
}

void sef_restart(forth_state_t* state) {
    sef_reset(state);
}

bool sef_ready_to_run(forth_state_t* state) {
    return !state->bye && !state->error_encountered;
}

bool sef_asked_bye(forth_state_t* state) {
    return state->bye;
}

bool sef_is_compiling(forth_state_t* state) {
    return state->compiling;
}

// TODO: error flag read and reset. Maybe in the same function ?

void sef_parse_string(forth_state_t* state, const char* s) {
    sef_set_c_string_as_input_source(state, s);
    sef_inter_compil_run(state);
}

#if SEF_USE_SOURCE_FILE
sef_error sef_parse_file(forth_state_t* state, const char* filename) {
    return sef_register_file(state->parser, filename);
}
#endif

#if SEF_PROGRAMMING_TOOLS
void sef_feed_arguments(forth_state_t* state, int argc, char** argv) {
    state->argc = argc;
    state->argv = argv;
}

int sef_exit_code(const forth_state_t* state) {
    return state->exit_code;
}
#endif


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
    sef_pop_input_source(state);
}

#if SEF_USE_SOURCE_FILE
sef_error sef_parse_file(forth_state_t* state, const char* filename) {
    return sef_register_file(state->parser, filename);
}
#endif

#if SEF_ARG_AND_EXIT_CODE
void sef_feed_arguments(forth_state_t* state, int argc, char** argv) {
    if (state->compiling) {
        SEF_ERROR_OUT(state, "sef_feed_arguments can't be called if the state is compiling.\n");
        return;
    }

    sef_push_data(state, (sef_int_t) argc);
    sef_push_data(state, (sef_int_t) argv);
    sef_parse_string(state, "feed-arguments-from-os");
}

int sef_exit_code(forth_state_t* state) {
    if (state->compiling) {
        SEF_ERROR_OUT(state, "sef_exit_code can't be called if the state is compiling.\n");
        return -1;
    }
    sef_parse_string(state, "exit-code @");
    return sef_pop_data(state);
}
#endif


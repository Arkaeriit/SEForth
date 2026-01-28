#include "private_api.h"
#include <string.h>

void sef_init(sef_forth_state_t* _state) {
    forth_state_t* state = (forth_state_t*) _state;
    sef_state_init(state);
}

void sef_restart(sef_forth_state_t* _state) {
    forth_state_t* state = (forth_state_t*) _state;
    sef_reset(state);
}

bool sef_ready_to_run(sef_forth_state_t* _state) {
    forth_state_t* state = (forth_state_t*) _state;
    return !state->bye && !state->error_encountered;
}

bool sef_asked_bye(sef_forth_state_t* _state) {
    forth_state_t* state = (forth_state_t*) _state;
    return state->bye;
}

bool sef_is_compiling(sef_forth_state_t* _state) {
    forth_state_t* state = (forth_state_t*) _state;
    return state->compiling;
}

// TODO: error flag read and reset. Maybe in the same function ?

void sef_parse_string(sef_forth_state_t* _state, const char* s) {
    forth_state_t* state = (forth_state_t*) _state;
    sef_set_c_string_as_input_source(state, s);
    sef_inter_compil_run(state);
    // TODO: can cause error is the compilation failed. Maybe condition on having an error
    // A general TODO for error recovery is probably needed
    sef_pop_input_source(state);
}

void sef_push_to_data_stack(sef_forth_state_t* _state, sef_int_t w) {
    forth_state_t* state = (forth_state_t*) _state;
    sef_push_data(state, w);
}

sef_int_t sef_pop_from_data_stack(sef_forth_state_t* _state) {
    forth_state_t* state = (forth_state_t*) _state;
    return sef_pop_data(state);
}

void sef_register_c_word(sef_forth_state_t* _state, const char* name, sef_c_word func, bool is_imediate) {
    forth_state_t* state = (forth_state_t*) _state;
    sef_register_cfunc(state, name, (void (*)(forth_state_t*)) func, is_imediate);
}

#if SEF_ARG_AND_EXIT_CODE
void sef_feed_arguments(sef_forth_state_t* _state, int argc, char** argv) {
    forth_state_t* state = (forth_state_t*) _state;
    if (state->compiling) {
        SEF_ERROR_OUT(state, "sef_feed_arguments can't be called if the state is compiling.\n");
        return;
    }

    sef_push_data(state, (sef_int_t) argc);
    sef_push_data(state, (sef_int_t) argv);
    sef_parse_string(_state, "feed-arguments-from-os");
}

int sef_exit_code(sef_forth_state_t* _state) {
    forth_state_t* state = (forth_state_t*) _state;
    if (state->compiling) {
        SEF_ERROR_OUT(state, "sef_exit_code can't be called if the state is compiling.\n");
        return -1;
    }
    sef_parse_string(_state, "exit-code @");
    return sef_pop_data(state);
}
#endif


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
    return !state->bye && (!state->error_encountered || SEF_ABORT_STOP_FORTH);
}

bool sef_asked_bye(sef_forth_state_t* _state) {
    forth_state_t* state = (forth_state_t*) _state;
    return state->bye;
}

bool sef_is_compiling(sef_forth_state_t* _state) {
    forth_state_t* state = (forth_state_t*) _state;
    return state->compiling;
}

int sef_exit_code(sef_forth_state_t* _state) {
    forth_state_t* state = (forth_state_t*) _state;
#if SEF_ARG_AND_EXIT_CODE
    if (!sef_ready_to_run(_state)) {
        sef_restart(_state);
    }
    bool is_compiling = sef_is_compiling(_state);
    if (is_compiling) {
        sef_parse_string(_state, "[");
    }
    sef_parse_string(_state, "exit-code @");
    int ret = sef_pop_from_data_stack(_state);
    sef_parse_string(_state, " 0 exit-code !");
    if (is_compiling) {
        sef_parse_string(_state, "]");
    }
#else
    int ret = state->error_encountered ? -1 : 0;
#endif
    state->error_encountered = false;
    return ret;
}

void sef_parse_string(sef_forth_state_t* _state, const char* s) {
    forth_state_t* state = (forth_state_t*) _state;
    sef_set_c_string_as_input_source(state, s);
    sef_inter_compil_run(state);
}

void sef_push_to_data_stack(sef_forth_state_t* _state, sef_int_t w) {
    forth_state_t* state = (forth_state_t*) _state;
    sef_push_data(state, w);
}

sef_int_t sef_pop_from_data_stack(sef_forth_state_t* _state) {
    forth_state_t* state = (forth_state_t*) _state;
    return sef_pop_data(state);
}

void sef_register_c_word(sef_forth_state_t* _state, const char* name, sef_c_word func, bool is_immediate) {
    forth_state_t* state = (forth_state_t*) _state;
    sef_register_cfunc(state, name, (void (*)(forth_state_t*)) func, is_immediate);
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

#endif


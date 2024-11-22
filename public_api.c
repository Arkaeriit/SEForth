#include "private_api.h"
#include <string.h>

forth_state_t* sef_init(void) {
    return sef_init_parser()->fs;
}

void sef_free(forth_state_t* state) {
    sef_clean_parser(state->parser);
}

void sef_restart(forth_state_t* state) {
    state->running = true;
}

bool sef_is_running(forth_state_t* state) {
    return state->running;
}

void sef_parse_string(forth_state_t* state, const char* s) {
    for (size_t i=0; i<strlen(s); i++) {
        sef_parse_char(state, s[i]);
    }
}

void sef_parse_char(forth_state_t* state, char c) {
    sef_parser_parse_char(state->parser, c);
}

#if SEF_USE_SOURCE_FILE
sef_error sef_parse_file(forth_state_t* state, const char* filename) {
    return sef_register_file(state->parser, filename);
}
#endif

#if SEF_CLI_ARGS
void sef_feed_arguments(forth_state_t* state, int argc, char** argv) {
    state->argc = argc;
    state->argv = argv;
}
#endif

#if SEF_PROGRAMMING_TOOLS
int sef_exit_code(const forth_state_t* state) {
    return state->exit_code;
}
#endif


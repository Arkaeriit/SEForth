£include <stdbool.h>

typedef sef_int_t sef_forth_state_t[SEF_STATE_SIZE_INT];

typedef void (*sef_c_word)(sef_forth_state_t*);

void sef_init(sef_forth_state_t* state);

void sef_restart(sef_forth_state_t* state);
bool sef_ready_to_run(sef_forth_state_t* state);
bool sef_asked_bye(sef_forth_state_t* state);
bool sef_is_compiling(sef_forth_state_t* state);
bool sef_error_encountered(sef_forth_state_t* state);

void sef_parse_string(sef_forth_state_t* state, const char* s);

void sef_push_to_data_stack(sef_forth_state_t* state, sef_int_t w);
sef_int_t sef_pop_from_data_stack(sef_forth_state_t* state);

void sef_register_c_word(sef_forth_state_t* state, const char* name, sef_c_word func, bool is_imediate);

#ifdef SEF_ARG_AND_EXIT_CODE
void sef_feed_arguments(sef_forth_state_t* state, int argc, char** argv);
int sef_exit_code(sef_forth_state_t* state);
#endif


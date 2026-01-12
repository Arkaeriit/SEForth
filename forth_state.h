#include "private_api.h"
#ifndef FORTH_STATE_H
#define FORTH_STATE_H

#include "stdbool.h"

void sef_state_init(forth_state_t* fs);

void sef_push_data(forth_state_t* fs, sef_int_t data);
void sef_push_code(forth_state_t* fs, sef_int_t data);
void sef_push_control_flow(forth_state_t* fs, sef_int_t data);
sef_int_t sef_pop_data(forth_state_t* fs);
sef_int_t sef_pop_code(forth_state_t* fs);
sef_int_t sef_pop_control_flow(forth_state_t* fs);

void sef_run(forth_state_t* fs);
void sef_quit(forth_state_t* fs);
void sef_exit(forth_state_t* fs);
void sef_abort(forth_state_t* fs);
void sef_call_entry(forth_state_t* fs, dictionary_entry_t entry);

#define SEF_ERROR_OUT(fs, error_msg...) \
    error_msg(fs, error_msg);           \
    sef_abort(fs)                        

typedef enum {
    STATE_IDLE,
    STATE_COMPILING,
    STATE_INTERPRETING,
    STATE_EXECUTING,
} possible_states_t;

possible_states_t sef_get_current_state(forth_state_t* fs);

typedef enum {
    WTM_COMPILE_TIME   = 1 << 0;
    WTM_POSTPONED      = 1 << 1;
    WTM_NUMBER_LITERAL = 1 << 2;
} word_tag_mask;

void sef_allot(forth_state_t* fs, size_t byte_requested);
static inline void sef_allot_cell(forth_state_t* fs) {
    sef_allot(fs, sizeof(sef_int_t));
}

#endif


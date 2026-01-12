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
void sef_abort(forth_state_t* fs);

#define SEF_ERROR_OUT(fs, error_msg...) \
    error_msg(fs, error_msg);           \
    sef_abort(fs)                        

typedef enum {
    STATE_IDLE,
    STATE_COMPILING,
    STATE_INTERPRETING,
    STATE_EXECUTING,
} possible_states_t;

possible_states_t sef_is_idle(forth_state_t* fs);

void sef_allot(forth_state_t* fs, size_t byte_requested);

#endif


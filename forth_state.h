#include "private_api.h"
#ifndef FORTH_STATE_H
#define FORTH_STATE_H

#include "stdbool.h"
#include "stddef.h"

#define FORTH_TRUE ((sef_int_t) ~0)
#define FORTH_BOOL(x) ((x) ? FORTH_TRUE : 0)
#define FORTH_FALSE FORTH_BOOL(false)

struct forth_state_s;
typedef bool (*input_source_refill_t)(struct forth_state_s* state, void* input_source);

struct forth_state_s {
    // Memory spaces
    uint8_t forth_memory[SEF_FORTH_MEMORY_SIZE];
    uint8_t pad[SEF_PAD_SIZE];
    sef_int_t data_stack[SEF_DATA_STACK_SIZE];
    sef_int_t return_stack[SEF_RETURN_STACK_SIZE];
    sef_int_t control_flow_stack[SEF_CONTROL_FLOW_STACK_SIZE];
    // Memory pointer and indexes
    union {
        uint8_t* byte;
        sef_int_t* cell;
    } here;
    dictionary_entry_t last_dictionary_entry;
    sef_int_t data_stack_index;
    sef_int_t return_stack_index;
    sef_int_t control_flow_stack_index;
    // Internal variables
    sef_int_t compiling;
    sef_int_t base;
    sef_int_t* code_pointer;
    sef_int_t exit_code;
    bool bye;
    bool quit;
    // Word cache
    dictionary_entry_t word_cache[WORD_IN_CACHE_COUNT];
    // Parser
    sef_int_t input_buffer_size;
    char* input_buffer;
    sef_int_t parse_area_offset;
    void* input_source;
    input_source_refill_t input_source_refill;
    sef_int_t source_id;
    bool compiling_system_words;
};

void sef_state_init(forth_state_t* fs);

void sef_push_data(forth_state_t* fs, sef_int_t data);
void sef_push_return(forth_state_t* fs, sef_int_t data);
void sef_push_control_flow(forth_state_t* fs, sef_int_t data);
sef_int_t sef_pop_data(forth_state_t* fs);
sef_int_t sef_pop_return(forth_state_t* fs);
sef_int_t sef_pop_control_flow(forth_state_t* fs);

void sef_run(forth_state_t* fs);
void sef_quit(forth_state_t* fs);
void sef_exit(forth_state_t* fs);
void sef_reset(forth_state_t* fs);
void sef_abort(forth_state_t* fs);
void sef_call_entry(forth_state_t* fs, dictionary_entry_t entry);

#define SEF_ERROR_OUT(fs, error_txt...) \
    error_msg(error_txt);               \
    sef_abort(fs)                        

typedef enum {
    WTM_IMMEDIATE      = 1 << 0,
    WTM_DOES_EXECUTION = 1 << 1,
    WTM_SYSTEM_WORD    = 1 << 2,
    WTM_C_WORD         = 1 << 3,
    WTM_FORTH_WORD     = 1 << 4,
    WTM_CREATE         = 1 << 5,
} word_tag_mask;

#define WORD_KIND (WTM_C_WORD | WTM_FORTH_WORD | WTM_CREATE | WTM_DOES_EXECUTION)

void sef_allot(forth_state_t* fs, size_t byte_requested);
static inline void sef_allot_cell(forth_state_t* fs) {
    sef_allot(fs, sizeof(sef_int_t));
}

#endif


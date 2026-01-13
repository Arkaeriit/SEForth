#include "forth_state.h"
#include "sef_debug.h"
#include "string.h"

/* -------------------------- State initialization -------------------------- */

static bool no_input_source(forth_state_t* fs, void* _) {
    (void) _;
    return false;
}

static void reset_parser(forth_state_t* fs) {
    fs->input_source_refill = no_input_source;
    fs->input_source_buffer_size = 0;
    sef->parse_area_offset = 0;
    sef->input_source = NULL;
}

// Init the interpreter
forth_state_t* sef_init_state(forth_state_t* fs) {
    fs->here = &fs->forth_memory[0];
    fs->last_dictionary_entry = NULL;
    fs->data_stack_index = 0;
    fs->code_stack_index = 0;
    fs->control_flow_stack_index = 0;
    fs->compiling = false;
    fs->base = 10;
    fs->code_pointer = NULL;
    fs->error_encountered = false;
    reset_parser(fs);
    fs->compiling_system_words = true;
    // TODO: fill dictionary
    fs->compiling_system_words = false;
}

/* --------------------------- Stack manipulation --------------------------- */

#if SEF_STACK_BOUND_CHECKS
#define STACK_BOUND_CHECK(fs, stack_name, stack_size)                                  \
    if (!(fs->stack_name ## _index < 0 || fs->stack_name ## _index >= stack_size)) {   \
        SEF_ERROR_OUT(fs, "Stack '%s' out of bound. Resetting state.\n", #stack_name); \
    }                                                                                   
#else
#define STACK_BOUND_CHECK(fs, stack_name, stack_size)
#endif

#define STACK_OPERATIONS(stack_name, stack_size)                  \
void sef_push_ ## stack_name(forth_state_t* fs, sef_int_t data) { \
    fs->stack_name[fs->stack_name ## _index++] = data;            \
    STACK_BOUND_CHECK(fs, stack_name, stack_size)                 \
}                                                                 \
                                                                  \
sef_int_t sef_pop_ ## stack_name(forth_state_t* fs) {             \
    sef_int_t ret = fs->stack_name[fs->stack_name ## _index--];   \
    STACK_BOUND_CHECK(fs, stack_name, stack_size)                 \
    return ret;                                                   \
}                                                                  

STACK_OPERATIONS(data_stack, SEF_DATA_STACK_SIZE)
STACK_OPERATIONS(code_stack, SEF_CODE_STACK_SIZE)
STACK_OPERATIONS(control_flow_stack, SEF_CONTROL_FLOW_SIZE)

/* ---------------------------- Taking down state --------------------------- */

// Puts the state back in an idle state, with all stacks but the data stack
// empty and no word being executed.
void sef_quit(forth_state_t* fs) {
    fs->code_pointer = NULL;
    fs->code_stack_index = 0;
    fs->compiling = false;
    reset_parser(fs);
}

#if SEF_STACK_TRACE
static void sef_stack_trace(forth_state_t* fs) {
    error_msg("Stack trace TODO!\n");
}
#endif

// Like quit but also reset the data stack. Display the stack trace if needed.
void sef_abort(forth_state_t* fs) {
#if SEF_STACK_TRACE
    //TODO: Maybe I should separate the error case and the normal case...
    sef_stack_trace(fs);
#endif
    sef_quit(fs);
    fs->data_stack_index = 0;
    fs->error_encountered = true;
}

/* ----------------------------- Word execution ----------------------------- */

// Executes the code pointer. Return true if it was executed and false if it
// wasn't because it is NULL.
static bool sef_execute_code_pointer(forth_state_t* fs) {
    if (fs->code_pointer == NULL) {
        return false;
    }

    sef_int_t entry = *code_pointer;
    sef_call_entry(fs, entry);
    fs->code_pointer += 1;
    return true;
}

void sef_run(forth_state_t* fs) {
    while (sef_execute_code_pointer(fs));
}

void sef_exit(forth_state_t* fs) {
    fs->code_pointer = sef_pop_code(fs);
}

void sef_call_entry(fs, dictionary_entry_t entry) {
    sef_int_t word_tags = *(sef_get_word_tag_field(entry))
    void* parameters = sef_get_entry_parameter(entry);
    word_executing_function wef = *(sef_get_word_executing_function(entry));

    if (word_tags & WTM_DOES_EXECUTION) {
        sef_int_t* new_code_pointer = (sef_int_t*) wef;
        sef_push_code(fs, fs->code_pointer);
        sef_push_data(fs, (sef_int_t) parameters);
        fs->code_pointer = new_code_pointer; // Currently pointing to `DOES>`, but will be shifted to what we cant to execute by the run word function.
    } else {
        // TODO: segfault catching
        wef(fs, parameters);
    }
}

// TODO: Do I realy need that?
possible_states_t sef_get_current_state(forth_state_t* fs) {
    if (fs->compiling) {
        return STATE_COMPILING;
    } else if (fs->code_pointer != NULL) {
        return STATE_EXECUTING;
    } else {
        return STATE_INTERPRETING;
    }
}

/* ------------------------- Forth memory management ------------------------ */

// Request some bytes from the forth memory and align the index
void sef_allot(forth_state_t* fs, size_t byte_requested) {
    fs->here->byte += byte_requested;
#if SEF_STACK_BOUND_CHECKS
    if (fs->here->byte - &fs->forth_memory[0] > FORTH_MEMORY_SIZE) {
        SEF_ERROR_OUT(fs, "Forth memory overflowed by %i bytes.\n", fs->forth_memory_index - FORTH_MEMORY_SIZE);
    }
#endif
}


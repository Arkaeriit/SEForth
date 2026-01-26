#include "forth_state.h"
#include "string.h"

/* -------------------------- State initialization -------------------------- */

static bool no_input_source(forth_state_t* fs, void* _) {
    (void) fs;
    (void) _;
    return false;
}

static void reset_parser(forth_state_t* fs) {
    fs->input_source_refill = no_input_source;
    fs->input_buffer_size = 0;
    fs->parse_area_offset = 0;
    fs->input_source = NULL;
    fs->source_id = 0;
}

static void compile_system_forth_words(forth_state_t* fs) {
    (void) fs;
    extern const char* core_forth_words;
    sef_parse_string(fs, core_forth_words);
    // The pre-processor would corrupt comments definitions
    sef_parse_string(fs, ": ( [char] ) parse 2drop ; immediate");
    sef_parse_string(fs, ": \\ 10 parse 2drop ; immediate");
#if SEF_PROGRAMMING_TOOLS
    extern const char* tools_forth_words;
    sef_parse_string(fs, tools_forth_words);
#endif
#if SEF_ARG_AND_EXIT_CODE
    extern const char* arg_and_exit_code_forth_words;
    sef_parse_string(fs, arg_and_exit_code_forth_words);
#endif
#if SEF_PROGRAMMING_TOOLS && SEF_ARG_AND_EXIT_CODE
    sef_parse_string(fs, ": (bye) exit-code ! bye ;");
#endif
}

// Init the interpreter
void sef_state_init(forth_state_t* fs) {
    fs->here.byte = &fs->forth_memory[0];
    fs->last_dictionary_entry = NULL;
    fs->data_stack_index = 0;
    fs->code_stack_index = 0;
    fs->control_flow_stack_index = 0;
    fs->compiling = false;
    fs->base = 10;
    fs->code_pointer = NULL;
    fs->error_encountered = false;
    fs->bye = false;
    reset_parser(fs);
    fs->compiling_system_words = true;
    sef_register_default_cfunc(fs);
    sef_register_parser_cfunc(fs);
    compile_system_forth_words(fs);
    fs->compiling_system_words = false;
}

/* --------------------------- Stack manipulation --------------------------- */

#if SEF_STACK_BOUND_CHECKS
#define STACK_BOUND_CHECK(fs, stack_name, stack_size)                                         \
    if (fs->stack_name ## _stack_index < 0 || fs->stack_name ## _stack_index >= stack_size) { \
        SEF_ERROR_OUT(fs, "Stack '%s' out of bound. Resetting state.\n", #stack_name);        \
    }                                                                                          
#else
#define STACK_BOUND_CHECK(fs, stack_name, stack_size)
#endif

#define STACK_OPERATIONS(stack_name, stack_size)                                \
void sef_push_ ## stack_name(forth_state_t* fs, sef_int_t data) {               \
    fs->stack_name ## _stack[fs->stack_name ## _stack_index++] = data;          \
    STACK_BOUND_CHECK(fs, stack_name, stack_size)                               \
}                                                                               \
                                                                                \
sef_int_t sef_pop_ ## stack_name(forth_state_t* fs) {                           \
    sef_int_t ret = fs->stack_name ## _stack[--fs->stack_name ## _stack_index]; \
    STACK_BOUND_CHECK(fs, stack_name, stack_size)                               \
    return ret;                                                                 \
}                                                                                

STACK_OPERATIONS(data, SEF_DATA_STACK_SIZE)
STACK_OPERATIONS(code, SEF_CODE_STACK_SIZE)
STACK_OPERATIONS(control_flow, SEF_CONTROL_FLOW_STACK_SIZE)

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
    (void) fs;
    error_msg("Stack trace TODO!\n");
}
#endif

// Trigerred on error. Do as quit but also reset data stack and set error flag.
void sef_abort(forth_state_t* fs) {
#if SEF_STACK_TRACE
    //TODO: Maybe I should separate the error case and the normal case...
    sef_stack_trace(fs);
#endif
    sef_quit(fs);
    fs->data_stack_index = 0;
    fs->error_encountered = true;
}

// Put the state back in working order, clears bye and error flag. Reset stacks
// and parsers.
void sef_reset(forth_state_t* fs) {
    sef_quit(fs);
    fs->error_encountered = false;
    fs->bye = false;
    fs->data_stack_index = 0;
}

/* ----------------------------- Word execution ----------------------------- */

// Executes the code pointer. Return true if it was executed and false if it
// wasn't because it is NULL.
static bool sef_execute_code_pointer(forth_state_t* fs) {
    if (fs->code_pointer == NULL) {
        return false;
    }

    sef_int_t entry = *fs->code_pointer;
    sef_call_entry(fs, (dictionary_entry_t) entry);
    if (fs->code_pointer != NULL) { // If we aborted, we don't want to increament the code pointer
        fs->code_pointer += 1;
    }
    return true;
}

void sef_run(forth_state_t* fs) {
    while (sef_execute_code_pointer(fs));
}

void sef_exit(forth_state_t* fs) {
    fs->code_pointer = (sef_int_t*) sef_pop_code(fs);
}

#if SEF_CATCH_SEGFAULTS
static void _sef_call_entry(forth_state_t* fs, dictionary_entry_t entry) {
#else
void sef_call_entry(forth_state_t* fs, dictionary_entry_t entry) {
#endif
    debug_msg("Calling %s\n", sef_get_entry_name(entry));
    sef_int_t word_tags = *(sef_get_word_tag_field(entry));
    void* parameters = sef_get_entry_parameter(entry);
    word_executing_function wef = *(sef_get_word_executing_function(entry));

    // TODO: consider the proposition about this in the design choices document.
    if (word_tags & WTM_DOES_EXECUTION) {
        sef_int_t* new_code_pointer = (sef_int_t*) wef;
        sef_push_code(fs, (sef_int_t) fs->code_pointer);
        sef_push_data(fs, (sef_int_t) parameters);
        if (fs->code_pointer != NULL) {
            fs->code_pointer = new_code_pointer; // Currently pointing to `DOES>`, but will be shifted to what we cant to execute by the run word function.
        } else {
            fs->code_pointer = new_code_pointer + 1;
        }
    } else {
        // TODO: segfault catching
        wef(fs, parameters);
    }
}

#if SEF_CATCH_SEGFAULTS
#include <setjmp.h>
#include <signal.h>
#define UNUSED(x) (void)(x)

static sigjmp_buf point;
static void segfault_handler(int sig, siginfo_t *dont_care, void *dont_care_either) {
    UNUSED(sig);
    UNUSED(dont_care);
    UNUSED(dont_care_either);
    longjmp(point, 1);
}

void sef_call_entry(forth_state_t* fs, dictionary_entry_t entry) {
    // Prepare catching of segfaults
    struct sigaction sa;
    memset(&sa, 0, sizeof(sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags     = SA_NODEFER;
    sa.sa_sigaction = segfault_handler;
    sigaction(SIGSEGV, &sa, NULL);

// Execute the risky code
    if (setjmp(point) == 0) {
        _sef_call_entry(fs, entry);
    } else {
        const char* entry_name = sef_get_entry_name(entry);
        SEF_ERROR_OUT(fs, "SEGFAULT while executing word %s.\n", entry_name);
    }

    // Stop the fault catcher
    sa.sa_sigaction = NULL;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGSEGV, &sa, NULL);
}
#endif

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
    fs->here.byte += byte_requested;
#if SEF_STACK_BOUND_CHECKS
    if (fs->here.byte - &fs->forth_memory[0] > SEF_FORTH_MEMORY_SIZE) {
        SEF_ERROR_OUT(fs, "Forth memory overflowed by %i bytes.\n", (fs->here.byte - fs->forth_memory) - SEF_FORTH_MEMORY_SIZE);
    }
#endif
}


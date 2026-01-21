£include <stdbool.h>

£define SPACE_TO_ADD_TO_ENSURE_ALIGNMENT(base_size, alignment_size) \\
    (alignment_size - (base_size % alignment_size))

// TODO: do I really need that or should I let the C compiler align everything ?
£define SPACE_ALLIGNED_TO(base_size, alignment_size) \\
    (base_size + SPACE_TO_ADD_TO_ENSURE_ALIGNMENT(base_size, alignment_size))


struct forth_state_s;
typedef bool (*input_source_refill_t)(struct forth_state_s* state, void* input_source);
// Forth state definition. It is not meant to be manipulated directly from outside,
// but it is included here to let SEForth users know the size of the state, to malloc
// it or declare a static instance.
// TODO: Maybe I should have the declaration internal, and in the public API, describe
// it as an array of amf_int_t, with a static assert to ensure both are of the same
// size... In that case, the public function could take the bytes as input and return
// a pointer to the state...
typedef sef_int_t* dictionary_entry_t;
typedef struct forth_state_s {
    // Memory spaces
    uint8_t forth_memory[SPACE_ALLIGNED_TO(SEF_FORTH_MEMORY_SIZE, sizeof(sef_int_t))];
    uint8_t pad[SPACE_ALLIGNED_TO(SEF_PAD_SIZE, sizeof(sef_int_t))];
    sef_int_t data_stack[SEF_DATA_STACK_SIZE];
    sef_int_t code_stack[SEF_CODE_STACK_SIZE];
    sef_int_t control_flow_stack[SEF_CONTROL_FLOW_STACK_SIZE];
    // Memory pointer and indexes
    union {
        uint8_t* byte;
        sef_int_t* cell;
    } here;
    dictionary_entry_t last_dictionary_entry;
    sef_int_t data_stack_index;
    sef_int_t code_stack_index;
    sef_int_t control_flow_stack_index;
    // Internal variables
    bool compiling;
    sef_int_t base;
    sef_int_t* code_pointer;
    bool error_encountered;
    bool bye;
    // Parser
    sef_int_t input_buffer_size;
    char* input_buffer;
    sef_int_t parse_area_offset;
    void* input_source;
    input_source_refill_t input_source_refill;
    sef_int_t source_id;
    bool compiling_system_words;
} forth_state_t;

void sef_init(forth_state_t* fs);

void sef_restart(forth_state_t* state);
bool sef_ready_to_run(forth_state_t* state);
bool sef_asked_bye(forth_state_t* state);
bool sef_is_compiling(forth_state_t* state);

void sef_parse_string(forth_state_t* state, const char* s);

void sef_push_data(forth_state_t* fs, sef_int_t w);
sef_int_t sef_pop_data(forth_state_t* fs);

#ifdef SEF_ARG_AND_EXIT_CODE
void sef_feed_arguments(forth_state_t* state, int argc, char** argv);
int sef_exit_code(forth_state_t* state);
#endif


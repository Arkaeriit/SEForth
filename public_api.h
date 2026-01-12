£include <stdbool.h>

>> This type defines the various errors we might encounter
typedef enum {
    sef_OK = 0,
    sef_no_memory = 1,
    sef_not_found = 2,
    sef_invalid_file = 3,
    sef_impossible_error = 4,
    sef_segfault = 5,
    sef_config_error = 6,
} sef_error;

#define SPACE_TO_ADD_TO_ENSURE_ALIGNMENT(base_size, alignment_size) \
    (alignment_size - (base_size % alignment_size))

// TODO: do I really need that or should I let the C compiler align everything ?
#define SPACE_ALLIGNED_TO(base_size, alignment_size) \
    (base_size + SPACE_TO_ADD_TO_ENSURE_ALIGNMENT(base_size, alignment_size))

// Forth state definition. It is not meant to be manipulated directly from outside,
// but it is included here to let SEForth users know the size of the state, to malloc
// it or declare a static instance.
// TODO: Maybe I should have the declaration internal, and in the public API, describe
// it as an array of amf_int_t, with a static assert to ensure both are of the same
// size... In that case, the public function could take the bytes as input and return
// a pointer to the state...
typedef struct forth_state_s {
    // Memory spaces
    uint8_t forth_memory[SPACE_ALLIGNED_TO(SEF_FORTH_MEMORY_SIZE, sizeof(sef_int_t))];
    uint8_t pad[SPACE_ALLIGNED_TO(SEF_PAD_SIZE, sizeof(sef_int_t))];
    sef_int_t data_stack[SEF_DATA_STACK_SIZE];
    sef_int_t code_stack[SEF_CODE_STACK_SIZE];
    sef_int_t control_flow_stack[SEF_CONTROL_FLOW_STACK];
    // Memory pointer and indexes
    uint8_t* here;
    last_dictionary_entry last_dictionary_entry;
    sef_int_t data_stack_index;
    sef_int_t code_stack_index;
    sef_int_t control_flow_stack_index;
    // Internal variables
    bool compiling;
    sef_int_t base;
    sef_int_t* code_pointer;
    bool error_encountered;
    // Parser
    sef_int_t input_buffer_size;
    char* input_buffer;
    sef_int_t parse_area_offset;
    void* input_source;
    bool (*input_source_refill)(struct forth_state_s* state, void* input_source);
#if SEF_CASE_INSENSITIVE == 0
    bool register_both_cases;
#endif
} forth_state_t;

forth_state_t* sef_init(void);
void sef_free(forth_state_t* state);

void sef_restart(forth_state_t* state);
bool sef_is_running(forth_state_t* state);
bool sef_asked_bye(forth_state_t* state);
bool sef_is_compiling(forth_state_t* state);

void sef_parse_string(forth_state_t* state, const char* s);
void sef_parse_char(forth_state_t* state, char c);

void sef_push_data(forth_state_t* fs, sef_int_t w);
sef_int_t sef_pop_data(forth_state_t* fs);
void sef_quit(forth_state_t* fs);
void sef_abort(forth_state_t* fs);

#if SEF_USE_SOURCE_FILE
sef_error sef_parse_file(forth_state_t* state, const char* filename);
#endif

#ifdef SEF_PROGRAMMING_TOOLS
void sef_feed_arguments(forth_state_t* state, int argc, char** argv);
int sef_exit_code(const forth_state_t* state);
#endif


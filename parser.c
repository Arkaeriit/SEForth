#include "parser.h"
#include "string.h"

static void exec_forth_word(forth_state_t* fs, void* parameter) {
    sef_int_t* first_sub_word = (sef_int_t*) parameter;
    sef_push_code(fs, (sef_int_t) fs->code_pointer);
    if (fs->code_pointer == NULL) {
        fs->code_pointer = first_sub_word; // Executing from the interpreting/compiling state, we want to start executing the given word.
    } else {
        fs->code_pointer = first_sub_word - 1; // From the executing state, sef_execute_code_pointer will increment the code pointer so we pro-actively decrement it.
    }
}

static void inter_compil_entry(forth_state_t* fs, dictionary_entry_t entry);
static void inter_compil_number(forth_state_t* fs, sef_int_t number);

// Add the NULL-terminated word to the current definition. Must be called from
// compilation context with a run-time word.
// If not called from compilation context, will call the given word.
static void add_word_to_current_definition(forth_state_t* fs, const char* word) {
        dictionary_entry_t entry = sef_find_entry(fs, word, strlen(word));
        inter_compil_entry(fs, entry);
}

/* ------------------------- Input source management ------------------------ */

#define CELL_USED_FOR_INPUT_SOURCE 6
#define MIN(a, b) ((a) > (b) ? (b) : (a))

void sef_push_input_source(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) fs->source_id);
    sef_push_data(fs, (sef_int_t) fs->input_buffer_size);
    sef_push_data(fs, (sef_int_t) fs->input_buffer);
    sef_push_data(fs, (sef_int_t) fs->parse_area_offset);
    sef_push_data(fs, (sef_int_t) fs->input_source);
    sef_push_data(fs, (sef_int_t) fs->input_source_refill);
    sef_push_data(fs, CELL_USED_FOR_INPUT_SOURCE);
}

void sef_pop_input_source(forth_state_t* fs) {
    sef_int_t input_source_storage[CELL_USED_FOR_INPUT_SOURCE];
    sef_int_t cells_in_stack = sef_pop_data(fs);

    for (int i=0; i<MIN(cells_in_stack, CELL_USED_FOR_INPUT_SOURCE); i++) {
        input_source_storage[i] = sef_pop_data(fs);
    }
    for (int i=MIN(cells_in_stack, CELL_USED_FOR_INPUT_SOURCE); i<cells_in_stack; i++) {
        sef_pop_data(fs);
    }

    if (cells_in_stack == CELL_USED_FOR_INPUT_SOURCE) {
        fs->input_source_refill = (input_source_refill_t) input_source_storage[0];
        fs->input_source = (void*) input_source_storage[1];
        fs->parse_area_offset = input_source_storage[2];
        fs->input_buffer = (char*) input_source_storage[3];
        fs->input_buffer_size = input_source_storage[4];
        fs->source_id = input_source_storage[5];
        // TODO: bool
    } else {
        // TODO: bool
        sef_push_data(fs, 0);
    }
}

static void refill(forth_state_t* fs) {
    bool refill_rc = fs->input_source_refill(fs, fs->input_source);
    fs->parse_area_offset = 0;
    // TODO C bool to Forth bool
    sef_push_data(fs, refill_rc);
}

/* -------------------------- C string input source ------------------------- */

static bool c_string_refill(forth_state_t* fs, void* input_source) {
    char* left_to_fill = ((char*) input_source) + fs->input_buffer_size;
    size_t chars_left = strlen(left_to_fill);
    if (chars_left == 0) {
        return false;
    }
    while (*left_to_fill == '\n') {
        left_to_fill++;
    }
    fs->input_buffer = left_to_fill;
    fs->input_source = left_to_fill;
    fs->input_buffer_size = 0;
    for (size_t i=0; i<strlen(fs->input_buffer); i++) {
        if (fs->input_buffer[i] == '\n') {
            break;
        }
        fs->input_buffer_size++;
    }
    return true;
}

void sef_set_c_string_as_input_source(forth_state_t* fs, const char* str) {
    fs->input_source = (char*) str; // I won't edit it with c_string refill, so no biggie with const.
    fs->input_buffer_size = 0;
    fs->parse_area_offset = 0;
    fs->input_source_refill = c_string_refill;
    fs->source_id = 0;
}

/* -------------------- Forth string input (for evaluate) ------------------- */

// Forth string as parsed as a single buffer, and don't need refill.
// But we still want to perform one refill to kickstart the parsing loop.
static bool forth_string_refill(forth_state_t* fs, void* input_source) {
    if (fs->input_buffer == NULL) {
        fs->input_buffer = input_source;
        return true;
    } else {
        return false;
    }
}

// The forth string is assumed to be on the stack
static void set_forth_string_as_input_source(forth_state_t* fs) {
    size_t str_len = (size_t) sef_pop_data(fs);
    char* str = (char*) sef_pop_data(fs);
    debug_msg("Evaluating '%.*s'.\n", str_len, str);
    sef_push_input_source(fs);
    fs->input_source = str;
    fs->input_buffer_size = str_len;
    fs->parse_area_offset = 0;
    fs->input_buffer = NULL;
    fs->input_source_refill = forth_string_refill;
    fs->source_id = -1;
}

static void evaluate(forth_state_t* fs) {
    set_forth_string_as_input_source(fs);
    sef_int_t cells_to_save = sef_pop_data(fs);
    for (int i=0; i<cells_to_save; i++) {
        sef_push_code(fs, sef_pop_data(fs));
    }
    sef_push_code(fs, (sef_int_t) fs->code_pointer);
    fs->code_pointer = NULL;
    sef_inter_compil_run(fs);
    fs->code_pointer = (dictionary_entry_t) sef_pop_code(fs);
    for (int i=0; i<cells_to_save; i++) {
        sef_push_data(fs, sef_pop_code(fs));
    }
    sef_push_data(fs, cells_to_save);
    sef_pop_input_source(fs);
}


/* ------------------------------ Parsing words ----------------------------- */

static void parse(forth_state_t* fs) {
    // TODO: test if there is nothing to parse such as in ":\n name". It should
    // fail gracefully.
    char delimiter = sef_pop_data(fs);
    char* content = NULL;
    sef_int_t content_size = 0;
    while (fs->parse_area_offset < fs->input_buffer_size) {
        bool found_content = content != NULL;
        bool on_delimiter = fs->input_buffer[fs->parse_area_offset] == delimiter;
        if (!found_content && on_delimiter) { // We haven't reach the content
            // Nothing to do
        } else if (!found_content && !on_delimiter) { // Starting to find interesting content
            content = fs->input_buffer + fs->parse_area_offset;
            content_size = 1;
        } else if (found_content && on_delimiter) { // End of the value to parse
            fs->parse_area_offset++;
            break;
        } else { // Simply parsing the content
            content_size++;
        }
        fs->parse_area_offset++;
    }
    debug_msg("Parsed `%.*s` of size %i.\n", (int) content_size, content, (int) content_size);
    sef_push_data(fs, (sef_int_t) content);
    sef_push_data(fs, content_size);
}

static void parse_name(forth_state_t* fs) {
    sef_push_data(fs, ' ');
    parse(fs);
}

/* ----------------------------- Compiling words ---------------------------- */

static void enter_compilation(forth_state_t* fs) {
    if (fs->compiling) {
        SEF_ERROR_OUT(fs, "Trying to enter compiling while already compiling.\n");
    }
    fs->compiling = true;
}

static void leave_compilation(forth_state_t* fs) {
    if (!fs->compiling) {
        SEF_ERROR_OUT(fs, "Trying to leave compiling while not compiling.\n");
    }
    fs->compiling = false;
}

// Part of the colon-sys used to ensure stack integrity during compilation
#define COLON_SYS_MAGIC 0x5EF
static void new_forth_word(forth_state_t* fs, const char* name, size_t name_len) {
    sef_push_data(fs, COLON_SYS_MAGIC);
    enter_compilation(fs);
    sef_register_new_word(fs, name, name_len, exec_forth_word);
}

static void colon(forth_state_t* fs) {
    parse_name(fs);
    size_t name_len = (size_t) sef_pop_data(fs);
    char* name = (char*) sef_pop_data(fs);
    new_forth_word(fs, name, name_len);
}

static void no_name(forth_state_t* fs) {
    new_forth_word(fs, "", 0);
    sef_int_t magic = sef_pop_data(fs);
    sef_push_data(fs, (sef_int_t) fs->last_dictionary_entry);
    sef_push_data(fs, magic);
}

static void semicolon(forth_state_t* fs) {
    add_word_to_current_definition(fs, "exit");
    leave_compilation(fs);
    sef_int_t magic = sef_pop_data(fs);
    if (magic != COLON_SYS_MAGIC) {
        SEF_ERROR_OUT(fs, "Imbalanced stack when compiling \"%s\".\n", sef_get_entry_name(fs->last_dictionary_entry));
    }
}

static void literal(forth_state_t* fs) {
    sef_int_t number = *(++fs->code_pointer);
    sef_push_data(fs, number);
}

static void immediate(forth_state_t* fs) {
    sef_int_t* tag_field = sef_get_word_tag_field(fs->last_dictionary_entry);
    *tag_field |= WTM_IMMEDIATE;
}

static void does(forth_state_t* fs) {
    sef_int_t* tag_field = sef_get_word_tag_field(fs->last_dictionary_entry);
    *tag_field |= WTM_DOES_EXECUTION;
    sef_int_t* wef_field = tag_field + 1; // I have to change this as well as the dictionary if I ever change entry layout
    *wef_field = (sef_int_t) (fs->code_pointer);
    sef_exit(fs); // We don't want to execute what is made for the other word.
}

/* --------------------------------- Create --------------------------------- */

static void exec_create(forth_state_t* fs, void* parameter) {
    sef_push_data(fs, (sef_int_t) parameter);
}

static void create(forth_state_t* fs) {
    parse_name(fs);
    size_t name_len = (size_t) sef_pop_data(fs);
    char* name = (char*) sef_pop_data(fs);
    sef_register_new_word(fs, name, name_len, exec_create);
}

/* --------------------------- Control-flow words --------------------------- */

// TODO: Maybe add a check that we are in a word definition for all those words.

static void if_compile_time(forth_state_t* fs) {
    // We start by adding an empty number, it will be edited by else or then.
    inter_compil_number(fs, 0);
    // We push the address of the number to fill in in the control flow stack.
    sef_push_control_flow(fs, (sef_int_t) (fs->here.cell - 1));
    // Then we add the runtime word
    add_word_to_current_definition(fs, "(if)");
}

static void else_compile_time(forth_state_t* fs) {
    // Pop the address of the if-targeted address.
    sef_int_t* empty_cell = (sef_int_t*) sef_pop_control_flow(fs);
    // Add an empty number to the control flow stack to target THEN
    inter_compil_number(fs, 0);
    sef_push_control_flow(fs, (sef_int_t) (fs->here.cell - 1));
    // Fill in the empty address with where we are putting (else)
    *empty_cell = (sef_int_t) fs->here.cell;
    debug_msg("Else wrote in if the value 0x%lX.\n", (long) *empty_cell);
    add_word_to_current_definition(fs, "(else)");
}

static void then(forth_state_t* fs) {
    // For THEN, we don't need to put in a word, as then does nothing.
    // We only need to fill-in the address for if or else.
    // But we need to decrease it by one so that the end-of-execution for
    // (if) or (else) make it point to the correct sub-word.
    sef_int_t* empty_cell = (sef_int_t*) sef_pop_control_flow(fs);
    *empty_cell = (sef_int_t) (fs->here.cell - 1);
    debug_msg("Then wrote in if the value 0x%lX.\n", (long) *empty_cell);
}

static void begin(forth_state_t* fs) {
    // Begin does nothing when executing so we to push the address of HERE which
    // will be used by WHILE and REPEAT
    sef_push_control_flow(fs, (sef_int_t) (fs->here.cell - 1)); // -1 as there won't be a runtime begin
}

static void while_compile_time(forth_state_t* fs) {
    sef_int_t begin_address = sef_pop_control_flow(fs);
    // While needs a black address that will be filled-in by repeat
    inter_compil_number(fs, 0);
    sef_push_control_flow(fs, (sef_int_t) (fs->here.cell - 1));
    sef_push_control_flow(fs, begin_address);
    add_word_to_current_definition(fs, "(while)");
}

static void repeat_compile_time(forth_state_t* fs) {
    sef_int_t begin_address = sef_pop_control_flow(fs);
    sef_int_t* while_empty_cell = (sef_int_t*) sef_pop_control_flow(fs);

    // Putting the address of begin as a literal, the runtime repeat will go to it.
    inter_compil_number(fs, begin_address);
    // Filling in the blank word from while with repeat address
    *while_empty_cell = (sef_int_t) (fs->here.cell);
    add_word_to_current_definition(fs, "(repeat)");

}

// For do loop, the loop-sys will be ( LOOP address, end value, current value )
static void question_do_compile_time(forth_state_t* fs) {
    // Getting address of the closing +loop to bail out there if needed
    inter_compil_number(fs, 0);
    sef_push_control_flow(fs, (sef_int_t) (fs->here.cell - 1));
    add_word_to_current_definition(fs, "(?do)");
}

static void question_do_run_time(forth_state_t* fs) {
    // It might be easier to that from Forth...
    sef_int_t end_of_loop_pointer = sef_pop_data(fs);
    sef_int_t loop_counter = sef_pop_data(fs);
    sef_int_t end_value = sef_pop_data(fs);
    // ?do can jump to the end
    if (end_value == loop_counter) {
        fs->code_pointer = (dictionary_entry_t) end_of_loop_pointer;
        return;
    }
    // Otherwise, we prepare the loop-sys
    sef_push_code(fs, end_of_loop_pointer);
    sef_push_code(fs, end_value);
    sef_push_code(fs, loop_counter);
}

static void plus_loop_compile_time(forth_state_t* fs) {
    sef_int_t* end_of_loop_pointer = (sef_int_t*) sef_pop_control_flow(fs);
    sef_int_t* question_do_address = end_of_loop_pointer + 1;
    inter_compil_number(fs, (sef_int_t) question_do_address);
    *end_of_loop_pointer = (sef_int_t) fs->here.cell;
    add_word_to_current_definition(fs, "(+loop)");
}

// (+loop) takes as argument ( increment-value begining-of-the-loop-pointer )
static void plus_loop_run_time(forth_state_t* fs) {
    sef_int_t question_do_address = sef_pop_data(fs);
    sef_int_t increment = sef_pop_data(fs);
    sef_int_t loop_counter = sef_pop_code(fs);
    sef_int_t end_value = sef_pop_code(fs);
    loop_counter += increment;
    if ((increment > 0 && loop_counter >= end_value) || (increment < 0 && (loop_counter < end_value))) { // TODO: I'm not sure if it should be < or <= in the negative case
        sef_pop_code(fs); // End of loop address
    } else {
        sef_push_code(fs, end_value);
        sef_push_code(fs, loop_counter);
        fs->code_pointer = (dictionary_entry_t) question_do_address;
    }
}

static void leave(forth_state_t* fs) {
    sef_pop_code(fs);
    sef_pop_code(fs);
    dictionary_entry_t end_off_loop = (dictionary_entry_t) sef_pop_code(fs);
    fs->code_pointer = end_off_loop;
}

static void case_compile_time(forth_state_t* fs) {
    // Case only pushes a case-sys. Which is the number of endof clauses on top
    // followed by one address for each of them.
    sef_push_control_flow(fs, 0);
}

static void of_compile_time(forth_state_t* fs) {
    inter_compil_number(fs, 0);
    sef_push_control_flow(fs, (sef_int_t) (fs->here.cell - 1));
    add_word_to_current_definition(fs, "(of)");
}

static void of_run_time(forth_state_t* fs) {
    dictionary_entry_t endof_pointer = (dictionary_entry_t) sef_pop_data(fs);
    sef_int_t reference = sef_pop_data(fs);
    sef_int_t element = sef_pop_data(fs);
    if (reference != element) {
        sef_push_data(fs, element);
        fs->code_pointer = endof_pointer;
    }
}

static void endof_compile_time(forth_state_t* fs) {
    sef_int_t* of_pointer = (sef_int_t*) sef_pop_control_flow(fs);
    sef_int_t number_of_cases = sef_pop_control_flow(fs);
    // Putting the address of endcase as a literal and putting it on the stack
    inter_compil_number(fs, 0);
    sef_push_control_flow(fs, (sef_int_t) (fs->here.cell - 1));
    sef_push_control_flow(fs, number_of_cases+1);
    // Runtime effect
    add_word_to_current_definition(fs, "(endof)");
    // Filling in the address for the of word
    *of_pointer = (sef_int_t) (fs->here.cell - 1);
}

static void endof_run_time(forth_state_t* fs) {
    dictionary_entry_t endcase_pointer = (dictionary_entry_t) sef_pop_data(fs);
    fs->code_pointer = endcase_pointer;
}

static void endcase(forth_state_t* fs) {
    // Filling in all the addresses for the endofs
    sef_int_t number_of_cases = sef_pop_control_flow(fs);
    for (sef_int_t i=0; i<number_of_cases; i++) {
        sef_int_t** endcase_pointer = (sef_int_t**) sef_pop_control_flow(fs);
        *endcase_pointer = fs->here.cell;
    }
    // The runtime is the same as drop
    add_word_to_current_definition(fs, "drop");
}

/* -------------------------------- Postpone -------------------------------- */

// Convert a sting to a number. Return true if it worked. As strtol stops at
// the first non valid char, we can use it even if the string is not
// null-terminated, as we can expect to have some random non-number data after.
// TODO: fix cases like "70abcd" being parsed as "70". Maybe feed-in str len.
static bool str_to_num(const char* str, sef_int_t* num, int base) {
    char* end;
    *num = strtol(str, &end, base);
    return end != str;
}

static void postpone_compile_time(forth_state_t* fs) {
    parse_name(fs);
    size_t name_len = (size_t) sef_pop_data(fs);
    char* name = (char*) sef_pop_data(fs);

    dictionary_entry_t entry = sef_find_entry(fs, name, name_len);
    if (entry != NULL) {
        inter_compil_number(fs, (sef_int_t) entry);
        add_word_to_current_definition(fs, "(postpone)");
    } else {
        SEF_ERROR_OUT(fs, "Error can't parse '%.*s' with POSTPONE.\n", name_len, name);
    }
}

// Takes from the stack one entry.
// If it is immediate, execute it with state as compiling.
// If it is not, add it to the current definition.
static void postpone_runtime(forth_state_t* fs) {
    dictionary_entry_t entry = (dictionary_entry_t) sef_pop_data(fs);
    sef_int_t tags = *(sef_get_word_tag_field(entry));
    if (tags & WTM_IMMEDIATE) {
        bool old_state = fs->compiling;
        fs->compiling = true;
        sef_call_entry(fs, entry);
        if (fs->compiling) {
            fs->compiling = old_state;
        }
    } else {
        *fs->here.cell++ = (sef_int_t) entry;
    }
}

/* ---------------------- Exporting compile time words ---------------------- */

struct c_func_s {
    const char* name;
    void (*func)(forth_state_t*);
    bool immediate;
};

struct c_func_s all_default_parser_c_func[] = {
    {"parse", parse, false},
    {"parse-name", parse_name, false},
    {"evaluate", evaluate, false},

    {"[", leave_compilation, true},
    {"]", enter_compilation, false},
    {":", colon, false},
    {";", semicolon, true},
    {":noname", no_name, false},
    {"(literal)", literal, false},
    {"immediate", immediate, false},
    {"does>", does, false},
    {"create", create, false},

    {"if", if_compile_time, true},
    {"else", else_compile_time, true},
    {"then", then, true},
    {"begin", begin, true},
    {"while", while_compile_time, true},
    {"repeat", repeat_compile_time, true},
    {"?do", question_do_compile_time, true},
    {"(?do)", question_do_run_time, false},
    {"+loop", plus_loop_compile_time, true},
    {"(+loop)", plus_loop_run_time, false},
    {"leave", leave, false},
    {"case", case_compile_time, true},
    {"of", of_compile_time, true},
    {"(of)", of_run_time, false},
    {"endof", endof_compile_time, true},
    {"(endof)", endof_run_time, false},
    {"endcase", endcase, true},

    {"refill", refill, false},
    {"save-input", sef_push_input_source, false},
    {"restore-input", sef_pop_input_source, false},

    {"postpone", postpone_compile_time, true},
    {"(postpone)", postpone_runtime, false},
};

void sef_register_parser_cfunc(forth_state_t* fs) {
    for (size_t i = 0; i < sizeof(all_default_parser_c_func) / sizeof(struct c_func_s); i++) {
        const char* name = all_default_parser_c_func[i].name;
        sef_register_cfunc(fs, name, all_default_parser_c_func[i].func, all_default_parser_c_func[i].immediate);
    }
}

/* ------------------------ Compile/Interpret routine ----------------------- */

// Handle compilation of interpretation of a word found in the dictionary.
static void inter_compil_entry(forth_state_t* fs, dictionary_entry_t entry) {
    sef_int_t tags = *(sef_get_word_tag_field(entry));
    bool should_execute = (!fs->compiling) || (tags & WTM_IMMEDIATE);
    if (should_execute) {
        sef_call_entry(fs, entry);
        sef_run(fs);
    } else {
        *fs->here.cell = (sef_int_t) entry;
        sef_allot_cell(fs);
    }
}

// Handle compilation of interpretation of a number.
static void inter_compil_number(forth_state_t* fs, sef_int_t number) {
    if (fs->compiling) {
        // TODO: replace all finds as here with a cache
        add_word_to_current_definition(fs, "(literal)");
        *fs->here.cell = number;
        sef_allot_cell(fs);
    } else {
        sef_push_data(fs, number);
    }
}

// Compile a single word. Does nothing if there is nothing in the input buffer.
static void inter_compil_step(forth_state_t* fs) {
    parse_name(fs);
    size_t name_len = (size_t) sef_pop_data(fs);
    char* name = (char*) sef_pop_data(fs);
    if (name_len == 0) {
        return;
    }

    dictionary_entry_t entry = sef_find_entry(fs, name, name_len);
    if (entry != NULL) {
        inter_compil_entry(fs, entry);
        return;
    }

    sef_int_t read_number;
    if (str_to_num(name, &read_number, fs->base)) {
        inter_compil_number(fs, read_number);
        return;
    }

    SEF_ERROR_OUT(fs, "Trying to compile \"%.*s\" which is neither a valid word nor a number.\n", name_len, name);
}

void sef_inter_compil_run(forth_state_t* fs) {
    refill(fs);
    bool refilled = sef_pop_data(fs);
    while (refilled) {
        while (fs->parse_area_offset < fs->input_buffer_size) {
            inter_compil_step(fs);
        }
        refill(fs);
        refilled = sef_pop_data(fs);
    }
    // TODO: maybe pop source as well
}

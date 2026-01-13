#include "parser.h"

static void exec_forth_word(forth_state_t* fs, void* parameter) {
    sef_int_t* first_sub_word = (sef_int_t*) parameter;
    sef_push_code(fs, (sef_int_t) fs->code_pointer);
    fs->code_pointer = first_sub_word - 1;
}

static void inter_compil_entry(forth_state_t* fs, dictionary_entry_t entry);

// Add the NULL-terminated word to the current definition. Must be called from
// compilation context with a run-time word.
// If not called from compilation context, will call the given word.
static void add_word_to_current_definition(forth_state_t* fs, const char* word) {
        dictionary_entry_t entry = sef_find_entry(fs, word, strlen(word));
        inter_compil_entry(fs, entry);
}

/* ------------------------- Input source management ------------------------ */

void sef_push_input_source(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) fs->input_buffer_size);
    sef_push_data(fs, (sef_int_t) fs->input_buffer);
    sef_push_data(fs, (sef_int_t) fs->parse_area_offset);
    sef_push_data(fs, (sef_int_t) fs->input_source);
    sef_push_data(fs, (sef_int_t) fs->input_source_refill);
}

void sef_pop_input_source(forth_state_t* fs) {
    fs->input_source_refill = (input_source_refill_t) sef_pop_data(fs);
    fs->input_source = (void*) sef_pop_data(fs);
    fs->parse_area_offset = sef_pop_data(fs);
    fs->input_buffer = (char*) sef_pop_data(fs);
    fs->input_buffer_size = sef_pop_data(fs);
}

/* -------------------------- C string input source ------------------------- */

static bool c_string_refill(forth_state_t* fs, void* input_source) {
    char* full_input = input_source;
    char* input_left_to_parse = full_input + fs->parse_area_offset;
    size_t chars_left = strlen(input_left_to_parse);
    if (chars_left == 0) {
        return false;
    }
    fs->input_buffer = input_left_to_parse + 1; // +1 as we assume refill was called because a \n was encountered in the input string. So we can safely try to reach the next char to see if it is more interesting than \n
    fs->input_buffer_size = 0;
    for (size_t i=0; i<strlen(fs->input_buffer); i++) {
        if (fs->input_buffer[i] == '\n') {
            return;
        }
        fs->input_buffer_size++;
    }
}

static void refill(forth_state_t* fs) {
    bool refill_rc = fs->input_source_refill(fs, fs->input_source);
    fs->parse_area_offset = 0;
    // TODO C bool to Forth bool
    sef_push_data(fs, refill_rc);
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
        if (!found_content && on_delimiter) { // Check that we started to find something interesting
            found_content = fs->input_buffer + fs->parse_area_offset;
        } else if (!found_content && !on_delimiter) { // Starting to find interesting content
            content = fs->input_buffer + fs->parse_area_offset;
            content_size = 1;
        } else if (found_content && on_delimiter) { // End of the value to parse
            break;
        } else { // Simply parsing the content
            content_size++;
        }
        fs->parse_area_offset++;
    }
    sef_push_data(fs, (sef_int_t) content);
    sef_pop_data(content_size);
}

static void parse_word(forth_state_t* fs) {
    sef_push_data(fs, ' ');
    parse(fs);
}

/* ----------------------------- Compiling words ---------------------------- */

static void leave_compilation(forth_state_t* fs) {
    if (fs->compiling) {
        SEF_ERROR_OUT(fs, "Trying to enter compiling while already compiling.\n");
    }
    fs->compiling = true;
}

static void enter_compilation(forth_state_t* fs) {
    if (!fs->compiling) {
        SEF_ERROR_OUT(fs, "Trying to leave compiling while not compiling.\n");
    }
    fs->compiling = false;
}

// Part of the colon-sys used to ensure stack integrity during compilation
#define COLON_SYS_MAGIC 0x5EF
static void colon(forth_state_t* fs) {
    parse_word(fs);
    char* name = (char*) sef_pop_data(fs);
    size_t name_len = (size_t) sef_pop_data(fs);
    sef_push_data(fs, COLON_SYS_MAGIC);
    enter_compilation(fs);
    sef_register_new_word(fs, name, name_len, exec_forth_word);
}

static void semicolon(forth_state_t* fs) {
    leave_compilation(fs);
    sef_int_t magic = sef_pop_data(fs);
    if (magic != COLON_SYS_MAGIC) {
        SEF_ERROR_OUT(fs, "Imbalanced stack when compiling \"%s\".\n", sef_get_word_name(fs->last_dictionary_entry));
    }
}

/* --------------------------- Control-flow words --------------------------- */

/* ----------------------------- String parsing ----------------------------- */

static void s(forth_state_t* fs) {
    sef_push_data(fs, '"');
    parse(fs);
    char* str = (char*) sef_pop_data(fs);
    size_t str_len = (size_t) sef_pop_data(fs);
    if (fs->compiling) {
        add_word_to_current_definition(fs, "(string)");
        sef_add_strinf_to_current_definition(fs, str, str_len);
    } else {
        sef_call_entry(fs, sef_register_string(fs, str, str_len));
    }
}

// TODO: this could probably be made easier if written from Forth.
static void dot_string(forth_state_t* fs) {
    s(fs);
    const char* type = "type";
    add_word_to_current_definition(fs, type);
}

/* ---------------------- Exporting compile time words ---------------------- */

/* ------------------------ Compile/Interpret routine ----------------------- */

// Convert a sting to a number. Return true if it worked. As strtol stops at
// the first non valid char, we can use it even if the string is not
// null-terminated, as we can expect to have some random non-number data after.
static bool str_to_num(const char* str, sef_int_t* num, int base) {
    char* end;
    *num = strtol(str, &end, base);
    return end != str;
}

// Handle compilation of interpretation of a word found in the dictionary.
static void inter_compil_entry(forth_state_t* fs, dictionary_entry_t entry) {
    sef_int_t tags = *(sef_int_t* sef_get_word_tag_field(entry));
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
    parse_word(fs);
    char* name = (char*) sef_pop_data(fs);
    size_t name_len = (size_t) sef_pop_data(fs);
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
        inter_compil_number(fs, entry);
        return;
    }

    SEF_ERROR_OUT("Trying to compile \"%*s\" which is neither a valid word nor a number.\n", name_len, name);
}

static inter_compil_run(forth_state_t* fs) {
    while (fs->input_source_refill(fs, fs->input_source)) {
        while (fs->parse_area_offset < fs->input_buffer_size) {
            inter_compil_step(fs);
        }
    }
}

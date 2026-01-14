#include "dictionary.h"
#include "string.h"
#include "stdio.h"

/* --------------------------- String manipulation -------------------------- */

#if SEF_CASE_INSENSITIVE
// Change lower case characters in the input string into their upper case versions
static void to_upper(char* str) {
    for (size_t i=0; i<strlen(str); i++) {
        if ('a' <= str[i] && str[i] <= 'z') {
            str[i] = str[i] - ('a' - 'A');
        }
    }
}
#endif

static bool case_insensitive_name_match(const char* name_from_dictionary, const char* outside_name, size_t outside_name_size) {
    if (strlen(name_from_dictionary) != outside_name_size || strlen(name_from_dictionary) == 0) {
        return false;
    }
    for (size_t i=0; i<strlen(name_from_dictionary); i++) {
        char char_from_outside = outside_name[i];
        if ('a' <= char_from_outside && char_from_outside <= 'z') {
            char_from_outside -= 'a' - 'A';
        }
        if (char_from_outside != name_from_dictionary[i]) {
            return false;
        }
    }
    return true;
}

static bool case_sensitive_name_match(const char* name_from_dictionary, const char* outside_name, size_t outside_name_size) {
    if (!name_from_dictionary[0]) { // Empty names should not match because they point to unfindable entries
        return false;
    }
    return (strncmp(name_from_dictionary, outside_name, outside_name_size) == 0) &&
           (strlen(name_from_dictionary) == outside_name_size);
}

/* -------------------------------- Name size ------------------------------- */

size_t sef_size_needed_to_store_string(size_t string_len) {
    size_t effective_size = string_len + 1;
    return SPACE_ALLIGNED_TO(effective_size, sizeof(sef_int_t));
}

/* --------------------------- Writing new entries -------------------------- */

// TODO: To ensure memory safety, I should probably move the allocation before
// the memory use, and return if an error was detected. But Forth and memory
// safety...
// TODO: maybe warning in case of word redefinition.
void sef_register_new_word(forth_state_t* fs, const char* name, size_t name_len, word_executing_function wef) {
    // Storing pointer to previous entry
    dictionary_entry_t new_entry = fs->here.cell;
    *new_entry = (sef_int_t) fs->last_dictionary_entry;
    fs->last_dictionary_entry = new_entry;
    sef_allot_cell(fs);
    // Storing name size
    dictionary_entry_t name_len_field = sef_get_entry_name_len(new_entry);
    *name_len_field = name_len;
    sef_allot_cell(fs);
    // Storing name
    char* name_field = sef_get_entry_name(new_entry);
    memcpy(name_field, name, name_len);
    name_field[name_len] = 0;
    sef_allot(fs, sef_size_needed_to_store_string(name_len));
#if SEF_CASE_INSENSITIVE
    to_upper(name_field);
#endif
    // Storing tags. For now the only one we can be sure of if the sytem word tag.
    sef_int_t* word_tags_field = sef_get_word_tag_field(new_entry);
    *word_tags_field = fs->compiling_system_words ? WTM_SYSTEM_WORD : 0;
    sef_allot_cell(fs);
    // Storing wef
    word_executing_function* wef_field = (word_executing_function*) fs->here.cell;
    *wef_field = wef;
    sef_allot_cell(fs);
}

// Execution function for a string
static void exec_string(forth_state_t* fs, void* parameters) {
   sef_int_t* len_field = parameters;
   sef_int_t* content = len_field + 1;
   sef_push_data(fs, (sef_int_t) content);
   sef_push_data(fs, *len_field);
}
    
dictionary_entry_t sef_register_string(forth_state_t* fs, const char* content, size_t content_len) {
    // Register empty dictionary entry
    sef_register_new_word(fs, "", 0, exec_string);
    sef_add_string_to_current_definition(fs, content, content_len);
    return fs->last_dictionary_entry;
}

void sef_add_string_to_current_definition(forth_state_t* fs, const char* content, size_t content_len) {
    // Write in string size
    sef_int_t* len_field = fs->here.cell;
    *len_field = (sef_int_t) content_len;
    sef_allot_cell(fs);
    // Write in string content
    char* content_in_entry = (char*) fs->here.byte;
    sef_allot(fs, sef_size_needed_to_store_string(content_len));
    memcpy(content_in_entry, content, content_len);
    content_in_entry[content_len] = 0;
}

/* ----------------------------- Reading entries ---------------------------- */

dictionary_entry_t sef_find_entry(forth_state_t* fs, const char* name, size_t name_len) {
    dictionary_entry_t searching = fs->last_dictionary_entry;
    while (searching != NULL) {
        const char* entry_name = sef_get_entry_name(searching);
        sef_int_t* word_tags_field = sef_get_word_tag_field(searching);
        bool (*name_match)(const char*, const char*, size_t) = *word_tags_field & WTM_SYSTEM_WORD || SEF_CASE_INSENSITIVE ? case_insensitive_name_match : case_sensitive_name_match;
        if (name_match(entry_name, name, name_len)) {
            return searching;
        }
        searching = (dictionary_entry_t) (*searching);
    }
    return NULL;
}

sef_int_t* sef_get_entry_name_len(dictionary_entry_t entry) {
    return entry + 1;
}

char* sef_get_entry_name(dictionary_entry_t entry) {
    return (char*) (sef_get_entry_name_len(entry) + 1);
}

sef_int_t* sef_get_word_tag_field(dictionary_entry_t entry) {
    size_t cells_taken_by_name = sef_size_needed_to_store_string(*sef_get_entry_name_len(entry)) / sizeof(sef_int_t);
    sef_int_t* ret = ((sef_int_t*) sef_get_entry_name(entry)) + cells_taken_by_name;
    return ret;
}

word_executing_function* sef_get_word_executing_function(dictionary_entry_t entry) {
    sef_int_t* ret = sef_get_word_tag_field(entry) + 1;
    return (word_executing_function*) ret;
}

void* sef_get_entry_parameter(dictionary_entry_t entry) {
    return (sef_int_t*) sef_get_word_executing_function(entry) + 1;
}

void sef_display_dictionary(forth_state_t* fs) {
    dictionary_entry_t entry = fs->last_dictionary_entry;
    while (entry != NULL) {
        const char* name = sef_get_entry_name(entry);
        if (name[0]) {
            for (size_t i=0; i<strlen(name); i++) {
                sef_output(name[i]);
            }
            sef_output(' ');
        }
        entry = (dictionary_entry_t) (*entry);
    }
}


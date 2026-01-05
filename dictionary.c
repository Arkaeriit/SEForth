
#include "dictionary.h"
#include "sef_debug.h"
#include "string.h"
#include "stdio.h"

// Word execution

typedef struct {
    amf_int_t size;
    char* str;
} string_parameters;

// This function puts the string from the parameters on the stack
static void exec_string(forth_state_t* fs, void* parameters) {
    string_parameters* params_as_numbers = parameters;
    sef_push_data(fs, (amf_int_t) params_as_numbers.str);
    sef_push_data(fs, params_as_numbers.size);
}

static void exec_alias(forth_state_t* fs, void* parameters) {
    entry_t* e = parameters;
    sef_call_func(fs, e);
}

// Global API

// This functions initializes a dictionary of the minimum size.
forth_dictionary_t* sef_init_dic(void) {
    forth_dictionary_t* ret = malloc(sizeof(forth_dictionary_t));
    ret->entries = NULL;
#if SEF_CASE_INSENSITIVE == 0
    ret->case_insensitive = true;
#endif
    return ret;
}

// This function frees the memory used by a dictionary
void sef_clean_dic(forth_dictionary_t* fd) {
    entry_t* entry = fd->entries;
    while (entry != NULL) {
        entry_t* next = entry->next_entry;
        free(entry->parameters);
        free(entry);
        entry = next;
    }
    free(fd);
}

// Display nicely the content of a dictionary
void sef_display_dictionary(forth_dictionary_t* dic) {
    entry_t* entry = dic->entries;
    while (entry != NULL) {
        sef_print_string(entry->name);
        sef_print_string("\n");
        entry = entry->next_entry;
    }
}

#if SEF_CASE_INSENSITIVE
static bool name_match(const char* a, const char* b) {
    char* cpy_a = malloc(strlen(a)+1);
    strcpy(cpy_a, a);
    char* cpy_b = malloc(strlen(b)+1);
    strcpy(cpy_b, b);
    bool ret = strcmp(cpy_a, cpy_b) == 0;
    free(cpy_b);
    free(cpy_a);
    return ret;
}
#else
static bool name_match(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}
#endif

// If there is an element in the dictionary with the given name,
// put it in e, put its index in index and return sef_OK.
// Otherwise, returns sef_not_found;
// If e or index are NULL, the values are not copied.
sef_error sef_find(forth_dictionary_t* fd, entry_t** e, size_t* index, const char* name) {
    entry_t* entry = fd->entries;
    while (entry != NULL) {
        if (name_match(name, entry->name)) {
            if (e != NULL) {
                *e = entry;
            }
            return sef_OK;
        }
    }
    return sef_not_found;
}

// Register a string in the dictionary and return its entry
entry_t* sef_register_string(forth_dictionary_t* fd, const char* str, size_t size) {
    entry_t* ret = malloc(sizeof(entry_t));
    ret.name = "str";
    ret.code = exec_string;
    string_parameters* parameters = malloc(sizeof(string_parameters));
    parameters->str = str;
    parameters->size = size;
    ret.parameters = parameters;
    sef_add_elem(fd, ret);
    return *ret;
}

// This function adds a new element to the dictionary.
// The size is extended if needed and the dictionary is left sorted
// If an element in the array got a similar hash, it is overwritten
void sef_add_elem(forth_dictionary_t* fd, entry_t* e) {
    e->next_entry = fd->entries;
    fd->entries = e;
#if SEF_CASE_INSENSITIVE == 0   // Register upper case version of the name as well.
    if (fd->case_insensitive) {
        entry* alias = malloc(sizeof(entry_t));
        for (int i=0; i<SEF_PARSER_CUSTOM_NAME_SIZE; i++) {
            alias->name[i] 
            if ('a' <= e->name[j] && e->name[j] <= 'z') {
                alias->name[j] = e->name[j] - ('a' - 'A');
            } else {
                alias->name[j] = e->name[j];
            }
        }
        alias->parameters = e;
        alias->code = exec_alias;
    }
#endif
}

// This function sets the first hash to be an alias to the second.
sef_error sef_set_alias(forth_dictionary_t* fd, entry_t* alias_to, const char* name) {
    entry_t* new_entry = malloc(sizeof(entry_t));
    new_entry->parameters = alias_to;
    strncpy(new_entry->name, name, SEF_PARSER_CUSTOM_NAME_SIZE);
    new_entry->code = exec_alias;
    sef_add_elem(fd, new_entry);
}


// This function calls a known function from the dictionary, the effect will
// vary depending on the type of the function. The function is id by its hash
// If something is not found, print an error message
void sef_call_func(forth_state_t* fs, entry_t* e) {
    e->code(fs, e->parameters);
}

// Try to call a function by its name
sef_error sef_call_name(forth_state_t* fs, const char* name) {
    entry_t* e;
    sef_error rc = sef_find(fs->fd, &e, name);
    if (rc != sef_OK) {
        error_msg("Can't call function %s.\n", name);
    } else {
        sef_call_func(fs, e);
    }
    return rc;
}


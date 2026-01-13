#include "private_api.h"
#ifndef DICTIONARY_H
#define DICTIONARY_H

typedef sef_int_t* dictionary_entry_t;

/* --------------------------- Writing new entries -------------------------- */

typedef void (*word_executing_function)(forth_state_t* fs, void* parameters);

// Register a new dictionary entry. As the new entry will be written HERE, no
// need to return any address. Furthermore, it's beginning can be found as the
// last dictionary entry.
void sef_register_new_word(forth_state_t* fs, const char* name, size_t name_size, word_executing_function wef);

// Register a string as an unnamed entry in the dictionary which will be returned.
dictionary_entry_t sef_register_string(forth_state_t* fs, const char* content, size_t content_size);

// Add a string to the current definition and manages alignment of HERE and
// things like that. Only adds the content and not the size so that it can
// also work with counted strings.
void sef_add_string_to_current_definition(forth_state_t* fs, const char* content, size_t content_len);

/* ----------------------------- Reading entries ---------------------------- */

// Return a pointer to an entry. Return NULL and error out if it is not found.
dictionary_entry_t sef_find_entry(forth_state_t* fs, const char* name, size_t name_size);

// Get the various constituent of an entry.
char* sef_get_entry_name(dictionary_entry_t entry);
sef_int_t* sef_get_entry_name_len(dictionary_entry_t entry);
sef_int_t* sef_get_word_tag_field(dictionary_entry_t entry);
word_executing_function* sef_get_word_executing_function(dictionary_entry_t entry);
void* sef_get_entry_parameter(dictionary_entry_t entry);

// Prints all words from the dictionary
void sef_display_dictionary(forth_state_t* fs);

#endif


#include "private_api.h"
#ifndef DICTIONARY_H
#define DICTIONARY_H

/* --------------------------- Writing new entries -------------------------- */

typedef void (*word_executing_function)(forth_state_t* fs, void* parameters);

// Register a new dictionary entry. As the new entry will be written HERE, no
// need to return any address. Furthermore, it's beginning can be found as the
// last dictionary entry.
void sef_register_new_word(forth_state_t* fs, const char* name, size_t name_size, word_executing_function wef);

/* ----------------------------- Reading entries ---------------------------- */

// Return a pointer to an entry. Return NULL and error out if it is not found.
dictionary_entry_t sef_find_entry(forth_state_t* fs, const char* name, size_t name_size);

// Get the various constituent of an entry.
sef_int_t* sef_get_entry_magic(dictionary_entry_t entry);
dictionary_entry_t* sef_get_previous_entry(dictionary_entry_t entry);
char* sef_get_entry_name(dictionary_entry_t entry);
sef_int_t* sef_get_entry_name_len(dictionary_entry_t entry);
sef_int_t* sef_get_word_tag_field(dictionary_entry_t entry);
word_executing_function* sef_get_word_executing_function(dictionary_entry_t entry);
void* sef_get_entry_parameter(dictionary_entry_t entry);

// Tell if a pointer is to an entry
bool sef_is_entry_valid(forth_state_t* fs, dictionary_entry_t entry);

// Prints all words from the dictionary
void sef_display_dictionary(forth_state_t* fs);

#endif


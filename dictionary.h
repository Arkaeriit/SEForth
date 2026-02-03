#include "private_api.h"
#ifndef DICTIONARY_H
#define DICTIONARY_H

/* --------------------------- Writing new entries -------------------------- */

// Register a new dictionary entry. As the new entry will be written HERE, no
// need to return any address. Furthermore, it's beginning can be found as the
// last dictionary entry.
void sef_register_new_word(forth_state_t* fs, const char* name, size_t name_size, sef_int_t default_tags);

/* ----------------------------- Reading entries ---------------------------- */

// Return a pointer to an entry. Return NULL and error out if it is not found.
dictionary_entry_t sef_find_entry(forth_state_t* fs, const char* name, size_t name_size);

// Get the various constituent of an entry.
sef_int_t* sef_get_entry_magic(dictionary_entry_t entry);
dictionary_entry_t* sef_get_previous_entry(dictionary_entry_t entry);
char* sef_get_entry_name(dictionary_entry_t entry);
sef_int_t* sef_get_entry_name_len(dictionary_entry_t entry);
sef_int_t* sef_get_word_tag_field(dictionary_entry_t entry);
sef_int_t* sef_get_entry_special_parameters(dictionary_entry_t entry);
void* sef_get_entry_parameter(dictionary_entry_t entry);

// From a pointer, if it is in the dictionary, try to return the entry it is
// from. Otherwise, return NULL.
dictionary_entry_t sef_try_to_find_entry(forth_state_t* fs, void* p);

// Prints all words from the dictionary
void sef_display_dictionary(forth_state_t* fs);

#endif


#include "private_api.h"
#ifndef DICTIONARY_H
#define DICTIONARY_H

typedef struct entry_s {
    struct entry_s next_entry;
    void (*code)(forth_state_t* fs, void* parameters);
    void* parameters;
    char name[SEF_PARSER_CUSTOM_NAME_SIZE];
} entry_t;

// This structure represent the dictionary. Should be used as a dynamic array. The values should be sorted 
typedef struct forth_dictionary_s {
    entry_t* entries;
#if SEF_CASE_INSENSITIVE == 0
    bool case_insensitive;
#endif
} forth_dictionary_t;

forth_dictionary_t* sef_init_dic(void);
void sef_display_dictionary(forth_dictionary_t* dic);
void sef_clean_dic(forth_dictionary_t* fd);
sef_error sef_find(forth_dictionary_t* fd, entry_t** e, const char* name);
void sef_add_elem(forth_dictionary_t* fd, entry_t e, const char* name);
void sef_set_alias(forth_dictionary_t* fd, entry_t* alias_to, const char* name);
entry_t* sef_register_string(forth_dictionary_t* fd, const char* str, size_t size);

// Deleted
hash_t sef_unused_special_hash(forth_dictionary_t* fd);

sef_error sef_call_name(forth_state_t* fs, const char* name);
void sef_call_func(forth_state_t* fs, entry_t* entry);

#endif


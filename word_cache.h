#ifndef WORD_CACHE_H
#define WORD_CACHE_H

enum word_in_cache {
    // Words defined in C
    QUIT,
    EXIT,
    IF,
    ELSE,
    WHILE,
    REPEAT,
    QUESTION_DO,
    DO,
    PLUS_LOOP,
    LOOP,
    OF,
    ENDOF,
    DROP,
    POSTPONE,
    PAREN_LITERAL,
    // Words defined in forth
    S_TO_D,
    ALIGN,
    REPL,
    // Weird one
    BLOCK_FILE_DATA,

    WORD_IN_CACHE_COUNT,
};

void sef_fill_c_func_in_cache(forth_state_t* fs);
void sef_fill_forth_words_in_cache(forth_state_t* fs);
void sef_add_word_in_cache(forth_state_t* fs, dictionary_entry_t entry, enum word_in_cache word);

// Return an entry from the cache. Return QUIT if the word is not in the cache.
dictionary_entry_t sef_get_word_from_cache(forth_state_t* fs, enum word_in_cache word);

#endif


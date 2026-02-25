#include "private_api.h"

const char* const words_to_cache[WORD_IN_CACHE_COUNT] = {
    [QUIT] = "quit",
    [EXIT] = "exit",
    [IF] = "(if)",
    [ELSE] = "(else)",
    [WHILE] = "(while)",
    [REPEAT] = "(repeat)",
    [QUESTION_DO] = "(?do)",
    [DO] = "(do)",
    [PLUS_LOOP] = "(+loop)",
    [LOOP] = "(loop)",
    [OF] = "(of)",
    [ENDOF] = "(endof)",
    [DROP] = "drop",
    [POSTPONE] = "(postpone)",
    [PAREN_LITERAL] = "(literal)",
    [S_TO_D] = "s>d",
    [ALIGN] = "align",
    [REPL] = "(repl)",
    [BLOCK_FILE_DATA] = "block_file_data",
};

static void automaticaly_add_word_in_cache(forth_state_t* fs, enum word_in_cache word) {
    const char* entry_name = words_to_cache[word];
    dictionary_entry_t entry = sef_find_entry(fs, entry_name, strlen(entry_name));
    if (entry == NULL) {
        SEF_ERROR_OUT(fs, "Can't find word %s for the cache.\n", entry_name);
    }
    sef_add_word_in_cache(fs, entry, word);
}

void sef_add_word_in_cache(forth_state_t* fs, dictionary_entry_t entry, enum word_in_cache word) {
    fs->word_cache[word] = entry;
}

void sef_fill_c_func_in_cache(forth_state_t* fs) {
    automaticaly_add_word_in_cache(fs, QUIT);
    automaticaly_add_word_in_cache(fs, EXIT);
    automaticaly_add_word_in_cache(fs, IF);
    automaticaly_add_word_in_cache(fs, ELSE);
    automaticaly_add_word_in_cache(fs, WHILE);
    automaticaly_add_word_in_cache(fs, REPEAT);
    automaticaly_add_word_in_cache(fs, QUESTION_DO);
    automaticaly_add_word_in_cache(fs, DO);
    automaticaly_add_word_in_cache(fs, PLUS_LOOP);
    automaticaly_add_word_in_cache(fs, LOOP);
    automaticaly_add_word_in_cache(fs, OF);
    automaticaly_add_word_in_cache(fs, ENDOF);
    automaticaly_add_word_in_cache(fs, DROP);
    automaticaly_add_word_in_cache(fs, POSTPONE);
    automaticaly_add_word_in_cache(fs, PAREN_LITERAL);
}

void sef_fill_forth_words_in_cache(forth_state_t* fs) {
    automaticaly_add_word_in_cache(fs, S_TO_D);
    automaticaly_add_word_in_cache(fs, ALIGN);
    automaticaly_add_word_in_cache(fs, REPL);
}


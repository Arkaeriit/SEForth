#include "private_api.h"

#if SEF_BLOCK_FILE

typedef struct {
    FILE* f;
    sef_int_t number_of_blocks;
} block_file_data;

static void go_to_block(FILE* f, sef_int_t block_number) {
    fseek(f, block_number * SEF_BLOCK_SIZE, SEEK_SET);
}

static block_file_data* get_block_file_data(forth_state_t* fs) {
    dictionary_entry_t bfd_entry = sef_get_word_from_cache(fs, BLOCK_FILE_DATA);
    if (bfd_entry == sef_get_word_from_cache(fs, ABORT)) {
        SEF_ERROR_OUT(fs, "Trying to use block file, but block file not registered by API user.\n");
        return NULL;
    }
    sef_call_entry(fs, bfd_entry);

    return (block_file_data*) sef_pop_data(fs);
}

#define BUFFER_ACTION_BOILERPLATE()                                                  \
    forth_state_t* fs = (forth_state_t*) _fs;                                        \
    block_file_data* bfd = get_block_file_data(fs);                                  \
    if (bfd == NULL) {                                                               \
        return;                                                                      \
    }                                                                                \
    if (block_number > bfd->number_of_blocks) {                                      \
        error_msg(                                                                   \
                "Trying to write to block %i which is higher than the max of %i.\n", \
                (int) block_number,                                                  \
                (int) bfd->number_of_blocks);                                        \
        return;                                                                      \
    }                                                                                \
    go_to_block(bfd->f, block_number)                                                 

void sef_write_buffer(sef_forth_state_t* _fs, sef_int_t block_number, const char* data) {
    BUFFER_ACTION_BOILERPLATE();
    fwrite(data, 1, SEF_BLOCK_SIZE, bfd->f);
    fflush(bfd->f);
}

void sef_read_buffer(sef_forth_state_t* _fs, sef_int_t block_number, char* data) {
    BUFFER_ACTION_BOILERPLATE();
    fread(data, 1, SEF_BLOCK_SIZE, bfd->f);
}

static sef_int_t number_of_blocks_already_in_file(FILE* f) {
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    return file_size / SEF_BLOCK_SIZE; 
}

static void add_a_block(block_file_data* bfd) {
    fseek(bfd->f, 0, SEEK_END);
    for (int i=0; i<SEF_BLOCK_SIZE; i++) {
        fputc(0, bfd->f);
    }
    bfd->number_of_blocks++;
    fflush(bfd->f);
}

void sef_register_block_file(sef_forth_state_t* _fs, const char* filename, int number_of_blocks) {
    forth_state_t* fs = (forth_state_t*) _fs;
    sef_call_entry(fs, sef_get_word_from_cache(fs, ALIGN));

    sef_create(fs, "", 0);
    dictionary_entry_t bfd_entry = fs->last_dictionary_entry;
    sef_add_word_in_cache(fs, bfd_entry, BLOCK_FILE_DATA);
    sef_allot(fs, sizeof(block_file_data));
    sef_call_entry(fs, sef_get_word_from_cache(fs, ALIGN));

    block_file_data* bfd = get_block_file_data(fs);
    bfd->f = fopen(filename, "r+b");
    if (bfd->f == NULL) {
        bfd->f = fopen(filename, "w+b");
    }
    if (bfd->f == NULL) {
        SEF_ERROR_OUT(fs, "Can't open block file.\n");
        return;
    }

    bfd->number_of_blocks = number_of_blocks_already_in_file(bfd->f);
    if (bfd->number_of_blocks < number_of_blocks) {
        warn_msg("Block file too small for the required number of blocks. Adding new blocks in it.\n")
        while (bfd->number_of_blocks < number_of_blocks) {
            add_a_block(bfd);
        }
    }
}

#endif


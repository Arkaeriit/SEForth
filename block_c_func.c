#include "private_api.h"

#ifndef SEF_BLOCK_SIZE
#define SEF_BLOCK_SIZE 1024
#endif

#if SEF_BLOCK
void sef_write_buffer(sef_forth_state_t* _fs, sef_int_t block_number, const char* data);
void sef_read_buffer(sef_forth_state_t* _fs, sef_int_t block_number, char* data);

static void write_buffer(forth_state_t* fs) {
    const char* data = (const char*) sef_pop_data(fs);
    sef_int_t block_number = sef_pop_data(fs);
    sef_write_buffer((sef_forth_state_t*) fs, block_number, data);
}

static void read_buffer(forth_state_t* fs) {
    char* data = (char*) sef_pop_data(fs);
    sef_int_t block_number = sef_pop_data(fs);
    sef_read_buffer((sef_forth_state_t*) fs, block_number, data);
}

static void block_size(forth_state_t* fs) {
    sef_push_data(fs, SEF_BLOCK_SIZE);
}

static void number_of_block_buffers(forth_state_t* fs) {
    sef_push_data(fs, SEF_NUMBER_OF_BLOCK_BUFFERS);
}

void sef_register_block_cfunc(forth_state_t* fs) {
    sef_register_cfunc(fs, "block-size",              block_size,              false);
    sef_register_cfunc(fs, "read-buffer",             read_buffer,             false);
    sef_register_cfunc(fs, "write-buffer",            write_buffer,            false);
    sef_register_cfunc(fs, "number-of-block-buffers", number_of_block_buffers, false);
}
#else
void sef_register_block_cfunc(forth_state_t* fs) {
    UNUSED(fs);
}
#endif


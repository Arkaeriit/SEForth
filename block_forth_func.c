#include "private_api.h"

#ifndef SEF_BLOCK_SIZE
#define SEF_BLOCK_SIZE 1024
#endif

#if SEF_BLOCK
void __attribute__((weak)) sef_write_buffer(sef_forth_state_t* _fs, sef_int_t block_number, const char* data) {
    UNUSED(block_number);
    UNUSED(data);
    forth_state_t* fs = (forth_state_t*) _fs;
    SEF_ERROR_OUT(fs, "sef_write_buffer not defined by library user.\n");
}

void __attribute__((weak)) sef_read_buffer(sef_forth_state_t* _fs, sef_int_t block_number, char* data) {
    UNUSED(block_number);
    UNUSED(data);
    forth_state_t* fs = (forth_state_t*) _fs;
    SEF_ERROR_OUT(fs, "sef_read_buffer not defined by library user.\n");
}

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

void sef_register_block_cfunc(forth_state_t* fs) {
    sef_register_cfunc(fs, "block-size",   block_size,   false);
    sef_register_cfunc(fs, "read-buffer",  read_buffer,  false);
    sef_register_cfunc(fs, "write-buffer", write_buffer, false);
}
#else
sef_register_block_cfunc(sef_forth_state_t* fs) {
    UNUSED(fs);
}
#endif


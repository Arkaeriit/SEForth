#include "private_api.h"

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
#endif


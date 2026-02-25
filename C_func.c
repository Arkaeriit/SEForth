#include "sef_debug.h"
#include "C_func.h"
#include "string.h"
#include "stdio.h"

// Functions used to manipulate C_fun

typedef void (*cfunc)(forth_state_t*);

void sef_exec_cfunc(forth_state_t* fs, void* parameters) {
    cfunc* func_field = parameters;
    cfunc func = *func_field;
    func(fs);
}

// Register a new C function
void sef_register_cfunc(forth_state_t* fs, const char* name, cfunc func, bool is_immediate) {
    sef_register_new_word(fs, name, strlen(name), WTM_C_WORD);
    sef_allot_cell(fs);
    sef_int_t* parameters = sef_get_entry_parameter(fs->last_dictionary_entry);
    *parameters = (sef_int_t) func;
    if (is_immediate) {
        sef_int_t* word_tag_field = sef_get_word_tag_field(fs->last_dictionary_entry);
        *word_tag_field |= WTM_IMMEDIATE;
    }
}

// List of default C_func

// Stack manipulation

// swap
static void swap(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2 = sef_pop_data(fs);
    sef_push_data(fs, w1);
    sef_push_data(fs, w2);
}

// rot
static void rot(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2 = sef_pop_data(fs);
    sef_int_t w3 = sef_pop_data(fs);
    sef_push_data(fs, w2);
    sef_push_data(fs, w1);
    sef_push_data(fs, w3);
}

// dup
static void DUP(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_push_data(fs, w1);
    sef_push_data(fs, w1);
}

// drop
static void drop(forth_state_t* fs) {
    sef_pop_data(fs);
}

// r>
static void r_from(forth_state_t* fs) {
    sef_int_t data = sef_pop_return(fs);
    sef_push_data(fs, data);
}

// >r
static void to_r(forth_state_t* fs) {
    sef_int_t w = sef_pop_data(fs);
    sef_push_return(fs, w);
}

// roll
static void roll(forth_state_t* fs) {
    sef_int_t pos = sef_pop_data(fs);
    for (sef_int_t i=0; i<pos; i++) {
        to_r(fs);
    }
    sef_int_t to_top = sef_pop_data(fs);
    for (sef_int_t i=0; i<pos; i++) {
        r_from(fs);
    }
    sef_push_data(fs, to_top);
}

// pick
static void pick(forth_state_t* fs) {
    sef_int_t pos = sef_pop_data(fs);
    sef_push_data(fs, fs->data_stack[fs->data_stack_index - pos - 1]);
}

// depth
static void depth(forth_state_t* fs) {
    sef_push_data(fs, fs->data_stack_index);
}

// Basic maths

// +
static void add(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2 = sef_pop_data(fs);
    sef_push_data(fs, w1 + w2);
}

// -
static void sub(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2 = sef_pop_data(fs);
    sef_push_data(fs, w2 - w1);
}

// I don't really want to do double words. One cell is big enough on modern
// system. To stick with the standard, I'll still have double words on the
// stack, But the'll just Be dumb sign extentions.
#define POP_DOUBLE_WORD(fs, simple_word) \
    sef_pop_data(fs);                    \
    simple_word = sef_pop_data(fs);       

// SM/REM
static void sm_slash_rem(forth_state_t* fs) {
    // C gives symetric division
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2;
    POP_DOUBLE_WORD(fs, w2);
    sef_int_t quotient = w2 / w1;
    sef_int_t rem = w2 % w1;
    sef_push_data(fs, rem);
    sef_push_data(fs, quotient);
}

// FM/MOD
static void fm_slash_mod(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2;
    POP_DOUBLE_WORD(fs, w2);
    sef_int_t quotient = w2 / w1;
    sef_int_t rem = w2 % w1;
    if (w1 * w2 < 0 && rem != 0) {
        rem *= -1;
        rem += rem >= 0 ? 1 : -1;
        quotient += quotient >= 0 ? 1 : -1;
    }
    sef_push_data(fs, rem);
    sef_push_data(fs, quotient);
}

// UM/MOD
static void um_slash_mod(forth_state_t* fs) {
    sef_unsigned_t w1 = (sef_unsigned_t) sef_pop_data(fs);
    sef_unsigned_t w2;
    POP_DOUBLE_WORD(fs, w2);
    sef_unsigned_t quotient = w2 / w1;
    sef_unsigned_t rem = w2 % w1;
    sef_push_data(fs, (sef_int_t) rem);
    sef_push_data(fs, (sef_int_t) quotient);
}

// *
static void mult(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2 = sef_pop_data(fs);
    sef_push_data(fs, w1 * w2);
}

// abs
static void abs_word(forth_state_t* fs) {
    sef_int_t w = sef_pop_data(fs);
    w = w < 0 ? -w : w;
    sef_push_data(fs, w);
}

// <
static void less_than(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2 = sef_pop_data(fs);
    sef_push_data(fs, FORTH_BOOL(w2 < w1));
}

// U<
static void u_less_than(forth_state_t* fs) {
    sef_unsigned_t w1 = (sef_unsigned_t) sef_pop_data(fs);
    sef_unsigned_t w2 = (sef_unsigned_t) sef_pop_data(fs);
    sef_push_data(fs, FORTH_BOOL(w2 < w1));
}

// Boolean logic

// 0<
static void less0(forth_state_t* fs) {
    sef_push_data(fs, FORTH_BOOL(sef_pop_data(fs) < 0));
}

// 0= 
static void eq0(forth_state_t* fs) {
    sef_push_data(fs, FORTH_BOOL(sef_pop_data(fs) == 0));
}

// = 
static void eq(forth_state_t* fs) {
    sef_push_data(fs, FORTH_BOOL(sef_pop_data(fs) == sef_pop_data(fs)));
}

// and
static void and(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2 = sef_pop_data(fs);
    sef_push_data(fs, w1 & w2);
}

// or
static void or(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2 = sef_pop_data(fs);
    sef_push_data(fs, w1 | w2);
}

// xor
static void xor(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2 = sef_pop_data(fs);
    sef_push_data(fs, w1 ^ w2);
}

// lshift
static void lshift(forth_state_t* fs) {
    sef_int_t w1 = sef_pop_data(fs);
    sef_int_t w2 = sef_pop_data(fs);
    sef_push_data(fs, w2 << w1);
}

// rshift
static void rshift(forth_state_t* fs) {
    sef_unsigned_t w1 = (sef_unsigned_t) sef_pop_data(fs);
    sef_unsigned_t w2 = (sef_unsigned_t) sef_pop_data(fs);
    sef_push_data(fs, (sef_int_t) (w2 >> w1));
}

// 2/
static void two_slash(forth_state_t* fs) {
    sef_int_t w1 = 1;
    sef_int_t w2 = sef_pop_data(fs);
    sef_push_data(fs, w2 >> w1);
}

// Flow control

// if
static void if_runtine(forth_state_t* fs) {
    sef_int_t destination_address = sef_pop_data(fs);
    sef_int_t flag = sef_pop_data(fs);
    if (!flag) {
        fs->code_pointer = (sef_int_t*) destination_address;
    }
}

// else
static void else_runtime(forth_state_t* fs) {
    sef_int_t destination_address = sef_pop_data(fs);
    fs->code_pointer = (sef_int_t*) destination_address;
}

// while
static void while_runtime(forth_state_t* fs) {
    dictionary_entry_t repeat_address = (dictionary_entry_t) sef_pop_data(fs);
    sef_int_t flag = sef_pop_data(fs);
    if (!flag) {
        fs->code_pointer = repeat_address;
    }
}

// repeat
static void repeat_runtime(forth_state_t* fs) {
    dictionary_entry_t begin_address = (dictionary_entry_t) sef_pop_data(fs);
    fs->code_pointer = begin_address;
}

static void question_do_run_time(forth_state_t* fs) {
    // It might be easier to that from Forth...
    sef_int_t end_of_loop_pointer = sef_pop_data(fs);
    sef_int_t loop_counter = sef_pop_data(fs);
    sef_int_t end_value = sef_pop_data(fs);
    // ?do can jump to the end
    if (end_value == loop_counter) {
        fs->code_pointer = (dictionary_entry_t) end_of_loop_pointer;
        return;
    }
    // Otherwise, we prepare the loop-sys
    sef_push_return(fs, end_of_loop_pointer);
    sef_push_return(fs, end_value);
    sef_push_return(fs, loop_counter);
}

static void do_run_time(forth_state_t* fs) {
    sef_int_t end_of_loop_pointer = sef_pop_data(fs);
    sef_int_t loop_counter = sef_pop_data(fs);
    sef_int_t end_value = sef_pop_data(fs);
    sef_push_return(fs, end_of_loop_pointer);
    sef_push_return(fs, end_value);
    sef_push_return(fs, loop_counter);
}

// (+loop) takes as argument ( increment-value begining-of-the-loop-pointer )
static void plus_loop_run_time(forth_state_t* fs) {
    sef_int_t question_do_address = sef_pop_data(fs);
    sef_int_t increment = sef_pop_data(fs);
    sef_int_t old_loop_counter = sef_pop_return(fs);
    sef_int_t end_value = sef_pop_return(fs);
    sef_int_t updated_loop_counter = old_loop_counter + increment;

    sef_int_t min_int = ((sef_unsigned_t) ~0 >> 1) + 1;
    sef_int_t check_sign_before = (old_loop_counter - end_value) + min_int;
    sef_int_t check_sign_after = check_sign_before + increment;
    bool overflowed = increment > 0 && check_sign_after < check_sign_before;
    bool underflowed = increment < 0 && check_sign_after > check_sign_before;
    if (overflowed || underflowed) {
        sef_pop_return(fs); // End of loop address
    } else {
        sef_push_return(fs, end_value);
        sef_push_return(fs, updated_loop_counter);
        fs->code_pointer = (dictionary_entry_t) question_do_address;
    }
}

// (loop) takes as argument ( begining-of-the-loop-pointer )
static void loop_run_time(forth_state_t* fs) {
    sef_int_t question_do_address = sef_pop_data(fs);
    sef_int_t loop_counter = sef_pop_return(fs);
    sef_int_t end_value = sef_pop_return(fs);
    loop_counter++;
    if (loop_counter == end_value) {
        sef_pop_return(fs); // End of loop address
    } else {
        sef_push_return(fs, end_value);
        sef_push_return(fs, loop_counter);
        fs->code_pointer = (dictionary_entry_t) question_do_address;
    }
}

static void of_run_time(forth_state_t* fs) {
    dictionary_entry_t endof_pointer = (dictionary_entry_t) sef_pop_data(fs);
    sef_int_t reference = sef_pop_data(fs);
    sef_int_t element = sef_pop_data(fs);
    if (reference != element) {
        sef_push_data(fs, element);
        fs->code_pointer = endof_pointer;
    }
}

static void endof_run_time(forth_state_t* fs) {
    dictionary_entry_t endcase_pointer = (dictionary_entry_t) sef_pop_data(fs);
    fs->code_pointer = endcase_pointer;
}

// Compilation helpers

// (literal)
static void literal(forth_state_t* fs) {
    sef_int_t number = *(++fs->code_pointer);
    sef_push_data(fs, number);
}

// Takes from the stack one entry.
// If it is immediate, execute it with state as compiling.
// If it is not, add it to the current definition.
static void postpone_run_time(forth_state_t* fs) {
    dictionary_entry_t entry = (dictionary_entry_t) sef_pop_data(fs);
    sef_int_t tags = *(sef_get_word_tag_field(entry));
    if (tags & WTM_IMMEDIATE) {
        bool old_state = fs->compiling;
        fs->compiling = true;
        sef_call_entry(fs, entry);
        if (fs->compiling) {
            fs->compiling = old_state;
        }
    } else {
        *fs->here.cell++ = (sef_int_t) entry;
    }
}

// Memory management

// cells
static void cells(forth_state_t* fs) {
    sef_push_data(fs, sef_pop_data(fs) * sizeof(sef_int_t));
}

// here
static void here(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) fs->here.byte);
}

// allot
static void allot(forth_state_t* fs) {
    sef_int_t size = sef_pop_data(fs);
    sef_allot(fs, size);
}

#if SEF_MEMORY_ALLOCATION
// allocate
static void allocate(forth_state_t* fs) {
    sef_int_t size = sef_pop_data(fs);
    char* mem = malloc(size);
    sef_push_data(fs, (sef_int_t) mem);
    sef_push_data(fs, mem == NULL);
}

// free
static void FREE(forth_state_t* fs) {
    free((void *) sef_pop_data(fs));
    sef_push_data(fs, 0);
}

// resize
static void resize(forth_state_t* fs) {
    sef_int_t new_size = sef_pop_data(fs);
    void* mem = (void*) sef_pop_data(fs);
    void* new_mem = realloc(mem, new_size);
    sef_push_data(fs, (sef_int_t) new_mem);
    sef_push_data(fs, new_mem == NULL);
}
#endif

// @
static void fetch(forth_state_t* fs) {
    sef_int_t* addr = (sef_int_t *) sef_pop_data(fs);
    sef_push_data(fs, *addr);
}

// !
static void store(forth_state_t* fs) {
    sef_int_t* addr = (sef_int_t *) sef_pop_data(fs);
    sef_int_t data = sef_pop_data(fs);
    *addr = data;
}

// c@
static void cfetch(forth_state_t* fs) {
    char* addr = (char *) sef_pop_data(fs);
    sef_push_data(fs, (sef_int_t) * addr);
}

// c!
static void cstore(forth_state_t* fs) {
    char* addr = (char *) sef_pop_data(fs);
    sef_int_t data = sef_pop_data(fs);
    *addr = (char) data;
}

// unused
static void unused(forth_state_t* fs) {
    sef_push_data(fs, SEF_FORTH_MEMORY_SIZE - (fs->here.byte - fs->forth_memory));
}

// C strings

// strlen
static void str_len(forth_state_t* fs) {
    char* str = (char *) sef_pop_data(fs);
    sef_push_data(fs, (sef_int_t) strlen(str));
}

#if SEF_FILE_ACCESS
// File manipulation

enum file_modes {
    m_ro = 1,
    m_wo = 2,
    m_rw = 3,
};

static const char* file_modes_to_create(enum file_modes fm) {
    switch (fm) {
        case m_wo:
            return "w";
        case m_rw:
            return "w+";
        default:
            error_msg("Invalid mode for file creation.");
            return NULL;
    }
}

static const char* file_modes_to_open(enum file_modes fm) {
    switch (fm) {
        case m_wo:
            return "a";
        case m_rw:
            return "a+";
        case m_ro:
            return "r";
        default:
            error_msg("Invalid mode for file opening.");
            return NULL;
    }
}

// r/o
static void ro(forth_state_t* fs) {
    sef_push_data(fs, m_ro);
}

// w/o
static void wo(forth_state_t* fs) {
    sef_push_data(fs, m_wo);
}

// r/w
static void rw(forth_state_t* fs) {
    sef_push_data(fs, m_rw);
}

// create-file and open-file generic
static void file_action(forth_state_t* fs, const char* (*mode_f)(enum file_modes)) {
    enum file_modes mode = sef_pop_data(fs);
    sef_int_t filename_size = sef_pop_data(fs);
    const char* filename_forth = (const char*) sef_pop_data(fs);
    char filename[filename_size+1]; // Convert forth string to C string
    memcpy(filename, filename_forth, filename_size);
    filename[filename_size] = 0;
    FILE* ret = fopen(filename, mode_f(mode));
    sef_push_data(fs, (sef_int_t) ret);
    sef_push_data(fs, ret == NULL);
}

// create-file
static void create_file(forth_state_t* fs) {
    file_action(fs, file_modes_to_create);
}

// open-file
static void open_file(forth_state_t* fs) {
    file_action(fs, file_modes_to_open);
}

// close-file
static void close_file(forth_state_t* fs) {
    FILE* f = (FILE*) sef_pop_data(fs);
    fclose(f);
    sef_push_data(fs, 0);
}

// read-file
static void read_file(forth_state_t* fs) {
    FILE* f = (FILE*) sef_pop_data(fs);
    size_t size = sef_pop_data(fs);
    char* dest = (char*) sef_pop_data(fs);
    size_t ret = fread(dest, 1, size, f);
    sef_push_data(fs, ret);
    sef_push_data(fs, ret <= 0);
}

// write-file
static void write_file(forth_state_t* fs) {
    FILE* f = (FILE*) sef_pop_data(fs);
    size_t size = sef_pop_data(fs);
    char* source = (char*) sef_pop_data(fs);
    size_t written = fwrite(source, 1, size, f);
    sef_push_data(fs, written != size);
}

// read-line
static void read_line(forth_state_t* fs) {
    FILE* f = (FILE*) sef_pop_data(fs);
    size_t size = sef_pop_data(fs);
    char* dest = (char*) sef_pop_data(fs);
    size_t dest_index = 0;
    bool eof = false;
    for (size_t i=0; i<size; i++) {
        int c = getc(f);
        switch (c) {
            case EOF:
                eof = true;
                goto endloop;
            case '\n':
                dest[dest_index] = c;
                goto endloop;
            default:
                dest[dest_index++] = c;
                break;
        }
    }
endloop:
    sef_push_data(fs, dest_index);
    if (dest_index == 0 && eof) { // Bad, no char were written; flag = false, ior = ~0
        sef_push_data(fs, 0);
        sef_push_data(fs, FORTH_TRUE);
    } else { // Good: flag true, ior = 0
        sef_push_data(fs, 1);
        sef_push_data(fs, 0);
    }
}

// write-line
static void write_line(forth_state_t* fs) {
    FILE* f = (FILE*) sef_pop_data(fs);
    size_t size = sef_pop_data(fs);
    const char* source = (char*) sef_pop_data(fs);
    size_t written = fwrite(source, size, 1, f);
    written += fwrite("\n", 1, 1, f);
    sef_push_data(fs, written != (size + 1));
}

// stdin
static void _stdin(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) stdin);
}

// stdout
static void _stdout(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) stdout);
}

// stderr
static void _stderr(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) stderr);
}
#endif

// words
static void words(forth_state_t* fs) {
    sef_display_dictionary(fs);
}

// Misc

// emit
static void emit(forth_state_t* fs) {
    sef_int_t w = sef_pop_data(fs);
    sef_output(w);
}

// key
static void key(forth_state_t* fs) {
    sef_int_t w = sef_input();
    sef_push_data(fs, w);
}

// exit
static void exit_word(forth_state_t* fs) {
    sef_exit(fs);
}

// CR
static void cr(forth_state_t* fs) {
    UNUSED(fs);
    sef_output('\n');
}

// base
static void base(forth_state_t* fs) {
    sef_int_t* base_pnt = &fs->base;
    sef_push_data(fs, (sef_int_t) base_pnt);
}

// execute
static void execute(forth_state_t* fs) {
    dictionary_entry_t exec_tocken = (dictionary_entry_t) sef_pop_data(fs);
    sef_call_entry(fs, exec_tocken);
}

// pad
static void pad(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) fs->pad);
}

// Given a string, return true if i's a valid query for environment?
// Also put in the return pointer the constants to put on the stack.
// ret[0] is the first element to put on the stack.
static bool environment_reply(const char* query, size_t size, sef_int_t ret[static 2], size_t* number_of_returned_values) {
    *number_of_returned_values = 1;
    if (!strncmp(query, "/COUNTED-STRING", size)) {
        *ret = 0xFF;
    } else if (!strncmp(query, "/HOLD", size)) {
        *ret = sizeof(sef_int_t) * 8 + 16;
    } else if (!strncmp(query, "/PAD", size)) {
        *ret = SEF_PAD_SIZE;
    } else if (!strncmp(query, "ADDRESS-UNIT-BITS", size)) {
        *ret = sizeof(char) * 8;
    } else if (!strncmp(query, "FLOORED", size)) {
        *ret = false; // We use sm/rem to define other math functions
    } else if (!strncmp(query, "MAX-CHAR", size)) {
        *ret = 0xFF;
    } else if (!strncmp(query, "MAX-D", size)) {
        ret[0] = ((sef_unsigned_t) ~0) >> 1;
        ret[1] = 0;
        *number_of_returned_values = 2;
    } else if (!strncmp(query, "MAX-N", size)) {
        *ret = ((sef_unsigned_t) ~0) >> 1;
    } else if (!strncmp(query, "MAX-U", size)) {
        *ret = ~0;
    } else if (!strncmp(query, "MAX-UD", size)) {
        ret[0] = ~0;
        ret[1] = 0;
        *number_of_returned_values = 2;
    } else if (!strncmp(query, "RETURN-STACK-CELLS", size)) {
        *ret = SEF_RETURN_STACK_SIZE;
    } else if (!strncmp(query, "STACK-CELLS", size)) {
        *ret = SEF_DATA_STACK_SIZE;
    } else {
        *number_of_returned_values = 0;
        return false;
    }
    return true;
}

// environment?
static void environment_query(forth_state_t* fs) {
    size_t size = (size_t) sef_pop_data(fs);
    const char* str = (const char*) sef_pop_data(fs);
    sef_int_t ret[2];
    size_t number_of_returned_values = 0;
    bool query_ret = environment_reply(str, size, ret, &number_of_returned_values);
    for (size_t i=0; i<number_of_returned_values; i++) {
        sef_push_data(fs, ret[i]);
    }
    sef_push_data(fs, FORTH_BOOL(query_ret));
}

// Internals

// (find) like find but with normal string instead of counted strings
// In the not-found case, return the address -1 to help with find defintion
static void find(forth_state_t* fs) {
    size_t name_size = (size_t) sef_pop_data(fs);
    const char* name = (const char*) sef_pop_data(fs);
    dictionary_entry_t entry = sef_find_entry(fs, name, name_size);

    if (entry == NULL) {
        sef_push_data(fs, (sef_int_t) (name-1));
        sef_push_data(fs, 0);
    } else {
        sef_int_t* word_tag_field = sef_get_word_tag_field(entry);
        sef_int_t return_flag = *word_tag_field & WTM_IMMEDIATE ? 1 : -1;
        sef_push_data(fs, (sef_int_t) entry);
        sef_push_data(fs, return_flag);
    }

}

// Pop an XT and push a pointer to its parameters
static void body(forth_state_t* fs) {
    dictionary_entry_t xt = (dictionary_entry_t) sef_pop_data(fs);
    void* parameters = sef_get_entry_parameter(xt);
    sef_push_data(fs, (sef_int_t) parameters);
}

static void state(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) &fs->compiling);
}

static void in(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) &fs->parse_area_offset);
}

static void source_id(forth_state_t* fs) {
    sef_push_data(fs, fs->source_id);
}

// Push the addresses of the input buffer and its size
static void source(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) &fs->input_buffer);
    sef_push_data(fs, (sef_int_t) &fs->input_buffer_size);
}

// Push the last dictionary entry's address
static void dictionary(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) &fs->last_dictionary_entry);
}

// Push the address of HERE
static void where(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) &fs->here.byte);
}

// Push the address of the code pointer
static void code_pointer(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) &fs->code_pointer);
}

// Push the address of the bye value
static void bye(forth_state_t* fs) {
    sef_push_data(fs, (sef_int_t) &fs->bye);
}

/* ------------------------ Registering the functions ----------------------- */

struct c_func_s {
    const char* name;
    void (*func)(forth_state_t*);
};

struct c_func_s all_default_c_func[] = {
    // Stack manipulation
    {"swap", swap},
    {"rot", rot},
    {"dup", DUP},
    {"drop", drop},
    {">r", to_r},
    {"r>", r_from},
    {"roll", roll},
    {"pick", pick},
    {"depth", depth},
    // Basic math
    {"+", add},
    {"-", sub},
    {"*", mult},
    {"sm/rem", sm_slash_rem},
    {"fm/mod", fm_slash_mod},
    {"um/mod", um_slash_mod},
    {"abs", abs_word},
    {"<", less_than},
    {"u<", u_less_than},
    // Boolean logic
    {"0<", less0},
    {"0=", eq0},
    {"=", eq},
    {"and", and},
    {"or", or},
    {"xor", xor},
    {"lshift", lshift},
    {"rshift", rshift},
    {"2/", two_slash},
    // Flow control
    {"(if)", if_runtine}, // TODO: if a cache is made for this kind of words (used in the parser), I could stop take new name and use name shadowing instead.
    {"(else)", else_runtime},
    {"(while)", while_runtime},
    {"(repeat)", repeat_runtime},
    {"(do)", do_run_time},
    {"(?do)", question_do_run_time},
    {"(+loop)", plus_loop_run_time},
    {"(loop)", loop_run_time},
    {"(of)", of_run_time},
    {"(endof)", endof_run_time},
    // Compilation helpers
    {"(literal)", literal},
    {"(postpone)", postpone_run_time},
    // Memory management
    {"allot", allot},
    {"cells", cells},
    {"here", here},
#if SEF_MEMORY_ALLOCATION
    {"allocate", allocate},
    {"free", FREE},
    {"resize", resize},
#endif
    {"@", fetch},
    {"!", store},
    {"c@", cfetch},
    {"c!", cstore},
    {"unused", unused},
    {"strlen", str_len},
#if SEF_FILE_ACCESS
    // File manipulation
    // TODO: test and document
    {"r/o", ro},
    {"r/w", rw},
    {"w/o", wo},
    {"create-file", create_file},
    {"open-file", open_file},
    {"close-file", close_file},
    {"read-file", read_file},
    {"write-file", write_file},
    {"read-line", read_line},
    {"write-line", write_line},
    {"stdin", _stdin},
    {"stdout", _stdout},
    {"stderr", _stderr},
#endif
    // Programming tools
    {"words", words},
    // Misc
    {"emit", emit},
    {"key", key},
    {"exit", exit_word},
    {"abort", sef_abort},
    {"quit", sef_quit},
    {"cr", cr},
    {"base", base},
    {"execute", execute},
    {"pad", pad},
    {"environment?", environment_query},
    // Internals
    {"(find)", find},
    {">body", body},
    {"state", state},
    {">in", in},
    {"source-id", source_id},
    {">source", source},
    {"dictionary", dictionary},
    {"where", where},
    {"code-pointer", code_pointer},
    {">bye", bye},
};

// Register all the default C_func
void sef_register_default_cfunc(forth_state_t* fs) {
    for (size_t i = 0; i < sizeof(all_default_c_func) / sizeof(struct c_func_s); i++) {
        const char* name = all_default_c_func[i].name;
        sef_register_cfunc(fs, name, all_default_c_func[i].func, false);
    }
}


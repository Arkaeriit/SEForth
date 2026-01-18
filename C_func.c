#include "sef_debug.h"
#include "C_func.h"
#include "string.h"
#include "stdio.h"

#define UNUSED(x) (void)(x)
#define FORTH_TRUE ((sef_int_t) ~0)
#define FORTH_BOOL(x) ((x) ? FORTH_TRUE : 0)

// Functions used to manipulate C_fun

static void exec_cfunc(forth_state_t* fs, void* parameters) {
    C_callback_t* func_field = parameters;
    C_callback_t func = *func_field;
    func(fs);
}

// Register a new C function
void sef_register_cfunc(forth_state_t* fs, const char* name, C_callback_t func, bool is_immediate) {
    sef_register_new_word(fs, name, strlen(name), exec_cfunc);
    *fs->here.cell = (sef_int_t) func;
    sef_allot_cell(fs);
    if (is_immediate) {
        sef_int_t* word_tag_field = sef_get_word_tag_field(fs->last_dictionary_entry);
        *word_tag_field |= WTM_IMMEDIATE;
    }
}

// TODO: Remove to keep the forth one
static void dot(forth_state_t* fs) {
    // I don't care about base as this is a debug word.
    sef_int_t w = sef_pop_data(fs);
    printf("%li ", w);
}
static void type(forth_state_t* fs) {
    sef_int_t str_len = sef_pop_data(fs);
    char* str = (char*) sef_pop_data(fs);
    printf("%.*s<-", (int) str_len, str);
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
    sef_int_t data = sef_pop_code(fs);
    sef_push_data(fs, data);
}

// >r
static void to_r(forth_state_t* fs) {
    sef_int_t w = sef_pop_data(fs);
    sef_push_code(fs, w);
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

// of
static void of(forth_state_t* fs) {
    SEF_ERROR_OUT(fs, "TODO of!\n");
}

// endof
static void endof(forth_state_t* fs) {
    SEF_ERROR_OUT(fs, "TODO endof!\n");
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

#if SEF_FILE
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

#if SEF_PROGRAMMING_TOOLS
// words
static void words(forth_state_t* fs) {
    sef_display_dictionary(fs);
}
#endif

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

// defer@
static void defer_fetch(forth_state_t* fs) {
    SEF_ERROR_OUT(fs, "TODO defer@!\n");
}

// defer!
static void defer_store(forth_state_t* fs) {
    SEF_ERROR_OUT(fs, "TODO dever!!\n");
}

// Given a string, return true if i's a valid query for environment?
// Also put in the return pointer the constants to put on the stack.
// ret[0] is the first element to put on the stack.
static bool environment_reply(const char* query, size_t size, sef_int_t ret[static 2], size_t* number_of_returned_values) {
    *number_of_returned_values = 1;
    if (!strncmp(query, "/COUNTED-STRING", size)) {
        *ret = 0xFF;
    } else if (!strncmp(query, "/HOLD", size)) {
#warning "Use a constant"
        *ret = 64;
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
        *ret = SEF_CODE_STACK_SIZE;
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
    sef_push_data(fs, FORTH_BOOL(query_ret));
    for (size_t i=0; i<number_of_returned_values; i++) {
        sef_push_data(fs, ret[i]);
    }
}

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

struct c_func_s {
    const char* name;
    void (*func)(forth_state_t*);
};

struct c_func_s all_default_c_func[] = {
    {".", dot},
    {"type", type},
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
    // Flow control
    {"(if)", if_runtine}, // TODO: if a cache is made for this kind of words (used in the parser), I could stop take new name and use name shadowing instead.
    {"(else)", else_runtime},
    {"(while)", while_runtime},
    {"(repeat)", repeat_runtime},
    {"of", of},
    {"endof", endof},
    // Memory management
    {"allot", allot},
    {"cells", cells},
    {"here", here},
    {"allocate", allocate},
    {"free", FREE},
    {"resize", resize},
    {"@", fetch},
    {"!", store},
    {"c@", cfetch},
    {"c!", cstore},
    {"unused", unused},
    {"strlen", str_len},
#if SEF_FILE
    // File manipulation
#warning TODO: test and document
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
#if SEF_PROGRAMMING_TOOLS
    // Programming tools
    {"words", words},
#endif
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
    {"defer@", defer_fetch},
    {"defer!", defer_store},
    {"environment?", environment_query},
    {"(find)", find}
};

// Register all the default C_func
void sef_register_default_cfunc(forth_state_t* fs) {
    for (size_t i = 0; i < sizeof(all_default_c_func) / sizeof(struct c_func_s); i++) {
        const char* name = all_default_c_func[i].name;
        sef_register_cfunc(fs, name, all_default_c_func[i].func, false);
    }
}


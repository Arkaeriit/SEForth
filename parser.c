#include "sef_debug.h"
#include "utils.h"
#include "parser.h"
#include "stdio.h"
#include "stddef.h"
#include "string.h"
#include "assert.h"

static void run_next_word_hook(parser_state_t* p);
static void invalid_hook(parser_state_t* p);
static void register_compile_time_words_list(parser_state_t* p);
static void macro(parser_state_t* p, const char* payload);

parser_state_t* sef_init_parser(void) {
    parser_state_t* ret = malloc(sizeof(parser_state_t));
    ret->fs = sef_init_state(ret);
    register_compile_time_words_list(ret);
    ret->buffer = malloc(SEF_PARSER_BUFFER_SIZE);
    ret->new_word_buffer = malloc(SEF_PARSER_BUFFER_SIZE);
    ret->custom_word_name = malloc(SEF_PARSER_CUSTOM_NAME_SIZE);
    ret->new_word_hook = run_next_word_hook;
    ret->end_block_hook = invalid_hook;
    ret->hooks_stack = sef_stack_init(SEF_COMPILATION_STACK_SIZE);
    ret->pnt = 0;
    ret->in_word = false;
    ret->in_def = false;
    ret->wait_until = 0;
    extern const char* base_forth_func;
    sef_parse_string(ret->fs, base_forth_func);
#if SEF_FILE
    extern const char* file_forth_func;
    sef_parse_string(ret->fs, file_forth_func);
#endif
#if SEF_STRING
    extern const char* string_forth_func;
    sef_parse_string(ret->fs, string_forth_func);
#endif
#if SEF_PROGRAMMING_TOOLS
    extern const char* programming_forth_func;
    sef_parse_string(ret->fs, programming_forth_func);
#endif
#if SEF_CASE_INSENSITIVE == 0
    ret->fs->dic->case_insensitive = false;
#endif
    return ret;
}

void sef_clean_parser(parser_state_t* parse) {
    sef_stack_free(parse->hooks_stack);
    free(parse->custom_word_name);
    free(parse->new_word_buffer);
    free(parse->buffer);
    sef_clean_state(parse->fs);
    free(parse);
}

void sef_parser_parse_char(parser_state_t* parse, char ch) {
    if (parse->wait_until != 0) {
        if (parse->wait_until == ch) {
            parse->buffer[parse->pnt] = 0;
            parse->wait_until = 0;
            parse->end_block_hook(parse);
        } else {
            parse->buffer[parse->pnt] = ch;
            parse->pnt++;
        }
        return;
    }
    if (sef_is_delimiter(ch)) {
        if (!parse->in_word) {
            return;
        }
        parse->in_word = false;
        parse->buffer[parse->pnt] = 0;
        entry_t compile_time_entry;
        if (sef_find(parse->fs->dic, &compile_time_entry, NULL, sef_hash(parse->buffer)) == sef_OK) {
            while (compile_time_entry.type == alias) {
                sef_error find_rc = sef_find(parse->fs->dic, &compile_time_entry, NULL, compile_time_entry.func.alias_to);
                if (find_rc != sef_OK) {
                    break;
                }
            }
            if (compile_time_entry.type == compile_word) {
                compile_time_entry.func.compile_func.func(parse, compile_time_entry.func.compile_func.payload);
                return;
            }
        }
        parse->new_word_hook(parse);
    } else {
        parse->in_word = true;
        parse->buffer[parse->pnt] = ch;
        parse->pnt++;
    }
}

#if SEF_USE_SOURCE_FILE
sef_error sef_register_file(parser_state_t* p, const char* filemane) {
    FILE* f = fopen(filemane, "r");
    if (f == NULL) {
        return sef_invalid_file;
    }
    int ch = fgetc(f);
    if (ch == '#') { // We ignore the starting shebang
        sef_parse_string(p->fs, "\\ ");
    }
    while (ch != EOF) {
        sef_parser_parse_char(p, ch);
        if (!sef_can_execute(p->fs)) {
            break;
        }
        ch = fgetc(f);
    }
    fclose(f);
    return sef_OK;
}
#endif

bool sef_parser_is_compiling(parser_state_t* parse) {
    return parse->new_word_hook != run_next_word_hook;
}

/* ---------------------------- Next words hooks ---------------------------- */

#if SEF_STACK_BOUND_CHECKS
#define STACK_BOUND_CHECK(p)                      \
    if (!sef_state_state_valid(p->hooks_stack)) { \
        error_msg("Hooks stack out of bound.\n"); \
    }                                              
#else
#define STACK_BOUND_CHECK(x...)
#endif

#define PUSH_HOOK(p, hook_name)                       \
    do {                                              \
        sef_int_t to_push = (sef_int_t) p->hook_name; \
        sef_stack_push(p->hooks_stack, to_push);      \
        STACK_BOUND_CHECK(p);                         \
    } while(0)                                         

#define POP_HOOK(p, hook_name)                                      \
    p->hook_name = (new_word_hook_t) sef_stack_pop(p->hooks_stack); \
    STACK_BOUND_CHECK(p)                                             

#define UNUSED(x) (void)(x)

// Run in the new word hook a number
static void hook_number(parser_state_t* p, sef_int_t number) {
    char tmp[SEF_MAX_NUMBER_DIGIT];
    snprintf(p->buffer, SEF_PARSER_BUFFER_SIZE, "%s", sef_base_format(number, tmp, p->fs->base));
    p->new_word_hook(p);
}

// Run the string in the new word hook
static void hook_str(parser_state_t* p, const char* str) {
    snprintf(p->buffer, SEF_PARSER_BUFFER_SIZE, "%s", str);
    p->new_word_hook(p);
}

// Error when no hooks registered
static void invalid_hook(parser_state_t* p) {
    UNUSED(p);
    error_msg("Invalid hook should not be run.\n");
}

// This hook is the parser's default one, it tries to run the buffer
static void run_next_word_hook(parser_state_t* p) {
    p->pnt = 0;
    word_node_t node_to_exe = sef_compile_node(p->buffer, p->fs->base);
    sef_error execute_rc = sef_executes_node(p->fs, &node_to_exe);
    if (execute_rc == sef_not_found) {
        error_msg("Calling word %s which is not defined.\n", p->buffer);
    } else if (execute_rc != sef_OK) {
        warn_msg("Error n°%i when calling word %s\n", execute_rc, p->buffer);
    }
    sef_run(p->fs);
}

// This hook is the one to use during a definition, add whitespace between words
static void in_def_hook(parser_state_t* p) {
    p->pnt = 0;
    strcat(p->new_word_buffer, p->buffer);
    strcat(p->new_word_buffer, " ");
}

// Hook to use when about to write a words name, first to run when defining a
// new word
static void definition_name_hook(parser_state_t* p) {
    p->pnt = 0;
    strcpy(p->custom_word_name, p->buffer);
    p->new_word_buffer[0] = 0;
    p->new_word_hook = in_def_hook;
}

// Same as definition_name_hook but prepare waiting until ";"
// This is needed to ensure compile time words are not executed in the macro
// definition.
static void definition_macro_name_hook(parser_state_t* p) {
    definition_name_hook(p);
    p->wait_until = ';';
}

// This hook is meant to read name of a constant
static void const_hook(parser_state_t* p) {
    p->pnt = 0;
    sef_compile_constant(p->buffer, p->fs);
    p->new_word_hook = run_next_word_hook;
}

// Replace the new word with its exec token
static void get_exec_token_hook(parser_state_t* p) {
    hash_t hash = sef_hash(p->buffer);
    POP_HOOK(p, new_word_hook);
    hook_number(p, hash);
}

// Register a string
static void register_string_hook(parser_state_t* p) {
    const char* str = p->buffer;
    size_t size = p->pnt - 1;
    while (*str++ != '"') {
        size--;
    }
    hash_t str_id = sef_register_string(p->fs->dic, str, size);
    char string_type = p->buffer[0];
    hook_number(p, str_id);
    hook_str(p, "execute");
    switch (string_type) {
        case 's':
        case 'S':
            break;
        case '.':
            hook_str(p, "type");
            break;
        case 'a': // For abort"
        case 'A':
            hook_str(p, "type");
            hook_str(p, "cr");
            hook_str(p, "abort");
            break;
        case 'c':
        case 'C':
            hook_str(p, "drop");
            p->new_word_hook(p); // For counted string, we drop the length value
            {
                entry_t e;
                sef_find(p->fs->dic, &e, NULL, str_id);
                char* string_in_memory = e.func.string.data;
                memmove(string_in_memory+1, string_in_memory, size); // We move all characters by one. As the string is null-terminated, there is no overflow. The counted string is not null terminated, but as they are cursed, it's not the worst thing about them.
                *string_in_memory = (char) size;
            } break;
        default:
            error_msg("Unknown string type %c\n", string_type);
    }
    POP_HOOK(p, end_block_hook);
}

// Return the numeric value of the ASCII hex digit c
char hex_digit(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    } else {
        error_msg("%c is not a valid hex digit!.", c);
        return 0;
    }
}

// Process escaped characters in s\"
static ptrdiff_t escape_s_blackslash_quote(char* str) {
    char* write = str;
    char* read = str;
    bool escaping = false;
    char c;
    while ((c = *read++)) {
        if (escaping) {
            escaping = false;
            switch (c) {
                case 'a':
                    c = '\a';
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'e':
                    c = 27;
                    break;
                case 'f':
                    c = '\f';
                    break;
                case 'l':
                    c = 10;
                    break;
                case 'm':
                    *write++ = 13;
                    c = 10;
                    break;
                case 'n':
                    c = '\n';
                    break;
                case 'q':
                    c = '"';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'v':
                    c = '\v';
                    break;
                case 'z':
                    c = '\0';
                    break;
                case '"':
                    c = '"';
                    break;
                case 'x': {
                    c = *read++;
                    char d1 = hex_digit(c);
                    c = *read++;
                    char d2 = hex_digit(c);
                    c = (d1 << 4) | d2;
                    } break;
                case '\\':
                default:
                    // In both cases, keep the char as-is.
                    break;
            }
            *write++ = c;
        } else if (c == '\\') {
            escaping = true;
        } else {
            *write++ = c;
        }
    }
    *write = 0;
    ptrdiff_t removed_chars = read - write - 1;
    return removed_chars;
}
static void register_escaped_string_hook(parser_state_t* p) {
    char* str = p->buffer;
    size_t size = strlen(str);
    if (str[size-1] == '\\') {
        // In that case, it means the buffer ends with a backslash. This happen if there was a \" in the string.
        // In that case, we want to do as we werent interrupted, and keep going with the reading of the string.
        str[size] = '"';
        str[size+1] = 0;
        p->wait_until = '"';
        p->pnt++;
    } else {
        ptrdiff_t removed_chars = escape_s_blackslash_quote(str);
        p->pnt -= removed_chars;
        register_string_hook(p);
    }
}

// Register a normal word definition
static void register_def_hook(parser_state_t* p) {
    sef_compile_string(p->fs->dic, p->custom_word_name, p->new_word_buffer, p->fs->base, 0);
}

// Register an unamed word
static void register_noname_hook(parser_state_t* p) {
    hash_t noname_hash = sef_unused_special_hash(p->fs->dic);
    sef_compile_string(p->fs->dic, "noname", p->new_word_buffer, p->fs->base, noname_hash);
    sef_push_data(p->fs, (sef_int_t) noname_hash);
}

// Register a macro
static void _register_macro_hook(parser_state_t* p) {
    char* payload = malloc(strlen(p->buffer)+1);
    strcpy(payload, p->buffer);
    debug_msg("macroing '%s' as '%s'\n", payload, p->custom_word_name);
    sef_register_compile_time_word(p, p->custom_word_name, macro, payload);
    p->pnt = 0;
}

// Register a :macro
static void register_macro_hook(parser_state_t* p) {
    _register_macro_hook(p);
    POP_HOOK(p, new_word_hook);
    POP_HOOK(p, end_block_hook);
}

// Register a string-macro
static void string_macro_hook(parser_state_t* p) {
    strcpy(p->custom_word_name, p->buffer);
    size_t macro_size = sef_pop_data(p->fs);
    const char* macro_content = (const char*) sef_pop_data(p->fs);
    memcpy(p->buffer, macro_content, macro_size);
    p->buffer[macro_size] = 0;
    _register_macro_hook(p);
    POP_HOOK(p, new_word_hook);
}

// Get the first character of the next word and use it instead as a raw word
static void char_hook(parser_state_t* p) {
    POP_HOOK(p, new_word_hook);
    size_t old_ptn = sef_stack_pop(p->hooks_stack);
    char letter = p->buffer[old_ptn];
    hook_number(p, letter);
}

// Set the name in the buffer to be a defered word
static void defer_hook(parser_state_t* p) {
    p->pnt = 0;
    POP_HOOK(p, new_word_hook);
    sef_register_defer(p->buffer, p->fs);
}

// Set the name in the buffer to be a defered word
static void is_hook(parser_state_t* p) {
    p->pnt = 0;
    POP_HOOK(p, new_word_hook);
    hash_t alias_to = (hash_t) sef_pop_data(p->fs);
    sef_set_alias(p->fs->dic, sef_hash(p->buffer), alias_to, p->buffer);
}

// Set the name in the buffer to be a defered word
static void action_of_hook(parser_state_t* p) {
    p->pnt = 0;
    POP_HOOK(p, new_word_hook);
    entry_t entry;
    if (sef_find(p->fs->dic, &entry, NULL, sef_hash(p->buffer)) != sef_OK || !(entry.type == alias || entry.type == defered)) {
        error_msg("Using action-of on invalid values.");
    }
    hash_t word_hash = sef_hash(p->buffer);
    hook_number(p, word_hash);
    hook_str(p, "defer@");
}

// Ignore the buffer
static void end_of_comment_hook(parser_state_t* p) {
    POP_HOOK(p, end_block_hook);
    p->pnt = 0;
}

// Print the buffer
static void compile_time_print_hook(parser_state_t* p) {
    sef_print_string(p->buffer);
    POP_HOOK(p, end_block_hook);
    p->pnt = 0;
}

/* --------------------------- Compile time words --------------------------- */

// (
static void open_par(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    p->wait_until = ')';
    PUSH_HOOK(p, end_block_hook);
    p->end_block_hook = end_of_comment_hook;
}

#define NOT_IN_DEF(p, name)                                            \
    if (p->in_def) {                                                   \
        error_msg("Using %s in a definition is not allowed.\n", name); \
        return;                                                        \
    }                                                                   
        

// :
static void colon(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    NOT_IN_DEF(p, ":");
    p->in_def = true;
    p->pnt = 0;
    p->new_word_hook = definition_name_hook;
    PUSH_HOOK(p, end_block_hook);
    p->end_block_hook = register_def_hook;
}

// :macro
static void colon_macro(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    NOT_IN_DEF(p, ":macro");
    p->pnt = 0;
    PUSH_HOOK(p, end_block_hook);
    PUSH_HOOK(p, new_word_hook);
    p->new_word_hook = definition_macro_name_hook;
    p->end_block_hook = register_macro_hook;
}

// :noname
static void colon_noname(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    NOT_IN_DEF(p, ":noname");
    p->in_def = true;
    p->pnt = 0;
    p->new_word_buffer[0] = 0;
    p->new_word_hook = in_def_hook;
    PUSH_HOOK(p, end_block_hook);
    p->end_block_hook = register_noname_hook;
}

// ;
static void semi_colon(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    if (p->in_def) {
        p->in_def = false;
        p->buffer[p->pnt] = 0;
        p->pnt = 0;
        p->end_block_hook(p);
        POP_HOOK(p, end_block_hook);
        p->new_word_hook = run_next_word_hook;
    } else {
        error_msg("Using ; outside of a definition is not allowed.\n");
    }
}

// constant
static void _constant(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    NOT_IN_DEF(p, "constant");
    p->new_word_hook = const_hook;
    p->pnt = 0;
}

// '
static_assert(sizeof(hash_t) <= sizeof(sef_int_t), "To handle execution tokens, hashes should fit in a cell.");
static void single_quote(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    PUSH_HOOK(p, new_word_hook);
    p->new_word_hook = get_exec_token_hook;
    p->pnt = 0;
}

// ." s" abort"
static void any_string(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    p->wait_until = '"';
    PUSH_HOOK(p, end_block_hook);
    p->end_block_hook = register_string_hook;
}

// s\"
static void escaped_string(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    p->wait_until = '"';
    PUSH_HOOK(p, end_block_hook);
    p->end_block_hook = register_escaped_string_hook;
}

// .(
static void compile_time_print(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    p->wait_until = ')';
    p->pnt = 0;
    PUSH_HOOK(p, end_block_hook);
    p->end_block_hook = compile_time_print_hook;
}

/* \ */
static void backslash(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    p->wait_until = '\n';
    PUSH_HOOK(p, end_block_hook);
    p->end_block_hook = end_of_comment_hook;
}

// macro-string
static void macro_string(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    NOT_IN_DEF(p, "macro-string");
    p->pnt = 0;
    PUSH_HOOK(p, new_word_hook);
    p->new_word_hook = string_macro_hook;
}

// char
static void _char(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    sef_stack_push(p->hooks_stack, (sef_int_t) p->pnt);
    PUSH_HOOK(p, new_word_hook);
    p->new_word_hook = char_hook;
}

// defer
static void defer(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    NOT_IN_DEF(p, "defer");
    p->pnt = 0;
    PUSH_HOOK(p, new_word_hook);
    p->new_word_hook = defer_hook;
}

// is
static void is(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    p->pnt = 0;
    PUSH_HOOK(p, new_word_hook);
    p->new_word_hook = is_hook;
}

// action-of
static void action_of(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    p->pnt = 0;
    PUSH_HOOK(p, new_word_hook);
    p->new_word_hook = action_of_hook;
}

// literal
static void literal(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    sef_int_t value = sef_pop_data(p->fs);
    hook_number(p, value);
}

// [
static void left_bracket(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    p->pnt = 0;
    PUSH_HOOK(p, new_word_hook);
    p->new_word_hook = run_next_word_hook;
}

// ]
static void right_bracket(parser_state_t* p, const char* payload) {
    UNUSED(payload);
    p->pnt = 0;
    POP_HOOK(p, new_word_hook);
}


// Generic word used by macros
static void macro(parser_state_t* p, const char* payload) {
    p->pnt = 0;
    for (size_t i=0; i<strlen(payload); i++) {
        sef_parser_parse_char(p, payload[i]);
    }
    sef_parser_parse_char(p, ' '); // If the macro doesn't end in whitespace and there is a single whitespace char between the macro and the next word, the last char of the macro would be concatenated with the following word. We add an extra space to ensure this can't happen.
}

// Register a compile time word
void sef_register_compile_time_word(parser_state_t* p, const char* name, compile_callback_t compile_func, char* payload) {
    entry_t e;
    e.type = compile_word;
    e.hash = sef_hash(name);
    e.func.compile_func.func = compile_func;
    e.func.compile_func.payload = payload;
#if SEF_STORE_NAME
    e.name = malloc(strlen(name) + 1);
    strcpy(e.name, name);
#endif
    sef_add_elem(p->fs->dic, e, name);
}

struct compile_func_s {
    const char* name;
    compile_callback_t func;
};

struct compile_func_s all_default_compile_words[] = {
    {"(", open_par},
    {":", colon},
    {":macro", colon_macro},
    {":noname", colon_noname},
    {";", semi_colon},
    {"constant", _constant},
    {"'", single_quote},
    {"s\"", any_string},
    {"c\"", any_string},
    {".\"", any_string},
    {"abort\"", any_string},
    {"s\\\"", escaped_string},
    {"\\", backslash},
    {"macro-string", macro_string},
    {"char", _char},
    {".(", compile_time_print},
    {"defer", defer},
    {"is", is},
    {"action-of", action_of},
    {"literal", literal},
    {"[", left_bracket},
    {"]", right_bracket},
};

// Register the previously defined words
static void register_compile_time_words_list(parser_state_t* p) {
    for (size_t i = 0; i < sizeof(all_default_compile_words) / sizeof(struct compile_func_s); i++) {
        const char* name = all_default_compile_words[i].name;
        sef_register_compile_time_word(p, name, all_default_compile_words[i].func, NULL);
    }
}


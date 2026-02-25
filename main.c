#include "SEForth.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

static void clear_shebang_from_first_line(char* file_content) {
    if (strlen(file_content) < 2) {
        return;
    }
    if (file_content[0] != '#' || file_content[1] != '!') {
        return;
    }
    for (size_t i=0; i<strlen(file_content); i++) {
        if (file_content[i] == '\n') {
            break;
        }
        file_content[i] = ' ';
    }
}

static void parse_a_file(sef_forth_state_t* fs, const char* file_name) {
    FILE* f = fopen(file_name, "r");
    if (f == NULL) {
        fprintf(stderr, "Can't open file %s.\n", file_name);
        exit(-1);
    }
    // We allocate a buffer that can contain the whole file.
    // We expect the dictionary growth to be proportional to the file size so
    // it should be fine.
    fseek(f, 0L, SEEK_END);
    size_t file_size = ftell(f);
    rewind(f);

    char* content = malloc(file_size + 1);
    if (content == NULL) {
        fprintf(stderr, "Can't allocate bytes to read file.\n");
        exit(-1);
    }

    fread(content, 1, file_size, f);
    content[file_size] = 0;
    clear_shebang_from_first_line(content);
    sef_eval_string(fs, content);
    free(content);
    fclose(f);
}

static void repl(sef_forth_state_t* fs) {
    do {
        sef_force_string_interpretation(fs, "(repl)");
    } while (!sef_asked_bye(fs)); // TODO: I wonder how I should leave the shell if bye is not there...
}

int main(int argc, char** argv) {
    sef_forth_state_t* fs = malloc(sizeof(sef_forth_state_t));
    sef_init(fs);

#if SEF_ARG_AND_EXIT_CODE
    if (argc > 1) {
        sef_feed_arguments(fs, argc - 1, argv + 1); // First argument is skipped as it will be handled from the C side.
    }
#endif

    if (argc > 1) {
        parse_a_file(fs, argv[1]);
        if (!sef_ready_to_run(fs) && !sef_asked_bye(fs)) {
            repl(fs);
        }
    } else {
        repl(fs);
    }

    int exit_code = sef_exit_code(fs);
    free(fs);
    return exit_code;
}


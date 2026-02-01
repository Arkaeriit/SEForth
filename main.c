#include "SEForth.h"
#include "stdlib.h"
#include "stdio.h"


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
    sef_parse_string(fs, content);
    free(content);
    fclose(f);
}

int main(int argc, char** argv) {
    static sef_forth_state_t fs;
    sef_init(&fs);

#if SEF_ARG_AND_EXIT_CODE
    if (argc > 1) {
        sef_feed_arguments(&fs, argc - 1, argv + 1);
    }
#endif

    if (argc > 1) {
        parse_a_file(&fs, argv[1]);
    } else {
        sef_parse_string(&fs, "(repl)");
    }

#if SEF_ARG_AND_EXIT_CODE
    sef_restart(&fs);
    sef_parse_string(&fs, "exit-code @");
    return sef_pop_from_data_stack(&fs);
#else
    return sef_error_encountered(&fs);
#endif
}


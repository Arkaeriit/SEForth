#include "SEForth.h"
#include "stdio.h"

#if 0
extern const char* shell;

int main(int argc, char** argv) {
    int rc = 0;
    forth_state_t* fs = sef_init();
#if SEF_USE_SOURCE_FILE
    if (argc > 1) {
#if SEF_PROGRAMMING_TOOLS
        sef_feed_arguments(fs, argc - 1, argv + 1);
#endif
        if (sef_parse_file(fs, argv[1])) {
            fprintf(stderr, "Error, unable to read file %s.\n", argv[1]);
            return 1;
        }
#if SEF_PROGRAMMING_TOOLS
        rc = sef_exit_code(fs);
#endif
        sef_free(fs);
        return rc;
    }
#endif
    sef_parse_string(fs, shell);
#if SEF_PROGRAMMING_TOOLS
    rc = sef_exit_code(fs);
#endif
    while (!sef_asked_bye(fs)) {
        sef_restart(fs);
        sef_parse_string(fs, "(repl) ");
    }
    sef_restart(fs);
    sef_parse_string(fs, "(repl-buffer) 1- free ");
    sef_free(fs);
    return rc;
}
#endif

int main(void) {
    static forth_state_t fs;
    sef_init(&fs);
    sef_parse_string(&fs, "123 . ");
    return fs.error_encountered;
}


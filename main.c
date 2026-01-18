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
    sef_parse_string(&fs, " : test-prt .\" printing works if I get a 5 \" 5 . ; test-prt cr ");
    printf("----\n");
    sef_parse_string(&fs, " : test-literals 5 . 15 . 18 . ; test-literals cr");
    printf("----\n");
    sef_parse_string(&fs, " : test if .\" oui \" else .\" non \" then .\"  enfin \" cr ; ");
    sef_parse_string(&fs, " 5 test 0 test");
    printf("----\n");
    sef_parse_string(&fs, ": test-loop 5 begin dup . dup while 1 - repeat cr ; test-loop ");
    printf("----\n");
    sef_parse_string(&fs, ": line1 1 . ;\n: line2 20 . ;\n: line3 300 . ;\nline3 line2 line1\ncr\n\n");
    printf("----\n");
    sef_parse_string(&fs, ": im-2 + . cr ; immediate : im-1 5 6 postpone im-2 ; immediate : nrm im-1 ;");
    printf("----\n");
    sef_parse_string(&fs, ": say-coucou .\" coucou\" cr ; s\" say-coucou\" evaluate");
    printf("----\n");
    sef_parse_string(&fs, ": test-does create here ! 64 allot does> @ 5 + . ; 5 test-does abc abc cr ");
    printf("----\n");
    sef_parse_string(&fs, "variable my-var 55 my-var ! my-var @ . cr ");
    return fs.error_encountered;
}


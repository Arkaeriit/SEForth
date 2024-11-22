>> This type defines the various errors we might encounter
typedef enum {
    OK = 0,
    no_memory = 1,
    not_found = 2,
    invalid_file = 3,
    impossible_error = 4,
    segfault = 5,
    sef_config_error = 6,
} error;

struct seforth_state_s;
typedef struct seforth_state_s forth_state_t;

forth_state_t* sef_init(void);
void sef_free(forth_state_t* state);

error sef_parse_file(forth_state_t* state, const char* filename);

>> TODO: remove
int sef_shell(void);

#ifdef SEF_CLI_ARGS
void sef_feed_arguments(forth_state_t* state, int argc, char** argv);
#endif

#ifdef SEF_PROGRAMMING_TOOLS
int sef_exit_code(const forth_state_t* state);
#endif


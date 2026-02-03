£include <stdbool.h>

>> ----------------------------- Initialization ----------------------------- >>

>> Type used to store the Forth interpreter. A pointer to it can be malloc'ed
>> (`sef_forth_state_t* state = malloc(sizeof(sef_forth_state_t))`) or an
>> instance of it can be declared as a static variable
>> (`static sef_forth_state_t state;`). Unless you use the File-Access or the
>> Memory-Allocation word sets, no other memory will be allocated by SEForth.
typedef sef_int_t sef_forth_state_t[SEF_STATE_SIZE_INT];

>> This function must be called on the Forth state before using it.
void sef_init(sef_forth_state_t* state);

>> -------------------------- Executing Forth code -------------------------- >>

>> Parse and execute the null-terminated string of Forth code `s`.
void sef_parse_string(sef_forth_state_t* state, const char* s);

>> ------------------------- Manipulating the state ------------------------- >>

>> Return true if the state is ready to parse and execute new code and false if
>> it can't. Either because the word `bye` has been called or if an error has
>> been encountered and `___SEF_ABORT_STOP_FORTH` is set.
bool sef_ready_to_run(sef_forth_state_t* state);

>> Empties the data and return stacks, put the state in interpreting mode,
>> clears flag that prevented it from running. The memory region addressed by
>> HERE and, by extension, the dictionary are preserved. This can be used to
>> reuse a state for which `sef_ready_to_run` returns false.
void sef_restart(sef_forth_state_t* state);

>> Return true if `bye` was called.
bool sef_asked_bye(sef_forth_state_t* state);

>> Return true if the state is in compiling mode and false if it is in
>> interpreting mode.
bool sef_is_compiling(sef_forth_state_t* state);

>> Return the exit code of the Forth state. If `___SEF_ARG_AND_EXIT_CODE` is set,
>> it is the value in `exit-code`. Otherwise, it's -1 if `abort` has been called
>> or 0 if it hasn't.
int sef_exit_code(sef_forth_state_t* state);

#ifdef SEF_ARG_AND_EXIT_CODE
>> This is only available if `___SEF_ARG_AND_EXIT_CODE` is set. This let you feed
>> command line arguments to SEForth by calling
>> `sef_feed_arguments(state, argc-1, argv+1);`.
void sef_feed_arguments(sef_forth_state_t* state, int argc, char** argv);
#endif

>> ------------------------- Manipulating the stack ------------------------- >>

>> Push the number `w` on top of the data stack.
void sef_push_to_data_stack(sef_forth_state_t* state, sef_int_t w);

>> Pop the top element from the data stack and return it.
sef_int_t sef_pop_from_data_stack(sef_forth_state_t* state);

>> --------------------------- Defining new words --------------------------- >>

>> This is the type of functions that can be added to the Forth dictionary. They
>> can push and pop number from the data stack to interact with the rest of the
>> Forth code.
typedef void (*sef_c_word)(sef_forth_state_t*);

>> Add a new word to the Forth dictionary. Its name should be a null-terminated
>> string. If `is_immediate` is set to true, `func` will be the compile-time
>> semantic of the word; if it is false, it will be the interpreting or
>> executing semantic.
void sef_register_c_word(sef_forth_state_t* state,
                         const char* name,
                         sef_c_word func,
                         bool is_immediate);


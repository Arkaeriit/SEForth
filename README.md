# SEForth

SEForth is a Forth interpreter designed to be portable and easy to embed in other applications. SEForth aims to follow the [Forth 2012 standard](https://forth-standard.org/).

## Supported word sets

### Core

All words from the Core word set and Core extension word set are provided.

### Optional word sets

Some of the optional word sets can be enabled on SEForth.

The following word sets are fully provided:

* Memory-Allocation

The following word sets are partially provided:

* File-Access
* Programming-Tools
* String

To know which words are supported, you can use `word`, which is always enabled, even if the rest of the Programming-Tools word set is not enabled. Similarly, some other words from optional word sets, such as `cmove>` for example, are always provided even if the rest of the optional word set is disabled.

### Non-standard word sets

A non standard word set can also be enabled, it is the _Arguments-and-Exit-Code_ word set. It is meant to manage command line arguments and program exit codes to run SEForth in an OS. It provides the following words:

* `exit-code ( -- addr )`: Push on the stack the address where the program exit code should be stored.
* `argc ( -- u )`: Push on the stack the number of arguments given to the program.
* `arg ( u -- addr u )`: Pop the index of an argument from the stack and then put that argument on the stack as a Forth string. The index number should be lower than the value given by `argc`.
* `next-arg ( -- addr u )`: Put the first non-processed argument on the stack as a Forth string. If all arguments have been processed, `0 0` is pushed on the stack.

The words `abort` and `abort"` will both put `-1` in `exit-code` in addition to their usual behavior.

If the Programming-Tools word set is enabled, the word `(bye)` is also added. This word puts the value from the top of the stack in `exit-code` and then calls `bye`.

## Case sensitivity

SEForth can be configured for the dictionary search to be either case-sensitive or case-insensitive. But even if it is configured to be case-sensitive, system words are searched in a case-insensitive way. This lets you call uppercase or lowercase system words depending on what you prefer.

## Deviations from the standard

Currently, SEForth doesn't support computation with double-cell numbers. Standard words that use double-cell numbers still use them in SEForth, but the most significant cell is only a sign extension and isn't used in computations. This behavior is susceptible to change in the future.

As SEForth doesn't want to make assumptions on the input source if it is embedded in an other application, `quit` and `abort` don't put you back in the Forth prompt on their own. If you want that behavior in your application, you can check how it is done in `main.c`.

## Included interpreter

Running `make` in this repository will compile `seforth.bin` (which can be installed as `seforth`). You can give it as argument a Forth file and some additional arguments to run the program. Alternatively, you can call it without arguments to enter a Forth REPL. This REPL is very bare-bones. For a more comfortable environment, you can use [ISEForth](https://github.com/Arkaeriit/iseforth).

# Configuring SEForth

As SEForth is meant to be embedded in other applications that can have varying requirements and resources, a lot of its features and settings can be configured. This configuration is made at compile time.

All the configurable values can be found in `sef_config.h`. If you want to change some of them, you can either change the default values in `sef_config.h`, or provide defined values in the CFLAGS when compiling. For example, you can do `make CFLAGS=-DSEF_MEMORY_ALLOCATION=0` to disable the memory allocation word set.

Here are all the configuration options:

* `SEF_INT_T`  
Type used to represent signed one-cell numbers. It must be big enough to store a pointer, but can be bigger if needed.
* `SEF_UNSIGNED_T`  
Type used to represent unsigned one-cell numbers. It must have the same size as `SEF_INT_T`.
* `SEF_FORTH_MEMORY_SIZE`  
Size in bytes of the memory region addressed by HERE. You might need around 30 kB on a system with `SEF_INT_T` set to `int64_t`.
* `SEF_PAD_SIZE`  
Size in bytes of the pad region.
* `SEF_DATA_STACK_SIZE`  
Number of cells in the data stack.
* `SEF_RETURN_STACK_SIZE`  
Number of cells in the return stack.
* `SEF_CONTROL_FLOW_STACK_SIZE`  
Number of cells in the control flow stack.
* `SEF_CASE_INSENSITIVE`  
If set to 1, all dictionary searches will be case-insensitive. If set to 0, dictionary searches will be case-sensitive for user-defined words and case-insensitive for system words.
* `SEF_LOG_LEVEL`  
This controls the amount of internal logs outputted by SEForth. If set to 0, no internal logs will be outputted; this is only suitable if SEForth runs in an environment with very limited resources and where the logs will never be read. If set to 1, error messages will be outputted; this is the most sensible configuration. If set to 2, some additional warnings like for word definition will be outputted; this is also a sensible configuration. If set to 3, debug messages will be outputted; as this is **extremely** verbose and noisy, it's only suitable to debug the SEForth interpreter itself.
* `SEF_LOG_OVER_STDERR`  
If set to 1, the logs controlled by `SEF_LOG_LEVEL` will be printed to `stderr`. If set to 0, they will be emitted with the `EMIT` word.
* `SEF_STACK_BOUND_CHECKS`  
If set to 1, there will be checks to ensure that none of the stacks can overflow and underflow, and that the memory space addressed by HERE doesn't overflow. If set to 0, those checks are disabled. The checks have some performance impact, but they are very convenient. 
* `SEF_CATCH_SEGFAULTS`  
With this option set to 1, segfaults caused by Forth code will be caught and the interpreter will be put back into an idle state if encountered. This relies on static variable and thus, this prevent the interpreter to be used on multiple threads. Furthermore, the system running SEForth needs to support POSIX signals.

The following configurations are all to enable or disable optional word set. Set them to 1 to enable the word set and to 0 to disable it.

* `SEF_FILE_ACCESS`
* `SEF_STRING`
* `SEF_PROGRAMMING_TOOLS`
* `SEF_MEMORY_ALLOCATION`
* `SEF_ARG_AND_EXIT_CODE`

## Internal behavior

If you want to learn more about how SEForth works on the insides, you can read `design-choices.md`.

# Using SEForth in other applications

## Generated library

When running `make` in this repository, the two files needed to use SEForth in other projects are generated. Those are `SEForth.h` and `libseforth.a`. They are built with the configuration options chosen during compilation.

They can be installed with `make install` along the interpreter to be used system-wide, or they can be used as-is.

You can then use them in other programs by adding `-lseforth` in your linker flags.

## API

### Initialization

* `typedef sef_int_t sef_forth_state_t[SEF_STATE_SIZE_INT];`  
Type used to store the Forth interpreter. A pointer to it can be malloc'ed (`sef_forth_state_t* state = malloc(sizeof(sef_forth_state_t))`) or an instance of it can be declared as a static variable (`static sef_forth_state_t state;`). Unless you use the File-Access or the Memory-Allocation word sets, no other memory will be allocated by SEForth. As the state is quite big, you shouldn't store it on the stack as a local variable.
* `void sef_init(sef_forth_state_t* state);`  
This function must be called on the Forth state before using it.

### Executing Forth code

* `void sef_eval_string(sef_forth_state_t* state, const char* s);`  
Parse and execute the null-terminated string of Forth code `s`.
* `void sef_force_string_interpretation(sef_forth_state_t* state, const char* s);`  
Force the interpretation of a string, even if the state isn't ready to interpret. If the state wasn't ready to run, call `sef_restart` before. If the state is compiling, put it back in interpreting mode before evaluating the string, and then put it back in compiling mode.

### Manipulating the state

* `bool sef_ready_to_run(sef_forth_state_t* state);`  
Return true if the state is ready to parse and execute new code and false if it can't because the words `bye`, `quit`, or `abort` have been called.
* `void sef_restart(sef_forth_state_t* state);`  
Empties the data and return stacks, put the state in interpreting mode, clears flag that prevented it from running. The memory region addressed by HERE and, by extension, the dictionary are preserved. This can be used to reuse a state for which `sef_ready_to_run` returns false.
* `bool sef_asked_bye(sef_forth_state_t* state);`  
Return true if `bye` was called.
* `bool sef_is_compiling(sef_forth_state_t* state);`  
Return true if the state is in compiling mode and false if it is in interpreting mode.
* `int sef_exit_code(sef_forth_state_t* state);`  
Return the exit code of the Forth state. If `SEF_ARG_AND_EXIT_CODE` is set, it is the value in `exit-code`. Otherwise, it's -1 if `abort` has been called or 0 if it hasn't.
* `void sef_feed_arguments(sef_forth_state_t* state, int argc, char** argv);`  
This is only available if `SEF_ARG_AND_EXIT_CODE` is set. This let you feed command line arguments to SEForth by calling `sef_feed_arguments(state, argc, argv);`.

### Manipulating the stack

* `void sef_push_to_data_stack(sef_forth_state_t* state, sef_int_t w);`  
Push the number `w` on top of the data stack.
* `sef_int_t sef_pop_from_data_stack(sef_forth_state_t* state);`  
Pop the top element from the data stack and return it.

### Defining new words

* `typedef void (*sef_c_word)(sef_forth_state_t*);`  
This is the type of functions that can be added to the Forth dictionary. They can push and pop number from the data stack to interact with the rest of the Forth code.
* `void sef_register_c_word(sef_forth_state_t* state, const char* name, sef_c_word func, bool is_immediate);`  
Add a new word to the Forth dictionary. Its name should be a null-terminated string. If `is_immediate` is set to true, `func` will be the compile-time semantic of the word; if it is false, it will be the interpreting or executing semantic.

### I/O

By default, SEForth will use `getchar` and `putchar` for input and output through `key` and `emit` respectively. But if you want another behavior, you can override those by defining one or both of the following functions:

* `char sef_input(void);`  
Ask the user for a character.
* `void sef_output(char);`
Display a character to the user.

Indeed, those function are defined in `libseforth.a`, but they are weak, so they can be overridden.

## Example of use

You can see basic usage of SEForth embedded in another program in `main.c`. Indeed the default interpreter only uses SEForth's public API to work. But some features aren't used in it.

### Declaring new words in C

Here is a small example program that defines a new word in C and then call it from Forth.

```c
#include <stdio.h>
#include <SEForth.h>

static void triple(sef_forth_state_t* state) {
    sef_int_t number = sef_pop_from_data_stack(state);
    sef_push_to_data_stack(state, 3 * number);
}

int main(void) {
    static sef_forth_state_t state;
    sef_init(&state);
    sef_register_c_word(&state, "triple", triple, false);
    sef_eval_string(&state, "5 triple . cr"); // This will print "15"
    return 0;
}
```

### Overriding the I/O functions

Here is a small program that changes the output function so that it prints everything in hex.

```c
#include <stdio.h>
#include <stdlib.h>
#include <SEForth.h>

void sef_output(char c) {
    printf("%02x ", c);
}

int main(void) {
    sef_forth_state_t* state = malloc(sizeof(sef_forth_state_t));
    sef_init(state);
    sef_eval_string(state, ".( hello )"); // This will print "68 65 6c 6c 6f 20 "
    free(state);
    return 0;
}
```


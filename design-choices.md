# Forth state

## Design choices

**Do I want to keep the stack, memory, and whatnot constant known when compiling sef or do I want the user to choose them when calling the sef API?** I think I would always need to have the included word selection optional to help embedding sef in other context such as embedded systems or the devzat plugin, so I will always need to have some sef-compilation options. I might as well go all in and keep the various sizes and configs the same.

**Do I want to add a dedicated control flow stack or could I reuse the data stack for that purpose?** I would probably benefit from a dedicated stack as it would make coding and debugging sef easier. If after the rewrite, it turns out that the data stack is stable enough for this purpose during word compilation, I could use it and remove the control flow stack.

## State content

The previous choice of requiring the use of malloc to build state was pretty poor if I want to put sef in more constrained environment. Instead, all the state will be in a contiguous memory space. Here are the elements I will have:

- Memory space, all as `sef_int_t`, but the forth memory will probably be casted to bytes at some point.
  - forth memory[]
  - pad[]
  - data stack[]
  - return stack[]
  - control flow stack[]
- Pointer or indexes to the memory (indexes point to the last used memory space)
  - here (pointer)
  - last dictionary entry (pointer)
  - data stack index
  - return stack index
  - control flow stack index
- Internal variables (there was more of them in the previous implementation, but I think I could use inside-forth words instead)
  - compiling (equivalent of the `STATE` word)
  - base
  - code pointer (either a pointer to the word-tag we are executing when executing a forth word, or NULL to tell that nothing is going on)
  - error encountered
- Parser content (some of the parser state, such as a postponing flag, will be store in the colon sys)
  - input buffer size
  - input buffer (will be a `char*` not owned by the state, no need to copy it)
  - parse area offset (see `>IN`)
  - input source
  - input refill
  - register both cases (used in case-sensitive mode to register both case of a standard word)

## Ideas for improvement

Adding a cache of words used by the compiler (such as `EXIT` or the run-time effect of control flow) would be very handy to speed-up compilation and make the system more robust to changes and redefinitions of compiler words.

## Error checking

If an error was encountered, the effect of `ABORT` are applied, and the `error encountered` field is set so that the user of sef as a library can see that something went wrong. The system is both too simple and too flexible for error bubbling to make much sense. 

# Dictionary

## Entries

The entries in the dictionary will be made of:
- address of the previous entry (null for the first element of the list)
- size of the name
-  content of the name (null terminated to make it easier to process in C)
- Padding if needed to align next entry to `sef_word_t`
- Word execution function
- parameters

The name is written as a NULL terminated string in the dictionary, but it could have come from outside not NULL terminated. The NULL termination is handled inside of the dictionary entry writing. Similarly, if the forth should be case-insensitive, the name will be processed during writing to be uppercase.

The need for NULL-terminated name means that NULL bytes are invalid in names, but I think I can live with that.

## Types of entries

### Literal-ish

Constants and string-literal will be stored in the dictionary, with the word executing function fetching the content from the parameters and putting it on the state's stack. Variable will be built around constants with a Forth definition. Number literal won't need to be put in the dictionary, as they don't need to be fetched.

I know that I can implement `VARIABLE` out of `CONSTANT`. I also saw on the forth standard website a definition of `CONSTANT` witch uses `POSTPONE` and `LITERAL`, so I might not need to define special behavior for constant at all. I still want strings to be handled from the C side, as I need to copy them anyway to give memory ownership to the forth state.

### C words

C words will be in the form `void fnc(sef_forth_state_t* fs);`. They will either be only run time or only compile time. The word executing function will check if the context is right for execution (compile or run time) and call the function in the entry's parameter if it is. If I need a word with effect at both compile and run time, I will have two word for compile and run time effect. They will either have separate names and or there will be a cache and some word shadowing.

It's still the same word executing function in both cases. The parameters to the dictionary entry will be as follow:
- word-tag
- function pointer
See the Forth word for the word-tag definition.

### Forth words

Forth words will be a bit more complex. The entry parameter will be like this:
- word-tags
  word-content
- word-tags
  word-content
[...]
- word-tags for both compile and run time
  exit

The word-tags will be a bit field with the following values
1. Compile-time word. 1 if the word should be called at compile time and 0 if it should be called at run time. This flag will be present on every sub-word, but is only relevant for the first one.
2. Postponed word. 1 if the word was defined after a `POSTPONE` and 0 otherwise. I feel like having a flag to tell if a word was postponed would be more comfortable that carrying a postpone execution in compiled words.
3. Number literal. 0 if the word is a normal word to call (word-content is an address), 1 if it as a number literal (word-content is a number as-is).

The word-content is either the value of the number literal, or the address of the dictionary entry of the word to call.

# Executing words

The loop execution process for normal runtime is the following:
- Read the code pointer word.
- If it is null, return without doing more. Otherwise, execute it.
- Increase the code pointer by two step, to point to the tag of the next word in the current forth word.

When calling a new forth word, the process of the word execution function is the following:
- Push the current word pointer to the return stack (don't forget to cache it).
- Read the address of the word to be executed and put it in the code pointer.
- Decreasing the code pointer by two step, so that the end of the word execution would make it point to the first element of the callee word.

The exit word will have a very simple behavior:
- Pop the top element of the return stack.
- Write it as the code pointer.
The word execution will point to the next word on its own.

# Compiling words

## Base process

`:` will put a colon-sys on the stack. Probably a single `sef_int_t`. Bit 0 of this number tells if we are postponing a word or not. I'll probably find more things to add to it in the future.

`:` reads the next word for the name to use, and create the new entry in the dictionary. Subsequent words are read and added to the definition until `;` or `[` unset the compiling flag. This will be done in a while loop checking the compiling flag. This C function will be called as-is by `]`, it will also be called after creating the dictionary entry by `:` and `:noname`. `:noname` will create the new entry and put the execution token on the stack before the colon-sys token.

When compiling `RECURSE`, the address to use will be read from the last dictionary entry field from the state. That entry must be set by whichever words put the state in compiling mode.

## Control flow

Control flow will use at compile time the control flow stack.

### If-else-then

At compile-time, `IF` is replaced by an blank number literal and a `if-runtime` word. The address of the blank number is added to the control flow stack. The next time `ELSE` or `THEN` are encountered, the address of the closing word is put inside the number literal. The runtime `if` pops the two elements from the stack (the flag and the injected address) and jumps if needed. `ELSE` will have a similar behavior but always jump to `THEN`. `THEN` doesn't needs to do anything. Note that the code pointer increasing after a subword execution is taken into account.
 
Similar mechanism will be used for loops.

# Interpreting

We can check that we are interpreting if the code pointer is NULL and the compiling flag is false.

For interpreting, we could have a dedicated function that parse the words, and either read numbers or search words in the dictionary.

# Input source

The input source is handled as a void pointer to some data (like a string or a `FILE*`) and a function to be called for `REFILL`. It could be fun to manage those field from Forth. But as a first implementation, doing everything in C is file.

From the outside, the API will be to feed the forth state an input source, such as a raw string. SEF will keep on parsing and executing until the refill function returns nothing.

# Output

I will keep the old behavior of a weak `sef_output` function.


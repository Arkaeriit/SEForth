# Forth state

## Design choices

**Do I want to keep the stack, memory, and whatnot constant known when compiling sef or do I want the user to choose them when calling the sef API?** I think I would always need to have the included word selection optional to help embedding sef in other contexts such as embedded systems or the devzat plugin, so I will always need to have some sef-compilation options. I might as well go all in and keep the various sizes and configs the same.

**Do I want to add a dedicated control flow stack or could I reuse the data stack for that purpose?** I would probably benefit from a dedicated stack as it would make coding and debugging sef easier. If after the rewrite, it turns out that the data stack is stable enough for this purpose during word compilation, I could use it and remove the control flow stack. On the other hand, not having a control flow stack ensures that the stack stays balanced between word definitions.

## State content

The previous choice of requiring the use of malloc to build state was pretty poor if I want to put sef in a more constrained environment. Instead, all the state will be in a contiguous memory space. Here are the elements I will have:

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
  - bye (set to true when the execution should finish)
  - quit (set to true when the execution should fall back in the prompt)
- Parser content (some of the parser state, such as a postponing flag, will be stored in the colon sys)
  - input buffer size
  - input buffer (will be a `char*` not owned by the state, no need to copy it)
  - parse area offset (see `>IN`)
  - input source
  - input refill
  - source id
  - compiling system word (set to true at init and then back to false just before giving the hand to the user, used to set the system word flag if needed)

## Ideas for improvement

Adding a cache of words used by the compiler (such as `EXIT` or the run-time effect of control flow) would be very handy to speed-up compilation and make the system more robust to changes and redefinitions of compiler words.

## Error checking

If an error was encountered, the effects of `ABORT` are applied, and the `error encountered` field is set so that the user of sef as a library can see that something went wrong. The system is both too simple and too flexible for error bubbling to make much sense. 

## Double cell numbers

As I intend for sef to run on 32-bit systems for embedded tasks and on 64-bit systems for applicative tasks, I don't think I need double cell numbers. I'll still store some numbers on two cells to comply with the standard, but the high-cell will only be a dumb sign extension.

# Dictionary

## Entries

The entries in the dictionary will be made of:
- magic word (used to tell if a pointer is to an entry)
- address of the previous entry (null for the first element of the list)
- size of the name
- content of the name (null terminated to make it easier to process in C)
- Padding if needed to align next entry to `sef_word_t`
- Word tags
- special parameters (only used for DOES> words)
- parameters

The name is written as a NULL terminated string in the dictionary, but it could have come from outside not NULL terminated. The NULL termination is handled inside of the dictionary entry writing. Similarly, if the forth should be case-insensitive, the name will be processed during writing to be uppercase.

The need for NULL-terminated name means that NULL bytes are invalid in names, but I think I can live with that.

The word tags is a bitfield with the following entries:
1. Immediate word: set to 1 if the word should be run at compile time.
2. Special execution: if set to 1, the word's effect have been chosen with `DOES>` and to execute it, we must get the code from the special parameters and the data from the parameters.
3. System word: if set to 1, it means that this is a system word, that can't be removed by `FORGET` and that should use a case-insensitive matching function.
4. C words: The word's execution will be made by calling the function stored in the parameters.
5. Forth word: The word's execution will be to move the code pointer to the parameters.
6. Create: The word's execution will be to push it's parameters on the stack.


## Types of entries

### Literal-ish

Number literal will be stored on the sub word list as a special `(LITERAL)` word followed by the raw value of the number literal on the next cell. `(LITERAL)` will perform the reading from the sub-word list.

During interpretation, string-literal will be pointed from the input buffer as a transient region. This means that the string won't be owned by the Forth state as it can change if new words are defined. During compilation, they will be appended to the current definition after a `(string)` word that behaves similarly to `(LITERAL)`. Registering a string inside of `[ ... ]` would corrupt the current definition.

Counted strings could probably be managed with `CREATE` in interpreted mode and like normal strings, in some way, in compiling mode.

`VARIABLE` and `CONSTANT` will be defined in forth, no need for a special type of entry.

### C words

C words will be in the form `void fnc(sef_forth_state_t* fs);`.

### Forth words

Forth words will be a bit more complex. The entry parameter will be like this:
- sub word
- sub word
[...]
- exit subword

In special case, such as the handling of `(LITERAL)`, the sub word is replaced by a literal.

# Executing words

The loop execution process for normal runtime is the following:
- Read the code pointer word.
- If it is null, return without doing more. Otherwise, execute it.
- Increase the code pointer by one, to point to the next word in the current forth word.

When calling a new forth word, the process of the word execution is the following:
- Push the current word pointer to the return stack.
- Read the address of the word to be executed and put it in the code pointer.
- Decreasing the code pointer by one step, so that the end of the word execution would make it point to the first element of the callee word.

The exit word will have a very simple behavior:
- Pop the top element of the return stack.
- Write it as the code pointer.
The word execution will point to the next word on its own.

# Compiling words

## Base process

`:` will put a colon-sys on the stack. Probably a single `sef_int_t`. I don't think I need any data, but I should probably add a magic word to check the integrity of the stack during a definition.

`:` reads the next word for the name to use, and creates the new entry in the dictionary. Subsequent words are read and added to the definition until `;` or `[` unset the compiling flag. This will be done in a while loop checking the compiling flag. This C function will be called as-is by `]`, it will also be called after creating the dictionary entry by `:` and `:noname`. `:noname` will create the new entry and put the execution token on the stack before the colon-sys token.

When compiling `RECURSE`, the address to use will be read from the last dictionary entry field from the state. That entry must be set by whichever words put the state in compiling mode.

The interpretation/compilation process will be the following.
- Try to read a word. If this fails, try to refill and retry. If refill fails, the process can stop.
- Find the word in the dictionary, if not found, decode it as a number literal. If it doesn't convert, complain and abort.
- Execute the word or add it to the definition being run depending on the compiling state and the immediate tag of the word.
- Let the execution run until we are back into an interpreting/compiling state.
- Restart this process.

## Control flow

Control flow will use at compile time the control flow stack.

### If-else-then

At compile-time, `IF` is replaced by a blank number literal and a `if-runtime` word. The address of the blank number is added to the control flow stack. The next time `ELSE` or `THEN` are encountered, the address of the closing word is put inside the number literal. The runtime `if` pops the two elements from the stack (the flag and the injected address) and jumps if needed. `ELSE` will have a similar behavior but always jump to `THEN`. `THEN` doesn't need to do anything. Note that the code pointer increasing after a subword execution is taken into account.
 
Similar mechanism will be used for `BEGIN` loops.

### Do-loop

Even if it could seem like do-loops can be made from begin-loops, the combination of `LEAVE` needing to be implemented and `BEGIN` not being allowed to touch at the stacks at runtime makes it too hard to derive one from the other.

For `?DO` and `DO`, I can make one from the other. As the overhead would be O(n) if I take `DO` as a base and `O(1)` if I take `?DO` as a base, I'll take `?DO` as a base.

## Postpone

Postpone would be hard to define for forth only. Indeed, it would benefit from writing literals of the execution token, but defining `LITERAL` can be done with `POSTPONE`. I could try to bypass the use of `LITERAL` and `POSTPONE`, but that would make it hard to read. And writing it in C makes it easier to display error messages if `POSTPONE` couldn't find what to do.

# Interpreting

We can check that we are interpreting if the code pointer is NULL and the compiling flag is false.

For interpreting, we could have a dedicated function that parses the words, and either read numbers or search words in the dictionary.

# Input source

The input source is handled as a void pointer to some data (like a string or a `FILE*`) and a function to be called for `REFILL`. It could be fun to manage those fields from Forth. But as a first implementation, doing everything in C is fine.

From the outside, the API will be to feed the forth state an input source, such as a raw string. SEF will keep on parsing and executing until the refill function returns nothing.

# I/O

Input and output will come from user-overridable weak functions. I would have preferred to have them written in Forth, but I need to print messages when the Forth state fails, in which case they couldn't be executed.


# Forth state

## Design choices

**Do I want to keep the stack, memory, and whatnot constant known when compiling sef or do I want the user to choose them when calling the sef API?** I think I would always need to have the included word selection optional to help embedding sef in other context such as embedded systems or the devzat plugin, so I will always need to have some sef-compilation options. I might as well go all in and keep the various sizes and configs the same.

**Do I want to add a dedicated control flow stack or could I reuse the data stack for that purpose?** I would probably benefit from a dedicated stack as it would make coding and debugging sef easier. If after the rewrite, it turns out that the data stack is stable enough for this purpose during word compilation, I could use it and remove the control flow stack. On the other hand. Not having a control flow stack ensures that the stack stay balanced between word definitions.

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
  - compiling system word (set to true at init and then back to false just before giving the hand to the user, used to set the system word flag if needed)

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
- Word tags
- Word execution function
- parameters

The name is written as a NULL terminated string in the dictionary, but it could have come from outside not NULL terminated. The NULL termination is handled inside of the dictionary entry writing. Similarly, if the forth should be case-insensitive, the name will be processed during writing to be uppercase.

The need for NULL-terminated name means that NULL bytes are invalid in names, but I think I can live with that.

The word tags is a bitfield with the following entries:
1. Immediate word: set to 1 if the word should be run at compile time.
2. Special execution: if set to 0, the word is executed by calling its word execution function, this is the normal case. If set to 1, it means that the word has a special meaning given by `DOES>`. The word execution function will be replaced by an address to the list of subwords from an other word in the dictionary. To execute the current word, we must push the code pointer (as in a word call) and replace it with the address at the place of the word executing function. To ensure execution of the desired word, `DOES>` will set its own address as the word executing function replacement, and the execution will set the code pointer to it. Then, the execution will slide it to the word we want to execute (right after the `DOES>`).
3. System word: if set to 1, it means that this is a system word, that can't be removed by `FORGET` and that should use a case-insensitive matching function.

I could probably get rid of the word executing function as a c function and replace it with a Forth one. This would remove the special treatment for `DOES>`-using words. I currently only have WEF for C words, forth words, and strings. Strings could probably be managed with a `CREATE` and a `DOES>`, Especially considering I don't need it to start bootstraping core words. Forth word would reuse the exeution code of `DOES>` words. For C words, I'll have to get hacky. Maybe create a dummy static entry somewhere in ROM wich will be the first entry... This would make the word execution more streamlined, but the state init a bit more hacky. It would also probably be closer to the standard's spirit. I think I'll probably test it at some point and do some performance monitoring to see if its worth it.

## Types of entries

### Literal-ish

Number literal will be stored on the sub word list as a special `(LITERAL)` word followed by the raw value of the number literal on the next cell. `(LITTERAL)` will perform the reading from the sub-word list.

During interpretation, string-literal will be stored in the dictionary, with the word executing function fetching the content from the parameters and putting it on the state's stack. This is the best way to ensure that we give to the forth state an owned copy of the string. During compilation, they will be appended to the current definition after a `(string)` word that behave similarly to `(LITERAL)`. Registering a string inside of `[ ... ]` would corrupt the current definition.

Conted strings could probably be managed with `CREATE` in interpreted mode and like normal strings, in some way, in compiling mode.

`VARIABLE` and `CONSTANT` will be defined in forth, no need for a special type of entry.

### C words

C words will be in the form `void fnc(sef_forth_state_t* fs);`. They will either be only run time or only compile time. The word executing function will check if the context is right for execution (compile or run time) and call the function in the entry's parameter if it is. If I need a word with effect at both compile and run time, I will have two word for compile and run time effect. They will either have separate names and or there will be a cache and some word shadowing.

It's still the same word executing function in both cases. The parameters to the dictionary entry will be as follow:
- word-tag
- function pointer
See the Forth word for the word-tag definition.

### Forth words

Forth words will be a bit more complex. The entry parameter will be like this:
- sub word
- sub word
[...]
- exit subword

In special case, such as the handling of `(LITTERAL)`, the sub word is replaced by a literal.

# Executing words

The loop execution process for normal runtime is the following:
- Read the code pointer word.
- If it is null, return without doing more. Otherwise, execute it.
- Increase the code pointer by two step, to point to the tag of the next word in the current forth word.

When calling a new forth word, the process of the word execution function is the following:
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

`:` reads the next word for the name to use, and create the new entry in the dictionary. Subsequent words are read and added to the definition until `;` or `[` unset the compiling flag. This will be done in a while loop checking the compiling flag. This C function will be called as-is by `]`, it will also be called after creating the dictionary entry by `:` and `:noname`. `:noname` will create the new entry and put the execution token on the stack before the colon-sys token.

When compiling `RECURSE`, the address to use will be read from the last dictionary entry field from the state. That entry must be set by whichever words put the state in compiling mode.

The interpretation/compilation process will be the following.
- Try to read a word. If this fail, try to refill and retry. If refill fails, the process can stop.
- Find the word in the dictionary, if not found, decode it as a number literal. If it doesn't convert, complain and abbort.
- Execute the word or add it to the definition being run depending on the compiling state and the imediate tag of the word.
- Let the execution run until we are back into an interpreting/compiling state.
- Restart this process.

## Control flow

Control flow will use at compile time the control flow stack.

### If-else-then

At compile-time, `IF` is replaced by an blank number literal and a `if-runtime` word. The address of the blank number is added to the control flow stack. The next time `ELSE` or `THEN` are encountered, the address of the closing word is put inside the number literal. The runtime `if` pops the two elements from the stack (the flag and the injected address) and jumps if needed. `ELSE` will have a similar behavior but always jump to `THEN`. `THEN` doesn't needs to do anything. Note that the code pointer increasing after a subword execution is taken into account.
 
Similar mechanism will be used for `BEGIN` loops.

### Do-loop

Even if it could seems like do-loops can be made from begin-loops, the combination of `LEAVE` needing to be implemented and `BEGIN` not being allowed to touch at the stacks at runtime makes it too hard to derive one from the other.

For `?DO` and `DO`, I can make one from the other. As the overhead would be O(n) if I take `DO` as a base abd `O(1)` if I take `?DO` as a base, I'll take `?DO` as a base.

## Postpone

Postpone would be hard to define for forth only. Indeed, it would benefit from writing literals of the execution token, but defining `LITERAL` can be done with `POSTPONE`. I could try to bypass the use of `LITERAL` and `POSTPONE`, but that would make it hard to read. And writing it in C makes it easier to display error messages if `POSTPONE` couldn't find what to do.

# Interpreting

We can check that we are interpreting if the code pointer is NULL and the compiling flag is false.

For interpreting, we could have a dedicated function that parse the words, and either read numbers or search words in the dictionary.

# Input source

The input source is handled as a void pointer to some data (like a string or a `FILE*`) and a function to be called for `REFILL`. It could be fun to manage those field from Forth. But as a first implementation, doing everything in C is file.

From the outside, the API will be to feed the forth state an input source, such as a raw string. SEF will keep on parsing and executing until the refill function returns nothing.

# Output

I will keep the old behavior of a weak `sef_output` function.


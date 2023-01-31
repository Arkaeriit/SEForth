# ASC miniForth

A simple Forth implementation. Aimed at being embedded in other projects or in embedded systemps.

This is a realy early work in progess.

## Differences with other Forth

### Dictionary

Unlike other Forth implementation, there is only a single entry for each word. This means that if you change the definition of a word, this will impact all the previously defined words that used the word being modified. On the plus side, making a recursive function is very easy to do. For example: `: rec-func ( n -- ) 1 - DUP 0< 0= IF ." Hello! " rec-func ELSE DROP THEN ;`

### Memory management

The traditional memory management of other Forth is completely ignored here. The memory is instead managed like in C. The word `ALLOT` takes a number of bytes and allocate it from the heap. A pointer to that memory is put on the stack. The memory should them be freed using `FREE`.

```Forth
1 CELLS ALLOT \ 1 cell is allocated on the heap
DUP 33 SWAP ! DUP @ . \ The memory is used
FREE \ The memory is freed when not needed anymore
```

For the sake of compatibility with other Forth, the word `HERE` is included but does not do anything. This means that you can use constructs such as `HERE 1 CELLS ALLOT` to write code compatible with other Forth.

### Cell size

To make the language easier, only one type of number is supported. There is no number using two cells. Furthermore, as the cell should be able to contain raw pointers, it should be large enough. The default type for the cell defined in `amf_config.h` is `intptr_t`.

### Case

By default, ASCminiForth is case-insensitive. You can enable case-sensitivity in `amf_config.h`. If the case-sensitivity is enabled, the default Forth words can be both upper or lower case.

### Strings

Strings, such as generated with `S"` are null-terminated. Thus you are free to manipulate them as Forth strings or as C strings. Some extra words have been added to manipulate them as C string such as `strlen` or `print`.

## Available words

Here is a list of all words, with some notes when the word differs from the standard:

| word | notes |
|-|-|
| `swap` | |
| `rot` | |
| `dup` | |
| `drop` | |
| `>r` | |
| `r>` | |
| `roll` | |
| `pick` | |
| `+` | |
| `-` | |
| `*` | |
| `*/` | |
| `*/mod` | |
| `/` | |
| `/mod` | |
| `abs` | |
| `<` | |
| `0<` | |
| `0=` | |
| `=` | |
| `and` | |
| `or` | |
| `xor` | |
| `if` | In case of unbalanced flow control, no checks are made, which will cause bugs or segfaults. |
| `else` | Same note as if. |
| `then` | Same note as if. |
| `begin` | Same note as if. |
| `until` | Same note as if. |
| `do` | Same note as if. |
| `i` | |
| `+loop` | Same note as if. |
| `loop` | Same note as if. |
| `allot` | Takes a number as input and returns a pointer to a chunk of memory of this size from the heap. |
| `cells` | |
| `here` | Does nothing. |
| `free` | Free memory allocated with allot. |
| `@` | |
| `!` | |
| `c@` | |
| `c!` | |
| `print` | Prints a null terminated string. |
| `strlen` | Returns the size of a null terminated string. |
| `.` | |
| `emit` | |
| `key` | |
| `exit` | |
| `abort` | |
| `cr` | |
| `base` | |
| `1+` | |
| `1-` | |
| `2dup` | |
| `2drop` | |
| `<>` | |
| `0<>` | |
| `>` | |
| `0>` | |
| `bl` | |
| `c` | |
| `cell+` | |
| `char+` | |
| `chars` | |
| `max` | |
| `negate` | |
| `min` | |
| `nip` | |
| `over` | |
| `tuck` | |
| `2over` | |
| `2>r` | |
| `2r>` | |
| `S" ` | The string given is null terminated. |
| `." ` | |

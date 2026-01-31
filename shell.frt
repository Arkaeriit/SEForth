1024 constant (repl-buffer-size)
(repl-buffer-size) buffer: (repl-buffer) \ 
: (rep) ." ok " (repl-buffer) (repl-buffer-size) accept (repl-buffer) swap 2dup
    + 1- c@ 4 = if 1
    else 0 (evaluate) 0 then ;
: (repl) begin (rep) until ;


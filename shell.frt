1024 constant (repl-buffer-size)
(repl-buffer-size) buffer: (repl-buffer) \ 
: (bye-if-exists) ( -- ) [ s" bye" (find) ] literal literal swap if execute else drop then ;
: (rep) ." ok " (repl-buffer) (repl-buffer-size) accept (repl-buffer) swap 2dup
    + 1- c@ 4 = if (bye-if-exists) 1
    else 0 (evaluate) 0 then ;
: (repl) begin (rep) until ;


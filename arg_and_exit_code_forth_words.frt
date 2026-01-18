variable exit-code

( -------------------- Getting arguments from the host OS -------------------- )

variable argc
variable argv
: feed-arguments-from-os ( n addr -- ) argv ! argc ! ;

( --------------------------- Giving I-th argument --------------------------- )

\ Invalid if u > argc @
: arg ( u -- addr u ) argv ! + dup strlen ;

( ------------------- Giving next argument by shifting them ------------------ )

variable (current-arg)
1 (current-arg) !
: shift-args ( -- ) 1 (current-arg) +! ;
: next-arg   ( -- addr u ) (current-arg) @ dup argc @ 1- > if drop 0 0 else arg shift-args then ;


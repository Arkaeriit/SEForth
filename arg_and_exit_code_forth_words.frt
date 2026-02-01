variable exit-code

( -------------------- Getting arguments from the host OS -------------------- )

0 value argc
0 value argv
: feed-arguments-from-os ( n addr -- ) to argv to argc ;

( --------------------------- Giving I-th argument --------------------------- )

\ Invalid if u > argc @
: arg ( u -- addr u ) argv swap cells + @ dup strlen ;

( ------------------- Giving next argument by shifting them ------------------ )

variable (current-arg)
1 (current-arg) !
: shift-args ( -- ) 1 (current-arg) +! ;
: next-arg   ( -- addr u ) (current-arg) @ dup argc @ 1- > if drop 0 0 else arg shift-args then ;

( --------------------- Making ABORT change the exit code -------------------- )

: abort ( -- ) -1 exit-code ! abort ;
: abort" ( parse until " -- ) postpone s" state @ if postpone type postpone cr postpone abort else 2dup . . cr type cr abort then ; immediate \ "


0 value block-buffers-list
0 value current-buffer

( The block buffers are stored in a linked list. Their content is the block id )
( on a cell, then a flag telling if they are updated on a cell, a flag telling )
( if the data is ready [either read from block or written] and then the block  )
( content on 1024 bytes. The flags are on 1 cells.                             )

: init-block-buffer ( -- ) 0 , 0 , 0 , block-size allot ;
: init-block-buffers number-of-block-buffers 0 ?do block-buffers-list add-new-node init-block-buffer to block-buffers-list loop ;
init-block-buffers

: get-block-number ( buffer -- addr ) ;
: get-update-flag ( buffer -- addr ) cell+ ;
: get-data-ready-flag ( buffer -- addr ) get-update-flag cell+ ;
: get-buffer-content ( buffer -- addr ) get-data-ready-flag cell+ ;

( Write f in the update field. Also set the data ready flag to 1. )
: (update) ( buffer f -- ) swap dup true swap get-data-ready-flag ! get-update-flag ! ;
: update ( -- ) current-buffer 1 (update) ;

( Call write-buffer with a buffer node )
: write-buffer ( buffer -- ) dup get-block-number @ swap get-buffer-content write-buffer ;

( Call read-buffer with a buffer node )
: read-buffer ( buffer -- ) dup dup get-block-number @ swap get-buffer-content read-buffer 0 (update) ;

( Save the buffer content if it needs saving and unset the update flag )
: (save-buffer) ( buffer -- ) dup get-update-flag @ if dup write-buffer 0 (update) else drop then ;

( Set found-buffer to the input buffer if its asigned block is the one in searched-block )
0 value found-buffer 0 value searched-block
: search-a-buffer ( buffer -- ) dup get-block-number @ searched-block = if to found-buffer else drop then ;

( If one of the buffers is assigned to the input number, return it. Otherwise return 0 )
: search-buffer ( u -- buffer? ) to searched-block 0 to found-buffer block-buffers-list ['] search-a-buffer exec-on-list found-buffer ;

( Return a buffer from the list. Cycle hrough indexes )
0 value forced-buffer-index
: get-next-forced-buffer ( -- buffer ) forced-buffer-index number-of-block-buffers mod dup 1+ to forced-buffer-index block-buffers-list swap get-node-at-index ;

: buffer ( u -- buffer ) dup search-buffer ?dup 0= if get-next-forced-buffer dup (save-buffer) 2dup get-block-number ! dup false swap get-data-ready-flag ! ( if we got a random buffer, then there is 0 in the data ready flag ) then
    ( on stack, there is only the buffer object )
    nip dup to current-buffer get-buffer-content ;
    
: block ( u -- buffer ) buffer current-buffer get-data-ready-flag @ 0= ( addr  f ) if current-buffer read-buffer then ;

variable blk
0 blk !
: restore-blk-if-needed ( -- ) blk @ if blk @ block >source drop ! then ;
: load ( ... u -- ... ) blk @ >r dup blk ! block block-size blk @ (evaluate) r> blk ! restore-blk-if-needed ;
: evaluate blk @ >r 0 blk ! evaluate r> blk ! ;

: (empty-buffer) ( buffer -- ) 0 swap 2dup get-block-number ! 2dup get-update-flag ! get-data-ready-flag ! ;
: empty-buffers block-buffers-list ['] (empty-buffer) exec-on-list ;
: save-buffers block-buffers-list ['] (save-buffer) exec-on-list ;
: flush save-buffers empty-buffers ;

64 constant block-line-len
: list ( u -- ) block block-size block-line-len / 0 do
    i 2 .r dup block-line-len type cr block-line-len +
loop drop ;

variable scr
0 scr !
: list ( u -- ) dup scr ! list ;

: thru ( ... u u -- ... )  1+ swap  do i load loop ;

: refill ( -- flag ) source-id blk @ = if 0 >in ! >source block-size swap ! 1 blk +! blk @ block swap ! true else refill then ;

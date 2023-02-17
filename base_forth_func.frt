: 1+ 1 + ;
: 1- 1 - ;
: 2dup swap dup rot dup rot swap ;
: 2drop drop drop ;
: <> = 0= ;
: 0<> 0 <> ;
: > 2dup < 0= rot rot <> and ;
: 0> 0 > ;
: bl s"  " drop c@ ;
: c 1 allot ;
: cell+ 1 cells + ;
: char+ 1+ ;
: chars ;
: max 2dup < if swap then drop ;
: negate -1 * ;
: min 2dup max negate + + ;
: nip swap drop ;
: over 2dup drop ;
: tuck dup rot swap ;
: 2over 3 pick 3 pick ;
: 2>r swap >r >r ;
: 2r> r> r> swap ;
: decimal 10 base ! ;
:macro loop 1 +loop ;
: space bl emit ;
: spaces dup 0> if 0 do space loop else drop then ;
: invert -1 xor ;
: type dup 0> if 0 do dup c@ emit 1+ loop drop else drop then ;
:macro while if ;
:macro repeat 0 until then ;
: ?dup dup if dup then ;


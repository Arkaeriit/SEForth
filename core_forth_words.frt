( ---------------------------- Stack manipulation ---------------------------- )

: 2dup ( x1 x2 -- x1 x2 x1 x2 )swap dup rot dup rot swap ;
: 2drop ( x1 x2 -- ) drop drop ;
: nip ( x1 x2 -- x2 ) swap drop ;
: over ( x1 x2 -- x1 x2 x1 ) 2dup drop ;
: tuck ( x1 x2 -- x2 x1 x2 ) dup rot swap ;
: 2over ( x1 x2 x3 x4 -- x1 x2 x3 x4 x1 x2 ) 3 pick 3 pick ;
: 2swap ( x1 x2 x3 x4 -- x3 x4 x1 x2) rot >r rot r> ;
: ?dup ( n1 -- n1 | [n1 n1] ) dup if dup then ;
: r@ ( -- x1 ) ( x1 -- x1 ) postpone r> postpone dup postpone >r ; immediate
: 2>r ( x1 x2 -- ) ( -- x1 x2 ) postpone swap postpone >r postpone >r ; immediate
: 2r> ( -- x1 x2 ) ( x1 x2 -- ) postpone r> postpone r> postpone swap ; immediate
: 2r@ ( -- x1 x2 ) ( x1 x2 -- x1 x2 ) postpone 2r> postpone 2dup postpone 2>r ; immediate

( --------------------------- Double words emulation ------------------------- )

: s>d ( n -- d ) dup 0 < if 1 else 0 then ;
: d>s ( d -- n ) drop ;
: um* ( u u -- d ) * 0 ;
:  m* ( u u -- d ) * s>d ;

( ----------------------------------- Math ----------------------------------- )

: 1+ ( n -- n ) 1 + ;
: 1- ( n -- n ) 1 - ;
: <> ( x1 x2 -- b ) = 0= ;
: 0<> ( n -- b ) 0 <> ;
: > ( n1 n2 -- b ) 2dup < 0= rot rot <> and ;
: 0> ( n1 -- b ) 0 > ;
: max ( n1 n2 -- n1 | n2 ) 2dup < if swap then drop ;
: /mod ( n1 n2 -- n3 n4 ) swap s>d rot sm/rem ;
: mod ( n n -- n ) /mod drop ;
: / ( n n -- n ) /mod swap drop ;
: */mod ( n n n -- n n ) >r * r> /mod ;
: */ ( n n n -- n ) >r * r> / ;
: negate ( n -- n ) -1 * ;
: min ( n1 n2 -- n1 | n2 ) 2dup max negate + + ;
: invert ( n -- n ) -1 xor ;
: +! ( n addr -- ) dup @ rot + swap ! ;
: 2* ( n -- n ) 2 * ;
: 2/ ( n -- n ) 2 / ;
: u> ( u u -- b ) 2dup u< 0= rot rot <> and ;
: within ( test low high -- flag ) over - >r - r> u< ;
: lshift ( x u -- x ) 0 ?do 2* 1 +loop ;
: rshift ( x u -- x ) 0 ?do 2/ 1 +loop ;

( ----------------------------- Memory management ---------------------------- )

: c ( -- ) 1 allot ;
: cell ( -- n ) 1 cells ;
: cell+ ( n -- n ) 1 cells + ;
: , ( x -- ) here cell allot ! ;
: char+ ( n -- n ) 1+ ;
: chars ( n -- n ) ;
: compile, ( xt -- ) , ;
: c, ( c -- ) here 1 chars allot c! ;
: 2! ( x x addr -- ) swap over ! cell+ ! ;
: 2@ ( addr -- x x ) dup cell+ @ swap @ ;
: aligned ( addr -- addr ) begin dup cell mod 0<> while 1+ repeat ;
: align ( -- ) begin here aligned here <> while 1 allot repeat ;

( ---------------------------------- Macros ---------------------------------- )

\ Not standard, but quite handy to define standard words

: char ( "word" -- c ) parse-name if c@ else drop 0 then ;
: save-here ( c-addr u -- ) dup , 0 ?do dup c@ c, char+ 1 +loop drop align ;
: read-mem-saved-here ( addr  -- c-addr u ) dup @ swap cell+ swap ;
: macro: ( "read a definition until ;" -- ) create immediate (literal) [ char ; , ] parse save-here does> read-mem-saved-here evaluate ;

( ------------------------------- Flow control ------------------------------- )

macro: do 2dup = if swap 1- swap then ?do ;
macro: loop 1 +loop ;
macro: until 0= while repeat ;
macro: again 0 until ;
macro: unloop 2drop drop ;
macro: i r@ ;
macro: j r> r> r> r@ swap >r swap >r swap >r ;

( ---------------------------------- Strings --------------------------------- )

: count ( addr -- addr n ) dup char+ swap c@ ;
\ TODO not sure it's working
: accept ( addr n -- n ) 1- dup 0 > 0= if 2drop 0 exit then
0 begin
key dup 10 = if drop nip nip exit then
( addr n1 n2 c )
swap >r swap >r over c! 1+ r> r> 1+
( addr n1 n2 )
2dup = if nip nip exit then until ;
\ Writes the given string as a counted string a few bytes away from HERE.
: (uncount-loop) ( addr c-addr -- addr+1 c-addr+1 ) 1+ >r dup c@ swap 1+ swap r@ c! r> ;
: uncount ( addr u -- c-addr ) HERE 8 cells + >r r@ 2dup c! swap 0 ?do (uncount-loop) loop 2drop r> ;

( -------------------------- Compiling-related words ------------------------- )

: buffer: ( u "name" -- ) create allot ;
: variable ( "name -- ) align cell buffer: ;
: constant ( n "name" -- ) create , does> @ ;
: literal ( x -- ) ( -- x ) postpone (literal) , ; immediate
: word ( c "parse a word" -- c-addr ) parse uncount ;
: find ( c-addr -- xt f ) count (find) ;
: recurse ( -- ) dictionary @ compile, ; immediate
: marker ( "consume a name" -- ) create dictionary @ , dictionary @ @ , does> dup @ where ! cell+ @ dictionary ! ;

( ---------------------------- Memory manipulation --------------------------- )

: fill ( addr u c -- ) swap 0 ?do
    swap 2dup c! 1+ swap
loop 2drop ;
: erase ( addr u -- ) 0 fill ;
: move ( addr addr u -- ) 0 ?do over c@ over c! 1+ swap 1+ swap loop 2drop ;

( ---------------------------------- Display --------------------------------- )

: decimal ( -- ) 10 base ! ;
: bl ( -- c ) s"  " drop c@ ;
: space ( -- ) bl emit ;
: spaces ( n -- ) dup 0> if 0 do space loop else drop then ;
: type ( addr n -- ) dup 0> if 0 do dup c@ emit 1+ loop drop else 2drop then ;
: [char] ( -- c ) ( consume a name ) char postpone literal ; immediate
: hex ( -- ) 16 base ! ;

( --------------------------------- Execution -------------------------------- )

: ' ( "name" -- xt ) parse-name (find) 0= if abort ( TODO better error message ) then ;
: ['] ( -- xt ) ( consume a name ) ' postpone literal ; immediate

( ----------------------------------- Misc. ---------------------------------- )

: false ( -- b ) 0 ;
: true  ( -- n ) 0 0= ;

( ---------------------------- Numeric conversion ---------------------------- )
64 constant <#-buff-len
variable <#-buff <#-buff-len allot
variable <#-cnt

: <# ( -- ) 0 <#-cnt ! ;
: (#) ( u -- c ) dup 9 > if 10 - [char] a + else [char] 0 + then ;
: <#-addr ( -- addr ) <#-buff <#-buff-len + <#-cnt @ - ;
: hold ( c -- ) 1 <#-cnt +! <#-addr c! ;
: holds ( addr n -- ) begin dup while 1- 2dup + c@ hold repeat 2drop ;
\ TODO: I'm not sure printing negative numbers work... Or is it reading negative numbers...
: sign ( n -- ) 0 < if [char] - hold then ;
: # ( ud -- ud ) base @ um/mod swap (#) hold s>d ;
: #s ( ud -- ud ) begin # 2dup d>s 0= until ;
: #n ( n -- ud ) dup abs s>d #s swap sign ;
: #> ( du -- addr n ) 2drop <#-addr <#-cnt @ ;
: . ( n -- ) <# #n #> type space ;
: u. ( u -- ) <# #s #> type space ;
: (x.r) ( n -- addr n ) <#-cnt @ - dup 0 > if
    0 do bl hold loop 0
    then #> ;
: u.r ( u n -- )swap s>d <# #s #> 2drop (x.r) type ;
: .r ( n n -- ) swap <# #n #> 2drop (x.r) type ;

: (to-digit) ( c -- n )
    dup [char] a < 0= if 10 + [char] a - exit then
    dup [char] A < 0= if 10 + [char] A - exit then
    dup [char] 9 >  if drop -1       exit then
                            [char] 0 - ;
: (is-digit?) ( c -- b ) (to-digit) dup 0 < 0= swap base @ < and ;
: (>number) ( u1 c-addr1 u1 -- u2 c-addr2 u2 ) dup 0 do
    over c@ (is-digit?) 0= if leave then
    1- rot rot dup c@ swap char+ swap
    ( u1- ud1 ca+ char )
    (to-digit) rot base @ * +
    ( u1- ca+ ud1+ )
    swap rot loop ;
: >number ( ud1 c-addr1 u1 -- ud2 c-addr2 u2 ) 2>r d>s 2r> (>number) 2>r s>d 2r> ;

( ----------------------------- Printing messages ---------------------------- )

: abort" ( parse until " -- ) postpone s" postpone type postpone abort ; immediate \ "
: .( [char] ) parse type ; immediate

( ----------------------------------- Defer ---------------------------------- )

\ Defered words are handled as a created word with an xt as parameter.
\ The default xt is exit so that calling it unititialized is okay.
: defer ( "parse a word" -- ) create ['] exit compile, does> @ execute ;
: defer! ( xt xt -- ) >body ! ;
: defer@ ( xt xt -- ) >body @ ;
: action-of ( "compile a name" -- xt ) state @ if postpone ['] postpone defer@ else ' defer@ then ; immediate
: is ( xt "compile a name -- )state @ if postpone ['] postpone defer! else ' defer! then ; immediate

( ------------------------------- Value and To ------------------------------- )

: value create , does> @ ;
: to state @ if postpone ['] postpone >body postpone ! else ' >body ! then ; immediate

( ----------------------------- String compiling ----------------------------- )

: (c") ( -- c-addr ) r> cell+ dup dup c@ + aligned cell - >r ;
: c" ( "parse string" -- c-addr ) ['] (c") compile, [char] " parse dup c,
    0 ?do dup c@ c, char+ loop drop align ; immediate


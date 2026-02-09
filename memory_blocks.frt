40 constant number-of-blocks
1024 constant block-size

block-size number-of-blocks * buffer: blocks
: get-block ( u -- addr ) 1- block-size * blocks + ;


: write-buffer ( u addr -- ) ." wr bu" cr swap get-block block-size move ;
: read-buffer ( u addr -- ) ." re bu" cr swap get-block swap block-size move ;


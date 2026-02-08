( This files contains code to handle write-once linked-list. The list will be  )
( stored on the memory indexed by HERE. A node of the list will be a pointer   )
( to the previous node followed by the content of the node. That content will  )
( be allocated and written by the creator of the new node. On the first node,  )
( the pointer to the previous node will be 0. Content of the list will be      )
( managed with the word exec-on-list which has the prototype [ addr -- ]       )
( where addr is the address of the node content, one cell after the pointer    )
( to the previous node.                                                        )

: add-new-node ( node -- node ) align here swap , ;
: create-new-list ( -- node ) 0 add-new-node ;
: exec-on-list ( node xt -- ) swap begin 2dup cell+ swap execute @ dup 0= until 2drop ;
: get-node-at-index ( node u -- node-content ) 0 ?do @ dup 0= if abort" Trying to get index lager than list size." then loop cell+ ;

: compare ( c-addr1 u1 c-addr2 u2 -- n ) rot 2dup = if
    drop 0 do
        ( c-addr1 c-addr2 )
        dup c@ swap 1+ swap rot dup c@ swap 1+ swap rot
        ( c-addr2 c-addr1 c2 c2)
        = 0= if 2drop -1 unloop exit then loop
        ( c-addrX c-addrY )
        2drop 0
    else 2drop 2drop 1 then ;


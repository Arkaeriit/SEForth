#!/bin/sh

# Run the command given as input and return the number of "OK"
# If the exit code is not 0 or if there is a "Failed", reuturn 0.
count_ok () {
    if ! $1 2> /dev/null > /dev/null
    then
        printf 0
        return
    fi
    if $1 | grep Failed 2> /dev/null > /dev/null
    then
        printf 0
        return
    fi
    $1 | sed 's: :\n:g' | grep -c OK
}

# Check if $1 exists. If it does not, write $2 to it. If it does, compare $2 to
# it's content. If $2 is bigger, write it to $1. If $2 is smaller, exit with an
# error.
compare_to_score () {
    if ! [ -e "$1" ]
    then
        printf '%s' "$2" > "$1"
    fi
    cmp=$(cat "$1")
    if [ "$2" -gt "$cmp" ]
    then
        printf '%s' "$2" > "$1"
    fi
    if [ "$2" -lt "$cmp" ]
    then
        echo "Error, not lower score than before!" > /dev/stderr
        exit 1
    fi
}

ok_std=$(count_ok "../seforth.bin ./standard-test.frt")

compare_to_score "./score" "$ok_std"
echo "OK!"


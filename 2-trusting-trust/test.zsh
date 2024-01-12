#!/bin/zsh

die() {
    echo "$1" >&2
    exit 1
}



setopt extended_glob
rm *~*.*

make quine
diff quine.c =(./quine) || die "Quine does not match"

make trojan-cc
make CC=./trojan-cc login
./login <<<'ken' || die 'Login as ken failed - trojan'

make CC=./trojan-cc cc
make CC=./cc login -B
./login <<<'ken' || die 'Login as ken failed - innocent-1'

make CC=./cc cc -B
make CC=./cc login -B
./login <<<'ken' || die 'Login as ken failed - innocent-2'

repeat 10 make CC=./cc cc -B
make CC=./cc login -B
./login <<<'ken' || die 'Login as ken failed - innocent-12'

echo 'All tests passed!'
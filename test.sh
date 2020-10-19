#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./scc "$input" > tmp.s || exit
  gcc -static -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42
assert 20 '5+19-4'
assert 20 ' 5+ 19 -4 '
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 4 '(3+5)/2'

echo OK

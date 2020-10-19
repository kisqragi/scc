#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./bin/scc "$input" > ./bin/tmp.s || exit
  gcc -static -o ./bin/tmp ./bin/tmp.s
  ./bin/tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'

assert 20 '5+19-4;'

assert 20 ' 5+ 19 -4 ;'

assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'

assert 10 '-10+20;'
assert 10 '- -10;'
assert 10 '- - +10;'

assert 0 '0==1;'
assert 1 '1==1;'
assert 1 '0!=1;'
assert 0 '1!=1;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 3 '1; 2; 3;'

assert 3 'a=3; a;'
assert 8 'a=5;z=3;a+z;'
assert 6 'a=b=3; a+b;'

assert 3 'foo=3; foo;'
assert 3 'foo123=3; foo123;'
assert 3 '_foo=3; _foo;'
assert 7 'foo=3; bar=4; foo+bar;'

echo OK

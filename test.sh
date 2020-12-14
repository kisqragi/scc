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

assert 0 '{ return 0; }'
assert 42 '{ return 42; }'

assert 20 '{ return 5+19-4; }'

assert 20 '{ return  5+ 19 -4 ; }'

assert 47 '{ return 5+6*7; }'
assert 15 '{ return 5*(9-6); }'
assert 4 '{ return (3+5)/2; }'

assert 10 '{ return -10+20; }'
assert 10 '{ return - -10; }'
assert 10 '{ return - - +10; }'

assert 0 '{ return 0==1; }'
assert 1 '{ return 1==1; }'
assert 1 '{ return 0!=1; }'
assert 0 '{ return 1!=1; }'

assert 1 '{ return 0<1; }'
assert 0 '{ return 1<1; }'
assert 0 '{ return 2<1; }'
assert 1 '{ return 0<=1; }'
assert 1 '{ return 1<=1; }'
assert 0 '{ return 2<=1; }'

assert 1 '{ return 1>0; }'
assert 0 '{ return 1>1; }'
assert 0 '{ return 1>2; }'
assert 1 '{ return 1>=0; }'
assert 1 '{ return 1>=1; }'
assert 0 '{ return 1>=2; }'

assert 3 '{ 1; 2; return 3; }'

assert 3 '{ a=3; return a; }'
assert 8 '{ a=5;z=3;return a+z; }'
assert 6 '{ a=b=3; return a+b; }'

assert 3 '{ foo=3; return foo; }'
assert 3 '{ foo123=3; return foo123; }'
assert 3 '{ _foo=3; return _foo; }'
assert 7 '{ foo=3; bar=4; return foo+bar; }'

assert 1 '{ return 1; 2; 3; }'
assert 2 '{ 1; return 2; 3; }'
assert 3 '{ 1; 2; return 3; }'
assert 2 '{ a=1; return b=2; 3; }'

assert 1 '{ ;;; return 1; ; }'

assert 2 '{ if (0) return 1; return 2; }'
assert 2 '{ if (1-1) return 1; return 2; }'
assert 1 '{ if (1) return 1; return 2; }'
assert 1 '{ if (2-1) return 1; return 2; }'
assert 2 '{ if (0) return 1; else { return 2; } }'
assert 3 '{ if (0) return 1; else if (0) return 2; return 3; }'

assert 45 '{ i;j=0; for(i=0;i<10;i=i+1) { j=j+i; } return j; }'
assert 3 '{ i=0; for (;;) { if (i>2) return i; i=i+1; } return 0; }'

assert 55 '{ i=0;j=0; while(i<10) { i=i+1; j=j+i; } return j; }'
assert 3 '{ i=0; while (1) { if (i>2) return i; i=i+1; } return 0; }'

assert 1 '{ x=1; return *&x; }'
assert 1 '{ x=1; y=&x; z=&y; return **z; }'
assert 2 '{ x=1; y=2; return *(&x+1); }'
assert 1 '{ x=1; y=2; return *(&y-1); }'
assert 5 '{ x=3; y=&x; *y=5; return x; }'
assert 5 '{ x=1; y=2; *(&x+1)=5; return y; }'
assert 5 '{ x=1; y=2; *(&y-1)=5; return x; }'

assert 3 '{ x=1; return (&x+3)-&x; }'
assert 5 '{ x=1; return (&x+3)-&x+2; }'

echo OK

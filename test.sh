#!/bin/bash

cat <<EOF | gcc -xc -c -o ./obj/tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }

int add6(int a, int b, int c, int d, int e, int f) {
    return a+b+c+d+e+f;
}
EOF

assert() {
  expected="$1"
  input="$2"

  ./bin/scc "$input" > ./bin/tmp.s || exit
  gcc -static -o ./bin/tmp ./bin/tmp.s ./obj/tmp2.o
  ./bin/tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 'int main() { return 0; }'
assert 42 'int main() { return 42; }'

assert 20 'int main() { return 5+19-4; }'

assert 20 'int main() { return  5+ 19 -4 ; }'

assert 47 'int main() { return 5+6*7; }'
assert 15 'int main() { return 5*(9-6); }'
assert 4 'int main() { return (3+5)/2; }'

assert 10 'int main() { return -10+20; }'
assert 10 'int main() { return - -10; }'
assert 10 'int main() { return - - +10; }'

assert 0 'int main() { return 0==1; }'
assert 1 'int main() { return 1==1; }'
assert 1 'int main() { return 0!=1; }'
assert 0 'int main() { return 1!=1; }'

assert 1 'int main() { return 0<1; }'
assert 0 'int main() { return 1<1; }'
assert 0 'int main() { return 2<1; }'
assert 1 'int main() { return 0<=1; }'
assert 1 'int main() { return 1<=1; }'
assert 0 'int main() { return 2<=1; }'

assert 1 'int main() { return 1>0; }'
assert 0 'int main() { return 1>1; }'
assert 0 'int main() { return 1>2; }'
assert 1 'int main() { return 1>=0; }'
assert 1 'int main() { return 1>=1; }'
assert 0 'int main() { return 1>=2; }'

assert 3 'int main() { 1; 2; return 3; }'

assert 3 'int main() { int a=3; return a; }'
assert 8 'int main() { int a=5;int z=3;return a+z; }'
assert 6 'int main() { int a,b; a=b=3; return a+b; }'

assert 3 'int main() { int foo=3; return foo; }'
assert 3 'int main() { int foo123=3; return foo123; }'
assert 3 'int main() { int _foo=3; return _foo; }'
assert 7 'int main() { int foo=3; int bar=4; return foo+bar; }'

assert 1 'int main() { return 1; 2; 3; }'
assert 2 'int main() { 1; return 2; 3; }'
assert 3 'int main() { 1; 2; return 3; }'
assert 2 'int main() { int a=1; return a=2; 3; }'

assert 1 'int main() { ;;; return 1; ; }'

assert 2 'int main() { if (0) return 1; return 2; }'
assert 2 'int main() { if (1-1) return 1; return 2; }'
assert 1 'int main() { if (1) return 1; return 2; }'
assert 1 'int main() { if (2-1) return 1; return 2; }'
assert 2 'int main() { if (0) return 1; else { return 2; } }'
assert 3 'int main() { if (0) return 1; else if (0) return 2; return 3; }'

assert 45 'int main() { int i;int j=0; for(i=0;i<10;i=i+1) { j=j+i; } return j; }'
assert 3 'int main() { int i=0; for (;;) { if (i>2) return i; i=i+1; } return 0; }'

assert 55 'int main() { int i=0;int j=0; while(i<10) { i=i+1; j=j+i; } return j; }'
assert 3 'int main() { int i=0; while (1) { if (i>2) return i; i=i+1; } return 0; }'

assert 1 'int main() { int x=1; return *&x; }'
assert 1 'int main() { int x=1; int *y=&x; int **z=&y; return **z; }'
assert 2 'int main() { int x=1; int y=2; return *(&x+1); }'
assert 1 'int main() { int x=1; int y=2; return *(&y-1); }'
assert 5 'int main() { int x=3; int *y=&x; *y=5; return x; }'
assert 5 'int main() { int x=1; int y=2; *(&x+1)=5; return y; }'
assert 5 'int main() { int x=1; int y=2; *(&y-1)=5; return x; }'

assert 3 'int main() { int x=1; return (&x+3)-&x; }'
assert 5 'int main() { int x=1; return (&x+3)-&x+2; }'

assert 3 'int main() { return ret3(); }'
assert 5 'int main() { return ret5(); }'

assert 7 'int main() { return add(4, 3); }'
assert 1 'int main() { return sub(4, 3); }'
assert 21 'int main() { return add6(1,2,3,4,5,6); }'
assert 66 'int main() { return  add6(1,2,add6(3,4,5,6,7,8),9,10,11);}'
assert 136 'int main() { return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); }'

assert 10 'int main() { return ret10(); } int ret10() { return 10; }'

assert 7 'int main() { return _add(3,4); } int _add(int x, int y) { return x+y; }'
assert 1 'int main() { return _sub(4,3); } int _sub(int x, int y) { return x-y; }'
assert 55 'int main() { return fib(9); } int fib(int x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }'

assert 3 'int main() { int x[2]; int *y=&x; *y=3; return *x; }'
assert 3 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *x; }'
assert 4 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+1); }'
assert 5 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+2); }'

assert 0 'int main() { int x[2][3]; int *y=x; *y=0; return **x; }'
assert 1 'int main() { int x[2][3]; int *y=x; *(y+1)=1; return *(*x+1); }'
assert 2 'int main() { int x[2][3]; int *y=x; *(y+2)=2; return *(*x+2); }'
assert 3 'int main() { int x[2][3]; int *y=x; *(y+3)=3; return **(x+1); }'
assert 4 'int main() { int x[2][3]; int *y=x; *(y+4)=4; return *(*(x+1)+1); }'
assert 5 'int main() { int x[2][3]; int *y=x; *(y+5)=5; return *(*(x+1)+2); }'
assert 1 'int main() { int x[2][3]; **(x+1)=1; return **(x+1); }'
assert 2 'int main() { int x[2][3]; *(*x+2)=2; return *(*x+2); }'
assert 3 'int main() { int x[2][3]; *(*x+3)=3; return **(x+1); }'
assert 4 'int main() { int x[2][3]; *(*x+4)=4; return *(*(x+1)+1); }'
assert 5 'int main() { int x[2][3]; *(*x+5)=5; return *(*(x+1)+2); }'

assert 3 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *x; }'
assert 4 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+1); }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; 2[x]=5; return *(x+2); }'
assert 0 'int main() { int x[2][3]; int *y=x; y[0]=0; return x[0][0]; }'
assert 1 'int main() { int x[2][3]; int *y=x; y[1]=1; return x[0][1]; }'
assert 2 'int main() { int x[2][3]; int *y=x; y[2]=2; return x[0][2]; }'
assert 3 'int main() { int x[2][3]; int *y=x; y[3]=3; return x[1][0]; }'
assert 4 'int main() { int x[2][3]; int *y=x; y[4]=4; return x[1][1]; }'
assert 5 'int main() { int x[2][3]; int *y=x; y[5]=5; return x[1][2]; }'
assert 0 'int main() { int x[2][3]; int *y=(x+1); y[0]=0; return x[1][0]; }'
assert 1 'int main() { int x[2][3]; int *y=(x+1); y[-3]=1; return x[0][0]; }'
assert 1 'int main() { int x[2][3]; int *y=(x+1); (-3)[y]=1; return x[0][0]; }'

echo OK

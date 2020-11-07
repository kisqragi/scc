int assert(int expected, int actual, char *code) {
    if (expected == actual)
        printf("%s => %d\n", code, actual);
    else {
        printf("%s => %d expected, but got %d\n", code, expected, actual);
        exit(1);
    }
}

int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }
int add6(int a, int b, int c, int d, int e, int f) {
    return a+b+c+d+e+f;
}
int for_loop() { int i=0; for (;;) { if (i>2) return i; i=i+1; } return 0; }
int while_loop() { int i=0; while (1) { if (i>2) return i; i=i+1; } return 0; }
int sub_char(char a, char b, char c) { return a-b-c; }

int main() {
    assert(0, 0, "0");
    assert(42, 42, "42");

    assert(20, 5+19-4, "5+19-4");

    assert(20,  5+ 19 -4, "5+ 19 -4");

    assert(47, 5+6*7, "5+6*7");
    assert(15, 5*(9-6), "5*(9-6)");
    assert(4, (3+5)/2, "(3+5)/2");

    assert(10, -10+20, "-10+20");
    assert(10, - -10, "- -10");
    assert(10, - - +10, "- - +10");

    assert(0, 0==1, "0==1");
    assert(1, 1==1, "1==1");
    assert(1, 0!=1, "0!=1");
    assert(0, 1!=1, "1!=1");

    assert(1, 0<1, "0<1");
    assert(0, 1<1, "1<1");
    assert(0, 2<1, "2<1");
    assert(1, 0<=1, "0<=1");
    assert(1, 1<=1, "1<=1");
    assert(0, 2<=1, "2<=1");

    assert(1, 1>0, "1>0");
    assert(0, 1>1, "1>1");
    assert(0, 1>2, "1>2");
    assert(1, 1>=0, "1>=0");
    assert(1, 1>=1, "1>=1");
    assert(0, 1>=2, "1>=2");

    assert(3, ({ 1; 2; 3; }), "({ 1; 2; 3; })");

    assert(3, ({ int a=3;  a; }), "int a=3;  a;");
    assert(8, ({ int a=5;int z=3; a+z; }), "int a=5;int z=3; a+z;");
    assert(6, ({ int a,b; a=b=3;  a+b; }), "int a,b; a=b=3;  a+b;");

    assert(3, ({ int foo=3; foo; }), "int foo=3;  foo;");
    assert(3, ({ int foo123=3; foo123; }), "int foo123  foo123;");
    assert(3, ({ int _foo=3; _foo; }), "int _foo=3; _foo;");
    assert(7, ({ int foo=3; int bar=4; foo+bar; }), "int foo=3; int bar=4; foo+bar;");

    assert(1, ({ ;;;  1; }) , ";;;  1;");
//assert(1, ({ ;;;  1;; }) , ";;;  1;;");

    assert(0, ({ int a=0; if (0) a=1; a; }), "int a=0; if (0) a=1; a;");
    assert(0, ({ int a=0; if (1-1) a=1; a; }), "int a=0; if (1-1) a=1; a;");
    assert(1, ({ int a=0; if (1) a=1; a; }), "int a=0; if (1) a=1; a;");
    assert(1, ({ int a=0; if (2-1) a=1; a; }), "int a=0; if (2-1) a=1; a;");
    assert(2, ({ int a=0; if (0)  a=1; else { a=2; }; a; }), "int a=0; if (0)  a=1; else { a=2; }; a;");
    assert(3, ({ int a=0; if (0)  a=1; else if (0) a=2; a=3; a; }), "int a=0; if (0)  a=1; else if (0) a=2; a=3; a;");

    assert(45, ({ int i;int j=0; for(i=0;i<10;i=i+1) { j=j+i; } j; }), "int i;int j=0; for(i=0;i<10;i=i+1) { j=j+i; } j;");
    assert(3, for_loop(), "int i=0; for (;;) { if (i>2) return i; i=i+1; } return 0;");

    assert(55, ({ int i=0;int j=0; while(i<10) { i=i+1; j=j+i; } j; }), "int i=0;int j=0; while(i<10) { i=i+1; j=j+i; } j;");
    assert(3, while_loop(), "int i=0; while (1) { if (i>2) return i; i=i+1; } return 0;");

    assert(1, ({ int x=1; *&x; }), "int x=1; *&x;");
    assert(1, ({ int x=1; int *y=&x; int **z=&y; **z; }), "int x=1; int *y=&x; int **z=&y; **z;");
    assert(2, ({ int x=1; int y=2; *(&x+1); }), "int x=1; int y=2; *(&x+1);");
    assert(1, ({ int x=1; int y=2; *(&y-1); }), "int x=1; int y=2; *(&y-1);");
    assert(5, ({ int x=3; int *y=&x; *y=5; x; }), "int x=3; int *y=&x; *y=5; x;");
    assert(5, ({ int x=1; int y=2; *(&x+1)=5; y; }), "int x=1; int y=2; *(&x+1)=5; y;");
    assert(5, ({ int x=1; int y=2; *(&y-1)=5; x; }), "int x=1; int y=2; *(&y-1)=5; x;");

    assert(3, ({ int x=1; (&x+3)-&x; }), "int x=1; (&x+3)-&x;");
    assert(5, ({ int x=1; (&x+3)-&x+2; }), "int x=1; (&x+3)-&x+2;");

    assert(3, ret3(), "ret3()");
    assert(5, ret5(), "ret5()");

    assert(7, add(4, 3), "add(4, 3)");
    assert(1, sub(4, 3), "sub(4, 3)");
    assert(21, add6(1,2,3,4,5,6), "add6(1,2,3,4,5,6)");
    assert(66,  add6(1,2,add6(3,4,5,6,7,8),9,10,11), "add6(1,2,add6(3,4,5,6,7,8),9,10,11)");
    assert(136, add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16),
                "add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16)");

    assert(3, ({ int x[2]; int *y=&x; *y=3; *x; }), "int x[2]; int *y=&x; *y=3; *x;");
    assert(3, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; }), "int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x;");
    assert(4, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); }), "int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1);");
    assert(5, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); }), "int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2);");

    assert(0, ({ int x[2][3]; int *y=x; *y=0; **x; }), "int x[2][3]; int *y=x; *y=0; **x;");
    assert(1, ({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); }), "int x[2][3]; int *y=x; *(y+1)=1; *(*x+1);");
    assert(2, ({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); }), "int x[2][3]; int *y=x; *(y+2)=2; *(*x+2);");
    assert(3, ({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); }), "int x[2][3]; int *y=x; *(y+3)=3; **(x+1);");
    assert(4, ({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); }), "int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1);");
    assert(5, ({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); }), "int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2);");
    assert(1, ({ int x[2][3]; **(x+1)=1; **(x+1); }), "int x[2][3]; **(x+1)=1; **(x+1);");
    assert(2, ({ int x[2][3]; *(*x+2)=2; *(*x+2); }), "int x[2][3]; *(*x+2)=2; *(*x+2);");
    assert(3, ({ int x[2][3]; *(*x+3)=3; **(x+1); }), "int x[2][3]; *(*x+3)=3; **(x+1);");
    assert(4, ({ int x[2][3]; *(*x+4)=4; *(*(x+1)+1); }), "int x[2][3]; *(*x+4)=4; *(*(x+1)+1);");
    assert(5, ({ int x[2][3]; *(*x+5)=5; *(*(x+1)+2); }), "int x[2][3]; *(*x+5)=5; *(*(x+1)+2);");

    assert(3, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; }), "int x[3]; *x=3; x[1]=4; x[2]=5; *x;");
    assert(4, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); }), "int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1);");
    assert(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }), "int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2);");
    assert(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }), "int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2);");
    assert(5, ({ int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2); }), "int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2);");
    assert(0, ({ int x[2][3]; int *y=x; y[0]=0; x[0][0]; }), "int x[2][3]; int *y=x; y[0]=0; x[0][0];");
    assert(1, ({ int x[2][3]; int *y=x; y[1]=1; x[0][1]; }), "int x[2][3]; int *y=x; y[1]=1; x[0][1];");
    assert(2, ({ int x[2][3]; int *y=x; y[2]=2; x[0][2]; }), "int x[2][3]; int *y=x; y[2]=2; x[0][2];");
    assert(3, ({ int x[2][3]; int *y=x; y[3]=3; x[1][0]; }), "int x[2][3]; int *y=x; y[3]=3; x[1][0];");
    assert(4, ({ int x[2][3]; int *y=x; y[4]=4; x[1][1]; }), "int x[2][3]; int *y=x; y[4]=4; x[1][1];");
    assert(5, ({ int x[2][3]; int *y=x; y[5]=5; x[1][2]; }), "int x[2][3]; int *y=x; y[5]=5; x[1][2];");
    assert(0, ({ int x[2][3]; int *y=(x+1); y[0]=0; x[1][0]; }), "int x[2][3]; int *y=(x+1); y[0]=0; x[1][0];");
    assert(1, ({ int x[2][3]; int *y=(x+1); y[-3]=1; x[0][0]; }), "int x[2][3]; int *y=(x+1); y[-3]=1; x[0][0];");
    assert(1, ({ int x[2][3]; int *y=(x+1); (-3)[y]=1; x[0][0]; }), "int x[2][3]; int *y=(x+1); (-3)[y]=1; x[0][0];");


    assert(8, ({ int x; sizeof(x); }), "int x; sizeof(x);");
    assert(8, ({ int x; sizeof x; }), "int x; sizeof x;");
    assert(8, ({ int *x; sizeof(x); }), "int *x; sizeof(x);");
    assert(32, ({ int x[4]; sizeof(x); }), "int x[4]; sizeof(x);");
    assert(96, ({ int x[3][4]; sizeof(x); }), "int x[3][4]; sizeof(x);");
    assert(100, ({ int x[3][4]; sizeof(x) + 4; }), "int x[3][4]; sizeof(x) + 4;");
    assert(32, ({ int x[3][4]; sizeof(*x); }), "int x[3][4]; sizeof(*x);");
    assert(40, ({ int x[3][4]; sizeof(*x)+sizeof(**x); }), "int x[3][4]; sizeof(*x)+sizeof(**x);");
    assert(8, ({ int x[3][4]; sizeof(**x); }), "int x[3][4]; sizeof(**x);");
    assert(9, ({ int x[3][4]; sizeof(**x) + 1; }), "int x[3][4]; sizeof(**x) + 1;");
    assert(9, ({ int x[3][4]; sizeof **x + 1; }), "int x[3][4]; sizeof **x + 1;");
    assert(8, ({ int x[3][4]; sizeof(**x + 1); }), "int x[3][4]; sizeof(**x + 1);");
    assert(8, ({ int x=1; sizeof(x=2); }), "int x=1; sizeof(x=2);");
    assert(1, ({ int x=1; sizeof(x=2); x; }), "int x=1; sizeof(x=2); x;");

/*
 * Implement the scope and then test it
 *
 *
    assert(0 'int x; int main() { return x, "");
    assert(3 'int x; int main() { x=3; return x, "");
    assert(7 'int x; int y; int main() { x=3; y=4; return x+y, "");
    assert(7 'int x, y; int main() { x=3; y=4; return x+y, "");
    assert(0 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[0], "");
    assert(1 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[1], "");
    assert(2 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[2], "");
    assert(3 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[3], "");

    assert(8 'int x; int main() { return sizeof(x), "");
    assert(32 'int x[4]; int main() { return sizeof(x), "");
*/

    assert(1, ({ char x=1; x; }), "char x=1; x;");
    assert(1, ({ char x=1; char y=2; x; }), "char x=1; char y=2; x;");
    assert(2, ({ char x=1; char y=2; y; }), "char x=1; char y=2; y;");

    assert(1, ({ char x; sizeof(x); }), "char x; sizeof(x);");
    assert(10, ({ char x[10]; sizeof(x); }), "char x[10]; sizeof(x);");
    assert(1, sub_char(7, 3, 3), "sub_char(7, 3, 3)");

    assert(0, ({ ""[0]; }), "\"\"[0];");
    assert(1, ({ sizeof(""); }), "sizeof(\"\");");

    assert(97, ({ "abc"[0]; }), "\"abc\"[0];");
    assert(98, ({ "abc"[1]; }), "\"abc\"[1];");
    assert(99, ({ "abc"[2]; }), "\"abc\"[2];");
    assert(0, ({ "abc"[3]; }), "\"abc\"[3];");
    assert(4, ({ sizeof("abc"); }), "sizeof(\"abc\");");
    assert(10, ({ sizeof("abc") + sizeof("defgh"); }), "sizeof(\"abc\") + sizeof(\"defgh\");");

    assert(7, ({ "\a"[0]; }), "\"\\a\"[0];");
    assert(8, ({ "\b"[0]; }), "\"\\b\"[0];");
    assert(9, ({ "\t"[0]; }), "\"\\t\"[0];");
    assert(10, ({ "\n"[0]; }), "\"\\n\"[0];");
    assert(11, ({ "\v"[0]; }), "\"\\v\"[0];");
    assert(12, ({ "\f"[0]; }), "\"\\f\"[0];");
    assert(13, ({ "\r"[0]; }), "\"\\r\"[0];");
    assert(27, ({ "\e"[0]; }), "\"\\e\"[0];");
    assert(106, ({ "\j"[0]; }), "\"\\j\"[0];");
    assert(107, ({ "\k"[0]; }), "\"\\k\"[0];");
    assert(108, ({ "\l"[0]; }), "\"\\l\"[0];");
    assert(7, ({ "\ax\ny"[0]; }), "\"\\ax\\ny\"[0];");
    assert(120, ({ "\ax\ny"[1]; }), "\"\\ax\\ny\"[1];");
    assert(10, ({ "\ax\ny"[2]; }), "\"\\ax\\ny\"[2];");
    assert(121, ({ "\ax\ny"[3]; }), "\"\\ax\\ny\"[3];");

    assert(0, ({ "\0"[0]; }), "\"\\0\"[0];");
    assert(16, ({ "\20"[0]; }), "\"\\20\"[0];");
    assert(65, ({ "\101"[0]; }), "\"\\101\"[0];");
    assert(104, ({ "\1500"[0]; }), "\"\\1500\"[0];");

    assert(0, ({ "\x00"[0]; }), "\"\\x00\"[0];");
    assert(119, ({ "\x77"[0]; }), "\"\\x77\"[0];");
    assert(-91, ({ "\xA5"[0]; }), "\"\\xA5\"[0];");
    assert(-1, ({ "\x00ff"[0]; }), "\"\\x00ff\"[0];");
    assert(66, ({ "\x4142"[0]; }), "\"\\x4142\"[0];");
    assert(67, ({ "\x414243"[0]; }), "\"\\x414243\"[0];");

    puts("OK");
    return 0;
}


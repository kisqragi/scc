int printf();

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

int x, y;
int z[4];

int a;

int main() {

    printf("------ global ----------\n");
    printf("int ret3() { return 3; }\n");
    printf("int ret5() { return 5; }\n");
    printf("int add(int x, int y) { return x+y; }\n");
    printf("int sub(int x, int y) { return x-y; }\n");
    printf("int add6(int a, int b, int c, int d, int e, int f) {\n");
    printf("    return a+b+c+d+e+f;\n");
    printf("}\n");
    printf("int for_loop() { int i=0; for (;;) { if (i>2) return i; i=i+1; } return 0; }\n");
    printf("int while_loop() { int i=0; while (1) { if (i>2) return i; i=i+1; } return 0; }\n");
    printf("int sub_char(char a, char b, char c) { return a-b-c; }\n");
    printf("int x, y;\n");
    printf("int z[4];\n");
    printf("int a;\n");
    printf("------------------------\n\n");

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

    assert(4, ({ int x; sizeof(x); }), "int x; sizeof(x);");
    assert(4, ({ int x; sizeof x; }), "int x; sizeof x;");
    assert(8, ({ int *x; sizeof(x); }), "int *x; sizeof(x);");
    assert(16, ({ int x[4]; sizeof(x); }), "int x[4]; sizeof(x);");
    assert(48, ({ int x[3][4]; sizeof(x); }), "int x[3][4]; sizeof(x);");
    assert(52, ({ int x[3][4]; sizeof(x) + 4; }), "int x[3][4]; sizeof(x) + 4;");
    assert(16, ({ int x[3][4]; sizeof(*x); }), "int x[3][4]; sizeof(*x);");
    assert(20, ({ int x[3][4]; sizeof(*x)+sizeof(**x); }), "int x[3][4]; sizeof(*x)+sizeof(**x);");
    assert(4, ({ int x[3][4]; sizeof(**x); }), "int x[3][4]; sizeof(**x);");
    assert(5, ({ int x[3][4]; sizeof(**x) + 1; }), "int x[3][4]; sizeof(**x) + 1;");
    assert(5, ({ int x[3][4]; sizeof **x + 1; }), "int x[3][4]; sizeof **x + 1;");
    assert(4, ({ int x[3][4]; sizeof(**x + 1); }), "int x[3][4]; sizeof(**x + 1);");
    assert(4, ({ int x=1; sizeof(x=2); }), "int x=1; sizeof(x=2);");
    assert(1, ({ int x=1; sizeof(x=2); x; }), "int x=1; sizeof(x=2); x;");

    assert(0, ({ x; }), "x;");
    assert(3, ({ x=3; x; }), "x=3; x;");
    assert(7, ({ x=3; y=4; x+y; }), "x=3; y=4; x+y;");
    assert(7, ({ x=3; y=4; x+y; }), "x=3; y=4; x+y;");
    assert(0, ({ z[0]=0; z[1]=1; z[2]=2; z[3]=3; z[0]; }), "z[0]=0; z[1]=1; z[2]=2; z[3]=3; z[0];");
    assert(1, ({ z[0]=0; z[1]=1; z[2]=2; z[3]=3; z[1]; }), "z[0]=0; z[1]=1; z[2]=2; z[3]=3; z[1];");
    assert(2, ({ z[0]=0; z[1]=1; z[2]=2; z[3]=3; z[2]; }), "z[0]=0; z[1]=1; z[2]=2; z[3]=3; z[2];");
    assert(3, ({ z[0]=0; z[1]=1; z[2]=2; z[3]=3; z[3]; }), "z[0]=0; z[1]=1; z[2]=2; z[3]=3; z[3];");

    assert(4, sizeof(x), "sizeof(x)");
    assert(16, sizeof(z), "sizeof(z)");

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

    assert(10, ({ x = 10; x; }), "x = 10; x;");
    assert(0, a, "a");
    assert(10, ({ x = 10; { int x = 20; } x; }), "x = 10; { int x = 20; } x;");
    assert(10, ({ int x = 20; { x = 10; } x; }), "int x = 20; { x = 10; } x;");
    assert(10, ({ int x = 10; { int x = 50; } x; }), "int x = 10; { int x = 50; } x;");
    assert(30, ({ x = 10; { x = 50; { x = 30; } } x; }), "x = 10; { x = 50; { x = 30; } } x;");

    assert(3, (1,2,3), "(1,2,3)");
    assert(10, add(5, (x=3, x+2)), "add(5, (x=3, x+2))");

    assert(1, ({ struct { int a; int b; } x; x.a=1; x.b=2; x.a; }), "struct { int a; int b; } x; x.a=1; x.b=2; x.a;");
    assert(2, ({ struct { int a; int b; } x; x.a=1; x.b=2; x.b; }), "struct { int a; int b; } x; x.a=1; x.b=2; x.b;");
    assert(0, ({ struct {char a; char b;} x[3]; char *p=x; p[0]=0; x[0].a; }), "struct {char a; char b;} x[3]; char *p=x; p[0]=0; x[0].a;");
    assert(1, ({ struct {char a; char b;} x[3]; char *p=x; p[1]=1; x[0].b; }), "struct {char a; char b;} x[3]; char *p=x; p[1]=1; x[0].b;");
    assert(2, ({ struct {char a; char b;} x[3]; char *p=x; p[2]=2; x[1].a; }), "struct {char a; char b;} x[3]; char *p=x; p[2]=2; x[1].a;");
    assert(3, ({ struct {char a; char b;} x[3]; char *p=x; p[3]=3; x[1].b; }), "struct {char a; char b;} x[3]; char *p=x; p[3]=3; x[1].b;");
    assert(6, ({ struct {char a[3]; char b[5];} x; char *p=&x; x.a[0]=6; p[0]; }), "struct {char a[3]; char b[5];} x; char *p=&x; x.a[0]=6; p[0];");
    assert(7, ({ struct {char a[3]; char b[5];} x; char *p=&x; x.b[0]=7; p[3]; }), "struct {char a[3]; char b[5];} x; char *p=&x; x.b[0]=7; p[3];");
    assert(6, ({ struct { struct { char b; } a; } x; x.a.b=6; x.a.b; }), "struct { struct { char b; } a; } x; x.a.b=6; x.a.b;");
    assert(4, ({ struct {int a;} x; sizeof(x); }), "struct {int a;} x; sizeof(x);");
    assert(8, ({ struct {int a; int b;} x; sizeof(x); }), "struct {int a; int b;} x; sizeof(x);");
    assert(8, ({ struct {int a, b;} x; sizeof(x); }), "struct {int a, b;} x; sizeof(x);");
    assert(12, ({ struct {int a[3];} x; sizeof(x); }), "struct {int a[3];} x; sizeof(x);");
    assert(16, ({ struct {int a;} x[4]; sizeof(x); }), "struct {int a;} x[4]; sizeof(x);");
    assert(24, ({ struct {int a[3];} x[2]; sizeof(x); }), "struct {int a[3];} x[2]; sizeof(x);");
    assert(2, ({ struct {char a; char b;} x; sizeof(x); }), "struct {char a; char b;} x; sizeof(x);");
    assert(8, ({ struct {char a; int b;} x; sizeof(x); }), "struct {char a; int b;} x; sizeof(x);");
    assert(0, ({ struct {} x; sizeof(x); }), "struct {} x; sizeof(x);");
    assert(8, ({ struct {} x; sizeof(&x); }), "struct {} x; sizeof(&x);");
    assert(8, ({ struct {} *x; sizeof(x); }), "struct {} *x; sizeof(x);");

    assert(2, ({ struct { char a; char b;} x; sizeof(x); }), "struct { char a; char b;} x; sizeof(x);");
    assert(8, ({ struct { char a; int b;} x; sizeof(x); }), "struct { char a; int b;} x; sizeof(x);");
    assert(8, ({ struct { int a; char b;} x; sizeof(x); }), "struct { int a; char b;} x; sizeof(x);");
    assert(8, ({ struct { int a; int b;} x; sizeof(x); }), "struct { int a; int b;} x; sizeof(x);");

    assert(7, ({ int x; int y; char z; char *a=&y; char *b=&z; b-a; }), "int x; int y; char z; char *a=&y; char *b=&z; b-a;");
    assert(1,  ({ int x; char y; int z; char *a=&y; char *b=&z; b-a; }), "int x; char y; int z; char *a=&y; char *b=&z; b-a;");
    assert(11, ({ int x; int y; char z; char *a=&x; char *b=&z; b-a; }), "int x; int y; char z; char *a=&x; char *b=&z; b-a;");
    assert(1,  ({ char x; char y; char z; char *a=&x; char *b=&y; b-a; }), "char x; char y; char z; char *a=&x; char *b=&y; b-a;");
    assert(2,  ({ char x; char y; char z; char *a=&x; char *b=&z; b-a; }), "char x; char y; char z; char *a=&x; char *b=&z; b-a;");
    assert(3,  ({ char x; char y; char z; int c; char *a=&x; char *b=&c; b-a; }), "char x; char y; char z; int c; char *a=&x; char *b=&c; b-a;");

    assert(8, ({ struct t {int a; int b;} x; struct t y; sizeof(y); }), "struct t {int a; int b;} x; struct t y; sizeof(y);");
    assert(8, ({ struct t {int a; int b;}; struct t y; sizeof(y); }), "struct t {int a; int b;}; struct t y; sizeof(y);");
    assert(2, ({ struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y); }),
                    "struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y);");
    assert(3, ({ struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x; }), "struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x;");
    assert(10, ({ struct t {int a; int b;} x; struct t y; y.a = 10; y.a; }), "struct t {int a; int b;} x; struct t y; y.a = 10; y.a;");
    assert(10, ({ struct t {int a; int b;} x; struct t y; y.b = 10; y.b; }), "struct t {int a; int b;} x; struct t y; y.b = 10; y.b;");

    assert(10, ({ struct t { int a; int b; } x; struct t *y = &x; x.a = 10; x.b = 11; y->a; }),
                 "struct t { int a; int b; } x; struct t *y = &x; x.a = 10; x.b = 11; y->a;");
    assert(11, ({ struct t { int a; int b; } x; struct t *y = &x; x.a = 10; x.b = 11; y->b; }),
                 "struct t { int a; int b; } x; struct t *y = &x; x.a = 10; x.b = 11; y->b;");
    assert(10, ({ struct t { int a; int b; } x; struct t *y = &x; x.a = 10; x.b = 11; (*y).a; }),
                 "struct t { int a; int b; } x; struct t *y = &x; x.a = 10; x.b = 11; (*y).a;");
    assert(11, ({ struct t { int a; int b; } x; struct t *y = &x; x.a = 10; x.b = 11; (*y).b; }),
                 "struct t { int a; int b; } x; struct t *y = &x; x.a = 10; x.b = 11; (*y).b;");
    assert(8, ({ struct t { int a; int b; } x; struct t *y = &x; sizeof(y); }), "struct t { int a; int b; } x; struct t *y = &x; sizeof(y); ");

    assert(10, ({ struct t { int a; }x; x.a = 10; struct t y; y = x; y.a; }), "struct t { int a; }x; x.a = 10; struct t y; y = x; y.a;");
    assert(3, ({ struct {int a,b;} x,y; x.a=3; y=x; y.a; }), "struct {int a,b;} x,y; x.a=3; y=x; y.a;");
    assert(7, ({ struct t {int a,b;}; struct t x; x.a=7; struct t y; struct t *z=&y; *z=x; y.a; }), "struct t {int a,b;}; struct t x; x.a=7; struct t y; struct t *z=&y; *z=x; y.a;");
    assert(7, ({ struct t {int a,b;}; struct t x; x.a=7; struct t y, *p=&x, *q=&y; *q=*p; y.a; }), "struct t {int a,b;}; struct t x; x.a=7; struct t y, *p=&x, *q=&y; *q=*p; y.a;");
    assert(5, ({ struct t {char a, b;} x, y; x.a=5; y=x; y.a; }), "struct t {char a, b;} x, y; x.a=5; y=x; y.a;");

    assert(3, ({ struct {int a,b;} x,y; x.a=3; y=x; y.a; }), "struct {int a,b;} x,y; x.a=3; y=x; y.a;");
    assert(7, ({ struct t {int a,b;}; struct t x; x.a=7; struct t y; struct t *z=&y; *z=x; y.a; }), "struct t {int a,b;}; struct t x; x.a=7; struct t y; struct t *z=&y; *z=x; y.a;");
    assert(7, ({ struct t {int a,b;}; struct t x; x.a=7; struct t y, *p=&x, *q=&y; *q=*p; y.a; }), "struct t {int a,b;}; struct t x; x.a=7; struct t y, *p=&x, *q=&y; *q=*p; y.a;");
    assert(5, ({ struct t {char a, b;} x, y; x.a=5; y=x; y.a; }), "struct t {char a, b;} x, y; x.a=5; y=x; y.a;");

    { void *x; }

    puts("OK");
    return 0;
}


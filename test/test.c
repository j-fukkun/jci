
//this is comment
/*this is second comment*/

int printf();
int exit();
int strcmp(char* p, char* q);
int memcmp(char* p, char* q);

typedef int MyInt;

int assert(int expected, int actual, char* code) {
  if (expected == actual) {
    printf("%s => %d\n", code, actual);
  } else {
    printf("%s => %d expected but got %d\n", code, expected, actual);
    exit(1);
  }
}


int ret3() {
  return 3;
  return 5;
}

int add2(int x, int y) {
  return x + y;
}

int sub2(int x, int y) {
  return x - y;
}

int add6(int a, int b, int c, int d, int e, int f) {
  return a + b + c + d + e + f;
}

int addx(int *x, int y) {
  return *x + y;
}

int sub_char(char a, char b) {
  return a - b;
}

int fib(int x){
  if(x == 0) return 0;
  else if (x == 1) return 1;
  else {
    return fib(x-1) + fib(x-2);
  }
}

void test_void(void){
  return;
}

struct TEST_G {
  int x;
  int y;
  short z;
  long w;
} t_g;

struct TEST_G2;

struct TEST_G3 {
  char x;
  short y;
  int z;
  long w;
} t_g2 = {1, 2, 3, 4};

typedef struct Tree_G{
  int val;
  struct Tree_G* lest;
  struct Tree_G* right;
} Tree_G;

/*
//compound-literal
Tree_G *tree_g = &(Tree_G){
  1,
  &(Tree_G){
    2,
    &(Tree_G){ 3, 0, 0 },
    &(Tree_G){ 4, 0, 0 },
  },
  0,
};
*/
typedef struct {char a; int b;} MyType;

/*
struct TEST_G test_ret_struct(){
  struct TEST_G t;
  t.x = 123;
  t.y = 456;
  return t;
}

struct TEST_G test_args_struct(struct TEST_G s){
  struct TEST_G t;
  t = s;
  return t;
}

void test_pointer_struct(struct TEST_G* s){
  s->x = 100;
  s->y = 1000;
}
*/

_Bool func_bool(_Bool b1, _Bool b2){
  return b1 && b2;
}

bool func_bool2(bool b1, bool b2){
  return b1 || b2;
}

int gvar1;
char gvar2;
int gvar3 = 10;
int gvar4[3] = {1,2,3};
int gvar5[5] = {1,2,3};
int gvar6[3] = {1,2,3,4,5};
char gvar7[] = "Hello, ";
char gvar8[] = "Compiler World!!";
int gvar9[][3] = {{1,2,3}, {4,5,6}, {7,8,9}};
short gvar_short = 1000;
long gvar_long = 10000;
char* gvar10[] = {"foo", "bar"};
struct {char a; int b;} gvar11[2] = {{1, 2}, {3, 4}};
struct {int a[2];} gvar12[2] = {{{1, 2}}};
struct {int a[2];} gvar13[2] = {{1, 2}, 3, 4};
struct {int a[2];} gvar14[2] = {1, 2, 3, 4};
char* gvar15 = {"foo"};
//char gvar16[][4] = {'f','o','o','b','a','r',0};
char gvar17[] = "foobar";
char gvar18[10] = "foobar";
char gvar19[3] = "foobar";
char* gvar20 = gvar17+0;
char* gvar21 = gvar17+3;
char* gvar22 = &gvar17-3;
char* gvar23[] = {gvar17+0, gvar17+3, gvar17-3};
int* gvar24 = &gvar3; //10
int* gvar25 = gvar4 + 1; //2

int main(){
  
  assert(0, 0, "0");
  assert(42, 42, "42");
  
  assert(41,  12 + 34 - 5 , "12 + 34 - 5 ");
  assert(15, 5*(9-6), "5*(9-6)");
  assert(4, (3+5)/2, "(3+5)/2");
  assert(-10, -10, "-10");
  assert(10, - -10, "- -10");
  assert(10, - - +10, "- - +10");
  assert(2, 12%5, "12%5");

  assert(0, 0==1, "0==1");
  assert(1, 42==42, "42==42");
  assert(1, 0!=1, "0!=1");
  assert(0, 42!=42, "42!=42");

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

  int a;
  a = ret3();
  assert(1, a == 3, "a == ret3()");

  int b;
  int c;
  b = 5;
  c = 10;
  assert(15, add2(b, c), "add2(5, 10)");
  assert(5, sub2(c, b), "sub2(10, 5)");
  assert(21, add6(1,2,3,4,5,6), "add6(1,2,3,4,5,6)");
  assert(15, addx(&b, c), "addx(&b,c)");

  char d;
  char e;
  d = 20;
  e = 30;
  assert(10, sub_char(e, d), "sub_char(30, 20)");
  
  assert(55, fib(10), "fib(10)");

  gvar1 = 1;
  gvar2 = 2;
  assert(1, gvar1, "gvar1");
  assert(2, gvar2, "gvar2");

  int i;
  a = 0;
  for(i = 0; i < 10; i = i+1) a = a + 1;
  assert(10, a, "for(i = 0; i < 10; i = i+1) a = a + 1;");

  i = 0; a = 0; b = 0;
  for(i = 0; i < 10; i++){
    a++;
    break;
    b++;
  }
  assert(1, a, "for(i = 0; i < 10; i++){a++; break; b++;} a;");
  assert(0, b, "for(i = 0; i < 10; i++){a++; break; b++;} b;");

  i = 0; a = 0; b = 0;
  for(i = 0; i < 10; i++){
    a++;
    continue;
    b++;
  }
  assert(10, a, "for(i = 0; i < 10; i++){a++; continue; b++;} a;");
  assert(0, b, "for(i = 0; i < 10; i++){a++; continue; b++;} b;");

  i = 0; a = 0;
  for(int i = 0; i < 10; i++){
    a++;
  }
  assert(10, a, "a = 0; for(int i = 0; i < 10; i++){a++;} a;");
  assert(0, i, "i = 0; for(int i = 0; i < 10; i++){a++;} i;");
  
  i = 0; a = 0;
  while(i < 10){
    a = a + 1;
    i = i + 1;
  }
  assert(10, a, "while(i<10){a=a+1; i=i+1;}");

  i = 0; a = 0; b = 0;
  while(i < 10){
    a++;
    i++;
    break;
    b++;
  }
  assert(1, a, "while(i<10){a++; i++; break; b++;} a;");
  assert(0, b, "while(i<10){a++; i++; break; b++;} b;");

  i = 0; a = 0; b = 0;
  while(i < 10){
    a++;
    i++;
    continue;
    b++;
  }
  assert(10, a, "while(i<10){a++; i++; continue; b++;} a;");
  assert(0, b, "while(i<10){a++; i++; continue; b++;} b;");

  i = 0; a = 0; b = 0;
  do {
    i++;
    a++;
    break;
    b++;
  } while(i < 10);
  assert(1, a, "do{i++; a++; break; b++;}while(i<10); a;");
  assert(0, b, "do{i++; a++; break; b++;}while(i<10); b;");
  
  i = 0; a = 0; b = 0;
  do {
    a++;
    i++;
    continue;
    b++;
  } while(i < 10);
  assert(10, a, "do{a++; i++; continue; b++;}while(i<10); a;");
  assert(0, b, "do{a++; i++; continue; b++;}while(i<10); b;");
  

  int arr[2+2];
  *arr = 1; *(arr+1)=2;
  assert(3, *arr + *(arr+1), "*arr + *(arr+1)");

  a = 10;
  int* p;
  p = &a;
  assert(10, *p, "*p");

  arr[2] = 3;
  arr[3] = 5;
  assert(8, arr[2] + arr[3], "arr[2] + arr[3]");

  assert(4, sizeof(a), "sizeof(a)");
  assert(1, sizeof(e), "sizeof(e)");
  assert(8, sizeof(p), "sizeof(p)");
  assert(16, sizeof(arr), "sizeof(arr)");

  char* str;
  str = "abc";
  printf("%s\n", str);
  
  a = 0;
  ++a;
  assert(1, a, "++a");

  --a;
  assert(0, a, "--a");

  a++;
  assert(1, a, "a++");

  a--;
  assert(0, a, "a--");

  p = arr;
  ++p;
  assert(2, *p, "++p; *p");

  p++;
  assert(3, *p, "p++; *p");

  --p;
  assert(2, *p, "--p; *p");

  p--;
  assert(1, *p, "p--; *p");

  test_void();
  printf("test_void is OK\n");

  assert(10, gvar3, "gvar3");
  assert(3, gvar4[2], "gvar4[2]");
  assert(3, gvar5[2], "gvar5[2]");
  assert(2, gvar6[1], "gvar6[1]");
  printf("%s%s\n", gvar7, gvar8);

  int fib10 = fib(10);
  assert(55, fib10, "fib10");

  int arr2[1+2] = {1,2,3};
  assert(3, arr2[2], "arr[2]");

  int arr3[] = {1,2,3,4,5};
  assert(5, arr3[4], "arr3[4]");

  char str2[] = "Local Initializer Test";
  printf("%s\n", str2);

  int arr4[2][3] = {{1,2,3}, {4,5,6}};
  assert(5, arr4[1][1], "arr4[1][1]");
  assert(7, gvar9[2][0], "gvar9[2][0]");
  
  int lvar1 = 1;
  int lvar2 = 0;
  int lvar3 = lvar1 || lvar2;
  assert(1, lvar3, "lvar1 || lvar2");
  assert(0, lvar1 && lvar2, "lvar1 && lvar2");

  lvar1 = 8;
  lvar1 = lvar1 << 2;
  assert(32, lvar1, "lvar1 << 2");

  lvar1 = lvar1 >> 1;
  assert(16, lvar1, "lvar1 >> 1");
  
  int lvar4 = 10;
  lvar4 += 5;
  assert(15, lvar4, "lvar4 += 5");

  lvar4 -= 3;
  assert(12, lvar4, "lvar4 -= 3");

  lvar4 *= 4;
  assert(48, lvar4, "lvar4 *= 48");

  lvar4 /= 2;
  assert(24, lvar4, "lvar4 /= 2");

  int* lvar_p = arr3;
  lvar_p += 3;
  assert(4, *lvar_p, "lvar_p = arr3; lvar_p += 3; *lvar_p;");

  lvar_p -= 2;
  assert(2, *lvar_p, "lvar_p = arr3; lvar_p -= 2; *lvar_p;");

  struct TEST {
    int x;
    int y;
    short z;
    long w;
  } t;
  
  struct Tree{
    int val;
    struct Tree* lest;
    struct Tree* right;
  };
  
  t.x = 1;
  t.y = 2;
  assert(1, t.x, "t.x = 1;");
  assert(2, t.y, "t.y = 2;");

  t_g.x = 3;
  t_g.y = 4;
  assert(3, t_g.x, "t_g.x = 3;");
  assert(4, t_g.y, "t_g.y = 4;");
  
  /*
  struct TEST_G t_rs = test_ret_struct();
  assert(123, t_rs.x, "t_rs.x = 123");
  assert(456, t_rs.y, "t_rs.y = 456");
  */
  /*
  test_pointer_struct(t_g);
  assert(100, t_g.x, "t_g.x = 100");
  assert(1000, t_g.y, "t_g.y = 1000");
  */
  
  short lvar_short = 123;
  short lvar_short2 = lvar_short;
  long lvar_long = 456;
  long lvar_long2 = lvar_long;
  
  assert(123, lvar_short, "lvar_short = 123");
  assert(123, lvar_short2, "lvar_short2 = lvar_short");
  assert(456, lvar_long, "lvar_long = 456");
  assert(456, lvar_long2, "lvar_long2 = lvar_long");
  assert(1000, gvar_short, "gvar_short = 1000");
  assert(10000, gvar_long, "gvar_long = 10000");
  assert(2, sizeof(lvar_short), "sizeof(lvar_short)");
  assert(8, sizeof(lvar_long), "sizeof(lvar_long)");

  t_g.z = 16000;
  t_g.w = 32000;
  t.z = t_g.z;
  t.w = t_g.w;
  assert(16000, t_g.z, "t_g.z = 16000");
  assert(32000, t_g.w, "t_g.w = 32000");
  assert(16000, t.z, "t.z = t_g.z");
  assert(32000, t.w, "t.w = t_g.w");

  assert(1, t_g2.x, "gvar_initializer test, t_g2.x");
  assert(2, t_g2.y, "gvar_initializer test, t_g2.y");
  assert(3, t_g2.z, "gvar_initializer test, t_g2.z");
  assert(4, t_g2.w, "gvar_initializer test, t_g2.w");

  int sw_test = 2;
  int sw_test2; 
  switch(sw_test){
  case 0: sw_test2 = 0; break;
  case 1: sw_test2 = 1; break;
  case 2: sw_test2 = 2; break;
  }
  assert(2, sw_test2, "sw_test2 == 2");

  sw_test = 0;
  sw_test2 = -1; int sw_test3 = 0;
  switch(sw_test){
  case 0: sw_test2 = 0; 
  case 1: sw_test2 = 1; sw_test3 = 100; break;
  case 2: sw_test2 = 2; break;
  default: sw_test2 = 3;
  }
  assert(1, sw_test2, "sw_test2 == 1");
  assert(100, sw_test3, "sw_test3 == 100");

  sw_test = 10;
  int default_test = -1;
  switch(sw_test){
  case 0: default_test = 0; break;
  case 1: default_test = 1; break;
  case 2: default_test = 2; break;
  default: default_test = 3;
  }
  assert(3, default_test, "default_test == 3");

  int goto_test = 0;
 GOTO_TEST_START:
  goto_test++;
  if(goto_test == 10) goto GOTO_TEST_END;
  goto GOTO_TEST_START;
  
 GOTO_TEST_END:
  assert(10, goto_test, "goto_test == 10");

  ; //null statement

  struct TEST_INIT {
    char x;
    int y;
    short z;
    long w;
  } t_init = {0, 1, 2, 3};

  assert(0, t_init.x, "lvar_initializer test, t_init.x");
  assert(1, t_init.y, "lvar_initializer test, t_init.y");
  assert(2, t_init.z, "lvar_initializer test, t_init.z");
  assert(3, t_init.w, "lvar_initializer test, t_init.w");

  _Bool bool_test = 0;
  assert(0, bool_test, "bool_test = 0; bool_test;");
  bool_test = 1;
  assert(1, bool_test, "bool_test = 1; bool_test;");
  bool_test = 2;
  assert(1, bool_test, "bool_test = 2; bool_test;");
  
  bool bool_test2 = 0;
  assert(0, bool_test2, "bool_test2 = 0; bool_test2;");
  bool_test2 = 1;
  assert(1, bool_test2, "bool_test2 = 1; bool_test2;");
  bool_test2 = 2;
  assert(1, bool_test2, "bool_test2 = 2; bool_test2;");

  bool_test = 0;
  bool_test = func_bool(bool_test, bool_test2);
  assert(0, bool_test, "bool_test = func_bool(bool_test, bool_test2); bool_test;");
  bool_test2 = func_bool2(bool_test, bool_test2);
  assert(1, bool_test2, "bool_test2 = func_bool2(bool_test, bool_test2); bool_test2;");

  assert(1, !bool_test, "bool_test = 0; !bool_test");
  assert(0, !bool_test2, "bool_test2 = 1; !bool_test2");

  int bittest = 1;
  int bittest2 = 2;
  assert(3, bittest | bittest2, "bittest = 1; bittest2 = 2; bittest | bittest2");
  assert(0, bittest & bittest2, "bittest = 1; bittest2 = 2; bittest & bittest2");
  assert(2, bittest ^ (bittest | bittest2), "bittest = 1; bittest2 = 2; bittest ^ (bittest | bittest2)");
  assert(-2, ~bittest, "bittest = 1; ~bittest");

  assert(0, strcmp(gvar10[0], "foo"), "strcmp(gvar10[0], foo");
  assert(0, strcmp(gvar10[1], "bar"), "strcmp(gvar10[1], bar");
  assert(0, gvar10[1][3], "gvar10[1][3]");
  assert(2, sizeof(gvar10)/sizeof(*gvar10), "sizeof(gvar10)/sizeof(*gvar10)");
  
  assert(1, gvar11[0].a, "gvar11[0].a");
  assert(2, gvar11[0].b, "gvar11[0].b");
  assert(3, gvar11[1].a, "gvar11[1].a");
  assert(4, gvar11[1].b, "gvar11[1].b");
  
  //assert(1, gvar12[0].a[0], "gvar12[0].a[0]");
  //assert(2, gvar12[0].a[1], "gvar12[0].a[1]");
  //assert(0, gvar12[1].a[0], "gvar12[1].a[0]");
  //assert(0, gvar12[1].a[1], "gvar12[1].a[1]");
  
  //assert(1, gvar13[0].a[0], "gvar13[0].a[0]");
  //assert(2, gvar13[0].a[1], "gvar13[0].a[1]");
  //assert(3, gvar13[1].a[0], "gvar13[1].a[0]");
  //assert(4, gvar13[1].a[1], "gvar13[1].a[1]");

  //assert(1, gvar14[0].a[0], "gvar14[0].a[0]");
  //assert(2, gvar14[0].a[1], "gvar14[0].a[1]");
  //assert(3, gvar14[1].a[0], "gvar14[1].a[0]");
  //assert(4, gvar14[1].a[1], "gvar14[1].a[1]");

  assert(0, strcmp(gvar15, "foo"), "strcmp(gvar15, foo)");
  //assert gvar16

  assert(7, sizeof(gvar17), "sizeof(gvar17)");
  assert(10, sizeof(gvar18), "sizeof(gvar18)");
  assert(3, sizeof(gvar19), "sizeof(gvar19)");

  assert(0, memcmp(gvar17, "foobar", 7), "memcmp(gvar17, foobar, 7)");
  //assert(0, memcmp(gvar18, "foobar\0\0\0, 10"), memcmp(gvar18, "foobar\0\0\0, 10"));
  assert(0, memcmp(gvar19, "foo", 3), "memcmp(gvar19, foo, 3)");

  assert(0, strcmp(gvar20, "foobar"), "strcmp(gvar20, foobar)");
  assert(0, strcmp(gvar21, "bar"), "strcmp(gvar21, bar)");
  assert(0, strcmp(gvar22+3, "foobar"), "strcmp(gvar22+3, foobar)");

  assert(0, strcmp(gvar23[0], "foobar"), "strcmp(gvar23[0], foobar)");
  assert(0, strcmp(gvar23[1], "bar"), "strcmp(gvar23[1], bar)");
  assert(0, strcmp(gvar23[2]+3, "foobar"), "strcmp(gvar23[2]+3, foobar)");

  assert(10, *gvar24, "*gvar24");
  assert(2, *gvar25, "*gvar25");

  int sc_test = 0;
  {
    int sc_test = 1;
    assert(1, sc_test, "int sc_test=0;{int sc_test=1; sc_test;}");
  }
  assert(0, sc_test, "int sc_test=0;{int sc_test=1;}sc_test;");

  struct SC_TEST{int x;} sc_struct_test = {0};
  {
    struct SC_TEST{int x;} sc_struct_test = {1};
    assert(1, sc_struct_test.x, "sc_struct_test.x;");
  }
  assert(0, sc_struct_test.x, "sc_struct_test.x;");

  enum {zero, one, two} x;
  assert(0, zero, "enum {zero, one, two}; zero;");
  assert(1, one, "enum {zero, one, two}; one;");
  assert(2, two, "enum {zero, one, two}; two;");
  enum {five=5, six, seven};
  assert(5, five, "enum {five=5, six, seven}; five;");
  assert(6, six, "enum {five=5, six, seven}; six;");
  assert(7, seven, "enum {five=5, six, seven}; seven;");
  enum {zero, five=5, three=3, four};
  assert(0, zero, "enum {zero, five=5, three=3, four}; zero;");
  assert(5, five, "enum {zero, five=5, three=3, four}; five;");
  assert(3, three, "enum {zero, five=5, three=3, four}; three;");
  assert(4, four, "enum {zero, five=5, three=3, four}; four;");
  assert(4, sizeof(x), "enum {zero, one, two} x; sizeof(x);");
  enum t {zero, one, two,}; enum t y;
  assert(4, sizeof(y), "enum t {zero, one, two,}; enum t y; sizeof(y);");

  {
    typedef int T; T x=1;
    assert(1, x, "typedef int T; T x=1; x;");
  }
  {
    typedef struct {int a;} t; t x; x.a=1;
    assert(1, x.a, "typedef struct {int a;} t; t x; x.a=1; x.a;");
  }
  {
    typedef int t; t t=1;
    assert(1, t, "typedef int t; t t=1; t;");
  }
  {
    typedef struct {int a;} t; {typedef int t;} t x; x.a=2;
    assert(2, x.a, "typedef struct {int a;} t; {typedef int t;} t x; x.a=2; x.a;");
  }
  {
    MyType x = {10, 20};
    assert(10, x.a, "MyType x = {10, 20}; x.a;");
    assert(20, x.b, "MyType x = {10, 20}; x.b;");
  }
  {
    MyInt x = 30;
    assert(30, x, "MyInt x = 30; x;");
  }
  {
    typedef t; t x;
    assert(4, sizeof(x), "typedef t; t x; sizeof(x);");
  }
  {
    typedef typedef t; t x;
    assert(4, sizeof(x), "typedef typedef t; t x; sizeof(x);");
  }
  {
    long int x;
    assert(8, sizeof(x), "long int x; sizeof(x);");
  }
  {
    int long x;
    assert(8, sizeof(x), "int long x; sizeof(x)");
  }
  {
    long long x;
    assert(8, sizeof(x), "long long x; sizeof(x);");
  }
  {
    long int long x;
    assert(8, sizeof(x), "long int long x; sizeof(x)");
  }

  assert(1, sizeof(char), "sizeof(char)");
  assert(2, sizeof(short), "sizeof(short)");
  assert(2, sizeof(short int), "sizeof(short int)");
  assert(2, sizeof(int short), "sizeof(int short)");
  assert(4, sizeof(int), "sizeof(int)");
  assert(8, sizeof(long), "sizeof(long)");
  assert(8, sizeof(long int), "sizeof(long int)");
  assert(8, sizeof(int long), "sizeof(int long)");
  assert(8, sizeof(char*), "sizeof(char*)");
  assert(8, sizeof(short*), "sizeof(short*)");
  assert(8, sizeof(int*), "sizeof(int*)");
  assert(8, sizeof(long*), "sizeof(long*)");
  assert(8, sizeof(int**), "sizeof(int**)");
  assert(8, sizeof(int(*)[4]), "sizeof(int(*)[4])");
  assert(32, sizeof(int*[4]), "sizeof(int*[4])");
  assert(16, sizeof(int[4]), "sizeof(int[4])");
  assert(48, sizeof(int[3][4]), "sizeof(int[3][4])");
  assert(8, sizeof(struct {int a; int b;}), "sizeof(struct {int a; int b;})");

  assert(131585, (int)8590066177, "(int)8590066177");
  assert(513, (short)8590066177, "(short)8590066177");
  assert(1, (char)8590066177, "(char)8590066177");
  assert(1, (_Bool)1, "(_Bool)1");
  assert(1, (_Bool)2, "(_Bool)2");
  assert(0, (_Bool)(char)256, "(_Bool)(char)256");
  assert(1, (long)1, "(long)1");
  assert(0, (long)&*(int *)0, "(long)&*(int *)0");
  {
    int x=5; long y=(long)&x;
    assert(5, *(int*)y, "int x=5; long y=(long)&x; *(int*)y;");
  }
  
  printf("OK.\n");
  return 0;
}


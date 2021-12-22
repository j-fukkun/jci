
//this is comment
/*this is second comment*/

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

struct Tree_G{
  int val;
  struct Tree_G* lest;
  struct Tree_G* right;
};

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

int main(){
  
  assert(0, 0, "0");
  assert(42, 42, "42");
  
  assert(41,  12 + 34 - 5 , "12 + 34 - 5 ");
  assert(15, 5*(9-6), "5*(9-6)");
  assert(4, (3+5)/2, "(3+5)/2");
  assert(-10, -10, "-10");
  assert(10, - -10, "- -10");
  assert(10, - - +10, "- - +10");

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

  i = 0; a = 0;
  while(i < 10){
    a = a + 1;
    i = i + 1;
  }
  assert(10, a, "while(i<10){a=a+1; i=i+1;}");

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
  
  printf("OK.\n");
  return 0;
}


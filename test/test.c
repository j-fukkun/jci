
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

int gvar1;
char gvar2;


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

  int arr[4];
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
  
  printf("OK.\n");
  return 0;
}


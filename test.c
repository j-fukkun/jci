#include <stdio.h>
#include <stdlib.h>

void test(int a, int b, int c, int d, int e, int f){
  printf("a = %d, b = %d, c = %d, d = %d, e = %d, f = %d\n", a, b, c, d, e, f);
  printf("function call with arguments is OK\n");
}

int* alloc4(int a, int b, int c, int d){
  int* p = (int*)calloc(4, sizeof(int));
  p[0] = a;
  p[1] = b;
  p[2] = c;
  p[3] = d;
  return p;
}

void print(int a){
  printf("%d\n", a);
}

//int main(){int *p; p = alloc4(1,2,3,4); int* q; q = p+2; print(*q); return 0;}



int func(int a, int b, int n){
  int c;
  if(n < 10){
    c = a + b;
  }
  c = a + b;
  c = a + 5;
  return c;
}

int main(){
  int a;
  int* p;
  p = &a;
  *p = 10;
  func(*p, 20, 3);

  int x = 5;
  x = x;
  printf("x = %d\n", x);
  return 0;
}

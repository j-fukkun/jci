
void func(){
  int c = 0;
  for(int j = 0; j < 10; j++){
    for(int i = 0; i < 10; i++){
      c = c + 1;
    }
  }
  
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

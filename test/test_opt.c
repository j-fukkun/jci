
int printf();

int assert(int expected, int actual, char* code) {
  if (expected == actual) {
    printf("%s => %d\n", code, actual);
  } else {
    printf("%s => %d expected but got %d\n", code, expected, actual);
    exit(1);
  }
}

int main(){

  {int x=5+10; assert(15, x, "int x=5+10; x;");}
  {int x=10-5; assert(5, x, "int x=10-5; x;");}
  {int x=2*3; assert(6, x, "int x=2*3; x;");}
  {int x=20/2; assert(10, x, "int x=20/2; x;");}
  {int x=18%4; assert(2, x, "int x=18%4; x;");}
  
  {int x=2==3; assert(0, x, "int x=2==3; x;");}
  {int x=2==2; assert(1, x, "int x=2==2; x;");}
  {int x=2<3; assert(1, x, "int x=2<3; x;");}
  {int x=2>3; assert(0, x, "int x=2>3; x;");}
  {int x=2<=3; assert(1, x, "int x=2<=3; x;");}
  {int x=2>=3; assert(0, x, "int x=2>=3; x;");}
  {int x=2<=2; assert(1, x, "int x=2<=2; x;");}

  {int x=1<<3; assert(8, x, "int x=1<<3; x;");}
  {int x=4>>2; assert(1, x, "int x=4>>2; x;");}

  {int x=6|3; assert(7, x, "int x=6|3; x;");}
  {int x=6&3; assert(2, x, "int x=6&3; x;");}
  {int x=15^5; assert(10, x, "int x=15^5; x;");}
  
  printf("OK.\n");
  return 0;
}

//This file is the test for optimization

int printf();
int exit();

int assert(int expected, int actual, char* code) {
  if (expected == actual) {
    printf("%s => %d\n", code, actual);
  } else {
    printf("%s => %d expected but got %d\n", code, expected, actual);
    exit(1);
  }
}

void f(){return;}

int test_mem2reg(int x, int y){
  int a = x;
  a = a + y;
  f();
  int b = a + 1;
  return a;
}

int peephole_unnecessary_comp_test(int a){
  //test for unnecessary computation
  int x = 1 * (0 + a + 0 - 0) * 1 / 1 % 1;
  assert(0, x, "int x = 1 * (0 + a + 0 - 0) * 1 / 1 % 1;");
  return 0;
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
  
  {int x = test_mem2reg(1,2); assert(3, x, "test_mem2reg(1,2);");}

  {int x; if(0){x = 1;}else{x = 2;} assert(2, x, "if(0){x = 1;}else{x = 2;}");}
  {int x; if(1){x = 1;}else{x = 2;} assert(1, x, "if(1){x = 1;}else{x = 2;}");}
  {int x; if(5){x = 1;}else{x = 2;} assert(1, x, "if(5){x = 1;}else{x = 2;}");}
  
  {int x = 10; peephole_unnecessary_comp_test(x);}

  printf("The optimization tests are passed.\n");
  return 0;
}

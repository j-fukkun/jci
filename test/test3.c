
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

  //{int x=5; x<<=1; assert(10, x, "int x=5; x<<=1; x;");}
  assert(513, (short)8590066177, "(short)8590066177");
}

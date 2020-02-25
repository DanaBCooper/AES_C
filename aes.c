#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int add_f28(int left, int right){
  return (left ^ right);
}

int mul_f28_by_x(int a){
  if ((a & 0x80) == 0x80)
    return 0x1B ^ ((a << 1) % 256);
  return ((a << 1 ) % 256);
}

int mul_f28(int a, int b){
  int t = 0;
  for(int i=7; i>= 0; i--){
    t = mul_f28_by_x(t);
    if ( (b & ((int)( pow (2,i) )) ) == ((int)( pow (2,i))) )
      t = t ^ a;
  }
  return t;
}

int inverse(int a){

}

int main( int argc, char* argv[] ) {

  int key_1 = (int)strtol("2B7E1516", NULL, 16);
  int key_2 = (int)strtol("28AED2A6", NULL, 16);
  int key_3 = (int)strtol("ABF71588", NULL, 16);
  int key_4 = (int)strtol("09CF4F3C", NULL, 16);

  int msg_1 = (int)strtol("3243F6A8", NULL, 16);
  int msg_2 = (int)strtol("885A308D", NULL, 16);
  int msg_3 = (int)strtol("313198A2", NULL, 16);
  int msg_4 = (int)strtol("E0370734", NULL, 16);

  printf("%d \n",mul_f28(90,90));

}

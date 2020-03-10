#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


u_int8_t next_round_constant(u_int8_t round, u_int8_t prev_rc){
  if(round == 1) return 1;
  if (round > 1 && prev_rc < 80) return 2* prev_rc;
  if (round >1 && prev_rc >= 80) return ((2 * prev_rc) ^ 0x1B);
}

void add_key(u_int8_t* state, u_int8_t* key){
  for(int i=0; i<16; i++){
    state[i] = state[i] ^ key[i];

  }
}



u_int8_t add_f28(u_int8_t left, u_int8_t right){
  return (left ^ right);
}

u_int8_t mul_f28_by_x(u_int8_t a){
  if ((a & 0x80) == 0x80)
    return 0x1B ^ ((a << 1) % 256);
  return ((a << 1 ) % 256);
}

u_int8_t mul_f28(u_int8_t a, u_int8_t b){
  u_int8_t t = 0;
  for(int i=7; i>= 0; i--){
    t = mul_f28_by_x(t);
    if ( (b & ((int)( pow (2,i) )) ) == ((int)( pow (2,i))) )
      t = t ^ a;
  }
  return t;
}

u_int8_t inverse(u_int8_t a){
  u_int8_t b = mul_f28(a, a); // b = a^2
  u_int8_t c = mul_f28(a, b); // c = a^3
  b = mul_f28(b, b);     // b = a^4
  c = mul_f28(b, c);     // c = a^7
  b = mul_f28(b, b);     // b = a^8
  b = mul_f28(b, c);     // b = a^15
  b = mul_f28(b, b);     // b = a^30
  b = mul_f28(b, b);     // b = a^60
  c = mul_f28(b, c);     // c = a^67
  c = mul_f28(b, c);     // c = a^127
  c = mul_f28(c, c);     // c = a^254
  return c;

}

u_int8_t s_box(u_int8_t elem){
  elem = inverse(elem);
  elem = 99 ^ elem ^ ((elem << 1) % 256) ^ ((elem << 2) % 256)
         ^ ((elem << 3) % 256) ^ ((elem << 4) % 256) ^ ((elem >> 4) % 256)
         ^ ((elem >> 5) % 256) ^ ((elem >> 6) % 256) ^ ((elem >> 7) % 256);
}

void sub_bytes(u_int8_t* s){
  for(int i=0; i<16; i++){
    s[i] = s_box(s[i]);
  }

}

void shift_rows(u_int8_t* s){
  u_int8_t a[16];

  for(int i=0; i<16;i++){
    a[i] = s[i];
  }

  s[0] = a[0];
  s[1] = a[5];
  s[2] = a[10];
  s[3] = a[15];
  s[4] = s[4];
  s[5] = a[9];
  s[6] = a[14];
  s[7] = a[3];
  s[8] = a[8];
  s[9] = a[13];
  s[10] = a[2];
  s[11] =a[7];
  s[12] = a[12];
  s[13] = a[1];
  s[14] = a[6];
  s[15] = a[11];

}

void mix_columns(u_int8_t* s){
  u_int8_t s_new[4][4];
  u_int8_t matrix[4][4] = {{2, 3 , 1 ,1}, {1,2,3,1}, {1,1,2,3}, {3,1,1,2}};

  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      s_new[i][j] = add_f28(mul_f28(matrix[j][0], s[i*4]), add_f28(mul_f28(matrix[j][1], s[(i*4)+1]),
                                  add_f28(mul_f28(matrix[j][2], s[(i*4) + 2]), mul_f28(matrix[j][3], s[(i*4) + 3]))));
    }
  }
  for( int j = 0; j< 4; j++){
    s[j*4] = s_new[j][0];
    s[(j*4)+1] = s_new[j][1];
    s[(j*4)+2] = s_new[j][2];
    s[(j*4)+3] = s_new[j][3];
  }
}


void next_key(u_int8_t* key, int r_const, u_int8_t* new_key){
  u_int8_t buffer[4];
  buffer[0] =(r_const ^ s_box(key[13]) ^ key[0]);
  buffer[1] = (s_box(key[14]) ^ key[1]);
  buffer[2] = (s_box(key[15]) ^ key[2]);
  buffer[3] = (s_box(key[12]) ^ key[3]);
  new_key[0] = buffer[0];
  new_key[1] = buffer[1];
  new_key[2] = buffer[2];
  new_key[3] = buffer[3];
  for(int i=1; i<4; i++){
  	for(int j = 0; j< 4; j++){
	    buffer[j] = buffer[j] ^ key[(i*4) +j];
	}
  new_key[(i*4) + 0] = buffer[0];
  new_key[(i*4) + 1] = buffer[1];
  new_key[(i*4) + 2] = buffer[2];
  new_key[(i*4) + 3] = buffer[3];
  }
}


void generate_round_keys(u_int8_t* init_key, u_int8_t keys[11][16]){
	int round_constant = 1;
	u_int8_t temp_key[16];
	for(int k=0; k< 16; k++){
		    keys[0][k] = init_key[k];

	}
	for(int i = 1; i< 11; i++){
		round_constant = next_round_constant(i, round_constant);
		next_key(keys[i-1], round_constant, temp_key);
		for(int k=0; k< 16; k++){
  		    keys[i][k] = temp_key[k];

		}
	}
}



void aes_enc_with_full_keys(u_int8_t keys[11][16], u_int8_t* state, u_int8_t* cipher){
	add_key(state,keys[0]);
	for(int rc = 1; rc<10; rc++){
		sub_bytes(state);
		shift_rows(state);
		mix_columns(state);
		add_key(state,keys[rc]);
	}
	sub_bytes(state);
	shift_rows(state);
	add_key(state,keys[10]);
	for(int i =0; i< 16; i++){
		cipher[i] = state[i];
	}

}


int main( int argc, char* argv[] ) {
  u_int8_t key[16];
  u_int8_t message[16];
  u_int8_t cipher[16];

  key[0] = (int)strtol("2B", NULL, 16);
  key[1] = (int)strtol("7E", NULL, 16);
  key[2] = (int)strtol("15", NULL, 16);
  key[3] = (int)strtol("16", NULL, 16);

  key[4] = (int)strtol("28", NULL, 16);
  key[5] = (int)strtol("AE", NULL, 16);
  key[6] = (int)strtol("D2", NULL, 16);
  key[7] = (int)strtol("A6", NULL, 16);

  key[8] = (int)strtol("AB", NULL, 16);
  key[9] = (int)strtol("F7", NULL, 16);
  key[10] = (int)strtol("15", NULL, 16);
  key[11] = (int)strtol("88", NULL, 16);

  key[12] = (int)strtol("09", NULL, 16);
  key[13] = (int)strtol("CF", NULL, 16);
  key[14] = (int)strtol("4F", NULL, 16);
  key[15] = (int)strtol("3C", NULL, 16);

  message[0] = (int)strtol("32", NULL, 16);
  message[1] = (int)strtol("43", NULL, 16);
  message[2] = (int)strtol("F6", NULL, 16);
  message[3] = (int)strtol("A8", NULL, 16);

  message[4] = (int)strtol("88", NULL, 16);
  message[5] = (int)strtol("5A", NULL, 16);
  message[6] = (int)strtol("30", NULL, 16);
  message[7] = (int)strtol("8D", NULL, 16);

  message[8] = (int)strtol("31", NULL, 16);
  message[9] = (int)strtol("31", NULL, 16);
  message[10] = (int)strtol("98", NULL, 16);
  message[11] = (int)strtol("A2", NULL, 16);

  message[12] = (int)strtol("E0", NULL, 16);
  message[13] = (int)strtol("37", NULL, 16);
  message[14] = (int)strtol("07", NULL, 16);
  message[15] = (int)strtol("34", NULL, 16);



  u_int8_t keys[11][16];
  generate_round_keys(key, keys);

  clock_t begin = clock();
  aes_enc_with_full_keys(keys,message,cipher);
  clock_t end = clock();
  double time_spent = (double) (end-begin) / CLOCKS_PER_SEC;
  printf("Time spent %f \n",time_spent);
  printf("%x \n",cipher[0]);
  printf("%x \n",cipher[1]);
  printf("%x \n",cipher[2]);
  printf("%x \n",cipher[3]);
  printf("%x \n",cipher[4]);
  printf("%x \n",cipher[5]);
  printf("%x \n",cipher[6]);
  printf("%x \n",cipher[7]);
  printf("%x \n",cipher[8]);
  printf("%x \n",cipher[9]);
  printf("%x \n",cipher[10]);
  printf("%x \n",cipher[11]);
  printf("%x \n",cipher[12]);
  printf("%x \n",cipher[13]);
  printf("%x \n",cipher[14]);
  printf("%x \n",cipher[15]);

}

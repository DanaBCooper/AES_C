#include <stdio.h>
#include <stdlib.h>
#include <math.h>



u_int8_t next_round_constant(u_int8_t round, u_int8_t prev_rc){
  if(round == 1) return 1;
  if (round > 1 && prev_rc < 80) return 2* prev_rc;
  if (round >1 && prev_rc >= 80) return ((2 * prev_rc) ^ 0x1B);
}

void add_key(int* state, int* key){
  u_int8_t a[4][4];
  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      a[i][j] = (state[i] & (0xff000000 >> (j * 8))) >> ((3-j)*8) ^ (key[i] & (0xff000000 >> (j * 8))) >> ((3-j)*8);
    }
    for( int j = 0; j< 4; j++){
      state[i] = (a[i][0] << 24) + (a[i][1] << 16) + (a[i][2] << 8) + a[i][3] ;
    }
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

void sub_bytes(int* s){
  int a[4];
  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      a[j] = (s[i] & (0xff000000 >> (j * 8))) >> ((3-j)*8);
      a[j] = s_box(a[j]);
    }
    s[i] = (a[0] << 24) + (a[1] << 16) + (a[2] << 8) + a[3] ;
  }

}

void shift_rows(int* s){
  int a[4][4];
  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      a[i][j] = (s[i] & (0xff000000 >> (j * 8))) >> ((3-j)*8);
    }
  }
  s[0] = (a[0][0] << 24) + (a[1][1] << 16) + (a[2][2] << 8) + a[3][3] ;
  s[1] = (a[1][0] << 24) + (a[2][1] << 16) + (a[3][2] << 8) + a[0][3] ;
  s[2] = (a[2][0] << 24) + (a[3][1] << 16) + (a[0][2] << 8) + a[1][3] ;
  s[3] = (a[3][0] << 24) + (a[0][1] << 16) + (a[1][2] << 8) + a[2][3] ;

}

void mix_columns(int* s){
  u_int8_t s_new[4][4];
  u_int8_t a[4][4];
  u_int8_t matrix[4][4] = {{2, 3 , 1 ,1}, {1,2,3,1}, {1,1,2,3}, {3,1,1,2}};

  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      a[i][j] = (s[i] & (0xff000000 >> (j * 8))) >> ((3-j)*8);
    }
  }
  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      s_new[i][j] = add_f28(mul_f28(matrix[j][0], a[i][0]), add_f28(mul_f28(matrix[j][1], a[i][1]),
                                  add_f28(mul_f28(matrix[j][2], a[i][2]), mul_f28(matrix[j][3], a[i][3]))));
    }
  }
  for( int j = 0; j< 4; j++){
    s[j] = (s_new[j][0] << 24) + (s_new[j][1] << 16) + (s_new[j][2] << 8) + s_new[j][3] ;
  }
}


void next_key(int* round_key, int r_const, int* new_key){
  u_int8_t a[4][4];
  u_int8_t buffer[4];
  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      a[i][j] = (round_key[i] & (0xff000000 >> (j * 8))) >> ((3-j)*8);    }
  }
  buffer[0] =(r_const ^ s_box(a[3][1]) ^ a[0][0]);
  buffer[1] = (s_box(a[3][2]) ^ a[0][1]);
  buffer[2] = (s_box(a[3][3]) ^ a[0][2]);
  buffer[3] = (s_box(a[3][0]) ^ a[0][3]);
  new_key[0] = ( buffer[0] << 24) + ( buffer[1] << 16) +
                ( buffer[2] << 8) + buffer[3];
  for(int i=1; i<4; i++){
  	for(int j = 0; j< 4; j++){
	    buffer[j] = buffer[j] ^ a[i][j];
	}
	new_key[i] = ( buffer[0] << 24) + ( buffer[1] << 16) +
                ( buffer[2] << 8) + buffer[3];
  }
}


void generate_round_keys(int* init_key, int keys[11][4]){
	int round_constant = 1;
	int temp_key[4];
	for(int k=0; k< 4; k++){
		keys[0][k] = init_key[k];
	}
	for(int i = 1; i< 11; i++){
		round_constant = next_round_constant(i, round_constant);
		next_key(keys[i-1], round_constant, temp_key);
		for(int k=0; k< 4; k++){
			keys[i][k] = temp_key[k];
		}
	}	
}


void aes_enc_with_full_keys(int keys[11][4], int* state, int* cipher){
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
	for(int i =0; i< 4; i++){
		cipher[i] = state[i];
	}

}

void aes_enc(int* key, int* message, int* cipher){
  int round_key[4];
  int state[4];
  int round_constant = 0;
  int new_key[4];
  for(int i=0; i<4; i++){
    round_key[i] = key[i];
    state[i] = message[i];
  }
  add_key(state,round_key);

  for(int round_count = 1; round_count< 10; round_count++){
    
    sub_bytes(state);
    shift_rows(state);
    mix_columns(state);
    round_constant = next_round_constant(round_count, round_constant);
    next_key(round_key, round_constant, new_key);

   for(int i=0; i<4; i++){
      round_key[i] = new_key[i];
    }
  
    add_key(state, round_key);
  }
  sub_bytes(state);
  shift_rows(state);
  round_constant = next_round_constant(10, round_constant);
  next_key(round_key, round_constant, new_key);
  for(int i=0; i<4; i++){
    round_key[i] = new_key[i];
  }
  add_key(state, round_key);
  for(int i=0; i<4; i++){
    cipher[i] = state[i];
  }
}

int main( int argc, char* argv[] ) {
  int key[4];
  int message[4];
  int cipher[4];
  key[0] = (int)strtol("2B7E1516", NULL, 16);
  key[1] = (int)strtol("28AED2A6", NULL, 16);
  key[2] = (int)strtol("ABF71588", NULL, 16);
  key[3] = (int)strtol("09CF4F3C", NULL, 16);

  message[0] = (int)strtol("3243F6A8", NULL, 16);
  message[1] = (int)strtol("885A308D", NULL, 16);
  message[2] = (int)strtol("313198A2", NULL, 16);
  message[3] = (int)strtol("E0370734", NULL, 16);

  aes_enc(key,message,cipher); 
  printf("%x \n",cipher[0]);
  printf("%x \n",cipher[1]);
  printf("%x \n",cipher[2]);
  printf("%x \n",cipher[3]);

  int keys[11][4];
  generate_round_keys(key, keys);

  aes_enc_with_full_keys(keys,message,cipher);
  printf("%x \n",cipher[0]);
  printf("%x \n",cipher[1]);
  printf("%x \n",cipher[2]);
  printf("%x \n",cipher[3]);


}

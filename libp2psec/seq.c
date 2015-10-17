

#ifndef F_SEQ_C
#define F_SEQ_C


#include <stdint.h>


#define seq_SIZE 8


#define seq_WINDOWSIZE 16384


struct s_seq_state {
  int64_t start;
  uint64_t mask;
};


static int64_t seqGet(struct s_seq_state *state) {
  return state->start;
}


static void seqInit(struct s_seq_state *state, const int64_t seq) {
  state->start = seq;
  state->mask = 0;
}


static int seqVerify(struct s_seq_state *state, const int64_t seq) {
  const uint_least64_t one = 1;
  int64_t start = state->start;
  int64_t seqdiff = (seq - start);
  uint64_t mask = state->mask;
  uint64_t vmask;
  if((seqdiff > 0) && (seqdiff < seq_WINDOWSIZE)) {
    if(seqdiff > 64) {
      seqdiff = (seqdiff - 64);
      start = (start + seqdiff);
      if(seqdiff > 64) {
        mask = 0;
      }
      else {
        mask = (mask << seqdiff);
      }
      seqdiff = 64;
    }

    vmask = (one << (64 - seqdiff));
    if((vmask & mask) == 0) {
      mask = (mask | vmask);
      state->start = start;
      state->mask = mask;
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    return 0;
  }
}


static int seqRQ(struct s_seq_state *state) {
  const int bitc[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
  };
  const unsigned char *bytes = (unsigned char *)&state->mask;
  int c = 0;
  int i;
  for(i=0; i<(sizeof(uint64_t)); i++) {
    c = c + bitc[bytes[i]];
  }
  return c;
}


#endif // F_SEQ_C

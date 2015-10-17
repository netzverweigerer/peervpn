

#ifndef F_DFRAG_C
#define F_DFRAG_C


#include <stdlib.h>
#include <stdint.h>
#include <string.h>


struct s_dfrag {
  unsigned char *fragbuf;
  int *used;
  int *peerct;
  int *peerid;
  int64_t *seq;
  int *length;
  int *msglength;
  int fragbuf_size;
  int fragbuf_count;
  int pos;
};


static void dfragReset(struct s_dfrag *dfrag) {
  int i, j;
  j = dfrag->fragbuf_count;
  for(i=0; i<j; i++) {
    dfrag->used[i] = 0;
  }
  dfrag->pos = 0;
}


static int dfragIsID(struct s_dfrag *dfrag, const int peerct, const int peerid, const int64_t seq, const int id) {
  if(dfrag->used[id] > 0) {
    if(dfrag->peerct[id] == peerct) {
      if(dfrag->peerid[id] == peerid) {
        if(dfrag->seq[id] == seq) {
          return 1;
        }
      }
    }
  }
  return 0;
}


static int dfragGetID(struct s_dfrag *dfrag, const int peerct, const int peerid, const int64_t seq) {
  int i, j;
  j = dfrag->fragbuf_count;
  for(i=0; i<j; i++) {
    if(dfragIsID(dfrag, peerct, peerid, seq, i)) {
      return i;
    }
  }
  return -1;
}


static int dfragAllocateID(struct s_dfrag *dfrag, const int fragment_count) {
  int i;
  int id;
  int pos = dfrag->pos;
  int fbcount = dfrag->fragbuf_count;
  if(fbcount >= fragment_count) {
    if((fbcount - pos) >= fragment_count) {
      id = pos;
    }
    else {
      id = 0;
    }
    dfrag->used[id] = fragment_count;
    dfrag->length[id] = 0;
    dfrag->msglength[id] = 0;
    dfrag->pos = id + fragment_count;
    
    for(i=1; i<fragment_count; i++) {
      dfrag->used[(id + i)] = 0;
      dfrag->length[(id + i)] = 0;
      dfrag->msglength[(id + i)] = 0;
    }
    
    return id;
  }
  else {
    return -1;
  }
}


static void dfragClear(struct s_dfrag *dfrag, const int id) {
  dfrag->used[id] = 0;
  dfrag->length[id] = 0;
  dfrag->msglength[id] = 0;
}


static int dfragLength(struct s_dfrag *dfrag, const int id) {
  return dfrag->msglength[id];
}


static unsigned char *dfragGet(struct s_dfrag *dfrag, const int id) {
  return &dfrag->fragbuf[(id * dfrag->fragbuf_size)];
}


static int dfragCalcLength(struct s_dfrag *dfrag, const int id) {
  int i;
  int len;
  int fragcount = dfrag->used[id];

  if(!(fragcount > 0)) { return 0; }

  len = dfrag->length[id + (fragcount - 1)];
  if(!(len > 0)) { return 0; }

  i = fragcount - 1;
  while(i > 0) {
    i = i - 1;
    if(dfrag->length[id + i] != dfrag->fragbuf_size) { return 0; }
    len = len + dfrag->fragbuf_size;
  }
  
  dfrag->msglength[id] = len;
  return len;
}


static int dfragAssemble(struct s_dfrag *dfrag, const int peerct, const int peerid, const int64_t seq, const unsigned char *fragment, const int fragment_len, const int fragment_pos, const int fragment_count) {
  int id;

  if((!(fragment_count > 0)) || (fragment_pos < 0) || (!(fragment_pos < fragment_count)) || (!(fragment_len > 0)) || (fragment_len > dfrag->fragbuf_size) || ((fragment_len != dfrag->fragbuf_size) && ((fragment_pos + 1) != fragment_count))) { return -1; }

  id = dfragGetID(dfrag, peerct, peerid, seq);
  
  if(id < 0) {
    id = dfragAllocateID(dfrag, fragment_count);
    if(id < 0) { return -1; }

    dfrag->peerct[id] = peerct;
    dfrag->peerid[id] = peerid;
    dfrag->seq[id] = seq;
  }
  else {
    if((fragment_count != dfrag->used[id]) || (peerid != dfrag->peerid[id]) || (seq != dfrag->seq[id]) || (!((id + fragment_pos) < dfrag->fragbuf_count))) { return -1; }
  }

  dfrag->length[(id + fragment_pos)] = fragment_len;
  memcpy(&dfrag->fragbuf[((id + fragment_pos) * dfrag->fragbuf_size)], fragment, fragment_len);
  
  dfragCalcLength(dfrag, id);
  if(dfragLength(dfrag, id) > 0) {
    return id;
  }
  else {
    return -1;
  }
}


static int dfragCreate(struct s_dfrag *dfrag, const int size, const int count) {
  unsigned char *fragbuf_mem = NULL;
  int *used_mem = NULL;
  int *peerct_mem = NULL;
  int *peerid_mem = NULL;
  int64_t *seq_mem = NULL;
  int *length_mem = NULL;
  int *msglength_mem = NULL;
  if(size > 0 && count > 0) {
    fragbuf_mem = malloc(size * count);
    if(fragbuf_mem != NULL) {
      used_mem = malloc(sizeof(int) * count);
      if(used_mem != NULL) {
        peerct_mem = malloc(sizeof(int) * count);
        if(peerct_mem != NULL) {
          peerid_mem = malloc(sizeof(int) * count);
          if(peerid_mem != NULL) {
            seq_mem = malloc(sizeof(int64_t) * count);
            if(seq_mem != NULL) {
              length_mem = malloc(sizeof(int) * count);
              if(length_mem != NULL) {
                msglength_mem = malloc(sizeof(int) * count);
                if(msglength_mem != NULL) {
                  dfrag->fragbuf = fragbuf_mem;
                  dfrag->used = used_mem;
                  dfrag->peerct = peerct_mem;
                  dfrag->peerid = peerid_mem;
                  dfrag->seq = seq_mem;
                  dfrag->length = length_mem;
                  dfrag->msglength = msglength_mem;
                  dfrag->fragbuf_count = count;
                  dfrag->fragbuf_size = size;
                  dfragReset(dfrag);
                  return 1;
                }
                free(length_mem);
              }
              free(seq_mem);
            }
            free(peerid_mem);
          }
          free(peerct_mem);
        }
        free(used_mem);
      }
      free(fragbuf_mem);
    }
  }
  return 0;
}


static void dfragDestroy(struct s_dfrag *dfrag) {
  free(dfrag->msglength);
  free(dfrag->length);
  free(dfrag->seq);
  free(dfrag->peerid);
  free(dfrag->peerct);
  free(dfrag->used);
  free(dfrag->fragbuf);
}


#endif // F_DFRAG_C

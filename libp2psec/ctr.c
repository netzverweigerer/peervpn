

#ifndef F_CTR_C
#define F_CTR_C


#include "util.c"


#define ctr_SECS 16


struct s_ctr_state {
  int total;
  int cur;
  int last[ctr_SECS + 16];
  int lasttime;
  int lastpos;
};


static void ctrClear(struct s_ctr_state *ctr) {
  int i;
  ctr->total = 0;
  ctr->cur = 0;
  for(i=0; i<ctr_SECS; i++) {
    ctr->last[i] = 0;
  }
}


static void ctrInit(struct s_ctr_state *ctr) {
  ctrClear(ctr);
  ctr->lasttime = utilGetClock();
  ctr->lastpos = 0;
}


static void ctrIncr(struct s_ctr_state *ctr, const int inc) {
  int diff;
  int lasttime = ctr->lasttime;
  int tnow = utilGetClock();
  
  diff = (tnow - lasttime);
  if(diff > ctr_SECS) {
    ctrClear(ctr);
  }
  else {
    while(diff > 1) {
      ctr->last[ctr->lastpos] = 0;
      ctr->lastpos = ((ctr->lastpos + 1) % ctr_SECS);
      diff--;
    }
    if(diff > 0) {
      ctr->last[ctr->lastpos] = ctr->cur;
      ctr->lastpos = ((ctr->lastpos + 1) % ctr_SECS);
      ctr->cur = 0;
      diff--;
    }
  }
  
  ctr->cur = (ctr->cur + inc);
  ctr->total = (ctr->total + inc);
  ctr->lasttime = tnow;
}


static int ctrAvg(struct s_ctr_state *ctr) {
  int i = 0;
  int sum = 0;
  while(i < ctr_SECS) {
    sum = sum + ctr->last[i];
    i++;
  }
  return(sum / ctr_SECS);
}


#endif // F_CTR_C

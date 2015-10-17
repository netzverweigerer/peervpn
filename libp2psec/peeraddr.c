

#ifndef F_PEERADDR_C
#define F_PEERADDR_C


#include "util.c"


#define peeraddr_INTERNAL_INDIRECT 1


#define peeraddr_SIZE 24


#if peeraddr_SIZE != 24
#error invalid peeraddr_SIZE
#endif


struct s_peeraddr {
  unsigned char addr[peeraddr_SIZE];
};


static int peeraddrIsInternal(const struct s_peeraddr *peeraddr) {
  int i;
  i = utilReadInt32(&peeraddr->addr[0]);
  if(i == 0) {
    return 1;
  }
  else {
    return 0;
  }
}


static int peeraddrGetInternalType(const struct s_peeraddr *peeraddr) {
  if(peeraddrIsInternal(peeraddr)) {
    return utilReadInt32(&peeraddr->addr[4]);
  }
  else {
    return -1;
  }
}


static int peeraddrGetIndirect(const struct s_peeraddr *peeraddr, int *relayid, int *relayct, int *peerid) {
  if(peeraddrGetInternalType(peeraddr) == peeraddr_INTERNAL_INDIRECT) {
    if(relayid != NULL) {
      *relayid = utilReadInt32(&peeraddr->addr[8]);
    }
    if(relayct != NULL) {
      *relayct = utilReadInt32(&peeraddr->addr[12]);
    }
    if(peerid != NULL) {
      *peerid = utilReadInt32(&peeraddr->addr[16]);
    }
    return 1;
  }
  else {
    return 0;
  }
}


static void peeraddrSetIndirect(struct s_peeraddr *peeraddr, const int relayid, const int relayct, const int peerid) {
  utilWriteInt32(&peeraddr->addr[0], 0);
  utilWriteInt32(&peeraddr->addr[4], peeraddr_INTERNAL_INDIRECT);
  utilWriteInt32(&peeraddr->addr[8], relayid);
  utilWriteInt32(&peeraddr->addr[12], relayct);
  utilWriteInt32(&peeraddr->addr[16], peerid);
  utilWriteInt32(&peeraddr->addr[20], 0);
}


#endif // F_PEERADDR_C

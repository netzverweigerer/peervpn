

#ifndef F_NETID_C
#define F_NETID_C


#include "crypto.c"


#define netid_SIZE 32


struct s_netid {
  unsigned char id[netid_SIZE];
};


static int netidSet(struct s_netid *netid, const char *netname, const int netname_len) {
  memset(netid->id, 0, netid_SIZE);
  return cryptoCalculateSHA256(netid->id, netid_SIZE, (unsigned char *)netname, netname_len);
}


#endif // F_NETID_C

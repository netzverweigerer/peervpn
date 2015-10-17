

#ifndef F_NODEID_C
#define F_NODEID_C


#include "rsa.c"


#define nodeid_SIZE 32


#define nodekey_MINSIZE rsa_MINSIZE
#define nodekey_MAXSIZE rsa_MAXSIZE


struct s_nodeid {
  unsigned char id[nodeid_SIZE];
};


struct s_nodekey {
  struct s_nodeid nodeid;
  struct s_rsa key;
};


static int nodekeyCreate(struct s_nodekey *nodekey) {
  return rsaCreate(&nodekey->key);
}


static int nodekeyGetDER(unsigned char *buf, const int buf_size, const struct s_nodekey *nodekey) {
  return rsaGetDER(buf, buf_size, &nodekey->key);
}


static int nodekeyGenerate(struct s_nodekey *nodekey, const int key_size) {
  if(rsaGenerate(&nodekey->key, key_size)) {    
    return rsaGetFingerprint(nodekey->nodeid.id, nodeid_SIZE, &nodekey->key);
  }
  else {
    return 0;
  }
}


static int nodekeyLoadDER(struct s_nodekey *nodekey, const unsigned char *pubkey, const int pubkey_size) {
  if(rsaLoadDER(&nodekey->key, pubkey, pubkey_size)) {
    return rsaGetFingerprint(nodekey->nodeid.id, nodeid_SIZE, &nodekey->key);
  }
  else {
    return 0;
  }
}


static int nodekeyLoadPEM(struct s_nodekey *nodekey, unsigned char *pubkey, const int pubkey_size) {
  if(rsaLoadPEM(&nodekey->key, pubkey, pubkey_size)) {
    return rsaGetFingerprint(nodekey->nodeid.id, nodeid_SIZE, &nodekey->key);
  }
  else {
    return 0;
  }
}


static int nodekeyLoadPrivatePEM(struct s_nodekey *nodekey, unsigned char *privkey, const int privkey_size) {
  if(rsaLoadPrivatePEM(&nodekey->key, privkey, privkey_size)) {
    return rsaGetFingerprint(nodekey->nodeid.id, nodeid_SIZE, &nodekey->key);
  }
  else {
    return 0;
  }
}


static void nodekeyDestroy(struct s_nodekey *nodekey) {
  rsaDestroy(&nodekey->key);
}



#endif // F_NODEID_C

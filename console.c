


static void printActivePeerTable() {
  char str[32768];
  p2psecStatus(g_p2psec, str, 32768);
  printf("%s\n", str);
}


static void printNodeDB() {
  char str[32768];
  p2psecNodeDBStatus(g_p2psec, str, 32768);
  printf("%s\n", str);
}


static void printRelayDB() {
  char str[32768];
  nodedbStatus(&g_p2psec->mgt.relaydb, str, 32768);
  printf("%s\n", str);
}


static void printMacTable() {
  char str[32768];
  switchStatus(&g_switchstate, str, 32768);
  printf("%s\n", str);
}


static void printNDPTable() {
  char str[32768];
  ndp6Status(&g_ndpstate, str, 32768);
  printf("%s\n", str);
}


static void decodeConsole(char *cmd, int cmdlen) {
  char text[4096];
  char pa[1024];
  char pb[1024];
  char pc[1024];
  int i, j;
  struct s_io_addrinfo new_peeraddrs;
  struct s_peeraddr new_peeraddr; 
  
  pa[0] = '\0';
  pb[0] = '\0';
  pc[0] = '\0';
  sscanf(cmd,"%s %s %s",pa,pb,pc);
  if(pa[0] == 'A' || pa[0] == 'a') {
    printActivePeerTable();
  }
  if(pa[0] == 'D' || pa[0] == 'd') {
    printNodeDB();
  }
  if(pa[0] == 'F' || pa[0] == 'f') {
    printRelayDB();
  }
  if(pa[0] == 'I' || pa[0] == 'i') {
    if(ioResolveName(&new_peeraddrs, pb, pc)) {
      for(i=0; i<new_peeraddrs.count; i++) {
        if(p2psecConnect(g_p2psec, new_peeraddrs.item[i].addr)) {
          utilByteArrayToHexstring(text, 4096, new_peeraddrs.item[i].addr, peeraddr_SIZE);
          printf("connecting to %s...\n", text);
        }
      }
    }
    else {
      printf("could not get peer address.\n");
    }
  }
  if(pa[0] == 'M' || pa[0] == 'm') {
    printMacTable();
  }
  if(pa[0] == 'N' || pa[0] == 'n') {
    printNDPTable();
  }
  if(pa[0] == 'P' || pa[0] == 'p') {
    printActivePeerTable();
  }
  if(pa[0] == 'R' || pa[0] == 'r') {
    sscanf(pb,"%d",&i);
    sscanf(pc,"%d",&j);
    if(peermgtIsActiveRemoteID(&g_p2psec->mgt, i)) {
      peeraddrSetIndirect(&new_peeraddr, i, g_p2psec->mgt.data[i].conntime, j);
      if(p2psecConnect(g_p2psec, new_peeraddr.addr)) {
        utilByteArrayToHexstring(text, 4096, new_peeraddr.addr, peeraddr_SIZE);
        printf("connecting to %s...\n", text);
      }
    }
    else {
      printf("could not get peer address.\n");
    }
  }
  if(pa[0] == 'Q' || pa[0] == 'q') {
    g_mainloop = 0;
  }
}

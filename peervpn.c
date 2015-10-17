

#include <signal.h>
#include <stdio.h>
#include <openssl/engine.h>


#include "ethernet/switch.c"
#include "ethernet/ndp6.c"
#include "ethernet/virtserv.c"
#include "libp2psec/p2psec.c"
#include "platform/io.c"
#include "platform/ifconfig.c"
#include "platform/seccomp.c"
#include "globals.c"
#include "console.c"
#include "mainloop.c"
#include "config.c"
#include "pwd.c"
#include "init.c"


int main(int argc, char **argv) {
  int confok;
  int conffd;
  int arglen;
  int i;
  struct s_initconfig config;

  strcpy(config.tapname,"");
  strcpy(config.ifconfig4,"");
  strcpy(config.ifconfig6,"");
  strcpy(config.upcmd,"");
  strcpy(config.sourceip,"");
  strcpy(config.sourceport,"");
  strcpy(config.userstr,"");
  strcpy(config.groupstr,"");
  strcpy(config.chrootstr,"");
  strcpy(config.networkname,"PEERVPN");
  strcpy(config.initpeers,"");
  strcpy(config.engines,"");
  config.password_len = 0;
  config.enableeth = 1;
  config.enablendpcache = 0;
  config.enablevirtserv = 0;
  config.enablerelay = 0;
  config.enableindirect = 0;
  config.enableconsole = 0;
  config.enableseccomp = 0;
  config.forceseccomp = 0;
  config.enableprivdrop = 1;
  config.enableipv4 = 1;
  config.enableipv6 = 1;
  config.enablenat64clat = 0;
  config.sockmark = 0;

  setbuf(stdout,NULL);

  confok = 0;
  if(argc == 2) {
    arglen = 0;
    for(i=0; i<3; i++) {
      if(argv[1][i] == '\0') break;
      arglen++;
    }
    if(arglen > 0) {
      if(argv[1][0] == '-') {
        if(!((arglen > 1) && (argv[1][1] >= '!') && (argv[1][1] <= '~'))) {
          conffd = STDIN_FILENO;
          parseConfigFile(conffd,&config);
          confok = 1;
        }
      }
      else {
        if((conffd = (open(argv[1],O_RDONLY))) < 0) throwError("could not open config file!");
        parseConfigFile(conffd,&config);
        close(conffd);
        confok = 1;
      }
    }

  }
  
  if(confok > 0) {
    init(&config);
  }
  else {
    printf("usage: %s <path_to_config_file>\n", argv[0]);
  }

  return 0;
}

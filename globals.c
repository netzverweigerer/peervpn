#define INITPEER_STORAGE 1024
#define CONFPARSER_LINEBUF_SIZE 4096
#define CONFPARSER_NAMEBUF_SIZE 512
#define IOGRP_DEFAULT 0
#define IOGRP_SOCKET 1
#define IOGRP_TAP 2
#define IOGRP_CONSOLE 3

struct s_io_state iostate;
P2PSEC_CTX *g_p2psec = NULL;
int g_mainloop;
char g_initpeers[INITPEER_STORAGE+1];
struct s_switch_state g_switchstate;
struct s_ndp6_state g_ndpstate;
struct s_virtserv_state g_virtserv;
int g_enableconsole;
int g_enableeth;
int g_enablendpcache;
int g_enablevirtserv;
int g_enableengines;

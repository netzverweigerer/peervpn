

#ifndef F_VIRTSERV_C
#define F_VIRTSERV_C


#include "../libp2psec/map.c"
#include "ndp6.c"


#define virtserv_LISTENADDR_COUNT 8
#define virtserv_ADDR_SIZE 16
#define virtserv_MAC_SIZE 6
#define virtserv_MAC "\x00\x22\x00\xed\x13\x37"


#if virtserv_ADDR_SIZE != 16
#error virtserv_ADDR_SIZE != 16
#endif
#if virtserv_MAC_SIZE != 6
#error virtserv_MAC_SIZE != 6
#endif


struct s_virtserv_state {
  struct s_map listenaddrs;
  unsigned char mac[virtserv_MAC_SIZE];
};


static int virtservAddAddress(struct s_virtserv_state *virtserv, const unsigned char *ipv6address) {
  int tnow;
  tnow = utilGetClock();
  return mapAdd(&virtserv->listenaddrs, ipv6address, &tnow);
}


static int virtservCheckMac(struct s_virtserv_state *virtserv, const unsigned char *macaddress) {
  return (memcmp(virtserv->mac, macaddress, virtserv_MAC_SIZE) == 0);
}


static int virtservCheckAddress(struct s_virtserv_state *virtserv, const unsigned char *ipv6address) {
  return (mapGet(&virtserv->listenaddrs, ipv6address) != NULL);
}


static int virtservDecodeEcho(struct s_virtserv_state *virtserv, unsigned char *outbuf, const int outbuf_len, const unsigned char *inbuf, const int inbuf_len) {
  if(inbuf_len <= outbuf_len) {
    memcpy(outbuf, inbuf, inbuf_len);
    return inbuf_len;
  }
  return 0;
}


static int virtservDecodeUDPv6(struct s_virtserv_state *virtserv, unsigned char *outbuf, const int outbuf_len, const unsigned char *inbuf, const int inbuf_len) {
  int portnumber;
  int payload_len;
  if(inbuf_len >= 8) {
    portnumber = utilReadInt16(&inbuf[2]);
    switch(portnumber) {
      case 7: // echo service
        payload_len = virtservDecodeEcho(virtserv, &outbuf[8], (outbuf_len - 8), &inbuf[8], (inbuf_len - 8));
        break;
      default: // service not implemented
        payload_len = 0;
        break;
    }
    if((payload_len > 0) && (outbuf_len >= (payload_len + 8 + 4))) { // 4 extra bytes are needed for the padding zeroes
      memcpy(&outbuf[0], &inbuf[2], 2); // source port
      memcpy(&outbuf[2], &inbuf[0], 2); // destination port
      utilWriteInt16(&outbuf[4], (payload_len + 8)); // header + payload length
      memset(&outbuf[6], 0, 2); // reset checksum field
      memset(&outbuf[(payload_len + 8)], 0, 4); // generate padding zeroes after the end of the message (for correct checksum calculation)
      return (payload_len + 8);
    }
  }
  return 0;
}


static int virtservDecodeICMPv6(struct s_virtserv_state *virtserv, unsigned char *outbuf, const int outbuf_len, const unsigned char *inbuf, const int inbuf_len) {
  if((inbuf_len >= 8) && (outbuf_len >= (inbuf_len + 4))) { // 4 extra bytes are needed for the padding zeroes
    if((inbuf[0] == 0x80) && (inbuf[1] == 0x00)) { // Echo Request
      outbuf[0] = 0x81; // Echo Reply
      outbuf[1] = 0x00; // code 0
      memset(&outbuf[2], 0, 2); // reset checksum field
      memcpy(&outbuf[4], &inbuf[4], (inbuf_len - 4)); // copy ping pattern
      memset(&outbuf[inbuf_len], 0, 4); // generate padding zeroes after the end of the message (for correct checksum calculation)
      return inbuf_len;
    }
  }
  return 0;
}


static int virtservDecodeFrame(struct s_virtserv_state *virtserv, unsigned char *outframe, const int outframe_len, const unsigned char *inframe, const int inframe_len) {
  struct s_checksum checksum;
  uint16_t u;
  int i;
  int outlen;
  int nextheader;

  checksumZero(&checksum); // reset checksum
  for(i=0; i<32; i=i+2) checksumAdd(&checksum, *((uint16_t *)(&inframe[22+i]))); // checksum add IPv6 pseudoheader src+dest address

  nextheader = inframe[20];
  switch(nextheader) {
    case 0x11: // packet is UDP
      outlen = virtservDecodeUDPv6(virtserv, &outframe[54], (outframe_len - 54), &inframe[54], (inframe_len - 54));
      if(outlen > 0) {
        outframe[20] = 0x11; // nextheader: UDP

        utilWriteInt16((unsigned char *)&u, outlen); checksumAdd(&checksum, u); // checksum add payload length
        memcpy(&u, "\x00\x11", 2); checksumAdd(&checksum, u); // checksum add nextheader
        for(i=0; i<=(outlen+2); i=i+2) checksumAdd(&checksum, *((uint16_t *)(&outframe[54+i]))); // checksum add UDP message
        u = checksumGet(&checksum); memcpy(&outframe[60], &u, 2); // write checksum

        return (outlen + 54);
      }
      break;
    case 0x3a: // packet is ICMPv6
      outlen = virtservDecodeICMPv6(virtserv, &outframe[54], (outframe_len - 54), &inframe[54], (inframe_len - 54));
      if(outlen > 0) {
        outframe[20] = 0x3a; // nextheader: ICMPv6

        utilWriteInt16((unsigned char *)&u, outlen); checksumAdd(&checksum, u); // checksum add payload length
        memcpy(&u, "\x00\x3a", 2); checksumAdd(&checksum, u); // checksum add nextheader
        for(i=0; i<=(outlen+2); i=i+2) checksumAdd(&checksum, *((uint16_t *)(&outframe[54+i]))); // checksum add ICMPv6 message
        u = checksumGet(&checksum); memcpy(&outframe[56], &u, 2); // write checksum

        return (outlen + 54);
      }
      break;
  }
  return 0;
}


static int virtservFrame(struct s_virtserv_state *virtserv, unsigned char *outframe, const int outframe_len, const unsigned char *inframe, const int inframe_len) {
  const unsigned char *src_ipv6addr;
  const unsigned char *src_macaddr;
  const unsigned char *dest_ipv6addr;
  const unsigned char *dest_macaddr;
  const unsigned char *req_ipv6addr;
  int outlen;
  if((inframe_len >= 86) && (outframe_len >= 86)) {
    src_ipv6addr = &inframe[22];
    src_macaddr = &inframe[6];
    if(
    ((src_macaddr[0] & 0x01) == 0) && // unicast source MAC
    ((inframe[14] >> 4) == 6) &&    // packet is IPv6
    (src_ipv6addr[0] != 0xff)   // unicast source address
    ) {
      dest_ipv6addr = &inframe[38];
      dest_macaddr = &inframe[0];
      if((dest_macaddr[0] & 0x01) == 0) { // unicast frame
        if((dest_ipv6addr[0] != 0xff) && (virtservCheckMac(virtserv, dest_macaddr)) && (virtservCheckAddress(virtserv, dest_ipv6addr))) {
          outlen = virtservDecodeFrame(virtserv, outframe, outframe_len, inframe, inframe_len);
          if(outlen > 0) {
            memcpy(&outframe[0], src_macaddr, ndp6_MAC_SIZE); // destination MAC
            memcpy(&outframe[6], dest_macaddr, ndp6_MAC_SIZE); // source MAC
            memcpy(&outframe[12], "\x86\xdd\x60\x00\x00\x00\x00\x00", 8); // header
            utilWriteInt16(&outframe[18], (outlen - 54)); // payload length
            outframe[21] = inframe[21]; // TTL
            memcpy(&outframe[22], dest_ipv6addr, ndp6_ADDR_SIZE); // source IPv6 address
            memcpy(&outframe[38], src_ipv6addr, ndp6_ADDR_SIZE); // destination IPv6 address
            return outlen;
          }
        }
      }
      else { // broadcast frame
        if(
        (inframe[20] == 0x3a) &&  // packet is ICMPv6
        (inframe[21] == 0xff) &&  // TTL is 255
        (inframe[54] == 0x87) &&  // packet is neighbor solicitation
        (memcmp(src_macaddr, &inframe[80], virtserv_MAC_SIZE) == 0) // source mac addresses match
        ) {
          req_ipv6addr = &inframe[62];
          if(virtservCheckAddress(virtserv, req_ipv6addr)) {
            return ndp6GenAdvFrame(outframe, outframe_len, req_ipv6addr, src_ipv6addr, virtserv->mac, src_macaddr);
          }
        }
      }
    }
  }
  return 0;
}


static int virtservCreate(struct s_virtserv_state *virtserv) {
  unsigned char mymacaddr[virtserv_MAC_SIZE];
  unsigned char myipv6addr[virtserv_ADDR_SIZE];

  memcpy(mymacaddr, virtserv_MAC, virtserv_MAC_SIZE);
  memcpy(&myipv6addr[0], "\xFE\x80\x00\x00\x00\x00\x00\x00", 8);
  myipv6addr[8] = (mymacaddr[0] | 0x02);
  memcpy(&myipv6addr[9], &mymacaddr[1], 2);
  memcpy(&myipv6addr[11], "\xFF\xFE", 2);
  memcpy(&myipv6addr[13], &mymacaddr[3], 3);

  if(mapCreate(&virtserv->listenaddrs, virtserv_LISTENADDR_COUNT, virtserv_ADDR_SIZE, 1)) {
    mapInit(&virtserv->listenaddrs);
    if(virtservAddAddress(virtserv, myipv6addr)) {
      memcpy(virtserv->mac, mymacaddr, virtserv_MAC_SIZE);
      return 1;
    }
    mapDestroy(&virtserv->listenaddrs);
  }
  return 0;
}


static void virtservDestroy(struct s_virtserv_state *virtserv) {
  mapDestroy(&virtserv->listenaddrs);
}


#endif // F_VIRTSERV_C

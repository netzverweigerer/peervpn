

#ifndef F_PACKET_C
#define F_PACKET_C


#include "crypto.c"
#include "seq.c"
#include "util.c"


#define packet_PEERID_SIZE 4 // peer ID
#define packet_HMAC_SIZE 32 // hmac that includes sequence number, node ID, pl* fields (pllen, pltype, plopt) and payload
#define packet_IV_SIZE 16 // IV
#define packet_SEQ_SIZE seq_SIZE // packet sequence number
#define packet_PLLEN_SIZE 2 // payload length
#define packet_PLTYPE_SIZE 1 // payload type
#define packet_PLOPT_SIZE 1 // payload options
#define packet_CRHDR_SIZE (packet_SEQ_SIZE + packet_PLLEN_SIZE + packet_PLTYPE_SIZE + packet_PLOPT_SIZE)


#define packet_CRHDR_SEQ_START (0)
#define packet_CRHDR_PLLEN_START (packet_CRHDR_SEQ_START + packet_SEQ_SIZE)
#define packet_CRHDR_PLTYPE_START (packet_CRHDR_PLLEN_START + packet_PLLEN_SIZE)
#define packet_CRHDR_PLOPT_START (packet_CRHDR_PLTYPE_START + packet_PLTYPE_SIZE)


#define packet_PLTYPE_USERDATA 0
#define packet_PLTYPE_USERDATA_FRAGMENT 1
#define packet_PLTYPE_AUTH 2
#define packet_PLTYPE_PEERINFO 3
#define packet_PLTYPE_PING 4
#define packet_PLTYPE_PONG 5
#define packet_PLTYPE_RELAY_IN 6
#define packet_PLTYPE_RELAY_OUT 7


#if packet_PEERID_SIZE != 4
#error invalid packet_PEERID_SIZE
#endif
#if packet_SEQ_SIZE != 8
#error invalid packet_SEQ_SIZE
#endif
#if packet_PLLEN_SIZE != 2
#error invalid packet_PLLEN_SIZE
#endif
#if packet_CRHDR_SIZE < (3 * packet_PEERID_SIZE)
#error invalid packet_CRHDR_SIZE
#endif


struct s_packet_data {
  int peerid;
  int64_t seq;
  int pl_length;
  int pl_type;
  int pl_options;
  unsigned char *pl_buf;
  int pl_buf_size;
};


static int packetGetPeerID(const unsigned char *pbuf) {
  int32_t *scr_peerid = ((int32_t *)pbuf);
  int32_t ne_peerid = (scr_peerid[0] ^ (scr_peerid[1] ^ scr_peerid[2]));
  return utilReadInt32((unsigned char *)&ne_peerid);
}


static int packetEncode(unsigned char *pbuf, const int pbuf_size, const struct s_packet_data *data, struct s_crypto *ctx) {
  unsigned char dec_buf[packet_CRHDR_SIZE + data->pl_buf_size];
  int32_t *scr_peerid = ((int32_t *)pbuf);
  int32_t ne_peerid;
  int len;
  
  if(data->pl_length > data->pl_buf_size) { return 0; }
  
  utilWriteInt64(&dec_buf[packet_CRHDR_SEQ_START], data->seq);
  utilWriteInt16(&dec_buf[packet_CRHDR_PLLEN_START], data->pl_length);
  dec_buf[packet_CRHDR_PLTYPE_START] = data->pl_type;
  dec_buf[packet_CRHDR_PLOPT_START] = data->pl_options;
  memcpy(&dec_buf[packet_CRHDR_SIZE], data->pl_buf, data->pl_length);
  
  len = cryptoEnc(ctx, &pbuf[packet_PEERID_SIZE], (pbuf_size - packet_PEERID_SIZE), dec_buf, (packet_CRHDR_SIZE + data->pl_length), packet_HMAC_SIZE, packet_IV_SIZE);
  if(len < (packet_HMAC_SIZE + packet_IV_SIZE + packet_CRHDR_SIZE)) { return 0; }
  
  utilWriteInt32((unsigned char *)&ne_peerid, data->peerid);
  scr_peerid[0] = (ne_peerid ^ (scr_peerid[1] ^ scr_peerid[2]));

  return (packet_PEERID_SIZE + len);
}


static int packetDecode(struct s_packet_data *data, const unsigned char *pbuf, const int pbuf_size, struct s_crypto *ctx, struct s_seq_state *seqstate) {
  unsigned char dec_buf[pbuf_size];
  int len;

  if(pbuf_size < (packet_PEERID_SIZE + packet_HMAC_SIZE + packet_IV_SIZE)) { return 0; }
  len = cryptoDec(ctx, dec_buf, pbuf_size, &pbuf[packet_PEERID_SIZE], (pbuf_size - packet_PEERID_SIZE), packet_HMAC_SIZE, packet_IV_SIZE);
  if(len < packet_CRHDR_SIZE) { return 0; };

  data->peerid = packetGetPeerID(pbuf);
  data->seq = utilReadInt64(&dec_buf[packet_CRHDR_SEQ_START]);
  if(seqstate != NULL) if(!seqVerify(seqstate, data->seq)) { return 0; }
  data->pl_options = dec_buf[packet_CRHDR_PLOPT_START];
  data->pl_type = dec_buf[packet_CRHDR_PLTYPE_START];
  data->pl_length = utilReadInt16(&dec_buf[packet_CRHDR_PLLEN_START]);
  if(!(data->pl_length > 0)) {
    data->pl_length = 0;
    return 0;
  }
  if(len < (packet_CRHDR_SIZE + data->pl_length)) { return 0; }
  if(data->pl_length > data->pl_buf_size) { return 0; }
  memcpy(data->pl_buf, &dec_buf[packet_CRHDR_SIZE], data->pl_length);

  return (data->pl_length);
}


#endif // F_PACKET_C



#ifndef F_CHECKSUM_C
#define F_CHECKSUM_C


struct s_checksum {
  uint32_t checksum;
};


static void checksumZero(struct s_checksum *cs) {
  cs->checksum = 0;
}


static void checksumAdd(struct s_checksum *cs, const uint16_t x) {
  cs->checksum += x;
}


static uint16_t checksumGet(struct s_checksum *cs) {
  uint16_t ret;
  cs->checksum = ((cs->checksum & 0xFFFF) + (cs->checksum >> 16));
  cs->checksum = ((cs->checksum & 0xFFFF) + (cs->checksum >> 16));
  ret = ~(cs->checksum);
  return ret;
}


#endif // F_CHECKSUM_C

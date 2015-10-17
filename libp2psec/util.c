

#ifndef F_UTIL_C
#define F_UTIL_C


#include <string.h>
#include <stdint.h>
#include <time.h>


static char util4BitToHexchar(const int n) {
  if(n < 10) return ('0' + n);
  if(n < 16) return ('A' - 10 + n);
  return ('\0');
}


static int utilByteArrayToHexstring(char *str, const int strlen, const unsigned char *arr, const int arrlen) {
  int l, r, p, i;
  if(!((arrlen * 2) < strlen)) return 0;
  p = 0;
  for(i=0; i<arrlen; i++) {
    l = ((arr[i] & 0xF0) >> 4);
    r = (arr[i] & 0x0F);
    str[p++] = util4BitToHexchar(l);
    str[p++] = util4BitToHexchar(r);
  }
  str[p++] = '\0';
  return 1;
}


static void utilStringFilter(char *strout, const char *strin, const int strlen) {
  int i, c;
  i = 0;
  while(i < strlen) {
    c = strin[i];
    if(c >= 'a' && c <= 'z') {
      strout[i] = (c - 'a' + 'A');
    }
    else if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) {
      strout[i] = c;
    }
    else if(c == '\0') {
      strout[i] = '\0';
      break;
    }
    else {
      strout[i] = '_';
    }
    i++;
  }
}


static int utilIsLittleEndian() {
  const unsigned char s[4] = { 1, 0, 0, 0 };
  const int32_t *n = (int32_t *)&s;
  return (*n == 1);
}


static int16_t utilReadInt16(const unsigned char *buf) {
  int16_t i;
  if(utilIsLittleEndian()) {
    ((unsigned char *)&i)[0] = buf[1];
    ((unsigned char *)&i)[1] = buf[0];
    return i;
  }
  else {
    memcpy((unsigned char *)&i, buf, 2);
    return i;
  }
}


static void utilWriteInt16(unsigned char *buf, int16_t i) {
  if(utilIsLittleEndian()) {
    buf[0] = ((unsigned char *)&i)[1];
    buf[1] = ((unsigned char *)&i)[0];
  }
  else {
    memcpy(buf, (unsigned char *)&i, 2);
  }
}


static int32_t utilReadInt32(const unsigned char *buf) {
  int32_t i;
  if(utilIsLittleEndian()) {
    ((unsigned char *)&i)[0] = buf[3];
    ((unsigned char *)&i)[1] = buf[2];
    ((unsigned char *)&i)[2] = buf[1];
    ((unsigned char *)&i)[3] = buf[0];
    return i;
  }
  else {
    memcpy((unsigned char *)&i, buf, 4);
    return i;
  }
}


static void utilWriteInt32(unsigned char *buf, int32_t i) {
  if(utilIsLittleEndian()) {
    buf[0] = ((unsigned char *)&i)[3];
    buf[1] = ((unsigned char *)&i)[2];
    buf[2] = ((unsigned char *)&i)[1];
    buf[3] = ((unsigned char *)&i)[0];
  }
  else {
    memcpy(buf, (unsigned char *)&i, 4);
  }
}


static int64_t utilReadInt64(const unsigned char *buf) {
  int64_t i;
  if(utilIsLittleEndian()) {
    ((unsigned char *)&i)[0] = buf[7];
    ((unsigned char *)&i)[1] = buf[6];
    ((unsigned char *)&i)[2] = buf[5];
    ((unsigned char *)&i)[3] = buf[4];
    ((unsigned char *)&i)[4] = buf[3];
    ((unsigned char *)&i)[5] = buf[2];
    ((unsigned char *)&i)[6] = buf[1];
    ((unsigned char *)&i)[7] = buf[0];
    return i;
  }
  else {
    memcpy((unsigned char *)&i, buf, 8);
    return i;
  }
}


static void utilWriteInt64(unsigned char *buf, int64_t i) {
  if(utilIsLittleEndian()) {
    buf[0] = ((unsigned char *)&i)[7];
    buf[1] = ((unsigned char *)&i)[6];
    buf[2] = ((unsigned char *)&i)[5];
    buf[3] = ((unsigned char *)&i)[4];
    buf[4] = ((unsigned char *)&i)[3];
    buf[5] = ((unsigned char *)&i)[2];
    buf[6] = ((unsigned char *)&i)[1];
    buf[7] = ((unsigned char *)&i)[0];
  }
  else {
    memcpy(buf, (unsigned char *)&i, 8);
  }
}


static int utilGetClock() {
  return time(NULL);
}


#endif // F_UTIL_C

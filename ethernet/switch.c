

#ifndef F_SWITCH_C
#define F_SWITCH_C


#include "../libp2psec/map.c"
#include "../libp2psec/util.c"


#define switch_FRAME_TYPE_INVALID 0
#define switch_FRAME_TYPE_BROADCAST 1
#define switch_FRAME_TYPE_UNICAST 2
#define switch_FRAME_MINSIZE 14
#define switch_MACADDR_SIZE 6
#define switch_MACMAP_SIZE 8192
#define switch_TIMEOUT 86400


#if switch_FRAME_MINSIZE < switch_MACADDR_SIZE + switch_MACADDR_SIZE
#error switch_FRAME_MINSIZE too small
#endif
#if switch_MACADDR_SIZE != 6
#error switch_MACADDR_SIZE is not 6
#endif


struct s_switch_mactable_entry {
  int portid;
  int portts;
  int ents;
};
struct s_switch_state {
  struct s_map mactable;
};


static int switchFrameOut(struct s_switch_state *switchstate, const unsigned char *frame, const int frame_len, int *portid, int *portts) {
  struct s_switch_mactable_entry *mapentry;
  const unsigned char *macaddr;
  int pos;
  if(frame_len > switch_FRAME_MINSIZE) {
    macaddr = &frame[0];
    pos = mapGetKeyID(&switchstate->mactable, macaddr);
    if(!(pos < 0)) {
      mapentry = (struct s_switch_mactable_entry *)mapGetValueByID(&switchstate->mactable, pos);
      if((utilGetClock() - mapentry->ents) < switch_TIMEOUT) {
        *portid = mapentry->portid;
        *portts = mapentry->portts;
        return switch_FRAME_TYPE_UNICAST;
      }
      else {
        return switch_FRAME_TYPE_BROADCAST;
      }
    }
    else {
      return switch_FRAME_TYPE_BROADCAST;
    }
  }
  else {
    return switch_FRAME_TYPE_INVALID;
  }
}


static void switchFrameIn(struct s_switch_state *switchstate, const unsigned char *frame, const int frame_len, const int portid, const int portts) {
  struct s_switch_mactable_entry mapentry;
  const unsigned char *macaddr;
  if(frame_len > switch_FRAME_MINSIZE) {
    macaddr = &frame[6];
    if((macaddr[0] & 0x01) == 0) { // only insert unicast address into mactable
      mapentry.portid = portid;
      mapentry.portts = portts;
      mapentry.ents = utilGetClock();
      mapSet(&switchstate->mactable, macaddr, &mapentry);
    }
  }
}


static void switchStatus(struct s_switch_state *switchstate, char *report, const int report_len) {
  int tnow = utilGetClock();
  struct s_map *map = &switchstate->mactable;
  struct s_switch_mactable_entry *mapentry;
  int pos = 0;
  int size = mapGetMapSize(map);
  int maxpos = (((size + 2) * (49)) + 1);
  unsigned char infomacaddr[switch_MACADDR_SIZE];
  unsigned char infoportid[4];
  unsigned char infoportts[4];
  unsigned char infoents[4];
  int i = 0;
  int j = 0;
  
  if(maxpos > report_len) { maxpos = report_len; }
  
  memcpy(&report[pos], "MAC                PortID    PortTS    LastFrm ", 47);
  pos = pos + 47;
  report[pos++] = '\n';

  while(i < size && pos < maxpos) {
    if(mapIsValidID(map, i)) {
      mapentry = (struct s_switch_mactable_entry *)mapGetValueByID(&switchstate->mactable, i);
      memcpy(infomacaddr, mapGetKeyByID(map, i), switch_MACADDR_SIZE);
      j = 0;
      while(j < switch_MACADDR_SIZE) {
        utilByteArrayToHexstring(&report[pos + (j * 3)], (4 + 2), &infomacaddr[j], 1);
        report[pos + (j * 3) + 2] = ':';
        j++;
      }
      pos = pos + ((switch_MACADDR_SIZE * 3) - 1);
      report[pos++] = ' ';
      report[pos++] = ' ';
      utilWriteInt32(infoportid, mapentry->portid);
      utilByteArrayToHexstring(&report[pos], ((4 * 2) + 2), infoportid, 4);
      pos = pos + (4 * 2);
      report[pos++] = ' ';
      report[pos++] = ' ';
      utilWriteInt32(infoportts, mapentry->portts);
      utilByteArrayToHexstring(&report[pos], ((4 * 2) + 2), infoportts, 4);
      pos = pos + (4 * 2);
      report[pos++] = ' ';
      report[pos++] = ' ';
      utilWriteInt32(infoents, (tnow - mapentry->ents));
      utilByteArrayToHexstring(&report[pos], ((4 * 2) + 2), infoents, 4);
      pos = pos + (4 * 2);
      report[pos++] = '\n';
    }
    i++;
  }
  report[pos++] = '\0';
}


static int switchCreate(struct s_switch_state *switchstate) {
  if(mapCreate(&switchstate->mactable, switch_MACMAP_SIZE, switch_MACADDR_SIZE, sizeof(struct s_switch_mactable_entry))) {
    mapEnableReplaceOld(&switchstate->mactable);
    mapInit(&switchstate->mactable);
    return 1;
  }
  return 0;
}


static void switchDestroy(struct s_switch_state *switchstate) {
  mapDestroy(&switchstate->mactable);
}


#endif // F_SWITCH_C

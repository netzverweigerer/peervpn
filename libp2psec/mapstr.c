

#ifndef F_MAPSTR_C
#define F_MAPSTR_C


#include "map.c"


#define mapStrNPrepKey(map, str, len) \
  int key_size = map->key_size; \
  char key[key_size]; \
  int x; \
  if((key_size - 1) < len) { \
    x = (key_size - 1); \
  } \
  else { \
    x = len; \
  } \
  memset(key, 0, key_size); \
  memcpy(key, str, x);


#define mapStrAdd(map, str, value) mapStrNAdd(map, str, strlen(str), value)
static int mapStrNAdd(struct s_map *map, const char *str, const int len, const void *value) {
  mapStrNPrepKey(map, str, len);
  return mapAdd(map, key, value);
}


#define mapStrGet(map, str) mapStrNGet(map, str, strlen(str))
static void *mapStrNGet(struct s_map *map, const char *str, const int len) {
  mapStrNPrepKey(map, str, len);
  return mapGet(map, key);
}


#define mapStrGetN(map, str) mapStrNGetN(map, str, strlen(str))
static void *mapStrNGetN(struct s_map *map, const char *str, const int len) {
  mapStrNPrepKey(map, str, len);
  return mapGetN(map, key, x);
}


#define mapStrRemove(map, str) mapStrNRemove(map, str, strlen(str))
static int mapStrNRemove(struct s_map *map, const char *str, const int len) {
  mapStrNPrepKey(map, str, len);
  return mapRemove(map, key);
}


#define mapStrSet(map, str, value) mapStrNSet(map, str, strlen(str), value)
static int mapStrNSet(struct s_map *map, const char *str, const int len, const void *value) {
  mapStrNPrepKey(map, str, len);
  return mapSet(map, key, value);
}


#endif // F_MAPSTR_C

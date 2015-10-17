

#ifndef F_MAP_C
#define F_MAP_C


#include "idsp.c"
#include <stdlib.h>
#include <string.h>


struct s_map {
  struct s_idsp idsp;    
  unsigned char *key;
  unsigned char *value;
  int *left;
  int *right;
  int maxnode;
  int rootid;
  int key_size;
  int value_size;
  int replace_old;
};



static void mapEnableReplaceOld(struct s_map *map) {
  map->replace_old = 1;
}


static void mapDisableReplaceOld(struct s_map *map) {
  map->replace_old = 0;
}


static int mapGetKeySize(struct s_map *map) {
  return map->key_size;
}


static int mapGetValueSize(struct s_map *map) {
  return map->value_size;
}


static int mapIsValidID(struct s_map *map, const int id) {
  return idspIsValid(&map->idsp, id);
}


static void *mapGetKeyByID(struct s_map *map, const int id) {
  if((!(id < 0)) && (map != NULL)) {
    return &map->key[id * mapGetKeySize(map)];
  }
  else {
    return NULL;
  }
}


static int mapComparePrefixExt(struct s_map *map, const int id, const void *prefix, const int prefixlen) {
  void *skey = mapGetKeyByID(map, id);
  return memcmp(skey, prefix, prefixlen);
}


static int mapCompareKeysExt(struct s_map *map, const int id, const void *key) {
  return mapComparePrefixExt(map, id, key, mapGetKeySize(map));
}


static int mapSplayPrefix(struct s_map *map, const void *prefix, const int prefixlen) {
  int ret = 0;
  int cur_nodeid = map->rootid;
  int maxnode = map->maxnode;
  int l = maxnode;
  int r = maxnode;
  int cur_cmp;
  int y;
  
  if(cur_nodeid < 0) return ret;
  
  map->left[maxnode] = -1;
  map->right[maxnode] = -1;
  
  for(;;) {
    cur_cmp = mapComparePrefixExt(map, cur_nodeid, prefix, prefixlen);
    if(cur_cmp > 0) {
      y = map->left[cur_nodeid];
      if(y < 0) break;
      if(mapComparePrefixExt(map, y, prefix, prefixlen) > 0) {
        map->left[cur_nodeid] = map->right[y];
        map->right[y] = cur_nodeid;
        cur_nodeid = y;
        if(map->left[cur_nodeid] < 0) break;
      }
      map->left[r] = cur_nodeid;
      r = cur_nodeid;
      cur_nodeid = map->left[cur_nodeid];
    }
    else if(cur_cmp < 0) {
      y = map->right[cur_nodeid];
      if(y < 0) break;
      if(mapComparePrefixExt(map, y, prefix, prefixlen) < 0) {
        map->right[cur_nodeid] = map->left[y];
        map->left[y] = cur_nodeid;
        cur_nodeid = y;
        if(map->right[cur_nodeid] < 0) break;
      }
      map->right[l] = cur_nodeid;
      l = cur_nodeid;
      cur_nodeid = map->right[cur_nodeid];
    }
    else {
      ret = 1;
      break;
    }
  }
  
  map->right[l] = map->left[cur_nodeid];
  map->left[r] = map->right[cur_nodeid];
  map->left[cur_nodeid] = map->right[maxnode];
  map->right[cur_nodeid] = map->left[maxnode];
  map->rootid = cur_nodeid;
  
  return ret;
}


static int mapSplayKey(struct s_map *map, const void *key) {
  return mapSplayPrefix(map, key, mapGetKeySize(map));
}


static void mapInit(struct s_map *map) {
  idspReset(&map->idsp);
  map->maxnode = 0;
  map->rootid = -1;
}


static int mapGetMapSize(struct s_map *map) {
  return idspSize(&map->idsp);
}


static int mapGetKeyCount(struct s_map *map) {
  return idspUsedCount(&map->idsp);
}


static int mapGetNextKeyID(struct s_map *map) {
  return idspNext(&map->idsp);
}


static int mapGetNextKeyIDN(struct s_map *map, const int start) {
  return idspNextN(&map->idsp, start);
}


static int mapGetPrefixID(struct s_map *map, const void *prefix, const int prefixlen) {
  if(mapSplayPrefix(map, prefix, prefixlen)) {
    return map->rootid;
  }
  else {
    return -1;
  }
}


static int mapGetKeyID(struct s_map *map, const void *key) {
  return mapGetPrefixID(map, key, mapGetKeySize(map));
}


static int mapGetOldKeyID(struct s_map *map) {
  int l;
  int r;
  int cur_nodeid = map->rootid;
  if(!(cur_nodeid < 0)) {
    if(!((l = (map->left[cur_nodeid])) < 0)) {
      cur_nodeid = l;
      while(!(((r = (map->right[cur_nodeid])) < 0) && ((l = (map->left[cur_nodeid])) < 0))) {
        if(r < 0) {
          cur_nodeid = l;
        }
        else {
          cur_nodeid = r;
        }
      }
      return cur_nodeid;
    }
    else {
      if(!((r = (map->right[cur_nodeid])) < 0)) {
        cur_nodeid = r;
        while(!(((l = (map->left[cur_nodeid])) < 0) && ((r = (map->right[cur_nodeid])) < 0))) {
          if(l < 0) {
            cur_nodeid = r;
          }
          else {
            cur_nodeid = l;
          }
        }
        return cur_nodeid;
      }
      else {
        return cur_nodeid;
      }
    }
  }
  else {
    return -1;
  }
}


static void *mapGetValueByID(struct s_map *map, const int id) {
  if((!(id < 0)) && (map != NULL)) {
    return &map->value[id * mapGetValueSize(map)];
  }
  else {
    return NULL;
  }
}


static void mapSetValueByID(struct s_map *map, const int id, const void *value) {
  unsigned char *tptr = mapGetValueByID(map, id);
  if((!(id < 0)) && (map != NULL)) {
    if(value == NULL) {
      memset(tptr, 0, mapGetValueSize(map));
    }
    else {
      memcpy(tptr, value, mapGetValueSize(map));
    }
  }
}


static int mapRemoveReturnID(struct s_map *map, const void *key) {
  int x = mapSplayKey(map, key);
  int rootid = map->rootid;
  
  if(!x) return -1;

  idspDelete(&map->idsp, rootid);

  if(idspUsedCount(&map->idsp) > 0) {
    if(map->left[rootid] < 0) {
      map->rootid = map->right[rootid];
    }
    else {
      x = map->right[rootid];
      map->rootid = map->left[rootid];
      mapSplayKey(map, key);
      map->right[map->rootid] = x;
    }
  }
  else {
    map->rootid = -1;
  }

  return rootid;
}


static int mapRemove(struct s_map *map, const void *key) {
  if(mapRemoveReturnID(map, key) < 0) {
    return 0;
  }
  else {
    return 1;
  }
}


static int mapAddReturnID(struct s_map *map, const void *key, const void *value) {
  int x;
  int rootid;
  int nodeid;
  
  if(!(idspUsedCount(&map->idsp) < idspSize(&map->idsp))) {
    if(map->replace_old > 0) {
      x = mapGetOldKeyID(map);
      if(x < 0) {
        return -1;
      }
      else {
        if(!(mapRemove(map, mapGetKeyByID(map, x)))) {
          return -1;
        }
      }
    }
    else {
      return -1;
    }
  }

  x = mapSplayKey(map, key);
  rootid = map->rootid;

  if(x) return -1;

  nodeid = idspNew(&map->idsp);
  if((nodeid + 1) > map->maxnode) map->maxnode = (nodeid + 1);

  if(rootid < 0) {
    map->left[nodeid] = -1;
    map->right[nodeid] = -1;
  }
  else {
    if(mapCompareKeysExt(map, rootid, key) > 0) {
      map->left[nodeid] = map->left[rootid];
      map->right[nodeid] = rootid;
      map->left[rootid] = -1;
    }
    else {
      map->right[nodeid] = map->right[rootid];
      map->left[nodeid] = rootid;
      map->right[rootid] = -1;
    }
  }

  memcpy(mapGetKeyByID(map, nodeid), key, mapGetKeySize(map));
  mapSetValueByID(map, nodeid, value);

  map->rootid = nodeid;
  
  return nodeid;
}


static int mapAdd(struct s_map *map, const void *key, const void *value) {
  if(mapAddReturnID(map, key, value) < 0) {
    return 0;
  }
  else {
    return 1;
  }
}


static int mapSetReturnID(struct s_map *map, const void *key, const void *value) {
  int ret = mapAddReturnID(map, key, value);
  if(ret < 0) {
    ret = mapGetKeyID(map, key);
    if(ret < 0) {
      return -1;
    }
    else {
      mapSetValueByID(map, ret, value);
      return ret;
    }
  }
  else {
    return ret;
  }
}


static int mapSet(struct s_map *map, const void *key, const void *value) {
  if(mapSetReturnID(map, key, value) < 0) {
    return 0;
  }
  else {
    return 1;
  }
}


static void *mapGetN(struct s_map *map, const void *prefix, const int prefixlen) {
  int id;
  id = mapGetPrefixID(map, prefix, prefixlen);
  if(id < 0) return NULL;
  return mapGetValueByID(map, id);
}


static void *mapGet(struct s_map *map, const void *key) {
  return mapGetN(map, key, mapGetKeySize(map));
}


static int mapMemSize(const int map_size, const int key_size, const int value_size) {
  const int align_boundary = idsp_ALIGN_BOUNDARY;
  int memsize;
  memsize = 0;
  memsize = memsize + ((((sizeof(struct s_map)) + (align_boundary - 1)) / align_boundary) * align_boundary);
  memsize = memsize + ((((map_size * key_size) + (align_boundary - 1)) / align_boundary) * align_boundary);
  memsize = memsize + ((((map_size * value_size) + (align_boundary - 1)) / align_boundary) * align_boundary);
  memsize = memsize + (((((map_size+1) * sizeof(int)) + (align_boundary - 1)) / align_boundary) * align_boundary);
  memsize = memsize + (((((map_size+1) * sizeof(int)) + (align_boundary - 1)) / align_boundary) * align_boundary);
  memsize = memsize + (((idspMemSize(map_size) + (align_boundary - 1)) / align_boundary) * align_boundary);
  return memsize;
}


static int mapMemInit(struct s_map *map, const int mem_size, const int map_size, const int key_size, const int value_size) {
  const int align_boundary = idsp_ALIGN_BOUNDARY;
  const int keymem_offset = ((((sizeof(struct s_map)) + (align_boundary - 1)) / align_boundary) * align_boundary);
  const int valuemem_offset = keymem_offset + ((((map_size * key_size) + (align_boundary - 1)) / align_boundary) * align_boundary);
  const int leftmem_offset = valuemem_offset + ((((map_size * value_size) + (align_boundary - 1)) / align_boundary) * align_boundary);
  const int rightmem_offset = leftmem_offset + (((((map_size+1) * sizeof(int)) + (align_boundary - 1)) / align_boundary) * align_boundary);
  const int idsp_offset = rightmem_offset + (((((map_size+1) * sizeof(int)) + (align_boundary - 1)) / align_boundary) * align_boundary);
  const int min_mem_size = idsp_offset + (((idspMemSize(map_size) + (align_boundary - 1)) / align_boundary) * align_boundary);
  struct s_idsp *idsp_new;
  unsigned char *map_mem;
  
  if(!((map_size > 0) && (key_size > 0) && (value_size > 0))) return 0;

  map_mem = (unsigned char *)map;
  if(min_mem_size == mapMemSize(map_size, key_size, value_size) && mem_size >= min_mem_size) {
    map->key_size = key_size;
    map->value_size = value_size;
    map->key = (unsigned char *)(&map_mem[keymem_offset]);
    map->value = (unsigned char *)(&map_mem[valuemem_offset]);
    map->left = (int *)(&map_mem[leftmem_offset]);
    map->right = (int *)(&map_mem[rightmem_offset]);
    idsp_new = (struct s_idsp *)(&map_mem[idsp_offset]);
    if(idspMemInit(idsp_new, (min_mem_size - idsp_offset), map_size)) {
      map->idsp = *idsp_new;
      mapInit(map);
      mapDisableReplaceOld(map);
      return 1;
    }
  }

  return 0;
}


static int mapCreate(struct s_map *map, const int map_size, const int key_size, const int value_size) {
  if(!((map_size > 0) && (key_size > 0) && (value_size > 0))) return 0;

  void *keymem = NULL;
  void *valuemem = NULL;
  int *leftmem = NULL;
  int *rightmem = NULL;
  if((keymem = malloc(map_size * key_size)) == NULL) { return 0; }
  if((valuemem = malloc(map_size * value_size)) == NULL) { free(keymem); return 0; }
  if((leftmem = malloc((map_size+1) * sizeof(int))) == NULL) { free(valuemem); free(keymem); return 0; }
  if((rightmem = malloc((map_size+1) * sizeof(int))) == NULL) { free(leftmem); free(valuemem); free(keymem); return 0; }
  if(!idspCreate(&map->idsp, map_size)) { free(rightmem); free(leftmem); free(valuemem); free(keymem); return 0; }
  map->key_size = key_size;
  map->value_size = value_size;
  map->key = keymem;
  map->value = valuemem;
  map->left = leftmem;
  map->right = rightmem;
  mapInit(map);
  mapDisableReplaceOld(map);

  return 1;
}


static int mapDestroy(struct s_map *map) {
  if(!((map != NULL) && (map->key != NULL) && (map->value != NULL) && (map->left != NULL) && (map->right != NULL))) return 0;
  idspDestroy(&map->idsp);
  free(map->right);
  free(map->left);
  free(map->value);
  free(map->key);
  map->right = NULL;
  map->left = NULL;
  map->value = NULL;
  map->key = NULL;

  return 1;
}


#endif // F_MAP_C

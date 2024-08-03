#include "salmagundi.h"
#include "rapidhash.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef HM_DEBUG
#include <stdio.h>
#endif

hm_hash_t hm_hash_byte(void const* k, hm_sz_t k_sz) {
  (void)k_sz;
  return *(uint8_t*)k;
}

int8_t hm_cmp_byte(void const* a, hm_sz_t a_sz, void const* b, hm_sz_t b_sz) {
  (void)a_sz;
  (void)b_sz;
  return *(uint8_t*)a - *(uint8_t*)b;
}

/*  The DJB-1 hash
    Daniel Bernstein's first implementation of a simple hash function
    Ref http://www.cse.yorku.ca/~oz/hash.html */
hm_hash_t hm_hash_djb1(void const* k, hm_sz_t k_sz) {
  uint8_t const* bytes = k;
  hm_sz_t hash = 5381;
  for (hm_sz_t i = 0; i < k_sz; i++) { hash = ((hash << 5) + hash) + bytes[i]; }
  return hash;
}

hm_hash_t hm_hash_rapidhash(void const* k, hm_sz_t k_sz) {
  return rapidhash(k, k_sz);
}

int8_t hm_cmp_str(void const* a, hm_sz_t a_sz, void const* b, hm_sz_t b_sz) {
  if (a_sz != b_sz)
    return 1;
  return memcmp(a, b, a_sz);
}

hm_t* hm_open(hm_hash_func hash, hm_cmp_func cmp) {
  hm_t* map = calloc(sizeof(hm_t), 1);
  map->items = calloc(HM_INITIAL_CAP, sizeof(hm_item_t));
  map->cap = HM_INITIAL_CAP;
  map->hash = hash;
  map->cmp = cmp;
  return map;
}

void hm_grow(hm_t* map) {
#ifdef HM_DEBUG
  printf("Growing map of sz=%u from cap=%u to cap=%u\n", map->sz, map->cap, map->cap * 2);
  map->n_grow++;
#endif
  hm_sz_t new_capacity = map->cap * 2;
  hm_item_t* new_entries = calloc(new_capacity, sizeof(hm_item_t));
  for (hm_sz_t i = 0; i < map->cap; i++) {
    if (map->items[i].k != NULL) {
      hm_item_t item = map->items[i];
      hm_sz_t idx = map->hash(item.k, item.k_sz) % new_capacity;
      while (new_entries[idx].k != NULL) {
        idx = (idx + 1) % new_capacity;
      }
      new_entries[idx] = map->items[i];
    }
  }
  free(map->items);
  map->items = new_entries;
  map->cap = new_capacity;
#ifdef HM_DEBUG
  printf("Map grown, cap=%u, sz=%u\n", map->cap, map->sz);
#endif
}

/*  A linear collision resolution strategy
    Ref https://en.wikipedia.org/wiki/Linear_probing */
hm_sz_t hm_put(hm_t* map, void* k, hm_sz_t k_sz, void* v, hm_sz_t v_sz) {
  if (map->sz >= map->cap * 0.75) {
    hm_grow(map);
  }
  hm_sz_t idx = map->hash(k, k_sz) % map->cap;
  hm_sz_t is_overwrite = 0;
  while(1) {
    if (map->items[idx].k == NULL) {
      // No key at this index. We can put one here.
      break;
    } else if (map->cmp(map->items[idx].k, map->items[idx].k_sz, k, k_sz) == 0) {
      // Key already exists (hash and memory match). We can update the value.
      is_overwrite = 1;
      break;
    } else {
      // Bump the index and try again.
      idx = (idx + 1) % map->cap;
#ifdef HM_DEBUG
      map->n_collision++;
#endif
    }
  }
  memset(&map->items[idx], 0, sizeof(hm_item_t));
  void* k_mem = malloc(k_sz);
  if (k_mem == NULL) {
    return -1;
  }
  void* v_mem = NULL;
  if (is_overwrite) {
    v_mem = realloc(map->items[idx].v, v_sz);
  } else {
    v_mem = malloc(v_sz);
  }
  if (v_mem == NULL) {
    free(k_mem);
    return -1;
  }
  memcpy(k_mem, k, k_sz);
  memcpy(v_mem, v, v_sz);
  map->items[idx].k = k_mem;
  map->items[idx].k_sz = k_sz;
  map->items[idx].v = v_mem;
  map->items[idx].v_sz = v_sz;
  map->sz += !is_overwrite;
  return idx;
}

hm_item_t hm_get(hm_t* map, void* k, hm_sz_t k_sz) {
  hm_sz_t idx = map->hash(k, k_sz) % map->cap;
  while (map->items[idx].k != NULL) {
    if (map->cmp(map->items[idx].k, map->items[idx].k_sz, k, k_sz) == 0) {
      return map->items[idx];
    }
    idx = (idx + 1) % map->cap;
#ifdef HM_DEBUG
    map->n_probe++;
#endif
  }
  hm_item_t none;
  memset(&none, 0, sizeof(hm_item_t));
  return none;
}

int8_t hm_del(hm_t* map, void* k, hm_sz_t k_sz) {
  hm_sz_t idx = map->hash(k, k_sz) % map->cap;
  while (map->items[idx].k != NULL) {
    if (map->cmp(map->items[idx].k, map->items[idx].k_sz, k, k_sz) == 0) {
      free(map->items[idx].k);
      free(map->items[idx].v);
      memset(&map->items[idx], 0, sizeof(hm_item_t));
      map->sz--;
      return -1;
    }
    idx = (idx + 1) % map->cap;
#ifdef HM_DEBUG
    map->n_probe++;
#endif
  }
  return 0;
}

void hm_close(hm_t* map) {
  for (hm_sz_t i = 0; i < map->cap; i++) {
    if (map->items[i].k != NULL) {
      free(map->items[i].k);
      free(map->items[i].v);
      memset(&map->items[i], 0, sizeof(hm_item_t));
    }
  }
  free(map->items);
  map->items = NULL;
  free(map);
  map = NULL;
}
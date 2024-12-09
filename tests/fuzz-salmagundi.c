#include "salmagundi.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int get_from_map_and_check_eq(hm_t* map, char* k, uint64_t k_sz, char* v, uint64_t v_sz) {
  hm_item_t r = hm_get(map, k, k_sz);
  assert(r.v_sz == v_sz && memcmp(r.v, v, v_sz) == 0);
  assert(r.k_sz == k_sz && memcmp(r.k, k, k_sz) == 0);
  return 0;
}

void checked_put(hm_t* map, char* k, uint64_t k_sz, char* v, uint64_t v_sz) {
  hm_put(map, k, k_sz, v, v_sz);
  get_from_map_and_check_eq(map, k, k_sz, v, v_sz);
}

void checked_del(hm_t* map, char* k, uint64_t k_sz) {
  hm_del(map, k, k_sz);
  assert(hm_get(map, k, k_sz).k == NULL);
}

void checked_grow(hm_t* map) {
  hm_item_t* old_items = map->items;
  hm_sz_t old_cap = map->cap;
  hm_sz_t old_sz = map->sz;
  hm_hash_func old_hash = map->hash;
  hm_cmp_func old_cmp = map->cmp;
  hm_grow(map);
  assert(map->items != old_items);
  assert(map->cap > old_cap);
  assert(map->sz == old_sz);
  assert(map->hash == old_hash);
  assert(map->cmp == old_cmp);
}

int LLVMFuzzerTestOneInput(uint8_t* data, size_t data_sz) {
  hm_t* map = hm_open(hm_hash_rapidhash, hm_cmp_str);
  char* k = (char*)data;
  char* v = (char*)data;
  checked_put(map, k, data_sz, v, data_sz);
  checked_grow(map);
  checked_del(map, k, data_sz);
  checked_del(map, k, data_sz);
  checked_put(map, k, data_sz, v, data_sz);
  checked_put(map, k, data_sz, v, data_sz);
  hm_close(map);
  return 0;
}

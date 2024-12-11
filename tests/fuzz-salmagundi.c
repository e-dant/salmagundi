#include "salmagundi.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int get_from_map_and_check_eq(hm_t* map, uint8_t* k, uint64_t k_sz, uint8_t* v, uint64_t v_sz) {
  hm_item_t r = hm_get(map, k, k_sz);
  assert(r.v_sz == v_sz && memcmp(r.v, v, v_sz) == 0);
  assert(r.k_sz == k_sz && memcmp(r.k, k, k_sz) == 0);
  return 0;
}

void checked_put(hm_t* map, uint8_t* k, uint64_t k_sz, uint8_t* v, uint64_t v_sz) {
  hm_put(map, k, k_sz, v, v_sz);
  get_from_map_and_check_eq(map, k, k_sz, v, v_sz);
}

void checked_del(hm_t* map, uint8_t* k, uint64_t k_sz) {
  hm_del(map, k, k_sz);
  assert(hm_get(map, k, k_sz).k == NULL);
}

void checked_grow(hm_t* map) {
  hm_item_t* old_items = map->items;
  hm_sz_t old_cap = map->cap;
  hm_sz_t old_sz = map->sz;
  hm_hash_func old_hash = map->hash;
  hm_cmp_func old_cmp = map->cmp;
  assert(hm_grow(map) == 0);
  assert(map->items != old_items);
  assert(map->cap > old_cap);
  assert(map->sz == old_sz);
  assert(map->hash == old_hash);
  assert(map->cmp == old_cmp);
}

int LLVMFuzzerTestOneInput(uint8_t* data, size_t data_sz) {
  static const size_t op_lim = 128;
  static const size_t op_section_sz = 32;
  static const size_t op_korv_sz = op_section_sz / 2;
  if (data_sz < op_lim + op_section_sz) {
    return -1; // Do not add this to the corpus; Not meaningful.
  }
  hm_hash_func hash_func = data[0] % 2 == 0 ? hm_hash_rapidhash : hm_hash_djb1;
  hm_t* map = hm_open(hash_func, hm_cmp_str);
  data_sz -= op_section_sz;
  uint8_t* k = (uint8_t*)data + op_section_sz;
  uint8_t* v = (uint8_t*)data + op_section_sz;
  checked_put(map, k, data_sz, v, data_sz);
  checked_grow(map);
  checked_del(map, k, data_sz);
  checked_del(map, k, data_sz);
  checked_put(map, k, data_sz, v, data_sz);
  checked_put(map, k, data_sz, v, data_sz);
  // Now let's get weird. This opens us up the paths we can go down.
  // Fuzzer-driven behavior kind of thing.
  for (size_t i = 0; i < op_lim; i++) {
    uint8_t some_k[op_korv_sz];
    uint8_t some_v[op_korv_sz];
    memcpy(some_k, data + i, op_korv_sz);
    memcpy(some_v, data + i + op_korv_sz, op_korv_sz);
    switch (data[i] % 3) {
      case 0:
        checked_put(map, k, data_sz, v, data_sz);
        checked_put(map, some_k, op_korv_sz, some_v, op_korv_sz);
        checked_put(map, k, data_sz, v, data_sz);
        checked_del(map, k, data_sz);
        checked_put(map, k, data_sz, v, data_sz);
        checked_del(map, some_k, op_korv_sz);
        checked_del(map, k, data_sz);
        checked_put(map, k, data_sz, v, data_sz);
        break;
      case 1:
        checked_del(map, k, data_sz);
        checked_del(map, some_k, op_korv_sz);
        break;
      case 2:
        checked_put(map, some_k, op_korv_sz, some_v, op_korv_sz);
        checked_del(map, some_k, op_korv_sz);
        break;
    }
  }
  hm_close(map);
  return 0;
}

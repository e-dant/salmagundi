#include "salmagundi.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#ifndef HM_DEBUG
#error "HM_DEBUG must be defined for map introspection"
#endif

void hm_print_item(hm_item_t item) {
  printf("k=@%p, k_sz=%u, v=@%p, v_sz=%u\n", item.k, item.k_sz, item.v, item.v_sz);
}

void hm_print_hm_detail(hm_t* map) {
  printf("cap=%u, sz=%u, n_collision=%u, n_probe=%u, n_grow=%u\n", map->cap, map->sz, map->n_collision, map->n_probe, map->n_grow);
}

void test_hm_lifetime(void) {
  hm_t* map = hm_open(hm_hash_djb1, hm_cmp_str);
  assert(map->cap == HM_INITIAL_CAP);
  assert(map->sz == 0);
  assert(map->hash == hm_hash_djb1);
  assert(map->cmp == hm_cmp_str);
  hm_grow(map);
  assert(map->cap == HM_INITIAL_CAP * 2);
  hm_close(map);
}

void test_hm_of_int_int(void) {
  hm_t* map = hm_open(hm_hash_byte, hm_cmp_byte);
  int k = 1;
  int v = 2;
  hm_put(map, &k, sizeof(k), &v, sizeof(v));
  assert(map->sz == 1);
  assert(*(int*)map->items[hm_hash_byte(&k, sizeof(k))].k == k);
  assert(*(int*)map->items[hm_hash_byte(&k, sizeof(k))].v == v);
  hm_item_t result = hm_get(map, &k, sizeof(k));
  assert(*(int*)result.k == k);
  hm_close(map);
}

void test_hm_of_str_str(void) {
  hm_t* map = hm_open(hm_hash_djb1, hm_cmp_str);
  char* k = "k";
  uint64_t k_sz = strlen(k);
  char* v = "v";
  uint64_t v_sz = strlen(v);
  uint64_t idx = hm_put(map, k, k_sz, v, v_sz);
  assert(idx < map->cap);
  assert(idx >= 0);
  assert(idx == hm_hash_djb1(k, k_sz) % map->cap);
  assert(map->sz == 1);
  assert(memcmp(map->items[idx].k, k, k_sz) == 0);
  assert(memcmp(map->items[idx].v, v, v_sz) == 0);
  hm_item_t result = hm_get(map, k, k_sz);
  assert(result.k_sz == k_sz);
  assert(result.v_sz == v_sz);
  assert(memcmp(result.k, k, k_sz) == 0);
  assert(memcmp(result.v, v, v_sz) == 0);
  hm_close(map);
}

#define SLOWRAND 0

void* rand_open(void) {
#if SLOWRAND
  FILE* r = fopen("/dev/urandom", "r");
  if (r) {
    return r;
  }
#endif
  srand(0);
  return NULL;
}

size_t rand_read(void* r, void* buf, size_t sz) {
#if SLOWRAND
  if (r) {
    return (size_t)fread(buf, 1, sz, r);
  }
#endif
  (void)r;
  for (int i = 0; i < sz; i++) { ((char*)buf)[i] = (rand() % 255) + 1; }
  return sz;
}

void rand_close(void* r) {
#if SLOWRAND
  if (r) {
    fclose(r);
  }
#endif
  (void)r;
}

void dump_map_str_str_items(hm_t* map) {
  for (hm_sz_t i = 0; i < map->cap; i++) {
    if (map->items[i].k != NULL) {
      char* k = map->items[i].k;
      char* v = map->items[i].v;
      hm_sz_t hash = map->hash(k, map->items[i].k_sz);
      char v_owned[map->items[i].v_sz + 1];
      memset(v_owned, 0, sizeof(v_owned));
      memcpy(v_owned, v, map->items[i].v_sz);
      v_owned[map->items[i].v_sz] = 0;
      fprintf(stderr, "map->items[%u] = { .k (hash) = %u, .v = %s }\n", i, hash, v_owned);
    }
  }
}

static uint8_t const LOW_COLLISION_RATE = 0;
static uint8_t const MEDIUM_COLLISION_RATE = 1;
static uint8_t const HIGH_COLLISION_RATE = 2;

void test_hm_torture(int8_t collision_rate, hm_hash_func hash_func) {
  int k_sz = collision_rate == LOW_COLLISION_RATE          ? 4096
           : collision_rate == MEDIUM_COLLISION_RATE       ? 256
                                                           : 2;
  int v_sz = collision_rate == LOW_COLLISION_RATE          ? 4096
           : collision_rate == MEDIUM_COLLISION_RATE       ? 256
                                                           : 2;
  int torture_n = collision_rate == LOW_COLLISION_RATE     ? 10000
                : collision_rate == MEDIUM_COLLISION_RATE  ? 10000
                                                           : 10000;
  fprintf(stderr, "k_sz=%d, v_sz=%d, torture_n=%d\n", k_sz, v_sz, torture_n);
  int print_at = -1; // Or torture_n / 10 for verbose output
  hm_t* map = hm_open(hash_func, hm_cmp_str);
  void* r = rand_open();
  for (int i = 0; i < torture_n; i++) {
    char k[k_sz + 1];
    char v[v_sz + 1];
    memset(k, 0, sizeof(k));
    memset(v, 0, sizeof(v));
    size_t k_read = rand_read(r, k, k_sz);
    size_t v_read = rand_read(r, v, v_sz);
    assert(k_read == k_sz);
    assert(v_read == v_sz);
    k[k_sz] = 0;
    v[v_sz] = 0;
    hm_sz_t itm_idx = hm_put(map, k, k_sz, v, v_sz);
    assert(itm_idx < map->cap);
    assert(memcmp(map->items[itm_idx].k, k, k_sz) == 0);
    assert(memcmp(map->items[itm_idx].v, v, v_sz) == 0);
    hm_item_t itm = hm_get(map, k, k_sz);
    assert(itm.k_sz == k_sz);
    assert(itm.v_sz == v_sz);
    assert(memcmp(itm.k, k, k_sz) == 0);
    assert(memcmp(itm.v, v, v_sz) == 0);
    if (i % print_at == print_at - 1) {
      fprintf(stderr, "size = %u (%uMB), capacity = %u\n", map->sz, map->sz * (k_sz + v_sz) / 1024 / 1024, map->cap);
    }
  }
  hm_print_hm_detail(map);
  rand_close(r);
  hm_close(map);
}

void test_hm_torture_low_collision_rate(void) {
  fprintf(stderr, "--------\n");
  fprintf(stderr, "test_hm_torture_low_collision_rate\n");
  test_hm_torture(LOW_COLLISION_RATE, hm_hash_rapidhash);
}

void test_hm_torture_medium_collision_rate(void) {
  fprintf(stderr, "--------\n");
  fprintf(stderr, "test_hm_torture_medium_collision_rate\n");
  test_hm_torture(MEDIUM_COLLISION_RATE, hm_hash_rapidhash);
}

void test_hm_torture_high_collision_rate(void) {
  fprintf(stderr, "--------\n");
  fprintf(stderr, "test_hm_torture_high_collision_rate\n");
  test_hm_torture(HIGH_COLLISION_RATE, hm_hash_rapidhash);
}

hm_hash_t hash_always_collide_func(void const* k, hm_sz_t k_sz) {
  (void)k;
  (void)k_sz;
  return 0;
}

// For a map with three colliding keys, k1..3,
// if the entry at k2 is deleted, the entry at k3 should be accessible.
void test_hm_deleted_colliding_keys() {
  hm_t* map = hm_open(hash_always_collide_func, hm_cmp_str);
  char* k1 = "k1";
  char* k2 = "k2";
  char* k3 = "k3";
  char* v = "v";
  hm_put(map, k1, strlen(k1), v, strlen(v));
  hm_put(map, k2, strlen(k2), v, strlen(v));
  hm_put(map, k3, strlen(k3), v, strlen(v));
  assert(map->sz == 3);
  assert(hm_get(map, k1, strlen(k1)).k != NULL);
  assert(hm_get(map, k2, strlen(k2)).k != NULL);
  assert(hm_get(map, k3, strlen(k3)).k != NULL);
  hm_del(map, k2, strlen(k2));
  assert(map->sz == 2);
  assert(hm_get(map, k1, strlen(k1)).k != NULL);
  assert(hm_get(map, k2, strlen(k2)).k == NULL);
  assert(hm_get(map, k3, strlen(k3)).k != NULL);
  hm_print_hm_detail(map);
  hm_close(map);
}

int main(int argc, char** argv) {
  if (argc != 1) {
    printf("%s takes no arguments.\n", argv[0]);
    return 1;
  }
  test_hm_deleted_colliding_keys();
  test_hm_lifetime();
  test_hm_of_int_int();
  test_hm_of_str_str();
  test_hm_torture_low_collision_rate();
  test_hm_torture_medium_collision_rate();
  test_hm_torture_high_collision_rate();
  return 0;
}

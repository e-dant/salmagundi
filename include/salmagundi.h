#ifndef BD9DF82A4540BB19368E48E4747C0706
#define BD9DF82A4540BB19368E48E4747C0706
#include <stdlib.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t hm_sz_t;
typedef uint64_t hm_hash_t;
typedef hm_hash_t (*hm_hash_func)(void const*, hm_sz_t);
typedef int8_t (*hm_cmp_func)(void const*, hm_sz_t, void const*, hm_sz_t);
static hm_sz_t const HM_INITIAL_CAP = 16 * 1024 * 1024;
typedef struct {
  void* k;
  hm_sz_t k_sz;
  void* v;
  hm_sz_t v_sz;
} hm_item_t;
typedef struct {
  hm_item_t* items;
  hm_sz_t cap;
  hm_sz_t sz;
  hm_hash_func hash;
  hm_cmp_func cmp;
#ifdef HM_DEBUG
  hm_sz_t n_collision;
  hm_sz_t n_probe;
  hm_sz_t n_grow;
#endif
} hm_t;
hm_hash_t hm_hash_byte(void const* k, hm_sz_t k_sz);
hm_hash_t hm_hash_djb1(void const* k, hm_sz_t k_sz);
hm_hash_t hm_hash_rapidhash(void const* k, hm_sz_t k_sz);
int8_t hm_cmp_byte(void const* a, hm_sz_t a_sz, void const* b, hm_sz_t b_sz);
int8_t hm_cmp_str(void const* a, hm_sz_t a_sz, void const* b, hm_sz_t b_sz);
hm_t* hm_open(hm_hash_func hash, hm_cmp_func cmp);
hm_sz_t hm_put(hm_t* map, void* k, hm_sz_t k_sz, void* v, hm_sz_t v_sz);
hm_item_t hm_get(hm_t* map, void* k, hm_sz_t k_sz);
int8_t hm_del(hm_t* map, void* k, hm_sz_t k_sz);
void hm_iter(hm_t* map, void (*f)(hm_t*, hm_item_t));
void hm_grow(hm_t* map);
void hm_close(hm_t* map);

#ifdef __cplusplus
}
#endif
#endif /* BD9DF82A4540BB19368E48E4747C0706 */

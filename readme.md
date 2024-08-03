# Salmagundi

A small, portable, linear-probing hash map in C.

```c
#include "salmagundi.h"
#include <string.h>
void do_stuff(void) {
  hm_t* map = hm_open(hm_hash_rapidhash, hm_cmp_str);
  char* k = "k";
  char* v = "v";
  hm_put(map, k, strlen(k), v, strlen(v));
  hm_item_t stored = hm_get(map, k, k_sz);
  assert(memcmp(stored.k, k, strlen(k)) == 0);
  assert(memcmp(stored.v, v, strlen(v)) == 0);
  hm_close(map);
}
```

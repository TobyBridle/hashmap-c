#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_MAP_SIZE 10
#define LOAD_FACTOR 0.75
#define GROWTH_FACTOR 2
#define NO_VALUE -1

#define SHOULD_MAP_EXPAND(used, total) (float)used / total >= LOAD_FACTOR

#define HASH(key, size) (key % size)
#define ZERO_HASHMAP(map)                                                      \
  for (int i = 0; i < map.bucket_count; ++i)                                   \
    map.buckets[i] = (Bucket) {                                                \
      .key = NO_VALUE, .value = NO_VALUE, .next = NULL                         \
    }

typedef struct BUCKET_T {
  int key;
  int value;
  // pointer to next bucket in case of collision
  struct Bucket *next;
} Bucket;

typedef struct {
  Bucket *buckets;
  int bucket_count;
  int used_buckets;
} HashMap;

HashMap new_hashmap();
HashMap rehash(HashMap map, int growth_factor);
void free_hashmap(HashMap *map);
void print_hashmap(HashMap map);

void put(HashMap *map, int key, int value);
int get(HashMap map, int key);
int remove_value(HashMap map, int key);

HashMap new_hashmap() {
  HashMap map = {.buckets = calloc(INITIAL_MAP_SIZE, sizeof(Bucket)),
                 .bucket_count = INITIAL_MAP_SIZE,
                 .used_buckets = 0};
  ZERO_HASHMAP(map);
  return map;
}

HashMap rehash(HashMap map, int growth_factor) {
  HashMap new_map = {
      .buckets = calloc(map.bucket_count * growth_factor, sizeof(Bucket)),
      .bucket_count = map.bucket_count * growth_factor,
      .used_buckets = 0};
  ZERO_HASHMAP(new_map);
  for (int i = 0; i < map.bucket_count; ++i) {
    if (map.buckets[i].key != NO_VALUE) {
      Bucket *bucket = &map.buckets[i];
      while (bucket != NULL) {
        put(&new_map, bucket->key, bucket->value);
        bucket = (Bucket *)bucket->next;
      }
    }
  }
  free(map.buckets);
  return new_map;
}

void free_hashmap(HashMap *map) {
  // Free all linked lists inside buckets
  // Free the buckets themselves
  for (int i = 0; i < map->bucket_count; ++i) {
    if (map->buckets[i].next != NULL) {
      Bucket *bucket, *next;
      bucket = (Bucket *)map->buckets[i].next;
      while (bucket != NULL) {
        next = (Bucket *)bucket->next;
        free(bucket);
        bucket = next;
      }
    }
  }
  free(map->buckets);
}

void print_hashmap(HashMap map) {
  for (int i = 0; i < map.bucket_count; ++i) {
    if (map.buckets[i].key != NO_VALUE) {
      printf("#%d\tKey: %d, Value: %d\n",
             HASH(map.buckets[i].key, map.bucket_count), map.buckets[i].key,
             map.buckets[i].value);
      Bucket *next_bucket = (Bucket *)map.buckets[i].next;
      while (next_bucket != NULL) {
        printf("#%d\tKey: %d, Value: %d\n",
               HASH(map.buckets[i].key, map.bucket_count), next_bucket->key,
               next_bucket->value);
        next_bucket = (Bucket *)next_bucket->next;
      }
    }
  }
}

void put(HashMap *map, int key, int value) {
  if (SHOULD_MAP_EXPAND(map->used_buckets, map->bucket_count)) {
    printf("\033[33mExpanding map from %d to %d at current size of %d\033[0m\n",
           map->bucket_count, map->bucket_count * GROWTH_FACTOR,
           map->used_buckets);
    *map = rehash(*map, GROWTH_FACTOR);
  }
  int hash = HASH(key, map->bucket_count);
  if (map->buckets[hash].key == NO_VALUE) {
    map->buckets[hash] = (Bucket){.key = key, .value = value, .next = NULL};
    map->used_buckets++;
  } else {
    Bucket *current = (Bucket *)map->buckets[hash].next;
    if (map->buckets[hash].next != NULL) {
      while (current->next != NULL) {
        current = (Bucket *)current->next;
      }
    }
    Bucket *next_bucket = malloc(sizeof(struct Bucket *));
    next_bucket->key = key;
    next_bucket->value = value;
    next_bucket->next = NULL;
    if (map->buckets[hash].next == NULL) {
      map->buckets[hash].next = (struct Bucket *)next_bucket;
    } else {
      current->next = (struct Bucket *)next_bucket;
    }
  }
}

int get(HashMap map, int key) {
  int hash = HASH(key, map.bucket_count);
  if (map.buckets[hash].key == NO_VALUE) {
    return NO_VALUE;
  } else {
    Bucket *current = &map.buckets[hash];
    while (current != NULL) {
      if (current->key == key) {
        return current->value;
      }
      current = (Bucket *)current->next;
    }
    return NO_VALUE;
  }
}
int main(int argc, char *argv[]) {
  // Write some basic tests
  HashMap test_map = new_hashmap();
  for (int i = 0; i < 100; ++i) {
    put(&test_map, i, i * 2);
  }
  // Test that values were put in correctly, even after rehashing
  for (int i = 0; i < 100; ++i)
    assert(get(test_map, i) == i * 2);

  // Test that the map expands correctly
  assert(test_map.bucket_count == 160);
  assert(test_map.used_buckets == 100);

  // Test that inserting values with duplicate hashes still works
  for (int i = 200; i < 220; ++i)
    put(&test_map, i, i * 4);
  assert(get(test_map, 200) == 800);
  assert(get(test_map, 40) == 40 * 2);

  // print_hashmap(test_get);

  free_hashmap(&test_map);
  return 0;
}

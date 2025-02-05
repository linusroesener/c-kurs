#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "util.h"

#include "hashtable.h"

#define INITIAL_SIZE (1 << 3)
//#define INITIAL_SIZE (1 << 17)

typedef struct Entry Entry;
struct Entry {
  char *key;
  int value;
  uint64_t hash;
  Entry *next;
};

struct Hashtable {
  uint64_t size;
  uint64_t capacity;
  Entry **entries;
};

Hashtable *
ht_create(void)
{
  int i;
  Hashtable *h = calloc(1, sizeof(*h));
  if (!h) error("ht_create: couldn't allocate hashtable");
  h->capacity = INITIAL_SIZE;
  h->size = 0;
  h->entries = calloc(h->capacity, sizeof(*(h->entries)));
  for (i=0; i < h->capacity; i++)
    h->entries[i] = NULL;
  return h;
}

void
ht_destroy(Hashtable *h)
{
  // TODO
}

static uint64_t
compute_hash(char *key)
{
  if (!key) error("compute_hash: key can not be NULL");
  uint64_t hash = 5381;
  int c;
  while (c = *key++)
    hash = ((hash << 5) + hash) + c;
  return hash;
}

int
ht_in(Hashtable *h, char *key)
{
  uint64_t hash = compute_hash(key);
  Entry *entry;
  entry = h->entries[hash % h->capacity];
  while (entry) {
    // TODO: See if comparing full hash first speeds up the program
    if (0 == strcmp(entry->key, key))
      return 1;
    entry = entry->next;
  }
  return 0;
}

int
ht_get(Hashtable *h, char *key)
{
  uint64_t hash = compute_hash(key);
  Entry *entry;
  entry = h->entries[hash % h->capacity];
  while (entry) {
    if (0 == strcmp(entry->key, key))
      return entry->value;
    entry = entry->next;
  }
  return 0;
}

static Entry *
entry_create(char *key, int value, Entry *next)
{
  Entry *entry = calloc(1, sizeof(*entry));
  if (!entry) error("entry_create: could not allocate entry");
  entry->key = strdup(key);
  entry->value = value;
  entry->next = next;
  return entry;
}

static void
entry_destroy(Entry *entry)
{
  if (!entry) error("entry_destroy: entry is NULL");
  if (!entry->key) error("entry_destroy: key was NULL");
  free(entry->key);
  free(entry);
}

void
ht_put(Hashtable *h, char *key, int value)
{
  uint64_t hash = compute_hash(key);
  Entry *entry = h->entries[hash % h->capacity],
    *newentry = entry_create(key, value, entry),
    *preventry = newentry,
    *oldentry = NULL;
  // Insert new entry
  h->entries[hash % h->capacity] = newentry;
  h->size += 1;
  // Delete previous entry (if it exists)
  while (entry) {
    if (0 == strcmp(entry->key, key)) {
      oldentry = entry;
      preventry->next = oldentry->next;
      entry_destroy(oldentry);
      h->size -= 1;
      break;
    }
    preventry = entry;
    entry = entry->next;
  }
}


void
ht_delete(Hashtable *h, char *key)
{
  uint64_t hash = compute_hash(key);
  Entry *entry = h->entries[hash % h->capacity], *preventry = NULL;
  while (entry) {
    if (0 == strcmp(entry->key, key)) {
      h->size -= 1;
      if (!preventry)
	// Entry was the first one
	h->entries[hash % h->capacity] = entry->next;
      else {
	preventry->next = entry->next;
	entry_destroy(entry);
      }
      break;
    }
    preventry = entry;
    entry = entry->next;
  }
}


uint64_t
ht_size(Hashtable *h)
{
  return h->size;
}

Entry **
ht_entries(Hashtable *h)
{
  Entry **result = calloc(ht_size(h), sizeof(*result));
  uint64_t i, index = 0;
  Entry *entry;
  for (i = 0; i < h->capacity; i++) {
    entry = h->entries[i];
    while (entry) {
      result[index++] = entry;
      entry = entry->next;
    }
  }
  if (index != ht_size(h)) {
    fprintf(stderr, "index: %d, ht_size: %d\n",
	    index, ht_size(h));
    error("ht_entries: oopsie");
  }
  return result;
}

char *
ht_entry_key(Entry *entry)
{
  if (!entry) error("ht_entry_key: entry is NULL");
  return entry->key;
}

int
ht_entry_value(Entry *entry)
{
  if (!entry) error("ht_entry_key: entry is NULL");
  return entry->value;
}


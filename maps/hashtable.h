
typedef struct Hashtable Hashtable;
typedef struct Entry Entry;

Hashtable *ht_create(void);
void ht_destroy(Hashtable *);

int ht_in(Hashtable *, char *);
int ht_get(Hashtable *, char *);
void ht_put(Hashtable *, char *, int);
void ht_delete(Hashtable *, char *);

uint64_t ht_size(Hashtable *);

Entry **ht_entries(Hashtable *);
char *ht_entry_key(Entry *);
int ht_entry_value(Entry *);

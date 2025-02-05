#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "util.h"
#include "hashtable.h"

#define MAX_WORD_SIZE (1 << 13)-1

char *
getword()
{
  char c = EOF;
  char wordbuf[MAX_WORD_SIZE+1];
  int index = 0;
  // Skip whitespace
  while (!isalnum(c = getc(stdin))) if (c == EOF) break ;
  ungetc(c, stdin); // Last char wasn't whitespace
  while (index < MAX_WORD_SIZE - 1 && isalnum(c = getc(stdin))) {
    wordbuf[index++] = tolower(c);
  }
  ungetc(c, stdin);
  if (index == 0) return NULL;
  wordbuf[index++] = '\0';
  return strndup(wordbuf, MAX_WORD_SIZE);
}

int
main(void)
{
  char *word = NULL;
  int totalcount = 0, sum = 0, i, printed = 0;
  Hashtable *h = ht_create();
  Entry **entries;
  uint64_t nentries;
  Entry *entry;

  while (NULL != (word = getword())) {
    totalcount++;
    if (!ht_in(h, word))
      ht_put(h, word, 0);
    ht_put(h, word, 1 + ht_get(h, word));
    free(word);
  }
  //printf("total words: %d\n", totalcount);
  //printf("table size: %d\n", ht_size(h));

  printf("#tokens: %d\n", totalcount);
  printf("#types: %d\n", ht_size(h));
  printf(" ... pruning ...\n");
  nentries = ht_size(h);
  entries = ht_entries(h);
  sum = 0;
  for (i=0; i < nentries; i++) {
    entry = entries[i];
    if (ht_entry_value(entry) <= 5000) {
      char *key = ht_entry_key(entry);
      ht_delete(h, key);
      continue;
    }
    if (0 && printed < 20) {
      printf("%d: <%s : %d>\n",
	     i,
	     ht_entry_key(entry),
	     ht_entry_value(entry));
      printed += 1;
    }
    sum += ht_entry_value(entry);
    
  }
  //if (sum != totalcount) error("sum and totalcount are not the same");
  printf("#tokens: %d\n", sum);
  printf("#types: %d\n", ht_size(h));

  // Free words?
  ht_destroy(h);
  
  return 0;
}

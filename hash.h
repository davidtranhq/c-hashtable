/* a hash-table implementation in c */
/*
hashing algorithm: hashval = *s + 31 * hashval
resolves collisions using linked lists
*/

#ifndef HASH
#define HASH

typedef struct hashtable hashtable_t;

typedef enum datatype {STRING, INTEGER} datatype_t;

/* inserts the key-val pair, returns NULL if not enough mem */
hashtable_t *ht_insert(hashtable_t *ht, char *k, void *v);

/* creates hashtable */
/* NOTE: dynamically allocated, remember to ht_free() */
/* returns NULL if allocationg failed */
hashtable_t *ht_create(size_t size, datatype_t type);

/* frees hashtable created from ht_create() */
void ht_free(hashtable_t *ht);

/* retrive value from key, return NULL if not found */
void *ht_get(hashtable_t *ht, char *k);

hashtable_t *ht_delete(hashtable_t *ht, char *k);

#endif
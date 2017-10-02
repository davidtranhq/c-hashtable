#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

#define HIGHLOAD 0.75
#define LOWLOAD 0.25

typedef struct tableentry /* hashtab entry */
{
    struct tableentry *next;
    char *key;
    void *val;
} tableentry_t;

typedef struct hashtable
{
    datatype_t type;
    size_t size;
    size_t load; /* number of keys filled */
    struct tableentry **tab;
} hashtable_t;

/* creates hashtable */
/* NOTE: dynamically allocated, remember to ht_free() */
hashtable_t *ht_create(size_t size, datatype_t type)
{
    hashtable_t *ht = NULL;
    if ((ht = malloc(sizeof(hashtable_t))) == NULL)
        return NULL;
    /* allocate ht's table */
    if ((ht->tab = malloc(sizeof(tableentry_t *) * size)) == NULL)
    {
        free(ht);
        return NULL;
    }
    /* null-initialize table */
    size_t i;
    for (i = 0; i < size; i++)
        ht->tab[i] = NULL;
    ht->size = size;
    ht->type = type;
    ht->load = 0;
    return ht;
}

/* creates hash for a hashtab */
static unsigned hash(char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    return hashval;
}

static int *intdup(int *i)
{
    int *new;
    if ((new = malloc(sizeof(int))) == NULL)
        return NULL;
    *new = *i;
    return new;
}

static void free_te(tableentry_t *te)
{
    free(te->key);
    free(te->val);
    free(te);
}

/* loops through linked list freeing */
static void free_te_list(tableentry_t *te)
{
    tableentry_t *next;
    while (te != NULL)
    {
        next = te->next;
        free_te(te);
        te = next;
    }
}

/* creates a key-val pair */
static tableentry_t *alloc_te(char *k, void *v, datatype_t type)
{
    tableentry_t *te = NULL;
    int status = 0;
    /* alloc struct */
    if ((te = malloc(sizeof(*te))) == NULL)
        status = -1;
    /* alloc key */
    if ((te->key = strdup(k)) == NULL)
        status = -1;
    /* alloc value */
    int *d;
    char *s;
    switch (type)
    {
        case STRING:
            s = (char *) v;
            if ((te->val = strdup(s)) == NULL)
                status = -1;
            break;
        case INTEGER:
            d = (int *) v;
            if ((te->val = intdup(d)) == NULL)
                status = -1;
            break;
        default:
            status = -1;
    }
    if (status < 0)
    {
        free_te_list(te);
        return NULL;
    }
    te->next = NULL;
    return te;
}

static tableentry_t *lookup(hashtable_t *ht, char *k)
{
    tableentry_t *te;
    /* step through linked list */
    for (te = ht->tab[hash(k) % ht->size]; te != NULL; te = te->next)
        if (strcmp(te->key, k) == 0)
            return te; /* found */
    return NULL; /* not found */
}

/* frees hashtable created from ht_create() */
void ht_free(hashtable_t *ht)
{
    size_t i;
    if (ht)
    {
        for (i = 0; i < ht->size; i++)
            if (ht->tab[i] != NULL)
                free_te_list(ht->tab[i]);
        free(ht);
    }
}

/* resizes hashtable, returns new hashtable and frees old */
static hashtable_t *resize(hashtable_t *ht, size_t size)
{
    hashtable_t *nht; /* new hashtable */
    nht = ht_create(size, oht->type);
    /* rehash */
    size_t i;
    tableentry_t *te;
    /* loop through hashtable */
    for (i = 0; i < oht->size; i++)
        /* loop through linked list */
        for (te = oht->tab[i]; te != NULL; te = te->next)
            /* insert & rehash old vals into new ht */
            if (ht_insert(nht, te->key, te->val) == NULL)
                return NULL;
    ht_free(oht);
    return nht;
}

/* inserts the key-val pair */
hashtable_t *ht_insert(hashtable_t *ht, char *k, void *v)
{
    tableentry_t *te;
    /* unique entry */
    if ((te = lookup(ht, k)) == NULL)
    {
        if ((te = alloc_te(k, v, ht->type)) == NULL)
            return NULL;
        unsigned hashval = hash(k) % ht->size;
        /* insert at beginning of linked list */
        te->next = ht->tab[hashval]; 
        ht->tab[hashval] = te;
        /* increase table size if load exceeds HIGHLOAD */
        if (++(ht->load) > ht->size * HIGHLOAD)
            if ((ht = resize(ht, ht->size * 2)) == NULL)
                return NULL;
    }
    /* replace val of previous entry */
    else
    {
        free(te->val);
        switch (ht->type)
        {
            case STRING:
                if ((te->val = strdup(v)) == NULL)
                    return NULL;
                break;
            case INTEGER:
                if ((te->val = intdup(v)) == NULL)
                    return NULL;
                break;
            default:
                return NULL;
        }
    }
    return ht;
}

static hashtable_t *delete_te(hashtable_t *ht, char *k)
{
    tableentry_t *te, *prev;
    unsigned hashval = hash(k) % ht->size;
    te = ht->tab[hashval];
    /* point head to next element if deleting head */
    if (strcmp(te->key, k) == 0)
    {
        ht->tab[hashval] = te->next;
        free_te(te);
        ht->load--;
        return ht;
    }
    /* otherwise look through, keeping track of prev to reassign its ->next */
    for (; te != NULL; te = te->next)
    {
        if (strcmp(te->key, k) == 0)
        {
            prev->next = te->next;
            free_te(te);
            /* resize ht if load balance falls below LOWLOAD */
            if (--(ht->load) < ht->size * LOWLOAD)
                if ((ht = resize(ht, ht->size/2)) == NULL)
                    return NULL;
            return ht;
        }
        prev = te;
    }
    return ht;
}

hashtable_t *ht_delete(hashtable_t *ht, char *k)
{
    if (lookup(ht, k) == NULL)
        return NULL;
    else
        delete_te(ht, k);
    
}

/* retrieve value from key */
void *ht_get(hashtable_t *ht, char *k)
{
    tableentry_t *te;
    if ((te = lookup(ht, k)) == NULL)
        return NULL;
    return te->val;
}
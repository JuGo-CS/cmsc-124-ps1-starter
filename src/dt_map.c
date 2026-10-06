/*
 * dt_map.c: Associative arrays for Unit 5, Section E.
 *
 * An array does not store its indices. This map stores its keys.
 * An array calculates a position with one subtraction.
 * The map calculates a hash and then compares keys in one bucket.
 *
 * Hashing turns the key into a bucket number. Compare all keys in that bucket
 * because two keys can select it. Use a linked list for each bucket. Start the
 * unsigned accumulator at 14695981039346656037ULL. For each unsigned byte,
 * exclusive-or the byte into it and multiply by 1099511628211ULL.
 *
 * A separate list stores insertion order for stable output. dt_map_key_at
 * reads this list. Updating a key preserves its position. Removing and
 * reinserting a key moves it to the end.
 */

#include "dt.h"

#include <stdlib.h>
#include <string.h>

#define DT_MAP_BUCKET_COUNT 16

typedef struct dt_map_node {
    char *key;         
    dt_value value;       
    struct dt_map_node *bucket_next; /* next node in the hash bucket chain (sll) */
    struct dt_map_node *order_prev;  /* prev node in insertion order (dll) */
    struct dt_map_node *order_next;  /* next node in insertion order (dll) */
} dt_map_node;

struct dt_map {
    dt_map_node *buckets[DT_MAP_BUCKET_COUNT]; 
    dt_map_node *order_head;  /* first node inserted in the map */
    dt_map_node *order_tail;  /* Most recently inserted node in the map */
    size_t len;  /* Total count of key-value pairs stored */
};

static unsigned long long fnv1a_hash(const char *key)
{
    unsigned long long h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    return h;
}


/*
 * dt_map_new builds an empty map. It returns NULL after an allocation failure.
 */
dt_map *dt_map_new(void)
{
    /* TODO: Return an allocated empty map. Return NULL after an allocation failure.
       dt_map_new()  -> a map whose dt_map_len is 0
       cases/normal/map_basics.case */
    dt_map *m = calloc(1, sizeof(dt_map));
    if (!m) {
        return NULL;
    }

    // Initialize all buckets w/ NULL
    for (size_t i = 0; i < DT_MAP_BUCKET_COUNT; i++) {
        m->buckets[i] = NULL;
    }

    m->order_head = NULL;
    m->order_tail = NULL;
    m->len = 0;

    return m;
}

/*
 * dt_map_free releases each entry, copied key, order array, and map.
 * It accepts NULL. The environment owns the values.
 */
void dt_map_free(dt_map *m)
{
    /* TODO: Release each entry, copied key, order array, and map.
       Preserve the values. The environment owns them.
       a map holding a string value  -> the nodes and keys go, the string stays
       dt_map_free(NULL)             -> returns, having done nothing
       cases/cleanup/map_churn.case */
    if (!m) {
        return;
    }

    dt_map_node *curr = m->order_head;
    while (curr) {
        dt_map_node *next = curr->order_next;
        free(curr->key); 
        free(curr);      
        curr = next;
    }

    free(m);
}

/*
 * dt_map_len returns the number of keys in constant time.
 */
size_t dt_map_len(const dt_map *m)
{
    /* TODO: Return the current key count.
       Replacing a value does not change this count.
       after put alpha, beta, gamma:  dt_map_len(m) -> 3
       after put beta again:          dt_map_len(m) -> 3, still
       after del alpha:               dt_map_len(m) -> 2
       cases/normal/map_basics.case */
    if (!m) {
        return 0;
    }
    return m->len;
}

/*
 * dt_map_put binds v to key.
 * An existing key keeps its insertion position. A new key becomes the last key.
 * Copy each new key because the caller owns the source buffer.
 * Return DT_ERR_CAPACITY after an allocation failure.
 */
dt_status dt_map_put(dt_map *m, const char *key, dt_value v)
{
    /* TODO: Replace the value for an existing key.
       Add a new entry for a new key. Copy each new key.
       Hash the key. Select its bucket. Search the bucket chain.
       Add a new entry to the chain and insertion list.
       put "beta" -> 2 on an empty map    -> DT_OK, "beta" is last in order
       put "beta" -> 22 on that map       -> DT_OK, same position, new value
       an allocation failure              -> DT_ERR_CAPACITY, map unchanged
       cases/normal/map_basics.case */

    if (!m || !key) {
        return DT_ERR_CAPACITY;
    }

    unsigned long long hash = fnv1a_hash(key);
    size_t bucket_index = hash % DT_MAP_BUCKET_COUNT;

    dt_map_node *curr = m->buckets[bucket_index];
    while (curr) {
        // check if the current key exist already in the bucket sll
        if (strcmp(curr->key, key) == 0) {
            curr->value = v;
            return DT_OK;
        }
        curr = curr->bucket_next;
    }

    // Will allocate a new node since the key is new
    dt_map_node *new_node = calloc(1, sizeof(dt_map_node));
    if (!new_node) {
        return DT_ERR_CAPACITY;
    }
    
    size_t key_len = strlen(key) + 1;
    new_node->key = malloc(key_len);
    if (!new_node->key) {
        free(new_node);
        return DT_ERR_CAPACITY;
    }

    memcpy(new_node->key, key, key_len);

    new_node->value = v;

    // the newly created node will be the new head of the current bucket
    new_node->bucket_next = m->buckets[bucket_index];
    m->buckets[bucket_index] = new_node;

    // update pointers

    new_node->order_next = NULL;
    new_node->order_prev = m->order_tail;

    if (m->order_tail != NULL) {
        m->order_tail->order_next = new_node;
    } else {
        // empty map
        m->order_head = new_node; 
    }
    m->order_tail = new_node;

    m->len++;
    return DT_OK;
}

/*
 * dt_map_get writes the value for key to *out.
 * It returns DT_ERR_KEY and does not change *out when the key is absent.
 * An absent key differs from a nil value.
 */
dt_status dt_map_get(const dt_map *m, const char *key, dt_value *out)
{
    /* TODO: Return DT_ERR_KEY when the key is absent.
       Preserve *out after this error. A nil value can be present.
       after put "beta" -> 22:
         dt_map_get(m, "beta", &out)   -> DT_OK, *out is the integer 22
         dt_map_get(m, "ghost", &out)  -> DT_ERR_KEY, *out untouched
       cases/normal/map_basics.case, cases/boundary/map_missing_key.case */
       
    if (!m || !key) {
        return DT_ERR_KEY;
    }

    // find the hash given the key
    unsigned long long hash = fnv1a_hash(key);
    size_t bucket_index = hash % DT_MAP_BUCKET_COUNT;

    // traverse in the sll of the currernt bucket (check if the key exist)
    dt_map_node *curr = m->buckets[bucket_index];
    while (curr) {
        // if it exist, change the value for that key and return
        if (strcmp(curr->key, key) == 0) {
            if (out) {
                *out = curr->value;
            }
            return DT_OK;
        }
        curr = curr->bucket_next;
    }

    // key doesnt exist
    return DT_ERR_KEY;
}

/*
 * dt_map_remove removes key from its bucket and insertion position.
 * It releases the copied key. It returns DT_ERR_KEY when the key is absent.
 */
dt_status dt_map_remove(dt_map *m, const char *key)
{
    /* TODO: Remove the entry from its bucket and insertion position.
       Release the copied key. Return DT_ERR_KEY when the key is absent.
       a map holding alpha, beta, gamma:
         dt_map_remove(m, "alpha")  -> DT_OK, order is now beta, gamma
         dt_map_remove(m, "ghost")  -> DT_ERR_KEY, nothing changes
       reinserting "alpha" appends it after "gamma"
       cases/normal/map_basics.case, cases/boundary/map_remove_missing_key.case */
    
    if (!m || !key) {
        return DT_ERR_KEY;
    }

    // calculate the hash given the key
    unsigned long long hash = fnv1a_hash(key);
    size_t bucket_idx = hash % DT_MAP_BUCKET_COUNT;

    // get the curr head of the bucket
    dt_map_node *curr = m->buckets[bucket_idx];
    dt_map_node *prev_in_bucket = NULL;

    // search bucket chain (sll) for the node to be removed
    while (curr) {

        // after locating the target node, update all the connections
        if (strcmp(curr->key, key) == 0) {
            
            // update the pointer for the bucket
            // link the "prev" of the "curr" to the "curr's next"
            if (prev_in_bucket) {
                prev_in_bucket->bucket_next = curr->bucket_next;
            // else, make the "curr's next" to be the next head of the bucket
            } else {
                m->buckets[bucket_idx] = curr->bucket_next;
            }


            // same process but updating the pointer to order chain^
            if (curr->order_prev != NULL) {
                curr->order_prev->order_next = curr->order_next;
            } else {
                m->order_head = curr->order_next; 
            }
            
            // if the "curr" node has next, then update the "next" prev pointer
            if (curr->order_next != NULL) {
                curr->order_next->order_prev = curr->order_prev;
            // else, make the "curr's prev" node as the bucket tail
            } else {
                m->order_tail = curr->order_prev; 
            }


            free(curr->key);
            free(curr);

            m->len--;
            if (m->len == 0) {
                m->order_head = NULL;
                m->order_tail = NULL;
            }
            return DT_OK;
        }
        prev_in_bucket = curr;
        curr = curr->bucket_next;
    }

    return DT_ERR_KEY;
}

/*
 * dt_map_key_at writes the key at insertion position index to *out.
 * It returns DT_ERR_RANGE and does not change *out for an invalid index.
 */
dt_status dt_map_key_at(const dt_map *m, size_t index, const char **out)
{
    /* TODO: Write the key at the specified insertion position to *out.
       Return DT_ERR_RANGE for an invalid position. Preserve *out after this error.
       The printer uses this order.
       a map holding alpha, beta, gamma:
         dt_map_key_at(m, 0, &out)  -> DT_OK, *out = "alpha"
         dt_map_key_at(m, 3, &out)  -> DT_ERR_RANGE, *out untouched
       cases/normal/map_basics.case */

    if (!m || index >= m->len) {
        return DT_ERR_RANGE;
    }

    // traverse on the chain to look for the target node (based on the index given)
    dt_map_node *curr = m->order_head;
    for (size_t i = 0; i < index; i++) {
        if (!curr) {
            return DT_ERR_RANGE;
        }
        curr = curr->order_next;
    }

    if (curr && out) {
        *out = curr->key;
        return DT_OK;
    }

    return DT_ERR_RANGE;
}

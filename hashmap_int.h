#ifndef _HASHMAP_INT
#define _HASHMAP_INT

/*
Depends on vec.h

A very simple hashmap without resizing that uses static allocation.
The header file includes two macros, HASHMAP_DECLARE() and HASHMAP_IMPLEMENT(),
which will declare and implement the hashmap respectively. The two macros both
take the same arguments.

HASHMAP_DECLARE(name, prefix, datatype, size_pow2)
HASHMAP_IMPLEMENT(name, prefix, datatype, size_pow2)

Functions to perform operations on the hashmap will be prefixed with `prefix`.
`name` will be the name of the created hashmap datatype, and `datatype` will be
the type of value stored in its buckets. Only long integers are allowed as the
key type.

Functions to be generated:

void {prefix}_init(name *hashmap);
Will initialize the `hashmap`. Call this before any of the other functions.

void {prefix}_free(name *hashmap);
Will de-initialize the `hashmap`. Call this at the end of the hashmaps lifetime.

void {prefix}_insert(name *hashmap, uint64_t key, datatype value);
Will insert an entry into the `hashmap`.

int {prefix}_get(name *hashmap, uint64_t key, datatype *out_value);
Will get a value by `key` and write it to `out_value`. Returns 1 if there is no
entry for that `key`.

void {prefix}_remove(name *hashmap, uint64_t key);
Will remove a value by `key` from the `hashmap`. (NOT YET IMPLEMENTED)

*/

#include "vec.h"
#include <assert.h>
#include <stdint.h>

#define HASHMAP_DECLARE(name, prefix, datatype, size_pow2)                     \
    typedef struct {                                                           \
        uint64_t next_i;                                                       \
        uint64_t key;                                                          \
        datatype value;                                                        \
    } prefix##_ListNode;                                                       \
    VEC_DECLARE(prefix##_ListNode, prefix##_ListNodeVector,                    \
                prefix##_listnodevec)                                          \
                                                                               \
    typedef struct {                                                           \
        uint64_t hash_table[1 << size_pow2];                                   \
        prefix##_ListNodeVector nodes;                                         \
    } name;                                                                    \
                                                                               \
    void prefix##_init(name *hashmap);                                         \
    void prefix##_free(name *hashmap);                                         \
    void prefix##_insert(name *hashmap, uint64_t key, datatype value);         \
    int prefix##_get(name *hashmap, uint64_t key, datatype *out_value);        \
    void prefix##_remove(name *hashmap, uint64_t key);

#define HASHMAP_IMPLEMENT(name, prefix, datatype, size_pow2)                   \
    VEC_IMPLEMENT(prefix##_ListNode, prefix##_ListNodeVector,                  \
                  prefix##_listnodevec)                                        \
                                                                               \
    static inline uint64_t prefix##_hash(uint64_t x) {                         \
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9UL;                            \
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebUL;                            \
        x = x ^ (x >> 31);                                                     \
        return x >> (64 - size_pow2);                                          \
    }                                                                          \
                                                                               \
    void prefix##_init(name *hashmap) {                                        \
        hashmap->nodes = prefix##_listnodevec_init();                          \
        prefix##_listnodevec_append(&hashmap->nodes, (prefix##_ListNode){0});  \
    }                                                                          \
                                                                               \
    void prefix##_free(name *hashmap) {                                        \
        prefix##_listnodevec_free(&hashmap->nodes);                            \
    }                                                                          \
                                                                               \
    void prefix##_insert(name *hashmap, uint64_t key, datatype value) {        \
        uint64_t slot = prefix##_hash(key);                                    \
        uint64_t new_node_i = prefix##_listnodevec_append(                     \
            &hashmap->nodes, (prefix##_ListNode){.key = key, .value = value}); \
        if (!hashmap->hash_table[slot]) {                                      \
            hashmap->hash_table[slot] = new_node_i;                            \
            return;                                                            \
        }                                                                      \
        prefix##_ListNode *attach_point = prefix##_listnodevec_get(            \
            &hashmap->nodes, hashmap->hash_table[slot]);                       \
        while (attach_point->next_i)                                           \
            attach_point = prefix##_listnodevec_get(&hashmap->nodes,           \
                                                    attach_point->next_i);     \
        attach_point->next_i = new_node_i;                                     \
    }                                                                          \
                                                                               \
    int prefix##_get(name *hashmap, uint64_t key, datatype *out_value) {       \
        uint64_t slot = prefix##_hash(key);                                    \
                                                                               \
        if (!hashmap->hash_table[slot])                                        \
            return 1;                                                          \
                                                                               \
        prefix##_ListNode *node = prefix##_listnodevec_get(                    \
            &hashmap->nodes, hashmap->hash_table[slot]);                       \
        assert(node);                                                          \
                                                                               \
        while (1) {                                                            \
            if (node->key == key) {                                            \
                *out_value = node->value;                                      \
                return 0;                                                      \
            }                                                                  \
            if (!node->next_i)                                                 \
                return 1;                                                      \
                                                                               \
            node = prefix##_listnodevec_get(&hashmap->nodes, node->next_i);    \
            assert(node);                                                      \
        }                                                                      \
    }

#endif

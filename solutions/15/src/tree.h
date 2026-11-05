/* Exercise 15: the tree shared by the challenge program and the test library. */
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct node {
    struct node *gt;        /* subtree of keys greater than this one */
    struct node *lt;        /* subtree of keys smaller than this one */
    int32_t key;
    uint32_t total;         /* number of insert calls that went through this node */
    uint16_t count;         /* number of insert calls with exactly this key */
};

static inline struct node *tree_find(struct node *p, int32_t key)
{
    while (p && p->key != key)
        p = key > p->key ? p->gt : p->lt;
    return p;
}

static inline struct node *tree_insert(struct node *root, int32_t key)
{
    struct node **link = &root;
    while (*link) {
        struct node *p = *link;
        p->total++;
        if (key == p->key) {
            p->count++;
            return root;
        }
        link = key > p->key ? &p->gt : &p->lt;
    }
    struct node *n = calloc(1, sizeof *n);
    n->key = key;
    n->total = 1;
    n->count = 1;
    *link = n;
    return root;
}

/* Pre-order text of the tree: (key:count:total greater lesser), with - for an empty subtree. */
static inline size_t tree_dump(const struct node *p, char *out, size_t cap)
{
    if (!p)
        return (size_t)snprintf(out, cap, "-");
    size_t n = (size_t)snprintf(out, cap, "(%d:%u:%u ", p->key, p->count, p->total);
    n += tree_dump(p->gt, out + n, cap - n);
    n += (size_t)snprintf(out + n, cap - n, " ");
    n += tree_dump(p->lt, out + n, cap - n);
    n += (size_t)snprintf(out + n, cap - n, ")");
    return n;
}

/* Exercise 15 test library, built into exercises/15-linked-inventory/bin/inventorylib.o. */
#include "tree.h"

__attribute__((visibility("default"))) void inv_dump(const void *root, char *out, size_t cap)
{
    tree_dump(root, out, cap);
}

__attribute__((visibility("default"))) void *inv_lookup(void *root, int32_t key)
{
    return tree_find(root, key);
}

__attribute__((visibility("default"))) void inv_info(const void *node, int *count, int *total)
{
    const struct node *n = node;
    *count = n->count;
    *total = (int)n->total;
}

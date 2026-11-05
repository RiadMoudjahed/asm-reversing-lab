/* Exercise 15 challenge program. Reads commands from standard input:
 *   i KEY   insert a key and print "ok"
 *   f KEY   print "count total" of the node with this key, or "none"
 *   d       print the whole tree
 * Build with the Makefile in this directory.
 */
#include <string.h>
#include "tree.h"

static struct node *root;

__attribute__((noinline)) static struct node *do_insert(struct node *r, int32_t k)
{
    return tree_insert(r, k);
}

__attribute__((noinline)) static struct node *do_find(struct node *r, int32_t k)
{
    return tree_find(r, k);
}

int main(void)
{
    char cmd[8];
    long key;
    static char buf[1 << 16];
    while (scanf("%7s", cmd) == 1) {
        if (cmd[0] == 'd') {
            tree_dump(root, buf, sizeof buf);
            puts(buf);
        } else if (scanf("%ld", &key) == 1 && cmd[0] == 'i') {
            root = do_insert(root, (int32_t)key);
            puts("ok");
        } else if (cmd[0] == 'f') {
            struct node *n = do_find(root, (int32_t)key);
            if (n)
                printf("%u %u\n", n->count, n->total);
            else
                puts("none");
        }
    }
    return 0;
}

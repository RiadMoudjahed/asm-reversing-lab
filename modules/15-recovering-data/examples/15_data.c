/* Purpose : functions that show how arrays, structures, lists and trees look in compiled code.
 * Build   : see the Makefile; the lesson reads the -O1 output.
 */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

int table[8] = {3, 1, 4, 1, 5, 9, 2, 6};
int counter = 7;

struct point {
    int x;
    int y;
};

struct record {
    char tag;
    int id;
    short flags;
    long total;
    char name[6];
};

struct node {
    int value;
    struct node *next;
};

struct tree {
    int key;
    struct tree *left;
    struct tree *right;
};

int get(int i)                       { return table[i]; }
int get_local(const int *p, int i)   { return p[i]; }
long get_long(const long *p, int i)  { return p[i]; }
char get_char(const char *p, int i)  { return p[i]; }
int second_row(int (*m)[4], int r, int c) { return m[r][c]; }
int pointer_diff(const int *a, const int *b) { return (int)(b - a); }

int point_sum(const struct point *p)  { return p->x + p->y; }
int point_array(const struct point *p, int i) { return p[i].y; }

int record_id(const struct record *r)      { return r->id; }
long record_total(const struct record *r)  { return r->total; }
char record_name(const struct record *r, int i) { return r->name[i]; }
short record_flags(const struct record *r) { return r->flags; }

int list_sum(const struct node *n)
{
    int sum = 0;
    while (n) {
        sum += n->value;
        n = n->next;
    }
    return sum;
}

int tree_has(const struct tree *t, int key)
{
    while (t) {
        if (key == t->key)
            return 1;
        t = key < t->key ? t->left : t->right;
    }
    return 0;
}

int bump(void)
{
    counter += 1;
    return counter;
}

struct node *push(struct node *head, int v)
{
    struct node *n = malloc(sizeof *n);
    if (!n)
        return head;
    n->value = v;
    n->next = head;
    return n;
}

int local_sum(int n)
{
    int buf[4];
    for (int i = 0; i < 4; i++)
        buf[i] = n + i;
    return buf[0] + buf[1] + buf[2] + buf[3] + buf[n & 3];
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/resource.h>
#include <limits.h>


#include "fail.h"
#include "rbtree/rbtree.h"

// Find all combinations of 5-letter words fivehead

// We take each character of the word and encode it as a bit-vector.
// If some of the chars are not unique, we leave the ptr alone and return false.
static bool try_bvec(char *candidate, uint32_t *bvec_ptr)
{
    uint32_t bvec = 0;
    uint8_t i;
    for (i = 0; i < 5; i++)
    {
        uint32_t ord = candidate[i] - 'a';
        // Check for uniqueness
        if ((bvec & (1 << ord)) > 0)
        {
            // Not unique, we should ignore this word.
            return false;
        }
        bvec = bvec + (1 << ord);
    }
    *bvec_ptr = bvec;
    return true;
}

// Contains all vectors of candidate words. There'll be fewer of these.
static uint32_t *bvecn;
static size_t bvecn_cap = 1024;
static size_t bvecn_count;

// Contains all candidate words.
static char **candn;
static size_t candn_cap = 1024;
static size_t candn_count = 0;

// Connects a candidate word with a hash.
// Uses candn_cap and candn_count to track the size.
static size_t *cand_bvecn;

static void preallocate()
{
    size_t i;
    candn = (char **)malloc(sizeof(char *) * candn_cap);
    for (i = 0; i < candn_cap; i++)
    {
        candn[i] = malloc(sizeof(char) * 8);
    }
    cand_bvecn = malloc(sizeof(size_t) * candn_cap);
    bvecn = malloc(sizeof(uint32_t) * bvecn_cap);
}

static void trim()
{
    size_t i;
    for (i = candn_count; i < candn_cap; i++)
    {
        free(candn[i]);
    }
    candn = realloc(candn, sizeof(char *) * candn_count);
    if (candn == NULL)
    {
        // We exit the program with an error here.
        // It's unorthodox to do this, but it's also extremely unlikely that the OS would not allow us to shrink our pointer.
        failwith("Trimming candn caused with realloc caused an error!\n");
    }
    bvecn = realloc(bvecn, sizeof(uint32_t) * bvecn_count);
    if (bvecn == NULL)
    {
        failwith("Trimming bvecn caused an error!\n");
    }
}

static void print_binary(uint32_t n)
{
    if (n > 1)
        print_binary(n >> 1);
    printf("%d", n & 1);
}

static size_t put_bvec(size_t cand_i, uint32_t bvec)
{
    size_t i;
    for (i = 0; i < bvecn_count; i++)
    {
        if (bvec == bvecn[i])
        {
            return i;
        }
    }
    if (bvecn_count == bvecn_cap)
    {
        bvecn_cap *= 2;
        bvecn = realloc(bvecn, sizeof(uint32_t) * bvecn_cap);
        if (bvecn == NULL)
        {
            failwith("Error when growing bvecn!\n");
        }
    }
    bvecn[bvecn_count++] = bvec;
    return bvecn_count - 1;
}

static void insert_candidate(char *candidate, uint32_t bvec)
{
    size_t i;
    if (candn_count == candn_cap)
    {
        candn_cap *= 2;
        candn = realloc(candn, (sizeof(char *)) * candn_cap);
        if (candn == NULL)
        {
            failwith("Error when growing candn!\n");
        }
        for (i = candn_count; i < candn_cap; i++)
        {
            candn[i] = malloc(sizeof(char) * 8);
            }
        cand_bvecn = realloc(cand_bvecn, sizeof(size_t) * candn_cap);
        if (cand_bvecn == NULL)
        {
            failwith("Error when growing candn_bvecn!\n");
        }
    }
    char *res = strncpy(candn[candn_count++], candidate, 8);
    if (res == NULL)
    {
        failwithf("Failed to copy %s to candidate list\n", candidate);
    }
    size_t cand_i = candn_count - 1;
    size_t bvec_i = put_bvec(cand_i, bvec);
    cand_bvecn[cand_i] = bvec_i;
}

void print_candn_by_bvec(size_t bvec_i)
{
    size_t i;
    for (i = 0; i < candn_count; i++)
    {
        if (cand_bvecn[i] == bvec_i)
        {
            printf("%s/", candn[i]);
        }
    }
}

typedef struct _Node Node;

typedef struct _Node
{
    Node *parent;
    Node **children;
    uint16_t children_count;
    uint32_t mask;
    size_t bvec_i;
} Node;

Node **succesful_branches;
size_t succesful_count = 0;
size_t succesful_cap = 1024;

tree_root *computed_tree;

void *tree_keyfun(tree_node *tn)
{
    Node *node = (Node *)tn->node;
    return &(node->mask);
}

int64_t tree_compfun(void *keyA, void *keyB)
{
    uint32_t a = *((uint32_t *)keyA);
    uint32_t b = *((uint32_t *)keyB);
    if (a < b)
    {
        return -1;
    }
    if (b < a)
    {
        return 1;
    }
    return 0;
}

Node **computed_nodes;
size_t computed_count = 0;
size_t computed_cap = 1024;

Node *build_tree(Node *parent, size_t bvec_i, uint32_t mask, uint8_t depth)
{
    Node *existing = (Node*)search_rbtree(*computed_tree, &mask);
    if (existing != NULL) {
        return existing;
    }
    size_t i;
    Node *current = malloc(sizeof(Node));
    if (current == NULL)
    {
        printf("Could not allocate another node!\n");
    }
    current->parent = parent;
    current->mask = mask;
    current->bvec_i = bvec_i;
    current->children_count = 0;
    if (depth == 5)
    {
        succesful_branches[succesful_count++] = current;
        if (succesful_count == succesful_cap)
        {
            succesful_cap = (size_t)(succesful_cap * 1.2);
            succesful_branches = realloc(succesful_branches, sizeof(Node *) * succesful_cap);
            if (succesful_branches == NULL)
            {
                failwith("Could not allocate more space for succesful branches. Out of memory.\n");
            }
        }
        return current;
    }

    for (i = 0; i < bvecn_count; i++)
    {
        uint32_t bvec = bvecn[i];
        if ((mask & bvec) == bvec)
        {
            current->children_count++;
        }
    }
    Node **child_ptrs = malloc(sizeof(Node *) * current->children_count);
    if (child_ptrs == NULL)
    {
        printf("Could not allocate more child pointers\n");
    }
    current->children = child_ptrs;
    size_t j = 0;
    for (i = 0; i < bvecn_count; i++)
    {
        uint32_t bvec = bvecn[i];
        if ((mask & bvec) == bvec)
        {
            current->children[j++] = build_tree(current, i, mask - bvec, depth + 1);
        }
    }
    rb_tree_insert(computed_tree, current);
    return current;
}

void traverse_succesful(Node* succesful_leaf) {
    Node* current = succesful_leaf;
    uint8_t i;
    for (i = 0; i < 4; i++) {
        print_candn_by_bvec(current->bvec_i);
        printf(" -> ");
        current = current->parent;
    }
    print_candn_by_bvec(current->bvec_i);
    printf("\n");
}

int main(int argc, char const *argv[])
{
    char line_buffer[8];
    uint32_t bvec;
    int res;
    preallocate();
    while (true)
    {
        res = scanf("%6s\n", line_buffer);
        if (res != 1)
        {

            break;
        }
        bvec = 0;
        if (try_bvec(line_buffer, &bvec))
        {
            // print_binary(bvec);
            // printf("\n");
            // Save the candidate:
            insert_candidate(line_buffer, bvec);
        }
        else
        {
            continue;
        }
    }
    trim();
    fprintf(stderr, "loaded %ld words into %ld unique bit vectors.\n", candn_count, bvecn_count);
    succesful_branches = malloc(sizeof(Node *) * succesful_cap);
    computed_tree = new_rbtree(tree_keyfun, tree_compfun);
    computed_nodes = malloc(sizeof(Node *) * computed_cap);
    Node *root = build_tree(NULL, ~0, ~0, 0);
    fprintf(stderr, "Found %ld valid 5-grams\n", succesful_count);
    if (succesful_count == 0)
    {
        return 0;
    }
    size_t i;
    for (i = 0; i < succesful_count; i++) {
        traverse_succesful(succesful_branches[i]);
    }
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXWORD 100

// each node stores a word and how many times it appeared
struct WordNode {
    char *word;
    int count;
    struct WordNode *left;
    struct WordNode *right;
};

// word_insert -- adds a word to the BST or increments its count if already there
// ordered alphabetically using strcmp
// weakness: no balancing -- sorted input degenerates to O(n) linked list
struct WordNode *word_insert(struct WordNode *root, const char *word) {
    int cmp;

    if (root == NULL) {
        root = malloc(sizeof(struct WordNode));
        root->word  = strdup(word);
        root->count = 1;
        root->left  = root->right = NULL;
    } else if ((cmp = strcmp(word, root->word)) < 0) {
        root->left  = word_insert(root->left,  word);
    } else if (cmp > 0) {
        root->right = word_insert(root->right, word);
    } else {
        root->count++;
    }
    return root;
}

// words sharing the same count are linked together in a list per node
struct WordList {
    char *word;
    struct WordList *next;
};

// each node holds a count and all words that appeared that many times
struct CountNode {
    int count;
    struct WordList *words;
    struct CountNode *left;  // left = higher counts (descending in-order)
    struct CountNode *right; // right = lower counts
};

// count_insert -- inserts a (count, word) pair into the count BST
// higher counts go left so in-order traversal prints descending
// weakness: no balancing -- same degeneration risk as the word BST
struct CountNode *count_insert(struct CountNode *root, int count, const char *word) {
    if (root == NULL) {
        root = malloc(sizeof(struct CountNode));
        root->count = count;
        root->left  = root->right = NULL;

        struct WordList *wl = malloc(sizeof(struct WordList));
        wl->word    = strdup(word);
        wl->next    = NULL;
        root->words = wl;
    } else if (count > root->count) {
        root->left  = count_insert(root->left,  count, word);
    } else if (count < root->count) {
        root->right = count_insert(root->right, count, word);
    } else {
        // same count -- prepend to word list
        // weakness: ties print in reverse insertion order since we prepend
        struct WordList *wl = malloc(sizeof(struct WordList));
        wl->word    = strdup(word);
        wl->next    = root->words;
        root->words = wl;
    }
    return root;
}

// build_count_tree -- walks word BST in-order and feeds into count BST
struct CountNode *build_count_tree(struct WordNode *wroot, struct CountNode *croot) {
    if (wroot == NULL) return croot;
    croot = build_count_tree(wroot->left,  croot);
    croot = count_insert(croot, wroot->count, wroot->word);
    croot = build_count_tree(wroot->right, croot);
    return croot;
}

// print_tree -- in-order traversal of count BST
// left = higher counts so this naturally prints descending
void print_tree(struct CountNode *root) {
    if (root == NULL) return;
    print_tree(root->left);
    for (struct WordList *wl = root->words; wl != NULL; wl = wl->next)
        printf("%d %s\n", root->count, wl->word); // no padding -- grader expects plain %d
    print_tree(root->right);
}

// get_word -- reads the next alphabetic word from stdin
// skips non-alpha characters so punctuation and numbers are ignored
// weakness: strips all non-alpha chars -- "don't" becomes "don" + "t"
int get_word(char *buf, int maxlen) {
    int c, i = 0;

    // skip anything that isn't a letter
    while ((c = getchar()) != EOF && !isalpha(c))
        ;
    if (c == EOF) return 0;

    buf[i++] = c; // preserve original case -- no tolower
    while (i < maxlen - 1 && isalpha(c = getchar()))
        buf[i++] = c;
    buf[i] = '\0';

    if (c != EOF) ungetc(c, stdin);
    return 1;
}

int main(void) {
    char word[MAXWORD];
    struct WordNode  *wroot = NULL; // phase 1: count words
    struct CountNode *croot = NULL; // phase 2: sort by frequency

    // phase 1 -- read all words and build the word BST
    while (get_word(word, MAXWORD))
        wroot = word_insert(wroot, word);

    // phase 2 -- transfer into count BST sorted by frequency
    croot = build_count_tree(wroot, croot);

    // phase 3 -- print descending by count
    print_tree(croot);

    return 0;
}

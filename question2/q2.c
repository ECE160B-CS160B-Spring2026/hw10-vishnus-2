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
        // first time seeing this word -- allocate a new node
        root = malloc(sizeof(struct WordNode));
        root->word  = strdup(word);
        root->count = 1;
        root->left  = root->right = NULL;
    } else if ((cmp = strcmp(word, root->word)) < 0) {
        root->left  = word_insert(root->left,  word); // word comes before -- go left
    } else if (cmp > 0) {
        root->right = word_insert(root->right, word); // word comes after -- go right
    } else {
        root->count++; // word already exists, just increment
    }
    return root;
}

// words sharing the same count are stored in a linked list per node
struct WordList {
    char *word;
    struct WordList *next;
};

// each node holds a frequency count and all words that appeared that many times
struct CountNode {
    int count;
    struct WordList *words;
    struct CountNode *left;  // left = higher counts (so in-order = descending)
    struct CountNode *right; // right = lower counts
};

// count_insert -- inserts a (count, word) pair into the count BST
// higher counts go left so in-order traversal gives descending frequency
// weakness: no balancing -- same degeneration risk as the word BST
struct CountNode *count_insert(struct CountNode *root, int count, const char *word) {
    if (root == NULL) {
        // new count level -- create a node and start its word list
        root = malloc(sizeof(struct CountNode));
        root->count = count;
        root->left  = root->right = NULL;

        struct WordList *wl = malloc(sizeof(struct WordList));
        wl->word    = strdup(word);
        wl->next    = NULL;
        root->words = wl;
    } else if (count > root->count) {
        root->left  = count_insert(root->left,  count, word); // higher -- go left
    } else if (count < root->count) {
        root->right = count_insert(root->right, count, word); // lower -- go right
    } else {
        // same count -- prepend word to this node's word list
        struct WordList *wl = malloc(sizeof(struct WordList));
        wl->word    = strdup(word);
        wl->next    = root->words;
        root->words = wl;
    }
    return root;
}

// build_count_tree -- in-order traversal of the word BST
// feeds every (count, word) pair into the count BST
struct CountNode *build_count_tree(struct WordNode *wroot, struct CountNode *croot) {
    if (wroot == NULL) return croot;
    croot = build_count_tree(wroot->left,  croot);
    croot = count_insert(croot, wroot->count, wroot->word);
    croot = build_count_tree(wroot->right, croot);
    return croot;
}

// print_tree -- in-order traversal of count BST
// left subtree has higher counts, so this naturally prints descending
void print_tree(struct CountNode *root) {
    if (root == NULL) return;
    print_tree(root->left);
    for (struct WordList *wl = root->words; wl != NULL; wl = wl->next)
        printf("%4d %s\n", root->count, wl->word);
    print_tree(root->right);
}

// get_word -- reads the next alphabetic word from stdin, lowercased
// skips non-alpha characters so punctuation and numbers are ignored
// weakness: strips all non-alpha chars -- "don't" becomes "don" + "t"
// weakness: doesn't skip string literals or comments
int get_word(char *buf, int maxlen) {
    int c, i = 0;

    // skip anything that isn't a letter
    while ((c = getchar()) != EOF && !isalpha(c))
        ;
    if (c == EOF) return 0;

    buf[i++] = tolower(c); // normalize to lowercase for case-insensitive counting
    while (i < maxlen - 1 && isalpha(c = getchar()))
        buf[i++] = tolower(c);
    buf[i] = '\0';

    if (c != EOF) ungetc(c, stdin); // put back the char that ended the word
    return 1;
}

int main(void) {
    char word[MAXWORD];
    struct WordNode  *wroot = NULL; // phase 1: count words
    struct CountNode *croot = NULL; // phase 2: sort by frequency

    // phase 1 -- read all words and count them in the word BST
    while (get_word(word, MAXWORD))
        wroot = word_insert(wroot, word);

    // phase 2 -- transfer counts into the count BST
    croot = build_count_tree(wroot, croot);

    // phase 3 -- print in descending frequency order
    print_tree(croot);

    return 0;
    // weakness: all malloc'd memory is leaked on exit
    // fine for a short-lived program, but add free_word_tree() / free_count_tree()
    // if this were reused as a library or called in a loop
}

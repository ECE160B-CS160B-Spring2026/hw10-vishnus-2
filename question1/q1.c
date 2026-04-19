#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAXWORD 100

// All C keywords across C89, C99, and C11 standards.
// Must stay in alphabetical order -- binsearch() depends on it.
// If you add a keyword out of order, binsearch() will silently give wrong results.
#define NKEYS (sizeof keytab / sizeof keytab[0])

struct key {
    char *word;
    int count;
} keytab[] = {
    // -- C89 --
    { "auto",           0 },
    { "break",          0 },
    { "case",           0 },
    { "char",           0 },
    { "const",          0 },
    { "continue",       0 },
    { "default",        0 },
    { "do",             0 },
    { "double",         0 },
    { "else",           0 },
    { "enum",           0 },
    { "extern",         0 },
    { "float",          0 },
    { "for",            0 },
    { "goto",           0 },
    { "if",             0 },
    { "int",            0 },  // was missing in the class version
    { "long",           0 },
    { "register",       0 },
    { "return",         0 },
    { "short",          0 },
    { "signed",         0 },
    { "sizeof",         0 },
    { "static",         0 },
    { "struct",         0 },
    { "switch",         0 },
    { "typedef",        0 },
    { "union",          0 },
    { "unsigned",       0 },
    { "void",           0 },
    { "volatile",       0 },
    { "while",          0 },
    // -- C99 --
    { "_Bool",          0 },
    { "_Complex",       0 },
    { "_Imaginary",     0 },
    { "inline",         0 },
    { "restrict",       0 },
    // -- C11 --
    { "_Alignas",       0 },
    { "_Alignof",       0 },
    { "_Atomic",        0 },
    { "_Generic",       0 },
    { "_Noreturn",      0 },
    { "_Static_assert", 0 },
    { "_Thread_local",  0 },
};

// BST node -- each node stores a keyword and how many times it appeared
struct tnode {
    char *word;
    int count;
    struct tnode *left;
    struct tnode *right;
};

// prototypes
int getword(char *, int);
struct key *binsearch(char *, struct key *, int);
struct tnode *addtree(struct tnode *, char *, int);
void treeprint(struct tnode *);
struct tnode *talloc(void);
char *mystrdup(char *);

int main(void) {
    char word[MAXWORD];
    struct key *kp;
    struct tnode *root = NULL;

    while (getword(word, MAXWORD) != EOF) {
        // only check words that could be keywords
        // C11 keywords start with '_', so we allow that too
        if (isalpha(word[0]) || word[0] == '_')
            if ((kp = binsearch(word, keytab, NKEYS)) != NULL) {
                kp->count++;
                // insert into BST, passing updated count to keep it in sync
                root = addtree(root, word, kp->count);
            }
    }

    printf("%-20s %s\n", "Keyword", "Count");
    printf("%-20s %s\n", "-------", "-----");
    treeprint(root);
    return 0;
}

// binsearch -- looks for 'word' in a sorted table of keywords
// returns a pointer to the matching entry, or NULL if not found
// O(log n) -- cuts the search space in half each iteration
// weakness: table must be sorted alphabetically, no runtime check for this
struct key *binsearch(char *word, struct key *tab, int n) {
    int cond;
    struct key *low  = &tab[0];
    struct key *high = &tab[n];
    struct key *mid;

    while (low < high) {
        mid = low + (high - low) / 2; // safer than (low + high) / 2 -- avoids overflow
        if ((cond = strcmp(word, mid->word)) < 0)
            high = mid;         // word is in the left half
        else if (cond > 0)
            low = mid + 1;      // word is in the right half
        else
            return mid;         // found it
    }
    return NULL; // not a keyword
}

// addtree -- inserts a keyword into the BST, or updates its count if already there
// BST is ordered alphabetically by word
// weakness: no balancing -- sorted insertion degenerates to O(n) linked list
struct tnode *addtree(struct tnode *p, char *w, int count) {
    int cond;

    if (p == NULL) {
        // first time seeing this keyword -- create a new node
        p = talloc();
        p->word  = mystrdup(w);
        p->count = count;
        p->left  = p->right = NULL;
    } else if ((cond = strcmp(w, p->word)) == 0) {
        p->count = count;   // already in tree, sync count from keytab
    } else if (cond < 0) {
        p->left  = addtree(p->left,  w, count);  // goes in left subtree
    } else {
        p->right = addtree(p->right, w, count);  // goes in right subtree
    }
    return p;
}

// treeprint -- in-order traversal of the BST
// since the tree is ordered alphabetically, output comes out A-Z
// skips keywords with count 0 (never appeared in input)
void treeprint(struct tnode *p) {
    if (p == NULL)
        return;
    treeprint(p->left);
    if (p->count > 0)
        printf("%-20s %d\n", p->word, p->count);
    treeprint(p->right);
}

// getword -- reads the next word or single character from stdin
// a "word" is any sequence of letters, digits, or underscores
// underscore included so C11 keywords like _Bool and _Atomic are read correctly
// weakness: doesn't skip string literals or comments, so a keyword inside
// "int" or // int will still be counted
int getword(char *word, int lim) {
    int c;
    char *w = word;

    // skip leading whitespace
    while (isspace(c = getchar()))
        ;

    if (c != EOF)
        *w++ = c;

    // if it doesn't start with a letter or underscore, it can't be a keyword
    if (!isalpha(c) && c != '_') {
        *w = '\0';
        return c;
    }

    // keep reading letters, digits, or underscores
    for (; --lim > 0; w++)
        if (!isalnum(*w = getchar()) && *w != '_') {
            ungetc(*w, stdin); // put back the char that ended the word
            break;
        }
    *w = '\0';
    return word[0];
}

// talloc -- allocates memory for a new BST node
struct tnode *talloc(void) {
    return malloc(sizeof(struct tnode));
}

// mystrdup -- makes a heap copy of a string
// stdlib's strdup() does the same but isn't guaranteed in C89
char *mystrdup(char *s) {
    char *p = malloc(strlen(s) + 1); // +1 for null terminator
    if (p != NULL)
        strcpy(p, s);
    return p;
}

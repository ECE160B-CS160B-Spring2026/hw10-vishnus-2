#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHSIZE 101  // using a prime number reduces collisions in the hash table
#define MAXOUT   100  // assuming intersection won't be larger than 100 elements

struct nlist {
    struct nlist *next; // next entry in chain (for handling collisions)
    char *name;         // the integer value stored as a string, e.g. "42"
    char *defn;         // not really needed here, keeping it to match class struct
};

static struct nlist *hashtab[HASHSIZE]; // the actual hash table, array of pointers

// hash -- same function from class, converts a string to a bucket index
// multiplying by 31 is a common trick that spreads values out well
unsigned hash(char *s) {
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    return hashval % HASHSIZE; // mod keeps it within bounds of the table
}

// lookup -- searches the hash table for a string key
// walks the linked list at the hashed bucket to handle collisions
// returns the entry if found, NULL otherwise
struct nlist *lookup(char *s) {
    struct nlist *np;
    for (np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
            return np; // found it
    return NULL;
}

// install -- adds a (name, defn) pair to the hash table
// if the name already exists it just updates defn, otherwise creates a new node
// inserting at the front of the chain since order doesn't matter here
struct nlist *install(char *name, char *defn) {
    struct nlist *np;
    unsigned hashval;

    if ((np = lookup(name)) == NULL) {
        // haven't seen this key before, make a new node
        np = malloc(sizeof(struct nlist));
        if (np == NULL) return NULL;

        np->name = strdup(name); // strdup copies the string onto the heap
        if (np->name == NULL) return NULL;

        hashval = hash(name);
        np->next = hashtab[hashval]; // new node points to whatever was at front
        hashtab[hashval] = np;       // new node becomes the new front
    } else {
        free(np->defn); // name already exists, free old defn before replacing
    }

    np->defn = strdup(defn);
    if (np->defn == NULL) return NULL;
    return np;
}

// int_to_str -- converts an int to a string so we can use it as a hash key
// the hash function works on strings, so we need to convert integers first
char *int_to_str(int n, char *buf, int len) {
    snprintf(buf, len, "%d", n);
    return buf;
}

// intersection -- finds elements that appear in both array1 and array2
// step 1: insert everything from array1 into the hash table
// step 2: for each element in array2, check if it's in the hash table
// step 3: if it is, add it to output and mark it so we don't count it twice
// returns the number of elements in the intersection
int intersection(int output[], int array1[], int lenArr1, int array2[], int lenArr2) {
    char buf[32]; // buffer for int_to_str, 32 chars is plenty for any int
    int outlen = 0;

    // insert all of array1 into the hash table
    // using "1" as a placeholder defn since we only care about membership
    for (int i = 0; i < lenArr1; i++)
        install(int_to_str(array1[i], buf, sizeof(buf)), "1");

    // check each element of array2 against the table
    for (int i = 0; i < lenArr2; i++) {
        struct nlist *np = lookup(int_to_str(array2[i], buf, sizeof(buf)));

        // only count it if it was in array1 AND we haven't counted it already
        // setting defn to NULL after a match is how we mark it as "used"
        // weakness: a cleaner solution would be adding a visited flag to the struct
        if (np != NULL && np->defn != NULL) {
            output[outlen++] = array2[i];
            free(np->defn);
            np->defn = NULL; // mark as seen so duplicates don't get counted again
        }
    }

    return outlen;
}

int main(void) {
    int lenArr1, lenArr2;
    int array1[MAXOUT], array2[MAXOUT], output[MAXOUT];

    // read array1 -- first input is the length, then the values
    scanf("%d", &lenArr1);
    for (int i = 0; i < lenArr1; i++)
        scanf("%d", &array1[i]);

    // read array2 -- same format
    scanf("%d", &lenArr2);
    for (int i = 0; i < lenArr2; i++)
        scanf("%d", &array2[i]);

    int outlen = intersection(output, array1, lenArr1, array2, lenArr2);

    // print the intersection separated by spaces
    // using the (i > 0) check so we don't print a leading space
    for (int i = 0; i < outlen; i++) {
        if (i > 0) printf(" ");
        printf("%d", output[i]);
    }
    printf("\n");

    return 0;
}

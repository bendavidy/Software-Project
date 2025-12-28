#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ------ Tasks: --------
// 1. read the input
// 2. float convertion function
// Y3. variable initialization
// Y4. find_minimal_dist_index function
// S5. update_assignment function
// S6. update_centroids function
// 7. main algorithm loop

// Global variables declerations
int K, iter, N, d;
// struct vector input_data[];
// each line is a linked list
// input_data is a vector of those lists

struct node
{
    double value;
    struct node *next;
};
struct vector
{
    struct vector *next;
    struct node *nodes;
};

// Funtion declerations (prototypes):
void update_assignments(void);
void update_centroids(void);
void find_minimal_dist_index(int i);

// Main
int main(int argc, char *argv[])
{
    // Read params and stdin
    // TODO: add validations
    if (argc != 3)
    {
        // TODO: print out error and exit (C tutorial → atoi)
    }
    K = atoi(argv[1]);
    iter = atoi(argv[2]);
    // printf("%s%d, %s%d", "K = ", K, "iter = ", iter);

    // iterate through stdin buffer
    char *line = NULL;
    size_t *n;
    int counter = 10;
    while (getline(&line, n, stdin) != -1 && counter > 0)
    {
        printf("%s\n", line);
        counter--;
    }
}
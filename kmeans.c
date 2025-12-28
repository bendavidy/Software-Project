#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// --------------- Tasks: ---------------
// Y[X] read the input
//  [ ] float convertion function
// Y[X] variable initialization
// Y[ ] find_minimal_dist_index function
// S[ ] update_assignment function
// S[ ] update_centroids function
//  [ ] main algorithm loop

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

    struct vector *head_vec, *curr_vec, *next_vec;
    struct node *head_node, *curr_node, *next_node;
    int N = 0, d = 0;
    double n;
    char c;

    head_node = malloc(sizeof(struct node));
    curr_node = head_node;
    curr_node->next = NULL;

    head_vec = malloc(sizeof(struct vector));
    curr_vec = head_vec;
    curr_vec->next = NULL;

    while (scanf("%lf%c", &n, &c) == 2)
    {
        N++;
        if (c == '\n')
        {
            curr_node->value = n;
            curr_vec->nodes = head_node;
            curr_vec->next = malloc(sizeof(struct vector));
            curr_vec = curr_vec->next;
            curr_vec->next = NULL;
            head_node = malloc(sizeof(struct node));
            curr_node = head_node;
            curr_node->next = NULL;
            if (d == 0)
            {
                d = N;
            }
            continue;
        }

        curr_node->value = n;
        curr_node->next = malloc(sizeof(struct node));
        curr_node = curr_node->next;
        curr_node->next = NULL;
    }

    N = (int)(N / d);
    iter = atoi(argv[2]);
    K = atoi(argv[1]);
    // printf("%s%d, %s%d, %s%d, %s%d", "K = ", K, "iter = ", iter, "N = ", N, "d = ", d);

    // main loop

    // free memory
    curr_vec = head_vec;
    struct vector *temp_vec;
    struct node *temp_node;
    while (curr_vec != NULL)
    {
        head_node = curr_node = curr_vec->nodes;
        while (curr_node != NULL)
        {
            temp_node = curr_node;
            curr_node = curr_node->next;
            free(temp_node);
        }
        temp_vec = curr_vec;
        curr_vec = curr_vec->next;
        free(temp_vec);
    }
}
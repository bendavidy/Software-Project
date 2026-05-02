#define _GNU_SOURCE // TODO: figure out if we really need that

#include "symnmf.h"
// TODO: what do we need out of those?
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


/* --------------- Global and extern variables declerations --------------- */
extern int N, K, iter, d;
extern double eps;
extern struct vector* head_vec;
// struct vector** assignments; /* assignments[j] points to the first vector in the centroids[j] cluster, connected with next_in_cluster */
// struct vector** centroids; /* centroids will point to the first elem of [vector*,vector*,..,vector*] after we will allocate space later.. */


/* --------------- Function declerations (prototypes) and implementations: --------------- */
void* check_alloc(void* p);
double euclidean_dist(struct node* a, struct node* b);
void free_nodes(struct node* head);
struct node* deep_clone_nodes(struct node* node); /* this function will take */

void* check_alloc(void* p)
{
    if (p == NULL) {
        printf("An Error Has Occurred\n");
        exit(1);
    }
    return p;
}

double euclidean_dist(struct node* a, struct node* b)
{
    int l;
    double sub, sum = 0.0;
    for (l = 0; l < d; l++) {
        sub = (a->value - b->value);
        sub = sub * sub;
        sum += sub;
        a = a->next;
        b = b->next;
    }
    sum = sqrt(sum);
    return sum;
}

void free_nodes(struct node* head)
{
    struct node *curr = head, *tmp;
    while (curr != NULL) {
        tmp = curr;
        curr = curr->next;
        free(tmp);
    }
}

struct node* deep_clone_nodes(struct node* src_node) /* recieve a pointer to some node and return a pointer to some node */
{
    struct node *head, *curr; /*head will point to the first node, curr will point at the end to the last node*/

    /* if(src_node == NULL) return NULL; i'm not sure if that line is needed */

    head = check_alloc(malloc(sizeof(struct node)));

    head->value = src_node->value;
    head->next = NULL;

    curr = head;
    src_node = src_node->next;

    while (src_node != NULL) {
        curr->next = check_alloc(malloc(sizeof(struct node)));
        curr = curr->next;
        curr->value = src_node->value;
        curr->next = NULL;
        src_node = src_node->next;
    }
    return head;
}

void print_vector_nodes(struct node* p)
// TODO: test this
{
    int first;
    first = 1;

    while (p != NULL) {
        if (!first) {
            printf(",");
        }
        printf("%.4f", p->value);
        first = 0;
        p = p->next;
    }
}

double** sym(struct vector* head_vec) {
    // TODO: implement
}

double* ddg(struct vector* head_vec) {
    // TODO: implement
}

double** norm(struct vector* head_vec) {
    // TODO: implement
}

double** symnmf(double** H, double** W) {
    // TODO: implement
}

// parse CMD arguments and print result based on goal
// Defined in 2.2
int main(int argc, char const *argv[])
{
    // TODO: implement
    return 0;
}



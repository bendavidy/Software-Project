#define _GNU_SOURCE

#include "kmeans.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* --------------- Global and extern variables declerations --------------- */
// int N, K, iter, d;
// double eps;
// struct vector* head_vec;
// struct vector** init_centroids;

extern int N, K, iter, d;
extern double eps;
extern struct vector* head_vec;
extern struct vector** init_centroids;
struct vector** assignments; /* assignments[j] points to the first vector in the centroids[j] cluster, connected with next_in_cluster */
struct vector** centroids; /* centroids will point to the first elem of [vector*,vector*,..,vector*] after we will allocate space later.. */

/* --------------- Function declerations (prototypes) and implementations: --------------- */
void* check_alloc(void* p);
double euclidean_dist(struct node* a, struct node* b);
int find_minimal_dist_index(struct vector* elem); /* recieves a pointer to an element (represented by a vector) of dimension d, and returns the index of the closest centroid. */
void free_nodes(struct node* head);
double update_centroids(void); /* updates centroids and returns the max delta in this update iteration */
void update_assignments(void);
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

int find_minimal_dist_index(struct vector* elem)
{
    double min = INFINITY;
    int min_index = 0;
    struct vector* cent;
    int j;

    for (j = 0; j < K; j++) {
        double sum;
        struct node* curr_elem_coor;
        struct node* curr_cent_coor;
        cent = centroids[j];
        /* calculate euclidean distance to centroid[j] */
        sum = 0;
        curr_elem_coor = elem->nodes;
        curr_cent_coor = cent->nodes;
        sum = euclidean_dist(curr_elem_coor, curr_cent_coor);
        if (sum < min) {
            min = sum;
            min_index = j;
        }
    }
    return min_index;
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

double update_centroids()
{
    extern struct vector* head_vec;
    double* coor_sum = check_alloc(calloc(d, sizeof(double)));
    double delta, max_delta = -INFINITY;
    int size; /*,index;*/
    struct vector *curr_cluster_elem, *curr_cent_vec, *old_cent_vec;
    struct node *curr_node, *cent_node;
    int j;

    old_cent_vec = check_alloc(malloc(sizeof(struct vector)));

    for (j = 0; j < K; j++) {
        curr_cent_vec = centroids[j];
        old_cent_vec->nodes = deep_clone_nodes(centroids[j]->nodes);
        cent_node = centroids[j]->nodes;
        /* calculate cluster size */
        size = 0;
        curr_cluster_elem = assignments[j];
        while (curr_cluster_elem != NULL) {
            int l;
            curr_node = curr_cluster_elem->nodes;
            size++;
            /* calculate avarage on every coordinate and update cent_node->value */
            for (l = 0; l < d; l++) {
                coor_sum[l] += curr_node->value;
                curr_node = curr_node->next;
            }
            curr_cluster_elem = curr_cluster_elem->next_in_cluster;
        }
        if (size == 0) {
            /* empty cluster - reassign to first element in data */
            int l;
            curr_node = head_vec->nodes;
            for (l = 0; l < d; l++) {
                cent_node->value = curr_node->value;
                cent_node = cent_node->next;
                curr_node = curr_node->next;
                coor_sum[l] = 0; /* prepare for next iteration */
            }
        } else {
            int l;
            for (l = 0; l < d; l++) {
                cent_node->value = coor_sum[l] / size;
                cent_node = cent_node->next;
                coor_sum[l] = 0; /* prepare for next iteration */
            }
        }

        /* calculate the change delta of centroids[j] */
        delta = euclidean_dist(curr_cent_vec->nodes, old_cent_vec->nodes);
        if (max_delta < delta) {
            max_delta = delta;
        }
        free_nodes(old_cent_vec->nodes);
    }

    free(old_cent_vec);
    free(coor_sum);
    return max_delta;
}

void update_assignments()
{
    /* reset assignments and calculate them again */
    int j;
    int i;
    struct vector* curr_vec;
    extern struct vector* head_vec;
    for (j = 0; j < K; j++) {
        assignments[j] = NULL;
    }

    curr_vec = head_vec;
    for (i = 0; i < N; i++) {
        int index = find_minimal_dist_index(curr_vec);
        curr_vec->next_in_cluster = assignments[index];
        assignments[index] = curr_vec;
        curr_vec = curr_vec->next;
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

/* --------------- Main --------------- */
struct vector** kmeans()
{
    struct vector* curr_vec;
    centroids = init_centroids;
    struct node* head_node;
    struct vector* temp_vec;
    int it;

    assignments = check_alloc(malloc(K * sizeof(struct vector*))); /* assignments[j] points to the element that starts the cluster */

    /* --------------- Main loop --------------- */
    for (it = 0; it < iter; it++) {
        double max_shift;
        update_assignments();
        max_shift = update_centroids();
        if (max_shift < eps)
            break;
    }

    /* --------------- free memory --------------- */
    free(assignments); /* free the array of pointers */

    curr_vec = head_vec;
    while (curr_vec != NULL) {
        head_node = curr_vec->nodes;
        free_nodes(head_node);
        temp_vec = curr_vec;
        curr_vec = curr_vec->next;
        free(temp_vec);
    }

    return centroids;
}
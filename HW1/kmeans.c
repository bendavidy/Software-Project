#define _GNU_SOURCE

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* --------------- Tasks: ---------------
Y[X] read the input
Y[X] variable initialization
Y[X] find_minimal_dist_index function
S[X] update_assignment function
Y[X] update_centroids function
S[X] main algorithm loop
S[X] add convergence condition
Y[X] Input validation and errors
B[X] Tests and memory management
    - Edge cases
    - NULL malloc/calloc operations
    - Examine with Valgrind
*/

/* --------------- Global variables declerations --------------- */
int K, iter, N, d;
double eps = 0.001;

/* each input line (element) is represented by a vector of nodes */
struct node {
    double value;
    struct node* next;
};
struct vector {
    struct vector* next;
    struct vector* next_in_cluster;
    struct node* nodes;
};

struct vector** centroids; /* centroids will point to the first elem of [vector*,vector*,..,vector*] after we will allocate space later.. */
struct vector** assignments; /* assignments[j] points to the first vector in the centroids[j] cluster, connected with next_in_cluster */
struct vector* head_vec; /* indicates the head vector of the input data */


/* --------------- Function declerations (prototypes) and implementations: --------------- */
void update_assignments(void);
double update_centroids(void); /* updates centroids and returns the max delta in this update iteration */
int find_minimal_dist_index(struct vector* elem); /* recieves a pointer to an element (represented by a vector) of dimension d, and returns the index of the closest centroid. */
struct node* deep_clone_nodes(struct node* node); /* this function will take */

void* check_alloc(void* p)
{
    if (p == NULL) {
        printf("An Error Has Occurred\n");
        exit(1);
    }
    return p;
}

double euclidean_dist(struct node *a, struct node *b)
{
    int l;
    double sub, sum = 0.0;
    for (l = 0; l < d; l++)
    {
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
    int min_index;
    struct vector* cent;
    int j;

    for (j = 0; j < K; j++) {
        double sum;
        struct node *curr_elem_coor;
        struct node *curr_cent_coor;
        cent = centroids[j];
        /* calculate euclidean distance to centroid[j] */
        sum = 0;
        curr_elem_coor = elem->nodes;
        curr_cent_coor = cent->nodes;
        sum = euclidean_dist(curr_elem_coor,curr_cent_coor);
        if (sum < min)
        {
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

void print_vector_nodes(struct node* p) /* given a pointer to head of linked list it will print the vector: f1,f2,...,fd */
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

/* --------------- Main --------------- */
int main(int argc, char* argv[])
{
    struct vector *curr_vec, *next_vec; /*, *printed_vec;*/
    struct node *head_node, *curr_node, *final_node; /* next_node; */
    /* struct vector **centroids; centroids will point to the first elem of [vector*,vector*,..,vector*] after we will allocate space later.. */
    double n;
    char c;
    struct vector *temp_vec;
    int k;
    int it;
    char* end;

    if (argc != 3) {
        if (argc == 2) {
            iter = 400;
        } else {
            printf("%s", "An Error Has Occurred\n");
            exit(1);
        }
    } else {
        iter = strtol(argv[2], &end, 10);
        if ((end == argv[2]) || (*end != '\0') || (iter <= 1) || (iter >= 800)) {
            printf("%s", "Incorrect maximum iteration!\n");
            exit(1);
        }
    }

    head_node = check_alloc(malloc(sizeof(struct node)));
    curr_node = head_node;
    curr_node->next = NULL;

    head_vec = check_alloc(malloc(sizeof(struct vector)));
    curr_vec = head_vec;
    curr_vec->next = NULL;

    while (scanf("%lf%c", &n, &c) == 2) {
        N++;
        if (c == '\n') {
            curr_node->value = n;
            curr_vec->nodes = head_node;
            curr_vec->next = check_alloc(malloc(sizeof(struct vector)));
            curr_vec = curr_vec->next;

            curr_vec->next = NULL;
            curr_vec->next_in_cluster = NULL;
            curr_vec->nodes = NULL; /*New line-Shalev*/

            head_node = check_alloc(malloc(sizeof(struct node)));
            curr_node = head_node;
            curr_node->next = NULL;
            if (d == 0) {
                d = N;
            }
            continue;
        }

        curr_node->value = n;
        curr_node->next = check_alloc(malloc(sizeof(struct node)));
        curr_node = curr_node->next;
        curr_node->next = NULL;
    }
    final_node = head_node;

    N = (int)(N / d);
    K = strtol(argv[1], &end, 10);
    if ((end == argv[1]) || (*end != '\0') || (K <= 1) || (K >= N)) {
        printf("%s", "Incorrect number of clusters!\n");
        exit(1);
    }

    /* assignments = malloc(N * sizeof(int)); */
    assignments = check_alloc(malloc(K * sizeof(struct vector*))); /* assignments[j] points to the element that starts the cluster */
    centroids = check_alloc(malloc(K * sizeof(struct vector*))); /* allocate K spaces, now centroids is pointing to the first elem of [vector*,vector*,..,vector*] */
    if ((assignments == NULL) || (centroids == NULL)) {
        printf("%s", "An Error Has Occurred\n");
        exit(1);
    }
    next_vec = head_vec;

    for (k = 0; k < K; k++) {
        /* do we need to make sure K < number of data points ? I assume it is true */
        centroids[k] = check_alloc(malloc(sizeof(struct vector))); /* each elem in [vector*,vector*,..,vector*] will have valid adress (we can store in each adress vector) */
        centroids[k]->next = NULL; /* each vector will be independent from the other in centroids */
        centroids[k]->nodes = deep_clone_nodes(next_vec->nodes); /* creating deep copy for each linked list */
        centroids[k]->next_in_cluster = NULL;                    /* each vector will be independent from the other in centroids */
        next_vec = next_vec->next;
    }

    /* --------------- Main loop --------------- */
    for (it = 0; it < iter; it++) {
        double max_shift;
        update_assignments();
        max_shift = update_centroids();
        if (max_shift < eps)
            break;
    }

    /* print output onscreen */
    for (k = 0; k < K; k++) {
        print_vector_nodes(centroids[k]->nodes);
        printf("\n");
    }

    /* --------------- free memory --------------- */
    for (k = 0; k < K; k++) {
        free_nodes(centroids[k]->nodes); /* free the linked list of the k'th vector */
        free(centroids[k]); /* free the k'th vector */
    }
    free(centroids); /* free the array of pointers */
    free(assignments); /* free the array of pointers */

    curr_vec = head_vec;
    while (curr_vec != NULL) {
        head_node = curr_vec->nodes;
        free_nodes(head_node);
        temp_vec = curr_vec;
        curr_vec = curr_vec->next;
        free(temp_vec);
    }
    free(final_node);

    return 0;
}
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
B[ ] Tests and memory management
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

int find_minimal_dist_index(struct vector *elem) /* TODO: test */
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
        /*
        for (int l = 0; l < d; l++)
        {
            sub = (curr_elem_coor->value - curr_cent_coor->value);
            sub = sub * sub;
            sum += sub;
            curr_elem_coor = curr_elem_coor->next;
            curr_cent_coor = curr_cent_coor->next;
        }
            */
        sum = 0;
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

double update_centroids() /* TODO: test */
{
    double* coor_sum = calloc(d, sizeof(double));
    double delta, max_delta = -INFINITY;
    int size; /*,index;*/
    struct vector *curr_cluster_elem, *curr_cent_vec, *old_cent_vec;
    struct node *curr_node, *cent_node;
    int j;

    old_cent_vec = malloc(sizeof(struct vector));

    for (j = 0; j < K; j++) {
        curr_cent_vec = centroids[j];
        old_cent_vec->nodes = deep_clone_nodes(centroids[j]->nodes);
        cent_node = centroids[j]->nodes;
        /* calculate cluster size */
        size = 0;
        curr_cluster_elem = assignments[j];
        while (curr_cluster_elem != NULL) {
            /*int sum;*/
            int l;
            curr_node = curr_cluster_elem->nodes;
            size++;
            /*sum = 0;*/
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
            for (l = 0; l < d; l++) {
                cent_node->value = head_vec->nodes->value;
                cent_node = cent_node->next;
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
        delta = euclidean_dist(curr_cent_vec->nodes, old_cent_vec->nodes); /*TODO: implement*/
        if (max_delta < delta) {
            max_delta = delta;
        }
        free_nodes(old_cent_vec->nodes);
    }

    free(old_cent_vec);
    free(coor_sum);
    return max_delta;
}

void update_assignments() /* TODO: test */
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

    head = malloc(sizeof(struct node));

    head->value = src_node->value;
    head->next = NULL;

    curr = head;
    src_node = src_node->next;

    while (src_node != NULL) {
        curr->next = malloc(sizeof(struct node));
        curr = curr->next;
        curr->value = src_node->value;
        curr->next = NULL;
        src_node = src_node->next;
    }
    return head;
}

struct vector **deep_copy_centroids(struct vector **centroids)
{ 
    int j;
    struct vector **deep_copy;
    deep_copy = malloc(K * sizeof(struct vector*));

    if (deep_copy == NULL)
    {
        return NULL;
    }

    for(j = 0; j < K; j++)
    {
        deep_copy[j] = malloc(sizeof(struct vector));
        if(deep_copy == NULL)
        {
            return NULL;
        }
        deep_copy[j]->next = NULL;
        deep_copy[j]->next_in_cluster = NULL;
        deep_copy[j]->nodes = deep_clone_nodes(centroids[j]->nodes);
    }
    return deep_copy;
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
    struct node *head_node, *curr_node; /* next_node; */
    /* struct vector **centroids; centroids will point to the first elem of [vector*,vector*,..,vector*] after we will allocate space later.. */
    /* int N = 0, d = 0, iter = 0, K = 0; */
    double n;
    char c;
    struct vector *temp_vec;
    int k;
    int it;
    char* end;
    /*struct vector **old_centroids; TODO*/

    /* struct node *temp_node; */
    /*
    Read params and stdin
    TODO: add validations
    */

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

    head_node = malloc(sizeof(struct node));
    curr_node = head_node;
    curr_node->next = NULL;

    head_vec = malloc(sizeof(struct vector));
    curr_vec = head_vec;
    curr_vec->next = NULL;

    while (scanf("%lf%c", &n, &c) == 2) {
        N++;
        if (c == '\n') {
            curr_node->value = n;
            curr_vec->nodes = head_node;
            curr_vec->next = malloc(sizeof(struct vector));
            curr_vec = curr_vec->next;
            curr_vec->next = NULL;
            curr_vec->next_in_cluster = NULL;
            curr_vec->nodes = NULL; /*New line-Shalev*/
            head_node = malloc(sizeof(struct node));
            curr_node = head_node;
            curr_node->next = NULL;
            if (d == 0) {
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
    K = strtol(argv[1], &end, 10);
    if ((end == argv[1]) || (*end != '\0') || (K <= 1) || (K >= N)) {
        printf("%s", "Incorrect number of clusters!\n");
        exit(1);
    }

    /* assignments = malloc(N * sizeof(int)); */
    assignments = malloc(K * sizeof(struct vector*)); /* assignments[j] points to the element that starts the cluster */
    centroids = malloc(K * sizeof(struct vector*)); /* allocate K spaces, now centroids is pointing to the first elem of [vector*,vector*,..,vector*] */
    if ((assignments == NULL) || (centroids == NULL)) {
        printf("%s", "An Error Has Occurred\n");
        exit(1);
    }
    next_vec = head_vec;

    for (k = 0; k < K; k++) {
        /* do we need to make sure K < number of data points ? I assume it is true */
        centroids[k] = malloc(sizeof(struct vector)); /* each elem in [vector*,vector*,..,vector*] will have valid adress (we can store in each adress vector) */
        centroids[k]->next = NULL; /* each vector will be independent from the other in centroids */
        centroids[k]->nodes = deep_clone_nodes(next_vec->nodes); /* creating deep copy for each linked list */
        centroids[k]->next_in_cluster = NULL;                    /* each vector will be independent from the other in centroids */
        next_vec = next_vec->next;
    }

    /* Prints for tests */

   
    /*
        printed_vec = head_vec;
        printf("VECTORS:\n");
        for (int k = 0; k < 10; k++) {
            printf("vector %d: ", k);
            print_vector_nodes(printed_vec->nodes);
            printf("\n");
            printed_vec = printed_vec->next;
    }
    
    
    */
    

    /* --------------- Main loop --------------- */

    /* TODO: implement */
    for (it = 0; it < iter; it++) {
        double max_shift;
        /*int j;*/
        /*old_centroids = deep_copy_centroids(centroids);*/
        update_assignments();
        max_shift = update_centroids();
        /*
            max_shift = -INFINITY;
        for(j = 0; j < K; j++)
        {
            double shift = euclidean_dist(old_centroids[j]->nodes,centroids[j]->nodes);
            if(shift > max_shift)
            {
                max_shift = shift;
            }

        }
        if (max_shift < eps)
            break;
        */
        if (max_shift < eps)
            break;
    }

    for (k = 0; k < K; k++) {
        /* printf("centroid %d: ", k); */
        print_vector_nodes(centroids[k]->nodes);
        printf("\n");
    }

        /*
        for i in range(iter):

    old_centroids = [c[:] for c in centroids]
    update_assignment()
    update_centroids()
    
    max_shift = float("-inf")

    for j in range(K):
        shift = math.dist(old_centroids[j],centroids[j])
        if shift > max_shift:
            max_shift = shift

    if max_shift < eps:
        break

for j in range(K):
    print(",".join(f"{x:.4f}" for x in centroids[j]))
        */



    


    /* --------------- free memory --------------- */
    for (k = 0; k < K; k++) {
        free_nodes(centroids[k]->nodes); /* free the linked list of the k'th vector */
        free(centroids[k]); /* free the k'th vector */
    }
    free(centroids); /* free the array of pointers.. */

    curr_vec = head_vec;
    while (curr_vec != NULL) {
        head_node = curr_vec->nodes;
        free_nodes(head_node);

        /*
        while (curr_node != NULL)
        {
            temp_node = curr_node;
            curr_node = curr_node->next;
            free(temp_node);
        }
        */
        temp_vec = curr_vec;
        curr_vec = curr_vec->next;
        free(temp_vec);
    }
    free(assignments); 

    return 0;
}
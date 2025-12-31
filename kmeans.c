#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* --------------- Tasks: ---------------
Y[X] read the input
Y[X] variable initialization
Y[X] find_minimal_dist_index function
S[X] update_assignment function
Y[ ] update_centroids function
S[ ] main algorithm loop
S[ ] add convergence condition
?[ ] Input validation and errors
 [ ] Test memory management
*/

/* --------------- Global variables declerations --------------- */
int K, iter, N, d;

/* each input line (element) is represented by a vector of nodes */
struct node
{
    double value;
    struct node *next;
};
struct vector
{
    struct vector *next;
    struct vector *next_in_cluster;
    struct node *nodes;
};

struct vector **centroids; /* centroids will point to the first elem of [vector*,vector*,..,vector*] after we will allocate space later.. */
struct vector **assignments;
struct vector *head_vec; /* indicates the head vector of the input data */

/* --------------- Function declerations (prototypes) and implementations: --------------- */
void update_assignments(void);
void update_centroids(void);
int find_minimal_dist_index(struct vector *elem); /* recieves a pointer to an element (represented by a vector) of dimension d, and returns the index of the closest centroid. */
struct node *deep_clone_nodes(struct node *node); /* this function will take */

int find_minimal_dist_index(struct vector *elem) /* TODO: test */
{
    double min = INFINITY;
    int min_index;
    struct vector *cent;

    for (int j = 0; j < K; j++)
    {
        cent = centroids[j];
        /* calculate euclidean distance to centroid[j] */
        int sub, sum = 0;
        struct node *curr_elem_coor = elem->nodes;
        struct node *curr_cent_coor = cent->nodes;
        for (int l = 0; l < d; l++)
        {
            sub = (curr_elem_coor->value - curr_cent_coor->value);
            sub = sub * sub;
            sum += sub;
            curr_elem_coor = curr_elem_coor->next;
            curr_cent_coor = curr_cent_coor->next;
        }
        sum = sqrt(sum);
        if (sum < min)
        {
            min = sum;
            min_index = j;
        }
    }
    return min_index;
}

void free_nodes(struct node *head)
{
    struct node *curr = head, *tmp;
    while (curr != NULL)
    {
        tmp = curr;
        curr = curr->next;
        free(tmp);
    }
}

void update_centroids()
{
    for (int j = 0; j < K; j++)
    {
        struct node *cent_node = centroids[j]->nodes;
        /* calculate cluster size */
        int size = 0;
        struct vector *curr = assignments[j];
        while (curr != NULL)
        {
            size++;
            curr = curr->next_in_cluster;
        }
        if (size == 0)
        {
            /* TODO */
            /* empty cluster - reassign to first element in data */
            continue;
        }
        /* calculate avarage on every coordinate and update cent_node->value */
        int sum = 0;
        for (int l = 0; l < d; l++)
        {
            /* TODO */
        }
    }
}

void update_assignments() /* TODO: test */
{
    /* reset assignments and calculate them again */
    for (int j = 0; j < K; j++)
    {
        assignments[j] = NULL;
    }
    struct vector *curr_vec;
    curr_vec = head_vec;
    for (int i = 0; i < N; i++)
    {
        int index = find_minimal_dist_index(curr_vec);
        curr_vec->next_in_cluster = assignments[index];
        assignments[index] = curr_vec;
        curr_vec = curr_vec->next;
    }
}

struct node *deep_clone_nodes(struct node *src_node) /* recieve a pointer to some node and return a pointer to some node */
{
    struct node *head, *curr; /*head will point to the first node, curr will point at the end to the last node*/

    /* if(src_node == NULL) return NULL; i'm not sure if that line is needed */

    head = malloc(sizeof(struct node));

    head->value = src_node->value;
    head->next = NULL;

    curr = head;
    src_node = src_node->next;

    while (src_node != NULL)
    {
        curr->next = malloc(sizeof(struct node));
        curr = curr->next;
        curr->value = src_node->value;
        curr->next = NULL;
        src_node = src_node->next;
    }
    return head;
}

void print_vector_nodes(struct node *p) /* given a pointer to head of linked list it will print the vector: f1,f2,...,fd */
{
    int first;
    first = 1;

    while (p != NULL)
    {
        if (!first)
        {
            printf(",");
        }
        printf("%.4f", p->value);
        first = 0;
        p = p->next;
    }
}

/* --------------- Main --------------- */
int main(int argc, char *argv[])
{
    struct vector *curr_vec, *next_vec, *printed_vec;
    struct node *head_node, *curr_node; /* next_node; */
    /* struct vector **centroids; centroids will point to the first elem of [vector*,vector*,..,vector*] after we will allocate space later.. */
    /* int N = 0, d = 0, iter = 0, K = 0; */
    double n;
    char c;
    struct vector *temp_vec;
    /* struct node *temp_node; */
    /*
    Read params and stdin
    TODO: add validations
    */

    if (argc != 3)
    { /*
         TODO: print out error and exit (C tutorial → atoi)
      */
    }

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
            curr_vec->next_in_cluster = NULL;
            curr_vec->nodes = NULL; /*New line-Shalev*/
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
    iter = iter + 0;
    K = atoi(argv[1]);
    /* printf("%s%d, %s%d, %s%d, %s%d", "K = ", K, "iter = ", iter, "N = ", N, "d = ", d); */

    /* assignments = malloc(N * sizeof(int)); */
    assignments = malloc(K * sizeof(struct vector *)); /* assignments[j] points to the element that starts the cluster */
    centroids = malloc(K * sizeof(struct vector *));   /* allocate K spaces, now centroids is pointing to the first elem of [vector*,vector*,..,vector*] */
    next_vec = head_vec;

    for (int k = 0; k < K; k++)
    {
        /* do we need to make sure K < number of data points ? I assume it is true */
        centroids[k] = malloc(sizeof(struct vector));            /* each elem in [vector*,vector*,..,vector*] will have valid adress (we can store in each adress vector) */
        centroids[k]->next = NULL;                               /* each vector will be independent from the other in centroids */
        centroids[k]->nodes = deep_clone_nodes(next_vec->nodes); /* creating deep copy for each linked list */
        next_vec = next_vec->next;
    }

    /* Prints for tests */
    printf("CENTROIDS:\n"); /* print the centroids.. centroid \n centroid \n ... */
    for (int k = 0; k < K; k++)
    {
        printf("centroid %d: ", k);
        print_vector_nodes(centroids[k]->nodes);
        printf("\n");
    }

    printed_vec = head_vec;
    printf("VECTORS:\n");
    for (int k = 0; k < 10; k++)
    {
        printf("vector %d: ", k);
        print_vector_nodes(printed_vec->nodes);
        printf("\n");
        printed_vec = printed_vec->next;
    }

    /* --------------- Main loop --------------- */

    /* TODO: implement */

    /* --------------- free memory --------------- */
    for (int k = 0; k < K; k++)
    {
        free_nodes(centroids[k]->nodes); /* free the linked list of the k'th vector */
        free(centroids[k]);              /* free the k'th vector */
    }
    free(centroids); /* free the array of pointers.. */

   
    curr_vec = head_vec;
    while (curr_vec != NULL)
    {
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
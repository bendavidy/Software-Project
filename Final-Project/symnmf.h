#ifndef SYMNMF_H_
#define SYMNMF_H_


struct node {
    double value;
    struct node* next;
};
struct vector {
    struct vector* next;
    struct node* nodes;
};

/* all space allocation should go through this */
void* check_alloc(void* p);

/* Freeing all the nodes reachable from head, including itself */
void free_nodes(struct node* head);

/* Deep cloning all the nodes reachable from head, including itself */
struct node* deep_clone_nodes(struct node* node);

/* TODO: fix these documentations */
/* Calculating the Similarity Matrix based on the N data points starting in the head node. */
/* Defined in 1.1 */
double** sym(double** C_in);

/* Calculating the Diagonal Degree Matrix based on sym. */
/* the DDG is a 1-dim array of doubles (of length N) representing the diagonal itself. */
/* Defined in 1.2 */
double* ddg(double** C_in);

/* Calculating the Normalized Similarity Matrix based on sym and ddg. */
/* Defined in 1.3 */
double** norm(double** C_in);

/* Calculating the Decomposition Matrix H that associates each data point to a cluster.  */
/* Called ONLY from Python, after calculating H_0 and W. */
/* Defined in 1.4 */
double** symnmf(double** H, double** W);

/* given a pointer to head of linked list it will print the vector: f1,f2,...,fd */
void print_vector_nodes(struct node* p);

#endif
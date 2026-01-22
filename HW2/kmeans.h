#ifndef KMEANS_H_
#define KMEANS_H_


struct node {
    double value;
    struct node* next;
};
struct vector {
    struct vector* next;
    struct vector* next_in_cluster;
    struct node* nodes;
};

// --------------- Function decleration ---------------
void* check_alloc(void* p);
struct vector** kmeans();
// double euclidean_dist(struct node* a, struct node* b);
// int find_minimal_dist_index(struct vector* elem); /* recieves a pointer to an element (represented by a vector) of dimension d, and returns the index of the closest centroid. */
void free_nodes(struct node* head);
// double update_centroids(void); /* updates centroids and returns the max delta in this update iteration */
// void update_assignments(void);
struct node* deep_clone_nodes(struct node* node); /* this function will take */
// void print_vector_nodes(struct node* p); /* given a pointer to head of linked list it will print the vector: f1,f2,...,fd */

#endif
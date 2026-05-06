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

double **A, **W;
double* D;

// struct vector** assignments; /* assignments[j] points to the first vector in the centroids[j] cluster, connected with next_in_cluster */
// struct vector** centroids; /* centroids will point to the first elem of [vector*,vector*,..,vector*] after we will allocate space later.. */

/* --------------- Function declerations (prototypes) and implementations: --------------- */
void* check_alloc(void* p);
double euclidean_dist_vec(struct node* a, struct node* b);
double euclidean_dist_squared(double* a, double* b);
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

double euclidean_dist_vec(struct node* a, struct node* b) {
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

double euclidean_dist_squared(double* a, double* b) {
    double sum = 0.0;
    for (int i = 0; i < d; i++) {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
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

// Calculating Frobenius norm (without the sqrt) of a NxK matrix
double frob_squared(double** M) {
    double sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < K; j++) {
            sum += (M[i][j]) * (M[i][j]);
        }
    }
    return sum;
}

double** sym(double** C_in) {
    A = check_alloc(malloc(N * sizeof(double*)));
    for (int i = 0; i < N; i++) {
        A[i] = check_alloc(malloc(N * sizeof(double)));
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = (i == j) ? exp(-euclidean_dist_squared(C_in[i], C_in[j]) / 2) : 0;
        }
    }

    return A;
}

double* ddg(double** C_in) {
    A = sym(C_in);
    D = check_alloc(malloc(N * sizeof(double)));
    double sum = 0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            sum += A[i][j];
        }
        D[i] = sum;
        sum = 0;
    }

    return D;
}

double** norm(double** C_in) {
    // W_ij = (d_i)(d_j)(A_ij)
    D = ddg(C_in);
    W = check_alloc(malloc(N * sizeof(double*)));
    for (int i = 0; i < N; i++) {
        W[i] = check_alloc(malloc(N * sizeof(double)));
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            W[i][j] = (D[i] * D[j]) * A[i][j];
        }
    }

    return W;
}

double** symnmf(double** H, double** W) { // This W is unrelated to the global W. This is what we got from Python
    double **H_new, **H_old, **temp, numerator = 0, denominator = 1e-6, mid_sum = 0;

    // H_old and H_new are being set in reverse roles because we swap them at the start of the iteration (for memory efficiency)
    H_old = check_alloc(malloc(N * sizeof(double*)));
    for (int i = 0; i < N; i++) {
        H_old[i] = check_alloc(malloc(K * sizeof(double)));
    }
    H_new = H;

    for (int k = 0; k < iter; k++) {
        temp = H_new;
        H_old = temp;
        H_new = H_old;
        // calculate H_new. add 1e-6 to denominator to avoid division by 0 (from class forum)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < K; j++) {
                // Calculating numerator & denominator
                for (int m = 0; m < N; m++) {
                    for (int l = 0; l < N; l++) {
                        numerator += (W[i][l] * H_old[l][j]);
                        mid_sum += (H_old[i][l] * H_old[m][l]);
                    }
                    denominator += (mid_sum * H_old[m][j]);
                }

                H_new[i][j] = H_old[i][j] * (0.5 + 0.5 * numerator / denominator);

                numerator = 0;
                denominator = 1e-6;
                mid_sum = 0;
            }
        }

        // H_old <= H_new - H_old   // (for eps condition check)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < K; j++) {
                H_old[i][j] = H_new[i][j] - H_old[i][j];
            }
        }

        if (frob_squared(H_old) < eps) {
            break;
        }
    }

    return H_new;
}

// parse CMD arguments and print result based on goal
// Defined in 2.2
int main(int argc, char const *argv[])
{
    // TODO: implement
    return 0;
}



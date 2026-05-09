#define _GNU_SOURCE /* TODO: figure out if we really need that */

#include "symnmf.h"
/* TODO: what do we need out of those? */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --------------- Global and extern variables declerations --------------- */
/* TODO: this was set to avoid compilation error while testing. figure out how to do this when submitting */
int N, K = 0, iter = 0, d = 0;
double eps = 0;
struct vector* head_vec;
/* extern int N, K, iter, d; */
/* extern double eps; */
/* extern struct vector* head_vec; */

double **A, **W;
double* D;

/* --------------- Function declerations (prototypes) and implementations: --------------- */
void* check_alloc(void* p);
/* double euclidean_dist_vec(struct node* a, struct node* b); */
double euclidean_dist_squared(double* a, double* b);
void free_nodes(struct node* head);
struct node* deep_clone_nodes(struct node* node); /* this function will take */
double** convert_vector_points_to_matrix(struct vector* head_vec, int first_dim, int second_dim);

void* check_alloc(void* p)
{
    if (p == NULL) {
        printf("An Error Has Occurred\n");
        exit(1);
    }
    return p;
}

double euclidean_dist_squared(double* a, double* b) {
    double sum = 0.0;
    int i;
    for (i = 0; i < d; i++) {
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

int check_file_extension(char* filename, char* ext) {
    char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        return 0;
    }
    return strcmp(dot + 1, ext) == 0;
}

void print_vector_nodes(struct node* p) {
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

void print_double_matrix(double** mat, int first_dim, int second_dim) {
    int i, j;
    for (i = 0; i < first_dim; i++) {
        printf("%.4f", mat[i][0]);
        for (j = 1; j < second_dim; j++) {
            printf(",%.4f", mat[i][j]);
        }
        printf("\n");
    }
}

double** convert_vector_points_to_matrix(struct vector* head_vec, int first_dim, int second_dim) {
    double** mat = check_alloc(malloc(first_dim * sizeof(double*)));
    struct node* curr_node;
    struct vector* curr_vec = head_vec;

    int i, j;
    for (i = 0; i < first_dim; i++) {
        curr_node = curr_vec->nodes;
        mat[i] = check_alloc(malloc(second_dim * sizeof(double)));
        for (j = 0; j < second_dim; j++) {
            mat[i][j] = curr_node->value;
            curr_node = curr_node->next;
        }
        curr_vec = curr_vec->next;
    }

    return mat;
}

void free_mat(double** mat) {
    int i;
    for (i = 0; i < N; i++) {
        free(mat[i]);
    }
    free(mat);
}

/* Calculating Frobenius norm (without the sqrt) of a NxK matrix */
double frob_squared(double** M) {
    double sum = 0;
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < K; j++) {
            sum += (M[i][j]) * (M[i][j]);
        }
    }
    return sum;
}

double** sym(double** C_in) {
    int i, j;
    printf("N = %i, d = %i\n", N, d);
    A = check_alloc(malloc(N * sizeof(double*)));
    for (i = 0; i < N; i++) {
        A[i] = check_alloc(malloc(N * sizeof(double)));
    }

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            /* if (i == j) { */
            /*     A[i][j] = 0; */
            /* } else { */
            /*     A[i][j] = exp(-euclidean_dist_squared(C_in[i], C_in[j]) / 2); */
            /* } */
            A[i][j] = (i != j) ? exp(-euclidean_dist_squared(C_in[i], C_in[j]) / 2) : 0;
        }
    }

    return A;
}

double* ddg(double** C_in) {
    double sum = 0;
    int i, j;
    A = sym(C_in);
    D = check_alloc(malloc(N * sizeof(double)));

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            sum += A[i][j];
        }
        D[i] = sum;
        sum = 0;
    }

    return D;
}

double** norm(double** C_in) {
    /* W_ij = (d_i^-1/2)(d_j^-1/2)(A_ij) */
    int i, j;
    D = ddg(C_in);
    W = check_alloc(malloc(N * sizeof(double*)));
    for (i = 0; i < N; i++) {
        W[i] = check_alloc(malloc(N * sizeof(double)));
    }

    for (i = 0; i < N; i++) {
        // D[i] = pow(D[i], -1 / 2);
        D[i] = 1 / sqrt(D[i]);
    }

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            W[i][j] = (D[i] * D[j]) * A[i][j];
        }
    }

    return W;
}

double** symnmf(double** H, double** W) { /* This W is unrelated to the global W. This is what we got from Python */
    double **H_new, **H_old, **temp, numerator = 0, denominator = 1e-6, mid_sum = 0;
    int i, j, k, m, l;

    /* H_old and H_new are being set in reverse roles because we swap them at the start of the iteration (for memory efficiency) */
    H_old = check_alloc(malloc(N * sizeof(double*)));
    for (i = 0; i < N; i++) {
        H_old[i] = check_alloc(malloc(K * sizeof(double)));
    }
    H_new = H;

    for (k = 0; k < iter; k++) {
        temp = H_old;
        H_old = H_new;
        H_new = temp;
        /* calculate H_new. add 1e-6 to denominator to avoid division by 0 (from class forum) */
        for (i = 0; i < N; i++) {
            for (j = 0; j < K; j++) {
                /* Calculating numerator & denominator */
                for (m = 0; m < N; m++) {
                    for (l = 0; l < N; l++) {
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

        /* H_old <= H_new - H_old  (for eps condition check) */
        for (i = 0; i < N; i++) {
            for (j = 0; j < K; j++) {
                H_old[i][j] = H_new[i][j] - H_old[i][j];
            }
        }

        if (frob_squared(H_old) < eps) {
            break;
        }
    }

    return H_new;
}

/* parse CMD arguments and print result based on goal */
/* Defined in 2.2 */
int main(int argc, char* argv[]) {
    struct vector* curr_vec;
    struct node *head_node, *curr_node, *final_node;
    double n;
    char c;
    struct vector* temp_vec;
    char *goal, *filename;
    FILE* input_file;
    double** input_mat;
    int i, j;

    if (argc != 3) {
        /* TODO: clean error message */
        printf("%s", "ERROR 1\n");
        printf("%s", "An Error Has Occurred\n");
        exit(1);
    } else {
        goal = argv[1];
        filename = argv[2];
        if (!check_file_extension(filename, "txt")) {
            /* TODO: clean error message */
            printf("%s", "ERROR 2\n");
            printf("%s", "An Error Has Occurred\n");
            exit(1);
        }
    }

    if (strcmp(goal, "sym") != 0 && strcmp(goal, "ddg") != 0 && strcmp(goal, "norm") != 0) {
        /* TODO: clean error message */
        printf("%s", "ERROR 3\n");
        printf("%s", "An Error Has Occurred\n");
        exit(1);
    }

    if ((input_file = fopen(filename, "r")) == NULL) {
        /* TODO: clean error message */
        printf("%s", "ERROR 4\n");
        printf("%s", "An Error Has Occurred\n");
        exit(1);
    }

    head_node = check_alloc(malloc(sizeof(struct node)));
    curr_node = head_node;
    curr_node->next = NULL;

    head_vec = check_alloc(malloc(sizeof(struct vector)));
    curr_vec = head_vec;
    curr_vec->next = NULL;

    while (fscanf(input_file, "%lf%c", &n, &c) == 2) {
        N++;
        if (c == '\n') {
            curr_node->value = n;
            curr_vec->nodes = head_node;
            curr_vec->next = check_alloc(malloc(sizeof(struct vector)));
            curr_vec = curr_vec->next;

            curr_vec->next = NULL;
            curr_vec->nodes = NULL;

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

    input_mat = convert_vector_points_to_matrix(head_vec, N, d);

    if (strcmp(goal, "sym") == 0) {
        A = sym(input_mat);
        print_double_matrix(A, N, N);

    } else if (strcmp(goal, "ddg") == 0) {
        D = ddg(input_mat);
        /* printing D */
        for (i = 0; i < N - 1; i++) {
            for (j = 0; j < N - 1; j++) {
                if (i == j) {
                    printf("%.4f,", D[i]);
                } else {
                    printf("%.4f,", 0.0);
                }
            }
            printf("%.4f\n", 0.0);
        }
        for (j = 0; j < N - 1; j++) {
            printf("%.4f,", 0.0);
        }
        printf("%.4f\n", D[N - 1]);

    } else if (strcmp(goal, "norm") == 0) {
        W = norm(input_mat);
        print_double_matrix(W, N, N);
    }

    /* --------------- free memory --------------- */
    curr_vec = head_vec;
    while (curr_vec != NULL) {
        head_node = curr_vec->nodes;
        free_nodes(head_node);
        temp_vec = curr_vec;
        curr_vec = curr_vec->next;
        free(temp_vec);
    }
    free(final_node);

    free_mat(input_mat);

    if (A)
        free_mat(A);
    if (D)
        free(D);
    if (W)
        free_mat(W);

    return 0;
}

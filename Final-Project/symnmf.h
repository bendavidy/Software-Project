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

/* Helper function for making sure the input file name has the 'txt' extension. */
int check_file_extension(char* filename, char* ext);

/* Calculates the squared Euclidean norm between two vectors of size d */
double euclidean_dist_squared(double* a, double* b);

/* Calculating Frobenius norm (without the sqrt) of a NxK matrix */
double frob_squared(double** M);

/* Converts struct vector points to a double** matrix, of dimensions first_dim and second_dim */
double** convert_vector_points_to_matrix(struct vector* head_vec, int first_dim, int second_dim);

/* Prints a NxN double** matrix onscreen */
void print_double_matrix(double** mat);

/* Checks whether an allocation was done successfully or not */
void* check_alloc(void* p);

/* Freeing all the nodes reachable from head, including itself */
void free_nodes(struct node* head);

/* Freeing a double** matrix of N rows */
void free_mat(double** mat);

/* Calculating the Similarity Matrix based on the N data points */
double** sym(double** C_in);

/* Calculating the Diagonal Degree Matrix based on sym. */
/* the DDG is a 1-dim array of doubles (of length N) representing the diagonal itself. */
double* ddg(double** C_in);

/* Calculating the Normalized Similarity Matrix based on sym and ddg. */
double** norm(double** C_in);

/* Calculating the Decomposition Matrix H that associates each data point to a cluster.  */
/* Called only from Python, after calculating H_0 and W. */
double** symnmf(double** H, double** W);

#endif
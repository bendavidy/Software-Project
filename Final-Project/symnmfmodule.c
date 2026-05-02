#define PY_SSIZE_T_CLEAN
#include "symnmf.h"
#include <Python.h> // still works with the warning

// --------------- Global variables ---------------
int N, K, d, iter = 300;
double eps = 1e-4;
struct vector* head_vec;
struct node* head_node;

#pragma region C Functions

// Given Python points and their dimensions, converts them to C vectors and returns head_vec and final_node for cleanup.
struct vector* convert_py_points_to_vectors(PyObject* points, int first_dim, int second_dim) {
    struct node* curr_node;
    struct vector* curr_vec;
    double node_value;
    PyObject* elem;

    head_node = check_alloc(malloc(sizeof(struct node)));
    curr_node = head_node;
    curr_node->next = NULL;

    head_vec = check_alloc(malloc(sizeof(struct vector)));
    curr_vec = head_vec;
    curr_vec->next = NULL;

    for (int i = 0; i < first_dim; i++) {
        elem = PyList_GetItem(points, i);
        // printf("%s%d%s%d%s%f\n", "i = ", i, ", first value of elem ", i, " is ", PyFloat_AsDouble(PyList_GetItem(elem, 0)));
        for (int j = 0; j < second_dim - 1; j++) {
            node_value = PyFloat_AsDouble(PyList_GetItem(elem, j));
            // printf("\t%s%d%s%f\n", "value in ", j, " place is ", node_value);
            curr_node->value = node_value;
            curr_node->next = check_alloc(malloc(sizeof(struct node)));
            curr_node = curr_node->next;
            curr_node->next = NULL;
        }
        node_value = PyFloat_AsDouble(PyList_GetItem(elem, second_dim - 1));
        curr_node->value = node_value;
        curr_vec->nodes = head_node;
        curr_vec->next = check_alloc(malloc(sizeof(struct vector)));
        curr_vec = curr_vec->next;

        curr_vec->next = NULL;
        // curr_vec->next_in_cluster = NULL;
        curr_vec->nodes = NULL; /*New line-Shalev*/

        head_node = check_alloc(malloc(sizeof(struct node)));
        curr_node = head_node;
        curr_node->next = NULL;
        continue;
    }

    free(head_node); // TODO: make sure this doesnt break anything (instead of freeing final_node later)

    return head_vec;
}

// Given Python points and their dimensions, converts them to C double** and returns the pointer.
double** convert_py_points_to_matrix(PyObject* points, int first_dim, int second_dim) {
    double** C_mat = check_alloc(first_dim * malloc(sizeof(double*)));

    for (int i = 0; i < first_dim; i++) {
        C_mat[i] = check_alloc(malloc(second_dim * sizeof(double)));
        for (int j = 0; j < second_dim; j++) {
            C_mat[i][j] = PyFloat_AsDouble(Pylist_GetItem(PyList_GetItem(points, i), j));
        }
    }

    return C_mat;
}

PyObject execute_C_func_from_data_points(PyObject* args, double** (*f)(struct vector*), int out_dim) {
    PyObject *data_points, *py_out;
    struct vector *curr_vec, *temp_vec;
    struct node* final_node;
    double **C_out_mat, **C_in_mat;
    double* C_out_arr;

    if (!PyArg_ParseTuple(args, "O", &data_points)) {
        // Recieving the data points as a 2D Python array
        return NULL;
    }

    // Inferring N,d
    N = PyList_Size(data_points);
    d = PyList_Size(PyList_GetItem(data_points, 0));

    // Convert the python data to our vectors & nodes format
    // head_vec = convert_py_points_to_vectors(data_points, N, d);
    C_in_mat = convert_py_points_to_matrix(data_points, N, d);

    // TODO: make sure this doesnt break anything (freeing in the function instead of now)
    // final_node = head_node; // for memory clearing

    if (out_dim == 2) {
        // C Function call
        C_out_mat = f(C_in_mat);

        // Converting our C_out_mat to a Python matrix
        py_out = PyList_New(N);
        for (int i = 0; i < N; i++) {
            PyList_SetItem(py_out, i, PyList_New(N));
            for (int j = 0; j < N; j++) {
                PyList_SetItem(PyList_GetItem(C_out_mat, i), j, Py_BuildValue("f", C_out_mat[i][j]));
            }
        }

        // Freeing C_out_mat's memory
        for (int i = 0; i < N; i++) {
            free(C_out_mat[i]);
        }
        free(C_out_mat);

        // Freeing data memory
        // curr_vec = head_vec;
        // while (curr_vec != NULL) {
        //     head_node = curr_vec->nodes;
        //     free_nodes(head_node);
        //     temp_vec = curr_vec;
        //     curr_vec = curr_vec->next;
        //     free(temp_vec);
        // }
        // free(final_node);

    }

    else if (out_dim == 1) {
        // C Function call
        C_out_arr = f(C_in_mat);

        // Converting our C_out_mat to a Python matrix
        py_out = PyList_New(N);
        for (int i = 0; i < N; i++) {
            PyList_SetItem(py_out, i, C_out_arr[i]);
        }

        // Freeing C_out_mat's memory
        free(C_out_arr);
    }

    return py_out;
}

#pragma endregion

#pragma region Function Wrappers

static PyObject* sym_w(PyObject* self, PyObject* args) {
    return execute_C_func_from_data_points(args, sym, 2);
}

static PyObject* ddg_w(PyObject* self, PyObject* args) {
    return execute_C_func_from_data_points(args, ddg, 1);
}

static PyObject* norm_w(PyObject* self, PyObject* args) {
    return execute_C_func_from_data_points(args, norm, 2);
}

static PyObject* symnmf_w(PyObject* self, PyObject* args) {
    PyObject *pyH_0, *pyW, *py_out_matrix;
    struct node* final_node;
    double **H_in_mat, **W_in_mat, **H_out;

    if (!PyArg_ParseTuple(args, "OOiii", &pyH_0, &pyW, &N, &d, &K)) {
        // Recieving the data points as a 2D Python array
        return NULL;
    }

    // Convert the python data to our vectors & nodes format
    H_in_mat = convert_py_points_to_matrix(pyH_0, N, K);
    W_in_mat = convert_py_points_to_matrix(pyW, N, N);

    // TODO: make sure this doesnt break anything (freeing in the function instead of now)
    // final_node = head_node; // for memory clearing

    // C Function call
    H_out = symnmf(H_in_mat, W_in_mat);

    // Converting our H_out to a Python matrix
    py_out_matrix = PyList_New(N);
    for (int i = 0; i < N; i++) {
        PyList_SetItem(py_out_matrix, i, PyList_New(K));
        for (int j = 0; j < K; j++) {
            PyList_SetItem(PyList_GetItem(H_out, i), j, Py_BuildValue("f", H_out[i][j]));
        }
    }

    // Freeing H_out's memory
    for (int i = 0; i < N; i++) {
        free(H_out[i]);
    }
    free(H_out);

    // Freeing data memory
    // curr_vec = head_vec;
    // while (curr_vec != NULL) {
    //     head_node = curr_vec->nodes;
    //     free_nodes(head_node);
    //     temp_vec = curr_vec;
    //     curr_vec = curr_vec->next;
    //     free(temp_vec);
    // }
    // free(final_node);

    return py_out_matrix;
}

#pragma endregion

// Telling python interpreter what methods we have in the module.
// This is a method list, each method represented by a structure with 4 members representing a single method in your module.
static PyMethodDef symnmfMethods[] = {
    { "sym", /* the Python method name that will be used */
        (PyCFunction)sym_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(  /*  The docstring for the function */
            """Calculating the Similarity Matrix based on the N data points.
            Expecting:
            data_points : list[list[float]]""") },
    { "ddg", /* the Python method name that will be used */
        (PyCFunction)ddg_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(  /*  The docstring for the function */
            """Calculating the Diagonal Degree Matrix based on the N data points.
            The resulting ddg is an array representing the diagonal itself.
            Expecting:
            data_points : list[list[float]]""") },
    { "norm", /* the Python method name that will be used */
        (PyCFunction)norm_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(  /*  The docstring for the function */
            """Calculating the Normalized Similarity Matrix based on the N data points.
            Expecting:
            data_points : list[list[float]]""") },
    { "symnmf", /* the Python method name that will be used */
        (PyCFunction)symnmf_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(  /*  The docstring for the function */
            """Calculating the Decomposition Matrix H that associates each data point to a cluster.
            Expecting:
            H_0 : list[list[float]], W : list[list[float]], N : int, d : int, K : int""") },
    { NULL, NULL, 0, NULL } /* The last entry must be all NULL as shown to act as a
                  sentinel. Python looks for this entry to know that all
                  of the functions for the module have been defined. */
};

// This initiates the module using the above definitions.
static struct PyModuleDef symnmfmodule = {
    PyModuleDef_HEAD_INIT, "symnmf", /* pythonic import name of module. */
    NULL, /* module documentation, may be NULL */
    -1, /* size of per-interpreter state of the module, or -1 if the module
           keeps state in global variables. */
    symnmfMethods /* the PyMethodDef array from before containing the methods of
                  the extension */
};

// When a Python program imports your module for the first time, it will call PyInit_symnmf():
PyMODINIT_FUNC PyInit_symnmf(void)
{
    PyObject* m;
    m = PyModule_Create(&symnmfmodule);
    if (!m) {
        return NULL;
    }
    return m;
}
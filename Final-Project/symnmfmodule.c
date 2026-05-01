#define PY_SSIZE_T_CLEAN
#include <Python.h> // still works with the warning
#include "kmeans.h"
#include "symnmf.h"

// --------------- Global variables ---------------
int N, K, d, iter = 300;
double eps = 1e-4;
struct vector* head_vec; /* indicates the head vector of the input data */
struct vector** init_centroids;

// --------------- C Function ---------------

#pragma region Function Wrappers

static PyObject* sym_w(PyObject* self, PyObject* args)
{
    PyObject *data_points, *elem, *py_A;
    double node_value;
    struct vector *curr_vec, *temp_vec;
    struct node *head_node, *curr_node, *final_node;
    double** A;

    /* This parses the Python arguments into a double (d)  variable named z and
     * int (i) variable named n*/
    // TODO: this functino should only recieve the data_points and infer N, d from that! K isn't relevant yet!
    if (!PyArg_ParseTuple(args, "Oiii", &data_points, &N, &d, &K)) {
        return NULL; /* In the CPython API, a NULL value is never valid for a
                        PyObject* so it is used to signal that an error has
                        occurred. */
    }

    // Convert the python data to our vectors & nodes format
    head_node = check_alloc(malloc(sizeof(struct node)));
    curr_node = head_node;
    curr_node->next = NULL;

    head_vec = check_alloc(malloc(sizeof(struct vector)));
    curr_vec = head_vec;
    curr_vec->next = NULL;

    for (int i = 0; i < N; i++) {
        elem = PyList_GetItem(data_points, i);
        // printf("%s%d%s%d%s%f\n", "i = ", i, ", first value of elem ", i, " is ", PyFloat_AsDouble(PyList_GetItem(elem, 0)));
        for (int j = 0; j < d - 1; j++) {
            node_value = PyFloat_AsDouble(PyList_GetItem(elem, j));
            // printf("\t%s%d%s%f\n", "value in ", j, " place is ", node_value);
            curr_node->value = node_value;
            curr_node->next = check_alloc(malloc(sizeof(struct node)));
            curr_node = curr_node->next;
            curr_node->next = NULL;
        }
        node_value = PyFloat_AsDouble(PyList_GetItem(elem, d - 1));
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
    final_node = head_node; // for memory clearing

    // C Function call
    A = sym(head_node);
    
    // Converting our A C-matrix to a Python matrix
    py_A = PyList_New(K);
    for (int i = 0; i < N; i++) {
        PyList_SetItem(py_A, i, PyList_New(N));
        for (int j = 0; j < N; j++) {
            PyList_SetItem(PyList_GetItem(A, i), j, Py_BuildValue("f", A[i][j]));
        }
    }

    // Freeing A's memory
    for (int i = 0; i < N; i++){
        free(A[i])
    }
    free(A)

    // Freeing data memory
    curr_vec = head_vec;
    while (curr_vec != NULL) {
        head_node = curr_vec->nodes;
        free_nodes(head_node);
        temp_vec = curr_vec;
        curr_vec = curr_vec->next;
        free(temp_vec);
    }
    free(final_node);

    return py_A;
}

static PyObject* ddg_w(PyObject* self, PyObject* args)
{
    // TODO: finish this
    PyObject *A, *elem, *py_D;
    double** A;

    // TODO: this functino should only recieve the data_points and infer N, d from that! K isn't relevant yet!
    if (!PyArg_ParseTuple(args, "O", &A)) {
        return NULL; 
    }

    // Convert the python data to our vectors & nodes format
    head_node = check_alloc(malloc(sizeof(struct node)));
    curr_node = head_node;
    curr_node->next = NULL;

    head_vec = check_alloc(malloc(sizeof(struct vector)));
    curr_vec = head_vec;
    curr_vec->next = NULL;

    for (int i = 0; i < N; i++) {
        elem = PyList_GetItem(data_points, i);
        // printf("%s%d%s%d%s%f\n", "i = ", i, ", first value of elem ", i, " is ", PyFloat_AsDouble(PyList_GetItem(elem, 0)));
        for (int j = 0; j < d - 1; j++) {
            node_value = PyFloat_AsDouble(PyList_GetItem(elem, j));
            // printf("\t%s%d%s%f\n", "value in ", j, " place is ", node_value);
            curr_node->value = node_value;
            curr_node->next = check_alloc(malloc(sizeof(struct node)));
            curr_node = curr_node->next;
            curr_node->next = NULL;
        }
        node_value = PyFloat_AsDouble(PyList_GetItem(elem, d - 1));
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
    final_node = head_node; // for memory clearing

    // C Function call
    A = sym(head_node);
    
    // Converting our A C-matrix to a Python matrix
    py_D = PyList_New(K);
    for (int i = 0; i < N; i++) {
        PyList_SetItem(py_D, i, PyList_New(N));
        for (int j = 0; j < N; j++) {
            PyList_SetItem(PyList_GetItem(A, i), j, Py_BuildValue("f", A[i][j]));
        }
    }

    // Freeing memory
    curr_vec = head_vec;
    while (curr_vec != NULL) {
        head_node = curr_vec->nodes;
        free_nodes(head_node);
        temp_vec = curr_vec;
        curr_vec = curr_vec->next;
        free(temp_vec);
    }
    free(final_node);

    return py_D;
}

static PyObject* norm_w(PyObject* self, PyObject* args){ 
    // TODO: implement
}

static PyObject* symnmf_w(PyObject* self, PyObject* args){
    // TODO: implement
}

#pragma endregion

// Telling python interpreter what methods we have in the module.
// This is a method list, each method represented by a structure with 4 members representing a single method in your module.
static PyMethodDef symnmfMethods[] = {
    { "sym", /* the Python method name that will be used */
        (PyCFunction)sym_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(  /*  The docstring for the function */
            """Calculating the Similarity Matrix based on the N data points starting in the head node.
            Expecting:
            data_points : list[list[float]]""") },
    { "ddg", /* the Python method name that will be used */
        (PyCFunction)ddg_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(  /*  The docstring for the function */
            """Calculating the Diagonal Degree Matrix based on the N data points starting in the head node.
            Expecting:
            data_points : list[list[float]]""") },
    { "norm", /* the Python method name that will be used */
        (PyCFunction)norm_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(  /*  The docstring for the function */
            """Calculating the Normalized Similarity Matrix based on the N data points starting in the head node.
            Expecting:
            data_points : list[list[float]]""") },
    { "symnmf", /* the Python method name that will be used */
        (PyCFunction)symnmf_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(  /*  The docstring for the function */
            """Calculating the Decomposition Matrix H that associates each data point to a cluster.
            Expecting:
            H_0 : ???, W : ???, N : int, d : int, K : int""") }, // TODO: complete this
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
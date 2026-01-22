#define PY_SSIZE_T_CLEAN
#include "kmeans.h"
#include <Python.h> // still works with the warning

// --------------- Global variables ---------------
int N, K, iter, d;
double eps;
struct vector* head_vec; /* indicates the head vector of the input data */
struct vector** init_centroids;

// --------------- C Function ---------------

// --------------- Function Wrappers ---------------

static PyObject* fit(PyObject* self, PyObject* args)
{
    PyObject *points, *elem, *init_centroids_py, *final_centroids, *curr_cent_py;
    double node_value;
    struct vector** centroids; /* centroids will point to the first elem of [vector*,vector*,..,vector*] after we will allocate space later.. */
    struct vector* curr_vec;
    struct node *head_node, *curr_node, *final_node;

    // init_centroids = [ [vector], [vector], .... ]

    /* This parses the Python arguments into a double (d)  variable named z and
     * int (i) variable named n*/
    if (!PyArg_ParseTuple(args, "OOiiiid", &points, &init_centroids_py, &N, &d, &K, &iter, &eps)) {
        return NULL; /* In the CPython API, a NULL value is never valid for a
                        PyObject* so it is used to signal that an error has
                        occurred. */
    }

    head_node = check_alloc(malloc(sizeof(struct node)));
    curr_node = head_node;
    curr_node->next = NULL;

    head_vec = check_alloc(malloc(sizeof(struct vector)));
    curr_vec = head_vec;
    curr_vec->next = NULL;

    // Convert the python data to our vectors & nodes format
    for (int i = 0; i < N; i++) {
        elem = PyList_GetItem(points, i);
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
        curr_vec->next_in_cluster = NULL;
        curr_vec->nodes = NULL; /*New line-Shalev*/

        head_node = check_alloc(malloc(sizeof(struct node)));
        curr_node = head_node;
        curr_node->next = NULL;
        continue;
    }
    final_node = head_node; // for memory clearing

    // Converting init_centroids to C format
    init_centroids = check_alloc(malloc(K * sizeof(struct vector*))); /* allocate K spaces, now centroids is pointing to the first elem of [vector*,vector*,..,vector*] */
    for (int k = 0; k < K; k++) {
        curr_cent_py = PyList_GetItem(init_centroids_py, k);
        init_centroids[k] = check_alloc(malloc(sizeof(struct vector))); /* each elem in [vector*,vector*,..,vector*] will have valid adress (we can store in each adress vector) */
        init_centroids[k]->next = NULL; /* each vector will be independent from the other in centroids */
        init_centroids[k]->next_in_cluster = NULL; /* each vector will be independent from the other in centroids */
        // centroids[k]->nodes = deep_clone_nodes(next_vec->nodes); /* creating deep copy for each linked list */
        curr_node = check_alloc(malloc(sizeof(struct node)));
        init_centroids[k]->nodes = curr_node;
        for (int j = 0; j < d - 1; j++) {
            node_value = PyFloat_AsDouble(PyList_GetItem(curr_cent_py, j));
            // printf("\t%s%d%s%f\n", "value in ", j, " place is ", node_value);
            curr_node->value = node_value;
            curr_node->next = check_alloc(malloc(sizeof(struct node)));
            curr_node = curr_node->next;
            curr_node->next = NULL;
        }
        node_value = PyFloat_AsDouble(PyList_GetItem(curr_cent_py, d - 1));
        // printf("\t%s%d%s%f\n", "value in ", j, " place is ", node_value);
        curr_node->value = node_value;
        curr_node->next = NULL;
    }

    // Processing output
    centroids = kmeans(); // array[vector, vector, ...]
    final_centroids = PyList_New(K);
    for (int i = 0; i < K; i++) {
        curr_vec = centroids[i];
        curr_node = curr_vec->nodes;
        PyList_SetItem(final_centroids, i, PyList_New(d));
        for (int j = 0; j < d; j++) {
            PyList_SetItem(PyList_GetItem(final_centroids, i), j, Py_BuildValue("f", curr_node->value));
            curr_node = curr_node->next;
        }
    }

    // Freeing memory
    free(final_node);
    for (int k = 0; k < K; k++) {
        free_nodes(centroids[k]->nodes); /* free the linked list of the k'th vector */
        free(centroids[k]); /* free the k'th vector */
    }
    free(centroids);

    return final_centroids;
}

// Telling python interpreter what methods we have in the module.
// This is a method list, each method represented by a structure with 4 members representing a single method in your module.
static PyMethodDef geoMethods[] = {
    { "fit", /* the Python method name that will be used */
        (PyCFunction)fit, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR("Expecting: data_points : list[list[float]], init_centroids_list : list[list[float]], N : int, d : int, K : int, iter : int, eps : float") },
    /*  The docstring for the function */
    { NULL, NULL, 0, NULL } /* The last entry must be all NULL as shown to act as a
                  sentinel. Python looks for this entry to know that all
                  of the functions for the module have been defined. */
};

// This initiates the module using the above definitions.
static struct PyModuleDef kmeansmodule = {
    PyModuleDef_HEAD_INIT, "mykmeanssp", /* pythonic import name of module. */
    NULL, /* module documentation, may be NULL */
    -1, /* size of per-interpreter state of the module, or -1 if the module
           keeps state in global variables. */
    geoMethods /* the PyMethodDef array from before containing the methods of
                  the extension */
};

// When a Python program imports your module for the first time, it will call PyInit_kmeans():
PyMODINIT_FUNC PyInit_mykmeanssp(void)
{
    PyObject* m;
    m = PyModule_Create(&kmeansmodule);
    if (!m) {
        return NULL;
    }
    return m;
}
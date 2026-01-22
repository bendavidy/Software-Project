#define PY_SSIZE_T_CLEAN
#include <Python.h> // still works with the warning

#include "kmeans.h"

// --------------- Global variables ---------------
int N, K, iter, d;
double eps;
struct vector* head_vec; /* indicates the head vector of the input data */
struct vector** init_centroids;

// --------------- C Function ---------------
double geo_c(double z, int n)
{
    double geo_sum = 0;
    int i;
    for (i = 0; i < n; i++) {
        /* pow(x,y) function raises x to the power of y - it is from <math.h> */
        geo_sum += pow(z, i);
    }
    return geo_sum;
}

// TODO: change input
int fit_c(int a, int b) { return 0; }

// --------------- Function Wrappers ---------------
// We would like to interface the geo_c function so one can call it from Python.
static PyObject* geo_sum(PyObject* self, PyObject* args)
{
    double z;
    int n;
    /* This parses the Python arguments into a double (d)  variable named z and
     * int (i) variable named n*/
    if (!PyArg_ParseTuple(args, "di", &z, &n)) {
        return NULL; /* In the CPython API, a NULL value is never valid for a
                        PyObject* so it is used to signal that an error has
                        occurred. */
    }

    /* This builds the answer ("d" = Convert a C double to a Python floating point
     * number) back into a python object */
    return Py_BuildValue(
        "d", geo_c(z, n)); /*  Py_BuildValue(...) returns a PyObject*  */
}

static PyObject* fit(PyObject* self, PyObject* args)
// TODO: add exception handler
{
    PyObject *points, *elem, *init_centroids_py, *final_centroids, *curr_cent_py;
    double node_value;
    struct vector** centroids; /* centroids will point to the first elem of [vector*,vector*,..,vector*] after we will allocate space later.. */
    struct vector *curr_vec, *next_vec;
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
    // next_vec = head_vec;

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

    // Test prints
    printf("VECTORS:\n");
    curr_vec = head_vec;
    curr_node = curr_vec->nodes;
    for (int k = 0; k < 10; k++) {
        printf("vector %d: ", k);
        int first = 1;
        while (curr_node != NULL) {
            if (!first) {
                printf(",");
            }
            printf("%.4f", curr_node->value);
            first = 0;
            curr_node = curr_node->next;
        }
        printf("\n");
        curr_vec = curr_vec->next;
        curr_node = curr_vec->nodes;
    }

    // Processing output
    centroids = kmeans();
    printf("After kmeans return");
    final_centroids = Py_BuildValue("O", centroids); /*  Py_BuildValue(...) returns a PyObject*  */

    // Freeing memory
    free(final_node);
    for (int k = 0; k < K; k++) {
        free_nodes(centroids[k]->nodes); /* free the linked list of the k'th vector */
        free_nodes(init_centroids[k]->nodes); /* free the linked list of the k'th vector */
        free(centroids[k]); /* free the k'th vector */
        free(init_centroids[k]); /* free the k'th vector */
    }
    free(centroids);
    free(init_centroids);

    // curr_vec = head_vec;
    // while (curr_vec != NULL) {
    //     head_node = curr_vec->nodes;
    //     free_nodes(head_node);
    //     temp_vec = curr_vec;
    //     curr_vec = curr_vec->next;
    //     free(temp_vec);
    // }

    return final_centroids;
}

// Telling python interpreter what methods we have in the module.
// This is a method list, each method represented by a structure with 4 members representing a single method in your module.
static PyMethodDef geoMethods[] = {
    { "geo_sum", /* the Python method name that will be used */
        (PyCFunction)geo_sum, /* the C-function that implements the Python function
                                 and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(
            "A geometric series up to n. sum_up_to_n(z^n)") }, /*  The docstring for
                                                                  the function */
    { "fit",
        (PyCFunction)fit,
        METH_VARARGS,
        // TODO: change this
        PyDoc_STR("Returns a sum of two integers") },

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
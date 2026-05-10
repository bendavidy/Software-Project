#define PY_SSIZE_T_CLEAN
#include "symnmf.h"
#include <Python.h> /* still works with the warning */

/* --------------- Global variables ---------------*/
extern int N, K, d, iter;
extern double **A, **W;
extern double* D;
extern double eps;
extern struct vector* head_vec;
struct node* head_node;

/* Given Python points and their dimensions, converts them to C double** and returns the pointer. */
double** convert_py_points_to_matrix(PyObject* points, int first_dim, int second_dim) {
    double** C_mat = check_alloc(malloc(first_dim * sizeof(double*)));

    for (int i = 0; i < first_dim; i++) {
        C_mat[i] = check_alloc(malloc(second_dim * sizeof(double)));
        for (int j = 0; j < second_dim; j++) {
            C_mat[i][j] = PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(points, i), j));
        }
    }

    return C_mat;
}

PyObject* execute_C_func_from_data_points(PyObject* args, double** (*f)(double**)) {
    PyObject *data_points, *py_out;
    double **C_out_mat, **C_in_mat;

    if (!PyArg_ParseTuple(args, "O", &data_points)) {
        /* Recieving the data points as a 2D Python array */
        return NULL;
    }

    /* Inferring N,d */
    N = PyList_Size(data_points);
    d = PyList_Size(PyList_GetItem(data_points, 0));

    /* Convert the python data to our vectors & nodes format */
    C_in_mat = convert_py_points_to_matrix(data_points, N, d);

    /* C Function call */
    C_out_mat = f(C_in_mat);

    for (int i = 0; i < N; i++) {
        free(C_in_mat[i]);
    }
    free(C_in_mat);

    /* we can get NULL if goal=='norm' and in case we have division by zero */
    if (!C_out_mat) {
        return NULL;
    }

    /* Converting our C_out_mat to a Python matrix  */
    py_out = PyList_New(N);
    for (int i = 0; i < N; i++) {
        PyList_SetItem(py_out, i, PyList_New(N));
        for (int j = 0; j < N; j++) {
            PyList_SetItem(PyList_GetItem(py_out, i), j, Py_BuildValue("f", C_out_mat[i][j]));
        }
    }

    // Freeing memory
    for (int i = 0; i < N; i++) {
        free(C_out_mat[i]);
    }
    free(C_out_mat);

    return py_out;
}
// created that one because ddg is returning double* and not double**.
// in the other function the input f function is expected to be f that return double** so this whole function here is for ddg that return double*.
PyObject* execute_C_func_from_data_points_1d(PyObject* args, double* (*f)(double**)) {
    PyObject *data_points, *py_out;
    double **C_in_mat;
    double *C_out_arr;

    if (!PyArg_ParseTuple(args, "O", &data_points)) {
        return NULL;
    }

    N = PyList_Size(data_points);
    d = PyList_Size(PyList_GetItem(data_points, 0));

    C_in_mat = convert_py_points_to_matrix(data_points, N, d);
    C_out_arr = f(C_in_mat);

    py_out = PyList_New(N);
    for (int i = 0; i < N; i++) {
        PyList_SetItem(py_out, i, Py_BuildValue("d", C_out_arr[i]));
    }

    // Free memory
    free(C_out_arr);
    for (int i = 0; i < N; i++) {
        free(C_in_mat[i]);
    }
    free(C_in_mat);

    return py_out;
}

static PyObject* sym_w(PyObject* self, PyObject* args) {
    return execute_C_func_from_data_points(args, sym);
}

static PyObject* ddg_w(PyObject* self, PyObject* args) {
    PyObject *out;
    out = execute_C_func_from_data_points_1d(args, ddg);
    free_mat(A);

    return out;
}

static PyObject* norm_w(PyObject* self, PyObject* args) {
    PyObject *out;
    out = execute_C_func_from_data_points(args, norm);
    free_mat(A);
    if (D)
        free(D);

    return out;
}

static PyObject* symnmf_w(PyObject* self, PyObject* args) {
    PyObject *pyH_0, *pyW, *py_out_matrix;
    double **H_in_mat, **W_in_mat, **H_out;

    if (!PyArg_ParseTuple(args, "OOii", &pyH_0, &pyW, &N, &K)) {
        // result = mysymnmf.symnmf(H0, W, len(X),k)
        // Recieving the data points as a 2D Python array
        return NULL;
    }

    // Convert the python data to our vectors & nodes format
    H_in_mat = convert_py_points_to_matrix(pyH_0, N, K);
    W_in_mat = convert_py_points_to_matrix(pyW, N, N);

    // C Function call
    H_out = symnmf(H_in_mat, W_in_mat);

    // Converting our H_out to a Python matrix
    py_out_matrix = PyList_New(N);
    for (int i = 0; i < N; i++) {
        PyList_SetItem(py_out_matrix, i, PyList_New(K));
        for (int j = 0; j < K; j++) {
            PyList_SetItem(PyList_GetItem(py_out_matrix, i), j, Py_BuildValue("d", H_out[i][j]));
        }
    }

    // Freeing memory
    free_mat(H_out);
    free_mat(H_in_mat);
    free_mat(W_in_mat);

    return py_out_matrix;
}

// Telling python interpreter what methods we have in the module.
// This is a method list, each method represented by a structure with 4 members representing a single method in your module.
static PyMethodDef symnmfMethods[] = {
    { "sym", /* the Python method name that will be used */
        (PyCFunction)sym_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(/*  The docstring for the function */
            "Calculating the Similarity Matrix based on the N data points.\n"
            "Expecting:\n"
            "data_points : list[list[float]]") },
    { "ddg", /* the Python method name that will be used */
        (PyCFunction)ddg_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(/*  The docstring for the function */
            "Calculating the Diagonal Degree Matrix based on the N data points.\n"
            "The resulting ddg is an array representing the diagonal itself.\n"
            "Expecting:\n"
            "data_points : list[list[float]]") },
    { "norm", /* the Python method name that will be used */
        (PyCFunction)norm_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(/*  The docstring for the function */
            "Calculating the Normalized Similarity Matrix based on the N data points.\n"
            "Expecting:\n"
            "data_points : list[list[float]]") },
    { "symnmf", /* the Python method name that will be used */
        (PyCFunction)symnmf_w, /* the C-function that implements the Python function and returns static PyObject*  */
        METH_VARARGS, /* flags indicating parameters accepted for this function */
        PyDoc_STR(/*  The docstring for the function */
            "Calculating the Decomposition Matrix H that associates each data point to a cluster.\n"
            "Expecting:\n"
            "H_0 : list[list[float]], W : list[list[float]], N : int, K : int") },
    { NULL, NULL, 0, NULL } /* The last entry must be all NULL as shown to act as a
                  sentinel. Python looks for this entry to know that all
                  of the functions for the module have been defined. */
};

// This initiates the module using the above definitions.
static struct PyModuleDef mysymnmfmodule = {
    PyModuleDef_HEAD_INIT, "mysymnmf", /* pythonic import name of module. */
    NULL, /* module documentation, may be NULL */
    -1, /* size of per-interpreter state of the module, or -1 if the module
           keeps state in global variables. */
    symnmfMethods /* the PyMethodDef array from before containing the methods of
                  the extension */
};

// When a Python program imports your module for the first time, it will call PyInit_symnmf():
PyMODINIT_FUNC PyInit_mysymnmf(void)
{
    PyObject* m;
    m = PyModule_Create(&mysymnmfmodule);
    if (!m) {
        return NULL;
    }
    return m;
}
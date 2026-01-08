#define PY_SSIZE_T_CLEAN
#include <Python.h> // still works with the warning

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

int fit_c(int a, int b) { return a + b; }

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
{
    int a;
    int b;
    /* This parses the Python arguments into a double (d)  variable named z and
     * int (i) variable named n*/
    if (!PyArg_ParseTuple(args, "ii", &a, &b)) {
        return NULL; /* In the CPython API, a NULL value is never valid for a
                        PyObject* so it is used to signal that an error has
                        occurred. */
    }

    /* This builds the answer ("d" = Convert a C double to a Python floating point
     * number) back into a python object */
    return Py_BuildValue(
        "i", fit_c(a, b)); /*  Py_BuildValue(...) returns a PyObject*  */
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
#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#if 1 //defined(_MSC_VER)
	// Visual Studio Compiler can not read biosig.h/biosig-dev.h //
	#define BIOSIG_FLAG_COMPRESSION        0x0001
	#define BIOSIG_FLAG_UCAL               0x0002
	#define BIOSIG_FLAG_OVERFLOWDETECTION  0x0004
	#define BIOSIG_FLAG_ROW_BASED_CHANNELS 0x0008
	typedef double biosig_data_type;
	typedef void HDRTYPE;

	HDRTYPE* sopen_extended(const char* FileName, const char* MODE, HDRTYPE* hdr, void* ptr);
	size_t   sread(biosig_data_type* DATA, size_t START, size_t LEN, HDRTYPE* hdr);
	int      serror2(HDRTYPE* hdr);
	int      asprintf_hdr2json(char **str, HDRTYPE* hdr);
	void     destructHDR(HDRTYPE* hdr);

	long     biosig_get_number_of_channels(HDRTYPE *hdr);
	size_t   biosig_get_number_of_samples(HDRTYPE *hdr);
	size_t   biosig_get_number_of_records(HDRTYPE *hdr);
	size_t   biosig_get_number_of_segments(HDRTYPE *hdr);

	int      biosig_get_flag(HDRTYPE *hdr, unsigned flags);
	int      biosig_set_flag(HDRTYPE *hdr, unsigned flags);
	int      biosig_reset_flag(HDRTYPE *hdr, unsigned flags);
#else
	#include <biosig.h>
#endif

#define BIOSIG_MODULE
#include "biosigmodule.h"

#if PY_MAJOR_VERSION >= 3
  #define MOD_ERROR_VAL NULL
  #define MOD_SUCCESS_VAL(val) val
  #define MOD_INIT(name) PyMODINIT_FUNC PyInit_##name(void)
  #define MOD_DEF(ob, name, doc, methods) \
          static struct PyModuleDef moduledef = { \
            PyModuleDef_HEAD_INIT, name, doc, -1, methods, }; \
          ob = PyModule_Create(&moduledef);
#else
  #define MOD_ERROR_VAL
  #define MOD_SUCCESS_VAL(val)
  #define MOD_INIT(name) void init##name(void)
  #define MOD_DEF(ob, name, doc, methods) \
          ob = Py_InitModule3(name, methods, doc);
#endif

static PyObject *BiosigError;

static int PyBiosig_Header(const char *filename, char **jsonstr) {
	HDRTYPE *hdr = NULL;
	hdr = sopen_extended(filename, "r", hdr, NULL);
	if (serror2(hdr)) {
	        PyErr_SetString(BiosigError, "could not open file");
		destructHDR(hdr);
	        return -1;
	}

	// convert to json-string
	char *str = NULL;
	asprintf_hdr2json(&str, hdr);
	destructHDR(hdr);
	*jsonstr = strdup(str);
	return 0;
}

static PyObject *biosig_json_header(PyObject *self, PyObject *args) {
	const char *filename = NULL;
	char *str = NULL;
	if (!PyArg_ParseTuple(args, "s", &filename)) return NULL;
	if (PyBiosig_Header(filename, &str)) return NULL;
	return Py_BuildValue("s", str);
}

static int PyBiosig_Data(const char *filename, PyArrayObject **D) {
	HDRTYPE *hdr = sopen_extended(filename, "r", NULL, NULL);
	if (serror2(hdr)) {
	        PyErr_SetString(BiosigError, "could not open file");
		destructHDR(hdr);
	        return -1;
	}

	const int nd=2;
	npy_intp dims[2];
	dims[0] = (int)biosig_get_number_of_samples(hdr);
	dims[1] = (int)biosig_get_number_of_channels(hdr);
	int type_num;

	switch (sizeof(biosig_data_type)) {
	case 4:
		type_num=NPY_FLOAT32;
		break;
	case 8:
		type_num=NPY_FLOAT64;
		break;
#if NPY_BITSOF_LONGDOUBLE >= 128
	case 16:
		type_num=NPY_FLOAT128;
		break;
#endif
	}
        *D = (PyArrayObject*) PyArray_SimpleNew(nd, dims, type_num);
	biosig_set_flag(hdr, BIOSIG_FLAG_ROW_BASED_CHANNELS);
        size_t count = sread( PyArray_DATA(*D), 0, biosig_get_number_of_records(hdr), hdr);
	destructHDR(hdr);
	return 0;
}

static PyObject *biosig_data(PyObject *self, PyObject *args) {
	const char *filename = NULL;
	if (!PyArg_ParseTuple(args, "s", &filename)) return NULL;
	PyArrayObject* Data;
	if (PyBiosig_Data(filename, &Data)) return NULL;
	return Data;
}

static PyMethodDef BiosigMethods[] = {
    {"header",  biosig_json_header, METH_VARARGS, "load biosig header and export as JSON string."},
    {"data",    biosig_data,        METH_VARARGS, "load biosig data."},
/*
    {"base64",  biosig_json_header, METH_VARARGS, "load biosig header and export as JSON ."},
    {"fhir_json_binary_template",  biosig_json_header, METH_VARARGS, "load biosig header and export as JSON ."},
    {"fhir_xml_binary_template",  biosig_json_header, METH_VARARGS, "load biosig header and export as JSON ."},
*/
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

const char module___doc__[] = "Biosig - a tool for loading biomedical signal data.\n"
                              "  Biosig can read about 50 different fileformats of EEG, ECG, etc.\n"
			      "  The data samples can be read into a single matrix\n"
			      "  using the function\n"
			      "         data = biosig.data(filename)\n"
			      "  The header- and meta information including events can be\n"
			      "  read into a JSON structure with the function\n"
			      "         header = biosig.header(filename)\n";

#if 0 // PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "biosig",     /* m_name */
        module___doc__,      /* m_doc */
        -1,                  /* m_size */
        BiosigMethods,       /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
    };
#endif

MOD_INIT(biosig) {
    import_array();

    PyObject *m;
    MOD_DEF(m, "biosig", module___doc__, BiosigMethods);
    if (m == NULL) return MOD_ERROR_VAL;

    BiosigError = PyErr_NewException("biosig.error", NULL, NULL);
    Py_INCREF(BiosigError);
    PyModule_AddObject(m, "error", BiosigError);

     /* additional initialization can happen here */

   return MOD_SUCCESS_VAL(m);
}


#include "py_helper.h"

#include <Python.h>

#include "../seika/src/utils/se_assert.h"
#include "../seika/src/data_structures/se_hash_map_string.h"

// Helper functions
const char* pyh_get_string_from_obj(PyObject* obj) {
    SE_ASSERT_FMT(PyUnicode_Check(obj), "Not a valid string!");
    const char* str = PyUnicode_AsUTF8(obj);
    return str;
}

const char* pyh_get_string_from_dict(PyObject* pyDict, const char* key) {
    PyObject* pStr = PyDict_GetItemString(pyDict, key);
    SE_ASSERT(pStr != NULL);
    const char* str = pyh_get_string_from_obj(pStr);
    Py_DecRef(pStr);
    return str;
}

const char* pyh_get_string_from_dict_default(PyObject* pyDict, const char* key, const char* defaultValue) {
    PyObject* pKey = PyUnicode_FromString(key);
    PyObject* pDefaultValue = PyUnicode_FromString(defaultValue);

    PyObject* pStr = PyDict_SetDefault(pyDict, pKey, pDefaultValue);
    SE_ASSERT(pStr != NULL);
    const char* str = pyh_get_string_from_obj(pStr);
    Py_DecRef(pKey);
    Py_DecRef(pDefaultValue);
    Py_DecRef(pStr);
    return str;
}

int pyh_get_int_from_dict(PyObject* pyDict, const char* key) {
    PyObject* pInt = PyDict_GetItemString(pyDict, key);
    SE_ASSERT(pInt != NULL);
    SE_ASSERT(PyLong_Check(pInt));
    const int intResult = PyLong_AsLong(pInt);
    Py_DecRef(pInt);
    return intResult;
}

int pyh_get_int_from_dict_default(PyObject* pyDict, const char* key, int defaultValue) {
    PyObject* pKey = PyUnicode_FromString(key);
    PyObject* pDefaultValue = PyLong_FromLong(defaultValue);
    PyObject* pInt = PyDict_SetDefault(pyDict, pKey, pDefaultValue);
    SE_ASSERT(pInt != NULL);
    SE_ASSERT(PyLong_Check(pInt));
    const int intResult = PyLong_AsLong(pInt);
    Py_DecRef(pKey);
    Py_DecRef(pDefaultValue);
    Py_DecRef(pInt);
    return intResult;
}

bool pyh_run_python_file(const char* filePath) {
    PyObject* pFilePath = Py_BuildValue("s", filePath);
    FILE* configFile = _Py_fopen_obj(pFilePath, "r+");
    if (configFile != NULL) {
        int result = PyRun_SimpleFile(configFile, filePath);
        Py_DECREF(pFilePath);
        return (result == 0);
    }
    Py_DECREF(pFilePath);
    return false;
}

const char* phy_get_string_from_var(struct _object* obj, const char* variableName) {
    SE_ASSERT(obj != NULL);
    Py_IncRef(obj);
    PyObject* objVar = PyObject_GetAttrString(obj, variableName);
    SE_ASSERT_FMT(objVar != NULL, "Failed to read variable '%s'", variableName);
    const char* varString = PyUnicode_AsUTF8(objVar);
    Py_DecRef(objVar);
    return varString;
}

int phy_get_int_from_var(struct _object* obj, const char* variableName) {
    SE_ASSERT(obj != NULL);
    PyObject* objVar = PyObject_GetAttrString(obj, variableName);
    SE_ASSERT(objVar != NULL);
    const int varInt = (int) PyLong_AsLong(objVar);
    Py_DecRef(objVar);
    return varInt;
}

float phy_get_float_from_var(struct _object* obj, const char* variableName) {
    SE_ASSERT_FMT(obj != NULL, "variable name = '%s'", variableName);
    PyObject* objVar = PyObject_GetAttrString(obj, variableName);
    SE_ASSERT(objVar != NULL);
    const float varFloat = (float) PyFloat_AsDouble(objVar);
    Py_DecRef(objVar);
    return varFloat;
}

bool phy_get_bool_from_var(struct _object* obj, const char* variableName) {
    SE_ASSERT(obj != NULL);
    PyObject* objVar = PyObject_GetAttrString(obj, variableName);
    SE_ASSERT(objVar != NULL);
    SE_ASSERT_FMT(PyBool_Check(objVar), "variable '%s' is not of a bool type!", variableName);
    const bool varBool = PyObject_IsTrue(objVar);
    Py_DecRef(objVar);
    return varBool;
}

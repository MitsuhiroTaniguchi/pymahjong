#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <Python.h>
#include <filesystem>
#include <string>
#include "calsht_dw.hpp"

namespace py = pybind11;

PYBIND11_MODULE(pymahjong, m) {
    m.doc() = "pymahjong"; // モジュールの説明

    py::class_<CalshtDW>(m, "Xiangting")
     .def(py::init<>())
     .def("initialize", &CalshtDW::initialize, "Initialize the model with a directory")
     .def("calculate", &CalshtDW::operator(), "Calculate Xiangting reduction",
          py::arg("hand"), py::arg("size"), py::arg("mode"),
          py::arg("check_hand") = false, py::arg("three_player") = false);
}

std::filesystem::path get_module_path() {
    // "pymahjong" はモジュール名です
    PyObject* module = PyImport_ImportModule("pymahjong");
    if (!module) {
        throw std::runtime_error("Failed to import module 'pymahjong'");
    }
    PyObject* file_attr = PyObject_GetAttrString(module, "__file__");
    Py_DECREF(module);
    if (!file_attr) {
        throw std::runtime_error("Module 'pymahjong' has no __file__ attribute");
    }
    const char* path_c = PyUnicode_AsUTF8(file_attr);
    Py_DECREF(file_attr);
    if (!path_c) {
        throw std::runtime_error("Failed to get module file path");
    }
    return std::filesystem::path(path_c);
}


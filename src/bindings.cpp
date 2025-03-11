#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "calsht_dw.hpp"

namespace py = pybind11;

PYBIND11_MODULE(necessary_and_unnecessary_tiles, m) {
    m.doc() = "Python bindings for necessary-and-unnecessary-tiles"; // モジュールの説明

    py::class_<CalshtDW>(m, "Xiangting")
     .def(py::init<>())
     .def("initialize", &CalshtDW::initialize, "Initialize the model with a directory")
     .def("calculate", &CalshtDW::operator(), "Calculate Xiangting reduction",
          py::arg("hand"), py::arg("size"), py::arg("mode"),
          py::arg("check_hand") = false, py::arg("three_player") = false);
}

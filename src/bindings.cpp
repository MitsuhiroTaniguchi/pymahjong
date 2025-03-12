#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "calsht_dw.hpp"

namespace py = pybind11;

PYBIND11_MODULE(pymahjong, m) {
    m.doc() = "pymahjong"; // モジュールの説明

    py::class_<CalshtDW>(m, "Xiangting")
     .def(py::init<>())
     .def("calculate", &CalshtDW::operator(), "Calculate Xiangting reduction",
          py::arg("hand"), py::arg("size"), py::arg("mode"),
          py::arg("check_hand") = false, py::arg("three_player") = false);
}

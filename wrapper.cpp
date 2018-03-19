#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "gbe.h"

namespace py = pybind11;

PYBIND11_MODULE(libgbe, m) {
    py::class_<gbe>(m, "GBE")
        .def(py::init< std::string >())
        .def("display", [](gbe &g) {
            return py::array({144, 160, 3}, g.display());
        })
        .def("run", &gbe::run)
        .def("input", &gbe::input)
        .def("read_memory", &gbe::mem);
}

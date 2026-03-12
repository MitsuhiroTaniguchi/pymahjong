## Third-Party Components

### `src/calsht_dw.cpp`, `src/calsht_dw.hpp`, `src/hash.hpp`, `src/mkind.cpp`, `data/index_dw_s.bin`, `data/index_dw_h.bin`

These files are derived from the shanten solver implementation in
[`tomohxx/shanten-number`](https://github.com/tomohxx/shanten-number),
licensed under the GNU Lesser General Public License v3.0.

This repository includes local modifications for packaging, Python binding
integration, packaged data lookup, and three-player support.

### `pybind11`

`pymahjong` uses [`pybind11`](https://github.com/pybind/pybind11) as a build
dependency and optional git submodule. `pybind11` is distributed under a
BSD-style license in its own repository.

# Project-local libraries and models

Only the external footprints and symbol libraries required by this project are included here. Project `fp-lib-table` and `sym-lib-table` entries take precedence over global libraries with the same names.

- TrueLib: the project's shared custom library, from <https://github.com/pavlig43/kicad-freecad>. Its license is preserved in `LICENSE-TrueLib.txt`.
- SparkFun capacitor and resistor symbols, footprints and STEP models: <https://github.com/sparkfun/SparkFun-KiCad-Libraries>.
- Stock KiCad STEP models copied into `../models/`: <https://gitlab.com/kicad/libraries/kicad-packages3D>.

Existing placed footprints retain their board-specific pad sizes and artwork. The relocation changes model file paths only; model bytes, scale, offset and rotation remain unchanged. Models are referenced with `${KIPRJMOD}/models/...` so a clone has no dependency on a machine-specific library directory.

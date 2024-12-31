# CXX Modules must be enabled before the project.
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
set(CMAKE_CXX_MODULE_STD ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_MODULE_STD ON)

add_library(mylir_lib)
target_sources(mylir_lib
  PUBLIC FILE_SET cxx_modules
  TYPE CXX_MODULES FILES
         lang.cppm
         program.cppm
         cpu.cppm
         lexer.cppm
         mod.cppm)

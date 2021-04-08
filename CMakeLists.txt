cmake_minimum_required(VERSION 3.0.0)

project(llang)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Packages
find_package(Boost 1.71 REQUIRED)
find_package(LLVM REQUIRED CONFIG)
llvm_map_components_to_libnames(llvm_libs
  support
  core
  irreader
  XCoreCodeGen
  X86CodeGen
  WebAssemblyCodeGen
  SystemZCodeGen
  SparcCodeGen
  RISCVCodeGen
  PowerPCCodeGen
  NVPTXCodeGen
  MSP430CodeGen
  MipsCodeGen
  LanaiCodeGen
  HexagonCodeGen
  BPFCodeGen
  AVRCodeGen
  ARMCodeGen
  AMDGPUCodeGen
  AArch64CodeGen
  CodeGen
  X86AsmParser
  WebAssemblyAsmParser
  SystemZAsmParser
  SparcAsmParser
  RISCVAsmParser
  PowerPCAsmParser
  MSP430AsmParser
  MipsAsmParser
  LanaiAsmParser
  HexagonAsmParser
  BPFAsmParser
  AVRAsmParser
  ARMAsmParser
  AsmParser
  AMDGPUAsmParser
  AArch64AsmParser
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS "-g")

set(SOURCES
  src/main.cc
  src/ast.cc
  src/lexer.cc
  src/source.cc
  src/parser.cc
)
set(INCLUDE_DIRS
  src/
  ${Boost_INCLUDE_DIRS}
  ${LLVM_INCLUDE_DIRS}
)
set(LIBRARIES
  ${Boost_LIBRARIES}
  ${llvm_libs}
)

add_executable(llang ${SOURCES})
target_include_directories(llang PRIVATE ${INCLUDE_DIRS})
target_link_libraries(llang ${LIBRARIES})

include(cmake/linting.cmake)
include(cmake/tests.cmake)

add_custom_target(
  run
  COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/llang
  DEPENDS llang
)

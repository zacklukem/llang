find_package(GTest REQUIRED)

set(TEST_SOURCES
  tests/all.cc
  tests/lexer.cc
  tests/source.cc
  tests/parser.cc
  src/gen.cc
  src/ast.cc
  src/lexer.cc
  src/source.cc
  src/parser.cc
)

add_executable(test_llang ${TEST_SOURCES})
target_include_directories(test_llang PRIVATE ${INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})
target_link_libraries(test_llang ${LIBRARIES} ${GTEST_BOTH_LIBRARIES})

add_custom_target(
  test
  COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_llang
  DEPENDS test_llang
)

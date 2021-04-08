file(GLOB FORMAT_FILES
  src/*.hh
  src/*.cc
  tests/*.cc
)

add_custom_target(
  lint
  COMMAND clang-format -Werror -n
  ${FORMAT_FILES}
)
add_custom_target(
  format
  COMMAND clang-format -i
  ${FORMAT_FILES}
)
add_custom_target(
  lint
  COMMAND clang-format -Werror -n
  ${SOURCES}
)
#include <gtest/gtest.h>
#include <lexer.hh>
#include <source.hh>

TEST(Lexer, LexerImpl) {
  std::string str = R"(
    function add(stuff: double, more: double) {

    }

    function add2(a: double, b: double) {
      if (a == 2.0) {
        return 3.0;
      } else {
        return 1.2;
      }
    }
  )";
  auto source = std::make_shared<llang::Source>(str);

  auto lexer = llang::Lexer(source);

  auto n = lexer.peek();

  while ((n = lexer.next()).type != llang::TokenType::END_OF_FILE) {
    std::cout << n.value.getValue() << "\n";
  }

  source->debugPrintMessages();
}

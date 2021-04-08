#include <gtest/gtest.h>
#include <lexer.hh>
#include <source.hh>

#define EXPECT_TOK(v, t)                                                                           \
  EXPECT_EQ(lexer.peek().value.getValue(), v);                                                     \
  EXPECT_EQ(lexer.next().type, t)

TEST(Lexer, LexerImpl) {
  std::string str = R"(
    function add2(a: double, b: double): double {
      if (a == 2.0) {
        return 3.0;
      } else {
        return 1.2;
      }
    }
  )";
  auto source = std::make_shared<llang::Source>(str);

  auto lexer = llang::Lexer(source);

  using tt = llang::TokenType;

  EXPECT_TOK("function", tt::FUNCTION);
  EXPECT_TOK("add2", tt::IDENT);
  EXPECT_TOK("(", tt::OPEN_PAREN);
  EXPECT_TOK("a", tt::IDENT);
  EXPECT_TOK(":", tt::COLON);
  EXPECT_TOK("double", tt::IDENT);
  EXPECT_TOK(",", tt::COMMA);
  EXPECT_TOK("b", tt::IDENT);
  EXPECT_TOK(":", tt::COLON);
  EXPECT_TOK("double", tt::IDENT);
  EXPECT_TOK(")", tt::CLOSE_PAREN);
  EXPECT_TOK(":", tt::COLON);
  EXPECT_TOK("double", tt::IDENT);
  EXPECT_TOK("{", tt::OPEN_BRACE);
  EXPECT_TOK("if", tt::IF);
  EXPECT_TOK("(", tt::OPEN_PAREN);
  EXPECT_TOK("a", tt::IDENT);
  EXPECT_TOK("==", tt::DEQ);
  EXPECT_TOK("2.0", tt::FLOAT);
  EXPECT_TOK(")", tt::CLOSE_PAREN);
  EXPECT_TOK("{", tt::OPEN_BRACE);
  EXPECT_TOK("return", tt::RETURN);
  EXPECT_TOK("3.0", tt::FLOAT);
  EXPECT_TOK(";", tt::SEMICOLON);
  EXPECT_TOK("}", tt::CLOSE_BRACE);
  EXPECT_TOK("else", tt::ELSE);
  EXPECT_TOK("{", tt::OPEN_BRACE);
  EXPECT_TOK("return", tt::RETURN);
  EXPECT_TOK("1.2", tt::FLOAT);
  EXPECT_TOK(";", tt::SEMICOLON);
  EXPECT_TOK("}", tt::CLOSE_BRACE);
  EXPECT_TOK("}", tt::CLOSE_BRACE);
  EXPECT_TOK("", tt::END_OF_FILE);

  EXPECT_FALSE(source->debugPrintMessages());
}

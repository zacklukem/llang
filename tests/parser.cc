#include <gtest/gtest.h>
#include <iostream>
#include <parser.hh>
#include <source.hh>
#include <sstream>

using namespace llang;

// std::unique_ptr<Document> parseDocument();
// std::unique_ptr<Node> parseFunction();
// std::unique_ptr<Node> parseStructDecl();
// std::unique_ptr<Node> parseImport();
//
// Block parseBlock();
// std::unique_ptr<Statement> parseStatement();
// std::unique_ptr<Statement> parseReturnStatement();
// std::unique_ptr<Statement> parseIfStatement();
// std::unique_ptr<Statement> parseLetStatement();
// std::unique_ptr<Statement> parseWhileStatement();
// std::unique_ptr<Statement> parseForStatement();
//
// std::unique_ptr<Expression> parseExpression();
TEST(Parser, ParseEspression) {
  std::string str = R"(
    (a[1] + b) + 3 - call(a.b)
    a = b + 1 + 2
    MyStruct { a: 3, b: 4 + -1, }
  )";

  auto source = std::make_shared<Source>(str);

  Parser p(source, nullptr);

  auto a = p.parseExpression();
  std::stringstream ss;
  ss << *a;
  EXPECT_EQ(ss.str(), "(- (+ (+ ([] a 1) b) 3) (call (. a b)))");

  auto b = p.parseExpression();
  ss = std::stringstream();
  ss << *b;
  EXPECT_EQ(ss.str(), "(= a (+ (+ b 1) 2))");

  auto c = p.parseExpression();
  ss = std::stringstream();
  ss << *c;
  EXPECT_EQ(ss.str(), "{MyStruct (a 3) (b (+ 4 (unary_temp 1)))}");
}
// std::unique_ptr<Expression> parseAssign();
// std::unique_ptr<Expression> parseComparison();
// std::unique_ptr<Expression> parseAddSub();
// std::unique_ptr<Expression> parseMulDiv();
// std::unique_ptr<Expression> parseUnary();
// std::unique_ptr<Expression> parseFunctionCall();
// std::unique_ptr<Expression> parseArrayAccess();
// std::unique_ptr<Expression> parseDotOp();
// std::unique_ptr<Expression> parsePrimary();
// std::unique_ptr<Expression> parseVariable();
// std::unique_ptr<Expression> parseStructInit();
//
// std::shared_ptr<Type> parseTypeName();
// std::shared_ptr<Type> parseBasicType();
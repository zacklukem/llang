#pragma once

#include "ast.hh"
#include "lexer.hh"
#include "source.hh"
#include <memory>

namespace llang {

class Parser {

public:
  Parser(std::shared_ptr<Source> source, std::shared_ptr<State> state)
      : source(source), lexer(source), state(state){};

  std::unique_ptr<Document> parseDocument();
  std::unique_ptr<Node> parseFunction();
  std::unique_ptr<Node> parseStructDecl();

  Block parseBlock();
  std::unique_ptr<Statement> parseStatement();
  std::unique_ptr<Statement> parseReturnStatement();
  std::unique_ptr<Statement> parseIfStatement();
  std::unique_ptr<Statement> parseLetStatement();
  std::unique_ptr<Statement> parseWhileStatement();
  std::unique_ptr<Statement> parseForStatement();

  std::unique_ptr<Expression> parseExpression();
  std::unique_ptr<Expression> parseAssign();
  std::unique_ptr<Expression> parseComparison();
  std::unique_ptr<Expression> parseAddSub();
  std::unique_ptr<Expression> parseMulDiv();
  std::unique_ptr<Expression> parseUnary();
  std::unique_ptr<Expression> parseFunctionCall();
  std::unique_ptr<Expression> parseArrayAccess();
  std::unique_ptr<Expression> parseDotOp();
  std::unique_ptr<Expression> parsePrimary();
  std::unique_ptr<Expression> parseVariable();
  std::unique_ptr<Expression> parseStructInit();

  std::shared_ptr<Type> parseTypeName();
  std::shared_ptr<Type> parseBasicType();

private:
  Lexer lexer;
  std::shared_ptr<Source> source;
  std::shared_ptr<State> state;
};

} // namespace llang
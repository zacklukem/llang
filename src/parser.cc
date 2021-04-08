#include "parser.hh"

#include <iostream>

using namespace llang;

std::unique_ptr<Document> Parser::parseDocument() {
  auto doc = std::make_unique<Document>(state);
  while (!lexer.peekEq(TokenType::END_OF_FILE)) {
    doc->data.push_back(parseFunction());
  }
  return std::move(doc);
}

std::unique_ptr<FunctionDecl> Parser::parseFunction() {
  lexer.next().expect(TokenType::FUNCTION) << "Expected function keyword (internal error)";
  auto func_name = lexer.next();
  func_name.expect(TokenType::IDENT) << "Expected function keyword (internal error)";
  lexer.next().expect(TokenType::OPEN_PAREN) << "Unexpected token (expected '(')";
  std::vector<std::pair<std::shared_ptr<Type>, std::string>> arguments;
  // Parse args
  while (!lexer.peekEq(TokenType::CLOSE_PAREN)) {
    auto ident = lexer.next();
    ident.expect(TokenType::IDENT) << "Unexpected token (expected IDENT)";
    lexer.next().expect(TokenType::COLON) << "Unexpected token (expected ':')";
    auto type = parseTypeName();
    arguments.push_back(std::make_pair(type, ident.value.getValue()));
    if (lexer.peekEq(TokenType::CLOSE_PAREN))
      break;
    lexer.next().expect(TokenType::COMMA);
  }
  lexer.next().expect(TokenType::CLOSE_PAREN) << "Unexpected token (expected ')')";
  lexer.next().expect(TokenType::COLON) << "Unexpected token (expected ':')";
  auto return_type = parseTypeName();

  auto body = parseBlock();

  auto proto_func =
      std::make_unique<ProtoFunc>(state, func_name.value.getValue(), return_type, arguments);

  return std::make_unique<FunctionDecl>(state, std::move(proto_func), std::move(body));
}

Block Parser::parseBlock() {
  lexer.next().expect(TokenType::OPEN_BRACE) << "Unexpected token (expected '{')";

  auto stl = Block::StmtList();
  while (!lexer.peekEq(TokenType::CLOSE_BRACE)) {
    stl.push_back(parseStatement());
    if (lexer.peekEq(TokenType::SEMICOLON))
      lexer.next();
  }
  auto body = Block(state, std::move(stl));
  lexer.next().expect(TokenType::CLOSE_BRACE) << "Unexpected token (expected '}')";
  return std::move(body);
}

std::unique_ptr<Statement> Parser::parseStatement() {
  auto next = lexer.peek();
  switch (next.type) {
  case RETURN:
    return parseReturnStatement();
  case IF:
    return parseIfStatement();
  case LET:
    return parseLetStatement();
  case WHILE:
    return parseWhileStatement();
  default:
    return parseExpression();
  }
}

std::unique_ptr<Statement> Parser::parseReturnStatement() {
  lexer.next().expect(TokenType::RETURN) << "Expected return keyword (internal error)";
  if (lexer.peekEq(TokenType::SEMICOLON)) {
    return std::make_unique<ReturnStatement>(state, std::nullopt);
  }
  auto expr = parseExpression();
  if (expr) {
    return std::make_unique<ReturnStatement>(std::move(expr));
  } else {
    return std::make_unique<ReturnStatement>(state, std::nullopt);
  }
}

std::unique_ptr<Statement> Parser::parseIfStatement() {
  auto out = std::make_unique<IfStatement>(state);

  bool first = true;

  do {
    if (!first && lexer.peekEq(TokenType::ELSE))
      lexer.next();
    first = false;
    lexer.next().expect(TokenType::IF) << "Expected if keyword (internal error)";

    lexer.next().expect(TokenType::OPEN_PAREN) << "Expected open paren";
    auto cond = parseExpression();
    lexer.next().expect(TokenType::CLOSE_PAREN) << "Expected close paren";

    auto body = parseBlock();

    out->pairs.push_back(std::make_pair(std::move(cond), std::move(body)));

  } while (lexer.peekEq(TokenType::ELSE) && lexer.peek(1).type == TokenType::IF);

  if (lexer.peekEq(TokenType::ELSE)) {
    lexer.next();
    out->else_block = parseBlock();
  }

  return std::move(out);
}

// let ident: type = <expr>;
std::unique_ptr<Statement> Parser::parseLetStatement() {
  lexer.next().expect(TokenType::LET) << "let stmt internal error";
  auto ident = lexer.next();
  ident.expect(TokenType::IDENT) << "expected identifier";
  lexer.next().expect(TokenType::COLON) << "expected colon";
  auto type = parseTypeName();

  if (lexer.peekEq(TokenType::EQ)) {
    lexer.next();
    auto expr = parseExpression();
    return std::make_unique<LetStatement>(state, type, ident.value.getValue(), std::move(expr));
  }

  return std::make_unique<LetStatement>(state, type, ident.value.getValue());
}

std::unique_ptr<Statement> Parser::parseWhileStatement() {
  lexer.next().expect(TokenType::WHILE) << "while stmt internal error";
  lexer.next().expect(TokenType::OPEN_PAREN) << "expected open paren";
  auto expr = parseExpression();
  lexer.next().expect(TokenType::CLOSE_PAREN) << "expected close paren";

  auto body = parseBlock();

  return std::make_unique<WhileStatement>(std::move(expr), std::move(body));
}

std::unique_ptr<Expression> Parser::parseExpression() { return parseAssign(); }

std::unique_ptr<Expression> Parser::parseAssign() {
  if (lexer.peekEq(TokenType::IDENT) && lexer.peek(1).type == TokenType::EQ) {
    auto ident = lexer.next();
    ident.expect(TokenType::IDENT) << "expect ident internal error";
    lexer.next().expect(TokenType::EQ) << "expect eq internal error"; // '='
    auto expr = parseExpression();
    return std::make_unique<AssignExpr>(ident.value.getValue(), std::move(expr));
  }
  return parseComparison();
}

std::unique_ptr<Expression> Parser::parseComparison() {
  auto lhs = parseAddSub();

  while (lexer.peekEq(TokenType::DEQ, TokenType::GE, TokenType::LE, TokenType::GT, TokenType::LT)) {
    auto op_tok = lexer.next();
    BinaryExpr::Op op;
    switch (op_tok.type) {
    case TokenType::DEQ:
      op = BinaryExpr::Op::EQ;
      break;
    case TokenType::GE:
      op = BinaryExpr::Op::GTE;
      break;
    case TokenType::LE:
      op = BinaryExpr::Op::LTE;
      break;
    case TokenType::GT:
      op = BinaryExpr::Op::GT;
      break;
    case TokenType::LT:
      op = BinaryExpr::Op::LT;
      break;
    default:
      op_tok.fail() << "internal error";
      break;
    }
    lhs = std::make_unique<BinaryExpr>(op, std::move(lhs), parseAddSub());
  }

  return lhs;
}
std::unique_ptr<Expression> Parser::parseAddSub() {
  auto lhs = parseMulDiv();

  while (lexer.peekEq(TokenType::ADD, TokenType::SUB)) {
    auto op_tok = lexer.next();
    BinaryExpr::Op op;
    switch (op_tok.type) {
    case TokenType::ADD:
      op = BinaryExpr::Op::ADD;
      break;
    case TokenType::SUB:
      op = BinaryExpr::Op::SUB;
      break;
    default:
      op_tok.fail() << "internal error";
      break;
    }
    lhs = std::make_unique<BinaryExpr>(op, std::move(lhs), parseMulDiv());
  }

  return lhs;
}

std::unique_ptr<Expression> Parser::parseMulDiv() {
  auto lhs = parseUnary();

  while (lexer.peekEq(TokenType::MUL, TokenType::DIV)) {
    auto op_tok = lexer.next();
    BinaryExpr::Op op;
    switch (op_tok.type) {
    case TokenType::MUL:
      op = BinaryExpr::Op::MUL;
      break;
    case TokenType::DIV:
      op = BinaryExpr::Op::DIV;
      break;
    default:
      op_tok.fail() << "internal error";
      break;
    }
    lhs = std::make_unique<BinaryExpr>(op, std::move(lhs), parseUnary());
  }

  return lhs;
}

std::unique_ptr<Expression> Parser::parseUnary() {
  if (lexer.peekEq(TokenType::SUB)) {
    lexer.next();
    return std::make_unique<UnaryExpr>(UnaryExpr::Op::NEG, parseUnary());
  }
  return parsePrimary();
}

std::unique_ptr<Expression> Parser::parseFunctionCall() {
  auto ident = lexer.next();
  ident.expect(TokenType::IDENT) << "Expected ident (internal error)";

  std::vector<std::unique_ptr<Expression>> args;

  lexer.next().expect(TokenType::OPEN_PAREN) << "Expected open paren";

  while (!lexer.peekEq(TokenType::CLOSE_PAREN)) {
    args.push_back(parseExpression());
    if (lexer.peekEq(TokenType::CLOSE_PAREN))
      break;
    lexer.next().expect(TokenType::COMMA) << "Expected comma after argument";
  }

  lexer.next().expect(TokenType::CLOSE_PAREN) << "Expected close paren";

  return std::make_unique<CallExpr>(state, ident.value.getValue(), std::move(args));
}

std::unique_ptr<Expression> Parser::parseArrayAccess() {
  auto ident = lexer.next();
  ident.expect(TokenType::IDENT) << "Expected ident (internal error)";
  auto target = std::make_unique<VariableExpr>(state, ident.value.getValue());

  lexer.next().expect(TokenType::OPEN_SQUARE) << "Expected open paren";

  auto index_expr = parseExpression();

  lexer.next().expect(TokenType::CLOSE_SQUARE) << "Expected close paren";

  return std::make_unique<ArrayAccessExpr>(state, std::move(target), std::move(index_expr));
}

std::unique_ptr<Expression> Parser::parsePrimary() {
  auto next = lexer.peek();
  switch (next.type) {
  case TokenType::IDENT: {
    if (lexer.peek(1).type == TokenType::OPEN_PAREN)
      return parseFunctionCall();
    else if (lexer.peek(1).type == TokenType::OPEN_SQUARE)
      return parseArrayAccess();
    else
      return std::make_unique<VariableExpr>(state, lexer.next().value.getValue());
  }
  case TokenType::OPEN_PAREN: {
    lexer.next().expect(TokenType::OPEN_PAREN) << "internal error";
    auto expr = parseExpression();
    lexer.next().expect(TokenType::CLOSE_PAREN) << "Expected closing paren";
    return std::move(expr);
  }
  case TokenType::FLOAT: {
    lexer.next();
    try {
      auto val = std::stod(next.value.getValue());
      return std::make_unique<FloatLiteral>(state, val);
    } catch (std::exception& e) {
      std::cout << "EXCEPT: " << e.what() << "\n";
      return std::make_unique<FloatLiteral>(state, 0.0);
    }
  }
  case TokenType::INT: {
    lexer.next();
    try {
      auto val = std::stoll(next.value.getValue());
      return std::make_unique<IntLiteral>(state, val);
    } catch (std::exception& e) {
      std::cout << "EXCEPT: " << e.what() << "\n";
      return std::make_unique<IntLiteral>(state, 0);
    }
  }
  case TokenType::STRING: {
    lexer.next();
    return std::make_unique<StringLiteral>(state, next.value.getValue());
  }
  default: {
    next.fail() << "Expected number (temp)";
    return nullptr;
  }
  }
}

std::shared_ptr<Type> Parser::parseTypeName() {
  auto next = lexer.peek();
  next.expect(TokenType::IDENT) << "Expected identifier";
  auto out = parseBasicType();
  while (lexer.peekEq(TokenType::MUL)) {
    lexer.next();
    out = std::make_shared<PtrType>(state, out);
  }
  return out;
}

std::shared_ptr<Type> Parser::parseBasicType() {
  auto next = lexer.next();
  auto val = next.value.getValue();
  if (val == "i8")
    return std::make_shared<IntType>(state, 8);
  else if (val == "i16")
    return std::make_shared<IntType>(state, 16);
  else if (val == "i32")
    return std::make_shared<IntType>(state, 32);
  else if (val == "i64")
    return std::make_shared<IntType>(state, 64);
  else if (val == "f8")
    return std::make_shared<FloatType>(state, 8);
  else if (val == "f16")
    return std::make_shared<FloatType>(state, 16);
  else if (val == "f32")
    return std::make_shared<FloatType>(state, 32);
  else if (val == "f64")
    return std::make_shared<FloatType>(state, 64);
  else if (val == "void")
    return std::make_shared<VoidType>(state);
  else if (val == "str")
    return std::make_shared<PtrType>(state, std::make_shared<IntType>(state, 8));
  next.fail() << "invalid type name";
  return nullptr;
}
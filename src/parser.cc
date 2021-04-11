#include "parser.hh"

#include <iostream>

using namespace llang;

std::unique_ptr<Document> Parser::parseDocument() {
  auto doc = std::make_unique<Document>(state);
  while (!lexer.peekEq(TokenType::END_OF_FILE)) {
    switch (lexer.peek().type) {
    case TokenType::FUNCTION:
      doc->data.push_back(parseFunction());
      break;
    case TokenType::STRUCT:
      doc->data.push_back(parseStructDecl());
      break;
    default:
      lexer.next().fail() << "Unexpected token";
      return std::move(doc);
    }
  }
  return std::move(doc);
}

std::unique_ptr<Node> Parser::parseFunction() {
  auto function_keyword = lexer.next();
  function_keyword.expect(TokenType::FUNCTION) << "Expected function keyword (internal error)";
  auto func_name = lexer.next();
  func_name.expect(TokenType::IDENT) << "Expected function keyword (internal error)";
  lexer.next().expect(TokenType::OPEN_PAREN) << "Unexpected token (expected '(')";
  std::vector<std::pair<std::shared_ptr<Type>, Span>> arguments;
  // Parse args
  while (!lexer.peekEq(TokenType::CLOSE_PAREN)) {
    auto ident = lexer.next();
    ident.expect(TokenType::IDENT) << "Unexpected token (expected IDENT)";
    lexer.next().expect(TokenType::COLON) << "Unexpected token (expected ':')";
    auto type = parseTypeName();
    arguments.push_back(std::make_pair(type, ident.value));
    if (lexer.peekEq(TokenType::CLOSE_PAREN))
      break;
    lexer.next().expect(TokenType::COMMA);
  }
  lexer.next().expect(TokenType::CLOSE_PAREN) << "Unexpected token (expected ')')";
  lexer.next().expect(TokenType::COLON) << "Unexpected token (expected ':')";
  auto return_type = parseTypeName();

  auto proto_func = std::make_unique<ProtoFunc>(state, function_keyword.value, func_name.value,
                                                return_type, arguments);

  if (lexer.peekEq(TokenType::SEMICOLON)) {
    lexer.next();
    return std::make_unique<ExternDecl>(state, std::move(proto_func));
  }

  auto body = parseBlock();

  return std::make_unique<FunctionDecl>(state, std::move(proto_func), std::move(body));
}

std::unique_ptr<Node> Parser::parseStructDecl() {
  auto keyword = lexer.next();
  keyword.expect(TokenType::STRUCT) << "Expected struct keyword (internal error)";
  auto struct_name = lexer.next();
  struct_name.expect(TokenType::IDENT) << "Expected ident";
  lexer.next().expect(TokenType::OPEN_BRACE) << "Expected open brace";
  auto str = std::make_unique<StructProto>(state, keyword.value, struct_name.value);
  // parse fields
  while (!lexer.peekEq(TokenType::CLOSE_BRACE)) {
    auto name = lexer.next();
    name.expect(TokenType::IDENT) << "expected identifier";
    lexer.next().expect(TokenType::COLON) << "expected colon";
    auto type = parseTypeName();
    lexer.next().expect(TokenType::SEMICOLON);
    str->fields.push_back(std::make_pair(name.value, type));
  }
  lexer.next().expect(TokenType::CLOSE_BRACE) << "Unexpected token (expected '}')";
  return std::move(str);
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
  case FOR:
    return parseForStatement();
  default:
    return parseExpression();
  }
}

std::unique_ptr<Statement> Parser::parseReturnStatement() {
  auto keyword = lexer.next();
  keyword.expect(TokenType::RETURN) << "Expected return keyword (internal error)";
  if (lexer.peekEq(TokenType::SEMICOLON)) {
    return std::make_unique<ReturnStatement>(state, keyword.value, std::nullopt);
  }
  auto expr = parseExpression();
  if (expr) {
    return std::make_unique<ReturnStatement>(keyword.value, std::move(expr));
  } else {
    return std::make_unique<ReturnStatement>(state, keyword.value, std::nullopt);
  }
}

std::unique_ptr<Statement> Parser::parseIfStatement() {
  auto out = std::make_unique<IfStatement>(state, lexer.peek().value);

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
  auto keyword = lexer.next();
  keyword.expect(TokenType::LET) << "let stmt internal error";
  auto ident = lexer.next();
  ident.expect(TokenType::IDENT) << "expected identifier";
  lexer.next().expect(TokenType::COLON) << "expected colon";
  auto type = parseTypeName();

  if (lexer.peekEq(TokenType::EQ)) {
    lexer.next();
    auto expr = parseExpression();
    return std::make_unique<LetStatement>(state, type, ident.value, keyword.value, std::move(expr));
  }

  return std::make_unique<LetStatement>(state, type, ident.value, keyword.value);
}

std::unique_ptr<Statement> Parser::parseWhileStatement() {
  auto keyword = lexer.next();
  keyword.expect(TokenType::WHILE) << "while stmt internal error";
  lexer.next().expect(TokenType::OPEN_PAREN) << "expected open paren";
  auto expr = parseExpression();
  lexer.next().expect(TokenType::CLOSE_PAREN) << "expected close paren";

  auto body = parseBlock();

  return std::make_unique<WhileStatement>(keyword.value, std::move(expr), std::move(body));
}

std::unique_ptr<Statement> Parser::parseForStatement() {
  auto keyword = lexer.next();
  keyword.expect(TokenType::FOR) << "for stmt internal error";
  lexer.next().expect(TokenType::OPEN_PAREN) << "expected open paren";
  auto init = parseStatement();
  lexer.next().expect(TokenType::SEMICOLON) << "expected semi";
  auto expr = parseExpression();
  lexer.next().expect(TokenType::SEMICOLON) << "expected semi";
  auto inc = parseExpression();
  lexer.next().expect(TokenType::CLOSE_PAREN) << "expected close paren";

  auto body = parseBlock();

  return std::make_unique<ForStatement>(std::move(init), keyword.value, std::move(expr),
                                        std::move(inc), std::move(body));
}

std::unique_ptr<Expression> Parser::parseExpression() { return parseAssign(); }

std::unique_ptr<Expression> Parser::parseAssign() {
  auto lhs = parseComparison();
  while (lexer.peekEq(TokenType::EQ, TokenType::ADD_EQ, TokenType::MUL_EQ, TokenType::SUB_EQ,
                      TokenType::DIV_EQ)) {
    switch (lexer.peek().type) {
    case EQ: {
      // auto ident = lexer.next();
      // ident.expect(TokenType::IDENT) << "expect ident internal error";
      lexer.next().expect(TokenType::EQ) << "expect eq internal error"; // '='
      auto expr = parseExpression();
      lhs = std::make_unique<AssignExpr>(std::move(lhs), std::move(expr));
      break;
    }
    case ADD_EQ:
    case MUL_EQ:
    case SUB_EQ:
    case DIV_EQ: {
      // auto ident = lexer.next();
      // ident.expect(TokenType::IDENT) << "expect ident internal error";
      auto op_tok = lexer.next();
      auto expr = parseExpression();
      BinaryExpr::Op op;
      if (op_tok.type == TokenType::ADD_EQ)
        op = BinaryExpr::Op::ADD;
      else if (op_tok.type == TokenType::SUB_EQ)
        op = BinaryExpr::Op::SUB;
      else if (op_tok.type == TokenType::DIV_EQ)
        op = BinaryExpr::Op::DIV;
      else if (op_tok.type == TokenType::MUL_EQ)
        op = BinaryExpr::Op::MUL;
      expr = std::make_unique<BinaryExpr>(op, lhs->clone(), std::move(expr));
      lhs = std::make_unique<AssignExpr>(std::move(lhs), std::move(expr));
      break;
    }
    default: {
      break;
    }
    }
  }
  return lhs;
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
  if (lexer.peekEq(TokenType::SUB, TokenType::MUL, TokenType::AND)) {
    auto op_tok = lexer.next();
    UnaryExpr::Op op;
    switch (op_tok.type) {
    case TokenType::SUB:
      op = UnaryExpr::Op::NEG;
      break;
    case TokenType::MUL:
      op = UnaryExpr::Op::DEREF;
      break;
    case TokenType::AND:
      op = UnaryExpr::Op::REF;
      break;
    default:
      op_tok.fail() << "internal error";
      break;
    }
    return std::make_unique<UnaryExpr>(op, parseUnary());
  }
  return parseDotOp();
}

std::unique_ptr<Expression> Parser::parseDotOp() {
  auto lhs = parsePrimary();
  while (lexer.peekEq(TokenType::DOT, TokenType::SUB_GT)) {
    auto op_tok = lexer.next();
    auto ident = lexer.next();
    ident.expect(TokenType::IDENT) << "expected identifier";
    switch (op_tok.type) {
    case TokenType::DOT:
      lhs = std::make_unique<DotExpr>(std::move(lhs), ident.value);
      break;
    case TokenType::SUB_GT:
      lhs = std::make_unique<DotExpr>(
          std::make_unique<UnaryExpr>(UnaryExpr::Op::DEREF, std::move(lhs)), ident.value);
      break;
    default:
      break;
    }
  }
  return lhs;
}
std::unique_ptr<Expression> Parser::parsePrimary() {
  auto next = lexer.peek();
  switch (next.type) {
  case TokenType::IDENT: {
    if (lexer.peek(1).type == TokenType::OPEN_PAREN)
      return parseFunctionCall();
    else if (lexer.peek(1).type == TokenType::OPEN_SQUARE)
      return parseArrayAccess();
    else if (lexer.peek(1).type == TokenType::OPEN_BRACE)
      return parseStructInit();
    else
      return parseVariable();
  }
  case TokenType::OPEN_PAREN: {
    lexer.next().expect(TokenType::OPEN_PAREN) << "internal error";
    auto expr = parseExpression();
    lexer.next().expect(TokenType::CLOSE_PAREN) << "Expected closing paren";
    return std::move(expr);
  }
  case TokenType::FLOAT: {
    lexer.next();
    return std::make_unique<FloatLiteral>(state, next.value);
  }
  case TokenType::INT: {
    lexer.next();
    return std::make_unique<IntLiteral>(state, next.value);
  }
  case TokenType::STRING: {
    lexer.next();
    return std::make_unique<StringLiteral>(state, next.value);
  }
  default: {
    next.fail() << "Expected number (temp)";
    return nullptr;
  }
  }
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

  return std::make_unique<CallExpr>(state, ident.value, std::move(args));
}

std::unique_ptr<Expression> Parser::parseArrayAccess() {
  auto target = parseVariable();

  lexer.next().expect(TokenType::OPEN_SQUARE) << "Expected open paren";

  auto index_expr = parseExpression();

  lexer.next().expect(TokenType::CLOSE_SQUARE) << "Expected close paren";

  return std::make_unique<ArrayAccessExpr>(state, std::move(target), std::move(index_expr));
}

std::unique_ptr<Expression> Parser::parseVariable() {
  auto next = lexer.next();
  next.expect(TokenType::IDENT);
  return std::make_unique<VariableExpr>(state, next.value);
}

std::unique_ptr<Expression> Parser::parseStructInit() {
  auto struct_name = lexer.next();
  struct_name.expect(TokenType::IDENT) << "Expected ident";
  lexer.next().expect(TokenType::OPEN_BRACE) << "Expected open brace";
  auto out = std::make_unique<StructLiteral>(state, struct_name.value);
  // parse fields
  while (!lexer.peekEq(TokenType::CLOSE_BRACE)) {
    auto name = lexer.next();
    name.expect(TokenType::IDENT) << "expected identifier";
    lexer.next().expect(TokenType::COLON) << "expected colon";
    auto expr = parseExpression();
    // parse expression
    lexer.next().expect(TokenType::COMMA);
    out->fields.push_back(std::make_pair(name.value, std::move(expr)));
    // add field
  }
  lexer.next().expect(TokenType::CLOSE_BRACE) << "Unexpected token (expected '}')";
  return std::move(out);
}

std::shared_ptr<Type> Parser::parseTypeName() {
  auto next = lexer.peek();
  next.expect(TokenType::IDENT) << "Expected identifier";
  auto out = parseBasicType();
  while (lexer.peekEq(TokenType::MUL)) {
    auto op = lexer.next();
    out = std::make_shared<PtrType>(state, out, op.value);
  }
  return out;
}

std::shared_ptr<Type> Parser::parseBasicType() {
  auto next = lexer.next();
  auto val = next.value.getValue();
  if (val == "i8")
    return std::make_shared<IntType>(state, 8, next.value);
  else if (val == "i16")
    return std::make_shared<IntType>(state, 16, next.value);
  else if (val == "i32")
    return std::make_shared<IntType>(state, 32, next.value);
  else if (val == "i64")
    return std::make_shared<IntType>(state, 64, next.value);
  else if (val == "f8")
    return std::make_shared<FloatType>(state, 8, next.value);
  else if (val == "f16")
    return std::make_shared<FloatType>(state, 16, next.value);
  else if (val == "f32")
    return std::make_shared<FloatType>(state, 32, next.value);
  else if (val == "f64")
    return std::make_shared<FloatType>(state, 64, next.value);
  else if (val == "void")
    return std::make_shared<VoidType>(state, next.value);
  else if (val == "str")
    return std::make_shared<PtrType>(state, std::make_shared<IntType>(state, 8, next.value),
                                     next.value);
  // next.fail() << "invalid type name";
  return std::make_shared<StructType>(state, next.value);
}
#include "lexer.hh"
#include <iostream>

using namespace llang;

char Lexer::nextCh() {
  if (eof())
    return ' ';
  return source->value.at(current_span.length++ + current_span.start);
}

char Lexer::peekCh() { return peekCh(0); }

char Lexer::peekCh(std::size_t i) {
  if (eof())
    return ' ';
  return source->value.at(current_span.length + current_span.start + i);
}

bool alpha(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }

bool numeric(char c) { return ('0' <= c && c <= '9'); }

bool dot(char c) { return (c == '.'); }

bool whitespace(char c) { return (c == ' ' || c == '\n' || c == '\t'); }

bool identifier(char c) { return alpha(c) || numeric(c) || c == '_'; }
bool identifierFirst(char c) { return alpha(c) || c == '_'; }

bool notQuote(char c) { return c != '"'; }
bool notNL(char c) { return c != '\n'; }

inline bool Lexer::eof() {
  return (current_span.length + current_span.start >= (source->value.size()));
}

bool Lexer::eat(EatTestFn fn) {
  auto peeked = peekCh();
  if (fn(peeked)) {
    nextCh();
    return true;
  }
  return false;
}

template <class... Args> bool Lexer::eat(EatTestFn fn, Args... args) {
  auto peeked = peekCh();
  if (fn(peeked)) {
    nextCh();
    return true;
  }
  return false || eat(args...);
}

bool Lexer::eat(char character) {
  auto peeked = peekCh();
  if (peeked == character) {
    nextCh();
    return true;
  }
  return false;
}

template <class... Args> std::size_t Lexer::consume(Args... args) {
  auto i = 0;
  while (eat(args...)) {
    i++;
    if (eof()) {
      break;
    }
  }
  return i;
}

Span Lexer::closeSpan() {
  auto result = current_span;
  current_span = Span(source, current_span.start + current_span.length, 0);
  return result;
}

Token Lexer::next() {
  if (!peek_stack.empty()) {
    // If only c++ pop returned the value...
    auto next = peek_stack.front();
    peek_stack.pop_front();
    return next;
  }

  return lexNext();
}

Token Lexer::lexNext() {
  if (eof()) {
    return Token(TokenType::END_OF_FILE, closeSpan());
  }
  auto next = nextCh();
  switch (next) {
  case '.':
    return Token(TokenType::DOT, closeSpan());
  case '(':
    return Token(TokenType::OPEN_PAREN, closeSpan());
  case ')':
    return Token(TokenType::CLOSE_PAREN, closeSpan());
  case '{':
    return Token(TokenType::OPEN_BRACE, closeSpan());
  case '}':
    return Token(TokenType::CLOSE_BRACE, closeSpan());
  case '[':
    return Token(TokenType::OPEN_SQUARE, closeSpan());
  case ']':
    return Token(TokenType::CLOSE_SQUARE, closeSpan());
  case '&':
    return Token(TokenType::AND, closeSpan());
  case '+':
    if (eat('='))
      return Token(TokenType::ADD_EQ, closeSpan());
    return Token(TokenType::ADD, closeSpan());
  case '-':
    if (eat('>'))
      return Token(TokenType::SUB_GT, closeSpan());
    if (eat('='))
      return Token(TokenType::SUB_EQ, closeSpan());
    return Token(TokenType::SUB, closeSpan());
  case '*':
    if (eat('='))
      return Token(TokenType::MUL_EQ, closeSpan());
    return Token(TokenType::MUL, closeSpan());
  case '/':
    if (eat('='))
      return Token(TokenType::DIV_EQ, closeSpan());
    if (peekCh() == '/') {
      consume(notNL);
      closeSpan();
      return lexNext();
    }
    return Token(TokenType::DIV, closeSpan());
  case ',':
    return Token(TokenType::COMMA, closeSpan());
  case ':':
    return Token(TokenType::COLON, closeSpan());
  case ';':
    return Token(TokenType::SEMICOLON, closeSpan());
  case '<':
    if (eat('='))
      return Token(TokenType::LE, closeSpan());
    return Token(TokenType::LT, closeSpan());
  case '>':
    if (eat('='))
      return Token(TokenType::GE, closeSpan());
    return Token(TokenType::GT, closeSpan());
  case '=':
    if (eat('='))
      return Token(TokenType::DEQ, closeSpan());
    return Token(TokenType::EQ, closeSpan());
    // return Token(TokenType::WHITESPACE, closeSpan());
  case '"':
    closeSpan();
    consume(notQuote);
    auto sp = closeSpan();
    eat('"');
    closeSpan();
    return Token(TokenType::STRING, sp);
  }

  if (whitespace(next)) {
    consume(whitespace);
    closeSpan();
    return lexNext();
  }

  if (identifierFirst(next)) {
    consume(identifier);
    auto v = current_span.getValue();
    if (v == "function")
      return Token(TokenType::FUNCTION, closeSpan());
    else if (v == "if")
      return Token(TokenType::IF, closeSpan());
    else if (v == "else")
      return Token(TokenType::ELSE, closeSpan());
    else if (v == "while")
      return Token(TokenType::WHILE, closeSpan());
    else if (v == "for")
      return Token(TokenType::FOR, closeSpan());
    else if (v == "let")
      return Token(TokenType::LET, closeSpan());
    else if (v == "struct")
      return Token(TokenType::STRUCT, closeSpan());
    else if (v == "return")
      return Token(TokenType::RETURN, closeSpan());
    return Token(TokenType::IDENT, closeSpan());
  }

  if (numeric(next)) {
    consume(numeric);
    if (eat(dot)) {
      consume(numeric);
      return Token(TokenType::FLOAT, closeSpan());
    }
    return Token(TokenType::INT, closeSpan());
  }

  // return Token(TokenType::END_OF_FILE, closeSpan());
  current_span.fail() << "Unexpected token";
  return Token(TokenType::END_OF_FILE, closeSpan());
}

// Token Lexer::nextString() {
//   consume(notQuote);
//   // TODO: implement escape quote
//   if (eof()) {
//     current_span.subspan(0, 1).fail() << "expected closing quote";
//     return lexNext();
//   }
//   current_span.expect(eat('"')) << "expected closing quote";
//   return Token(TokenType::STRING, closeSpan());
// }

const Token& Lexer::peek(std::size_t n) {
  if (!peek_stack.empty() && n < peek_stack.size() - 1) {
    return peek_stack.at(n);
  }

  for (int i = peek_stack.size(); i <= n; i++) {
    peek_stack.push_back(lexNext());
  }

  return peek_stack.at(n);
}

bool Lexer::peekEq(TokenType type) { return peek().type == type; }

std::ostream& Token::expect(TokenType kind, MessageType m_type) {
  return value.expect(type == kind, m_type);
}

std::ostream& Token::fail(MessageType m_type) { return value.fail(m_type); }

#pragma once

#include "source.hh"
#include <cstdlib>
#include <deque>
#include <memory>
#include <ostream>
#include <string>

namespace llang {

typedef bool (*EatTestFn)(char c);

enum TokenType {
  WHITESPACE,
  END_OF_FILE,
  OPEN_PAREN,
  CLOSE_PAREN,
  OPEN_BRACE,
  CLOSE_BRACE,
  OPEN_SQUARE,
  CLOSE_SQUARE,
  ADD,
  SUB,
  MUL,
  DIV,
  EQ,
  DEQ,
  LT,
  GT,
  LE,
  GE,
  COMMA,
  IDENT,
  FUNCTION,
  COLON,
  IF,
  ELSE,
  WHILE,
  RETURN,
  FLOAT,
  INT,
  SEMICOLON,
  LET,
  STRING
};

class Token {
public:
  Token(TokenType type, Span value) : type(type), value(value) {}
  Token(TokenType type, std::shared_ptr<Source> source, std::size_t start, std::size_t length)
      : type(type), value(source, start, length) {}
  std::ostream& expect(TokenType kind, MessageType m_type = MessageType::ERROR);
  std::ostream& fail(MessageType m_type = MessageType::ERROR);

public:
  TokenType type;
  Span value;
};

class Lexer {
public:
  Lexer(std::shared_ptr<Source> source) : source(source), current_span(source) {}
  Token next();
  bool peekEq(TokenType type);
  template <class... Args> bool peekEq(TokenType type, Args... args) {
    return peek().type == type || peekEq(args...);
  }
  const Token& peek(std::size_t n = 0);

private:
  Token lexNext();
  Token nextString();

  char nextCh();
  char peekCh();
  char peekCh(std::size_t i);

  template <class... Args> bool eat(EatTestFn fn, Args... args);
  bool eat(EatTestFn fn);
  bool eat(char character);
  inline bool eof();
  template <class... Args> std::size_t consume(Args... args);
  Span closeSpan();

public:
  std::shared_ptr<Source> source;

private:
  std::deque<Token> peek_stack;
  Span current_span;
};

}; // namespace llang

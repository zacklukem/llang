// Copyright 2020 Zachary Mayhew
#pragma once

#include "state.hh"
#include <llvm/IR/Value.h>
#include <memory>

namespace llang {

class Node {
public:
  Node(std::shared_ptr<State> state) : state(state){};
  Node() = delete;
  std::shared_ptr<State> state;
  virtual ~Node(){};
  virtual llvm::Value* codegen() = 0;
};

class Statement : public Node {
public:
  Statement(std::shared_ptr<State> state) : Node(state){};
};

class Block : public Node {
public:
  using StmtList = std::vector<std::unique_ptr<Statement>>;

  StmtList stmts;

  Block(std::shared_ptr<State> state, StmtList stmts) : Node(state), stmts(std::move(stmts)){};

  virtual llvm::Value* codegen() override;
};

class Expression : public Statement {
public:
  Expression(std::shared_ptr<State> state) : Statement(state){};
};

class ReturnStatement : public Statement {
public:
  std::unique_ptr<Expression> rhs;

  ReturnStatement(std::shared_ptr<State> state, std::unique_ptr<Expression> rhs)
      : Statement(state), rhs(std::move(rhs)){};

  ReturnStatement(std::unique_ptr<Expression> rhs) : Statement(rhs->state), rhs(std::move(rhs)){};

  virtual llvm::Value* codegen() override;
};

class IfStatement : public Statement {
public:
  using IfPair = std::pair<std::unique_ptr<Expression>, Block>;

  std::vector<IfPair> pairs;
  std::optional<Block> else_block;

  IfStatement(std::shared_ptr<State> state, std::optional<Block> else_block = std::nullopt)
      : Statement(state), else_block(std::move(else_block)) {}

  virtual llvm::Value* codegen() override;
};

class BinaryExpr : public Expression {
public:
  enum Op { ADD, SUB, DIV, MUL, EQ, LT, GT, LTE, GTE };
  Op oper;
  std::unique_ptr<Expression> lhs;
  std::unique_ptr<Expression> rhs;

  BinaryExpr(std::shared_ptr<State> state, Op oper, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
      : Expression(state), oper(oper), lhs(std::move(lhs)), rhs(std::move(rhs)){};

  BinaryExpr(Op oper, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
      : Expression(lhs->state), oper(oper), lhs(std::move(lhs)), rhs(std::move(rhs)){};

  virtual llvm::Value* codegen() override;
};

class UnaryExpr : public Expression {
public:
  enum Op { NEG };
  Op oper;
  std::unique_ptr<Expression> rhs;

  UnaryExpr(std::shared_ptr<State> state, Op oper, std::unique_ptr<Expression> rhs)
      : Expression(state), oper(oper), rhs(std::move(rhs)){};

  UnaryExpr(Op oper, std::unique_ptr<Expression> rhs) : Expression(rhs->state), oper(oper), rhs(std::move(rhs)){};

  virtual llvm::Value* codegen() override;
};

class CallExpr : public Expression {
public:
  std::string target;
  std::vector<std::unique_ptr<Expression>> args;

  CallExpr(std::shared_ptr<State> state, std::string target, std::vector<std::unique_ptr<Expression>> args)
      : Expression(state), target(target), args(std::move(args)){};

  virtual llvm::Value* codegen() override;
};

class VariableExpr : public Expression {
public:
  std::string value;

  VariableExpr(std::shared_ptr<State> state, std::string value) : Expression(state), value(value){};
  virtual llvm::Value* codegen() override;
};

enum Type { NUMBER };

class NumberLiteral : public Expression {
public:
  double value;

  NumberLiteral(std::shared_ptr<State> state, double value) : Expression(state), value(value){};
  virtual llvm::Value* codegen() override;
};

class ProtoFunc : public Node {
public:
  std::string name;
  Type return_type;
  std::vector<std::pair<Type, std::string>> args;

  ProtoFunc(std::shared_ptr<State> state, const std::string& name, Type return_type,
            std::vector<std::pair<Type, std::string>> args)
      : Node(state), name(name), return_type(return_type), args(std::move(args)){};

  virtual llvm::Value* codegen() override { return proto_codegen(); };
  llvm::Function* proto_codegen();
};

class FunctionDecl : public Node {
public:
  std::unique_ptr<ProtoFunc> proto;
  Block body;

  FunctionDecl(std::shared_ptr<State> state, std::unique_ptr<ProtoFunc> proto, Block body)
      : Node(state), proto(std::move(proto)), body(std::move(body)) {}

  virtual llvm::Value* codegen() override;
};

} // namespace llang

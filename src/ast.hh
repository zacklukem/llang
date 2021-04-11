// Copyright 2020 Zachary Mayhew
#pragma once

#include "source.hh"
namespace llang {
class Type;
class StructProto;
class ProtoFunc;
} // namespace llang
#include "state.hh"
#include <llvm/IR/Value.h>
#include <memory>
#include <variant>

namespace llang {

class Type {
public:
  enum TypeKind { TK_PTR_TYPE, TK_INT_TYPE, TK_FLOAT_TYPE, TK_STRUCT_TYPE, TK_VOID_TYPE };
  const TypeKind kind;
  Type(TypeKind kind, std::shared_ptr<State> state) : kind(kind), state(state){};
  Type() = delete;
  std::shared_ptr<State> state;
  virtual ~Type(){};
  virtual llvm::Type* codegen() = 0;
  virtual Span span() const = 0;
  virtual std::string str() const = 0;
  virtual bool operator==(const Type& rhs) const = 0;
  inline bool operator!=(const Type& rhs) const { return !(*this == rhs); };
};

class PtrType : public Type {
public:
  std::shared_ptr<Type> type;
  Span mul;
  PtrType(std::shared_ptr<State> state, std::shared_ptr<Type> type, Span mul)
      : Type(Type::TypeKind::TK_PTR_TYPE, state), type(std::move(type)), mul(mul){};
  PtrType() = delete;
  virtual llvm::Type* codegen() override;
  virtual Span span() const override;
  virtual std::string str() const override;
  virtual bool operator==(const Type& rhs) const override;
  static bool classof(const Type* s) { return s->kind == TK_PTR_TYPE; };
};

class IntType : public Type {
public:
  uint8_t size;
  Span name;
  IntType(std::shared_ptr<State> state, uint8_t size, Span name)
      : Type(Type::TypeKind::TK_INT_TYPE, state), size(size), name(name){};
  IntType() = delete;
  virtual llvm::Type* codegen() override;
  virtual Span span() const override;
  virtual std::string str() const override;
  virtual bool operator==(const Type& rhs) const override;
  static bool classof(const Type* s) { return s->kind == TK_INT_TYPE; };
};

class FloatType : public Type {
public:
  uint8_t size;
  Span name;
  FloatType(std::shared_ptr<State> state, uint8_t size, Span name)
      : Type(Type::TypeKind::TK_FLOAT_TYPE, state), size(size), name(name){};
  FloatType() = delete;
  virtual llvm::Type* codegen() override;
  virtual Span span() const override;
  virtual std::string str() const override;
  virtual bool operator==(const Type& rhs) const override;
  static bool classof(const Type* s) { return s->kind == TK_FLOAT_TYPE; };
};

class StructType : public Type {
public:
  Span name;
  StructType(std::shared_ptr<State> state, Span name)
      : Type(Type::TypeKind::TK_STRUCT_TYPE, state), name(name){};
  StructType() = delete;
  virtual llvm::Type* codegen() override;
  virtual Span span() const override;
  virtual std::string str() const override;
  virtual bool operator==(const Type& rhs) const override;
  static bool classof(const Type* s) { return s->kind == TK_STRUCT_TYPE; };
};

class VoidType : public Type {
public:
  Span name;
  VoidType(std::shared_ptr<State> state, Span name)
      : Type(Type::TypeKind::TK_VOID_TYPE, state), name(name){};
  VoidType() = delete;
  virtual llvm::Type* codegen() override;
  virtual Span span() const override;
  virtual std::string str() const override;
  virtual bool operator==(const Type& rhs) const override;
  static bool classof(const Type* s) { return s->kind == TK_VOID_TYPE; };
};

class Node {
public:
  Node(std::shared_ptr<State> state) : state(state){};
  Node() = delete;
  std::shared_ptr<State> state;
  virtual ~Node(){};
  virtual llvm::Value* codegen() = 0;
  virtual void print(std::ostream& target) const {};
  virtual Span span() const = 0;
  virtual std::shared_ptr<Type> verify() = 0;
};

class Document : public Node {
public:
  std::vector<std::unique_ptr<Node>> data;
  Document(std::shared_ptr<State> state) : Node(state){};
  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
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
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class Expression : public Statement {
public:
  Expression(std::shared_ptr<State> state) : Statement(state){};
  virtual std::unique_ptr<Expression> clone() const { return nullptr; };
};

class LetStatement : public Statement {
public:
  Span ident;
  Span name;
  std::shared_ptr<Type> type;
  std::optional<std::unique_ptr<Expression>> rhs;

  LetStatement(std::shared_ptr<State> state, std::shared_ptr<Type> type, Span name, Span ident,
               std::optional<std::unique_ptr<Expression>> rhs = std::nullopt)
      : Statement(state), type(std::move(type)), name(name), ident(ident), rhs(std::move(rhs)){};

  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class ReturnStatement : public Statement {
public:
  Span ident;
  std::optional<std::unique_ptr<Expression>> rhs;

  ReturnStatement(std::shared_ptr<State> state, Span ident,
                  std::optional<std::unique_ptr<Expression>> rhs)
      : Statement(state), ident(ident), rhs(std::move(rhs)){};

  ReturnStatement(Span ident, std::unique_ptr<Expression> rhs)
      : Statement(rhs->state), ident(ident), rhs(std::move(std::make_optional(std::move(rhs)))){};

  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class IfStatement : public Statement {
public:
  using IfPair = std::pair<std::unique_ptr<Expression>, Block>;

  Span ident;
  std::vector<IfPair> pairs;
  std::optional<Block> else_block;

  IfStatement(std::shared_ptr<State> state, Span ident,
              std::optional<Block> else_block = std::nullopt)
      : Statement(state), ident(ident), else_block(std::move(else_block)) {}

  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class WhileStatement : public Statement {
public:
  Span ident;
  std::unique_ptr<Expression> cond;
  Block body;

  WhileStatement(std::shared_ptr<State> state, Span ident, std::unique_ptr<Expression> cond,
                 Block body)
      : Statement(state), ident(ident), cond(std::move(cond)), body(std::move(body)){};
  WhileStatement(Span ident, std::unique_ptr<Expression> cond, Block body)
      : Statement(cond->state), ident(ident), cond(std::move(cond)), body(std::move(body)){};

  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class ForStatement : public Statement {
public:
  Span ident;
  std::unique_ptr<Statement> init;
  std::unique_ptr<Expression> cond;
  std::unique_ptr<Expression> inc;

  Block body;

  ForStatement(std::shared_ptr<State> state, Span ident, std::unique_ptr<Statement> init,
               std::unique_ptr<Expression> cond, std::unique_ptr<Expression> inc, Block body)
      : Statement(state), ident(ident), init(std::move(init)), cond(std::move(cond)),
        inc(std::move(inc)), body(std::move(body)){};
  ForStatement(std::unique_ptr<Statement> init, Span ident, std::unique_ptr<Expression> cond,
               std::unique_ptr<Expression> inc, Block body)
      : Statement(cond->state), ident(ident), init(std::move(init)), cond(std::move(cond)),
        inc(std::move(inc)), body(std::move(body)){};

  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class AssignExpr : public Expression {
public:
  std::unique_ptr<Expression> lhs;
  std::unique_ptr<Expression> rhs;

  AssignExpr(std::shared_ptr<State> state, std::unique_ptr<Expression> lhs,
             std::unique_ptr<Expression> rhs)
      : Expression(state), lhs(std::move(lhs)), rhs(std::move(rhs)){};

  AssignExpr(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
      : Expression(rhs->state), lhs(std::move(lhs)), rhs(std::move(rhs)){};

  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class BinaryExpr : public Expression {
public:
  enum Op { ADD, SUB, DIV, MUL, EQ, LT, GT, LTE, GTE };
  Op oper;
  std::unique_ptr<Expression> lhs;
  std::unique_ptr<Expression> rhs;

  BinaryExpr(std::shared_ptr<State> state, Op oper, std::unique_ptr<Expression> lhs,
             std::unique_ptr<Expression> rhs)
      : Expression(state), oper(oper), lhs(std::move(lhs)), rhs(std::move(rhs)){};

  BinaryExpr(Op oper, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
      : Expression(lhs->state), oper(oper), lhs(std::move(lhs)), rhs(std::move(rhs)){};

  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class UnaryExpr : public Expression {
public:
  enum Op { NEG, DEREF, REF };
  Op oper;
  std::unique_ptr<Expression> rhs;

  UnaryExpr(std::shared_ptr<State> state, Op oper, std::unique_ptr<Expression> rhs)
      : Expression(state), oper(oper), rhs(std::move(rhs)){};

  UnaryExpr(Op oper, std::unique_ptr<Expression> rhs)
      : Expression(rhs->state), oper(oper), rhs(std::move(rhs)){};

  virtual llvm::Value* codegen() override;
  virtual std::unique_ptr<Expression> clone() const override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class CallExpr : public Expression {
public:
  Span target;
  std::vector<std::unique_ptr<Expression>> args;

  CallExpr(std::shared_ptr<State> state, Span target, std::vector<std::unique_ptr<Expression>> args)
      : Expression(state), target(target), args(std::move(args)){};

  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class VariableExpr : public Expression {
public:
  Span value;

  VariableExpr(std::shared_ptr<State> state, Span value) : Expression(state), value(value){};
  virtual llvm::Value* codegen() override;
  virtual std::unique_ptr<Expression> clone() const override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class DotExpr : public Expression {
public:
  std::unique_ptr<Expression> lhs;
  Span rhs;

  DotExpr(std::shared_ptr<State> state, std::unique_ptr<Expression> lhs, Span rhs)
      : Expression(state), lhs(std::move(lhs)), rhs(rhs){};
  DotExpr(std::unique_ptr<Expression> lhs, Span rhs)
      : Expression(lhs->state), lhs(std::move(lhs)), rhs(rhs){};

  virtual llvm::Value* codegen() override;
  virtual std::unique_ptr<Expression> clone() const override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class ArrayAccessExpr : public Expression {
public:
  std::unique_ptr<Expression> target;
  std::unique_ptr<Expression> index;

  ArrayAccessExpr(std::shared_ptr<State> state, std::unique_ptr<Expression> target,
                  std::unique_ptr<Expression> index)
      : Expression(state), target(std::move(target)), index(std::move(index)){};

  virtual llvm::Value* codegen() override;
  virtual std::unique_ptr<Expression> clone() const override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class FloatLiteral : public Expression {
public:
  Span value;

  FloatLiteral(std::shared_ptr<State> state, Span value) : Expression(state), value(value){};
  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class StringLiteral : public Expression {
public:
  Span value;

  StringLiteral(std::shared_ptr<State> state, Span value) : Expression(state), value(value){};
  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class IntLiteral : public Expression {
public:
  Span value;

  IntLiteral(std::shared_ptr<State> state, Span value) : Expression(state), value(value){};
  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class StructLiteral : public Expression {
public:
  std::vector<std::pair<Span, std::unique_ptr<Expression>>> fields;
  Span name;

  StructLiteral(std::shared_ptr<State> state, Span name) : Expression(state), name(name){};
  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class ProtoFunc : public Node {
public:
  Span name;
  std::shared_ptr<Type> return_type;
  std::vector<std::pair<std::shared_ptr<Type>, Span>> args;
  Span ident;

  ProtoFunc(std::shared_ptr<State> state, Span ident, Span name, std::shared_ptr<Type> return_type,
            std::vector<std::pair<std::shared_ptr<Type>, Span>> args)
      : Node(state), ident(ident), name(name), return_type(std::move(return_type)),
        args(std::move(args)){};

  virtual llvm::Value* codegen() override { return proto_codegen(); };
  llvm::Function* proto_codegen();
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class ExternDecl : public Node {
public:
  std::unique_ptr<ProtoFunc> proto;
  ExternDecl(std::shared_ptr<State> state, std::unique_ptr<ProtoFunc> proto)
      : Node(state), proto(std::move(proto)) {}
  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class FunctionDecl : public Node {
public:
  std::unique_ptr<ProtoFunc> proto;
  Block body;

  FunctionDecl(std::shared_ptr<State> state, std::unique_ptr<ProtoFunc> proto, Block body)
      : Node(state), proto(std::move(proto)), body(std::move(body)) {}

  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class StructProto : public Node {
public:
  Span ident;
  Span name;
  std::vector<std::pair<Span, std::shared_ptr<Type>>> fields;
  StructProto(std::shared_ptr<State> state, Span ident, Span name)
      : Node(state), ident(ident), name(name){};

  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

class ImportDecl : public Node {
public:
  Span file_name;
  ImportDecl(std::shared_ptr<State> state, Span file_name) : Node(state), file_name(file_name){};

  virtual llvm::Value* codegen() override;
  virtual Span span() const override;
  virtual std::shared_ptr<Type> verify() override;
};

} // namespace llang

std::ostream& operator<<(std::ostream& out, const llang::Node& node);
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

class PtrType;
class IntType;
class FloatType;
class StructType;
class VoidType;
class Document;
class Block;
class LetStatement;
class ReturnStatement;
class IfStatement;
class WhileStatement;
class ForStatement;
class AssignExpr;
class BinaryExpr;
class UnaryExpr;
class CallExpr;
class VariableExpr;
class DotExpr;
class ArrayAccessExpr;
class FloatLiteral;
class StringLiteral;
class IntLiteral;
class StructLiteral;
class ProtoFunc;
class ExternDecl;
class FunctionDecl;
class StructProto;
class ImportDecl;

class AstVisitorBase {
public:
  virtual void doVisitPtrType(PtrType& self) = 0;
  virtual void doVisitIntType(IntType& self) = 0;
  virtual void doVisitFloatType(FloatType& self) = 0;
  virtual void doVisitStructType(StructType& self) = 0;
  virtual void doVisitVoidType(VoidType& self) = 0;
  virtual void doVisitDocument(Document& self) = 0;
  virtual void doVisitBlock(Block& self) = 0;
  virtual void doVisitLetStatement(LetStatement& self) = 0;
  virtual void doVisitReturnStatement(ReturnStatement& self) = 0;
  virtual void doVisitIfStatement(IfStatement& self) = 0;
  virtual void doVisitWhileStatement(WhileStatement& self) = 0;
  virtual void doVisitForStatement(ForStatement& self) = 0;
  virtual void doVisitAssignExpr(AssignExpr& self) = 0;
  virtual void doVisitBinaryExpr(BinaryExpr& self) = 0;
  virtual void doVisitUnaryExpr(UnaryExpr& self) = 0;
  virtual void doVisitCallExpr(CallExpr& self) = 0;
  virtual void doVisitVariableExpr(VariableExpr& self) = 0;
  virtual void doVisitDotExpr(DotExpr& self) = 0;
  virtual void doVisitArrayAccessExpr(ArrayAccessExpr& self) = 0;
  virtual void doVisitFloatLiteral(FloatLiteral& self) = 0;
  virtual void doVisitStringLiteral(StringLiteral& self) = 0;
  virtual void doVisitIntLiteral(IntLiteral& self) = 0;
  virtual void doVisitStructLiteral(StructLiteral& self) = 0;
  virtual void doVisitProtoFunc(ProtoFunc& self) = 0;
  virtual void doVisitExternDecl(ExternDecl& self) = 0;
  virtual void doVisitFunctionDecl(FunctionDecl& self) = 0;
  virtual void doVisitStructProto(StructProto& self) = 0;
  virtual void doVisitImportDecl(ImportDecl& self) = 0;
};

enum AstVisitorExpected { _T, _N, _S, _E };
template <typename T, typename N, typename S, typename E> class AstVisitor : public AstVisitorBase {
public:
  T ret_val_t;
  N ret_val_n;
  S ret_val_s;
  E ret_val_e;
  AstVisitorExpected expected;

protected:
  virtual T visitPtrType(PtrType& self) = 0;
  virtual T visitIntType(IntType& self) = 0;
  virtual T visitFloatType(FloatType& self) = 0;
  virtual T visitStructType(StructType& self) = 0;
  virtual T visitVoidType(VoidType& self) = 0;
  virtual N visitDocument(Document& self) = 0;
  virtual N visitBlock(Block& self) = 0;
  virtual S visitLetStatement(LetStatement& self) = 0;
  virtual S visitReturnStatement(ReturnStatement& self) = 0;
  virtual S visitIfStatement(IfStatement& self) = 0;
  virtual S visitWhileStatement(WhileStatement& self) = 0;
  virtual S visitForStatement(ForStatement& self) = 0;
  virtual E visitAssignExpr(AssignExpr& self) = 0;
  virtual E visitBinaryExpr(BinaryExpr& self) = 0;
  virtual E visitUnaryExpr(UnaryExpr& self) = 0;
  virtual E visitCallExpr(CallExpr& self) = 0;
  virtual E visitVariableExpr(VariableExpr& self) = 0;
  virtual E visitDotExpr(DotExpr& self) = 0;
  virtual E visitArrayAccessExpr(ArrayAccessExpr& self) = 0;
  virtual E visitFloatLiteral(FloatLiteral& self) = 0;
  virtual E visitStringLiteral(StringLiteral& self) = 0;
  virtual E visitIntLiteral(IntLiteral& self) = 0;
  virtual E visitStructLiteral(StructLiteral& self) = 0;
  virtual N visitProtoFunc(ProtoFunc& self) = 0;
  virtual N visitExternDecl(ExternDecl& self) = 0;
  virtual N visitFunctionDecl(FunctionDecl& self) = 0;
  virtual N visitStructProto(StructProto& self) = 0;
  virtual N visitImportDecl(ImportDecl& self) = 0;

public:
  virtual void doVisitPtrType(PtrType& self) override {
    ret_val_t = visitPtrType(self);
    expected = AstVisitorExpected::_T;
  };
  virtual void doVisitIntType(IntType& self) override {
    ret_val_t = visitIntType(self);
    expected = AstVisitorExpected::_T;
  };
  virtual void doVisitFloatType(FloatType& self) override {
    ret_val_t = visitFloatType(self);
    expected = AstVisitorExpected::_T;
  };
  virtual void doVisitStructType(StructType& self) override {
    ret_val_t = visitStructType(self);
    expected = AstVisitorExpected::_T;
  };
  virtual void doVisitVoidType(VoidType& self) override {
    ret_val_t = visitVoidType(self);
    expected = AstVisitorExpected::_T;
  };
  virtual void doVisitDocument(Document& self) override {
    ret_val_n = visitDocument(self);
    expected = AstVisitorExpected::_N;
  };
  virtual void doVisitBlock(Block& self) override {
    ret_val_n = visitBlock(self);
    expected = AstVisitorExpected::_N;
  };
  virtual void doVisitLetStatement(LetStatement& self) override {
    ret_val_s = visitLetStatement(self);
    expected = AstVisitorExpected::_S;
  };
  virtual void doVisitReturnStatement(ReturnStatement& self) override {
    ret_val_s = visitReturnStatement(self);
    expected = AstVisitorExpected::_S;
  };
  virtual void doVisitIfStatement(IfStatement& self) override {
    ret_val_s = visitIfStatement(self);
    expected = AstVisitorExpected::_S;
  };
  virtual void doVisitWhileStatement(WhileStatement& self) override {
    ret_val_s = visitWhileStatement(self);
    expected = AstVisitorExpected::_S;
  };
  virtual void doVisitForStatement(ForStatement& self) override {
    ret_val_s = visitForStatement(self);
    expected = AstVisitorExpected::_S;
  };
  virtual void doVisitAssignExpr(AssignExpr& self) override {
    ret_val_e = visitAssignExpr(self);
    expected = AstVisitorExpected::_E;
  };
  virtual void doVisitBinaryExpr(BinaryExpr& self) override {
    ret_val_e = visitBinaryExpr(self);
    expected = AstVisitorExpected::_E;
  };
  virtual void doVisitUnaryExpr(UnaryExpr& self) override {
    ret_val_e = visitUnaryExpr(self);
    expected = AstVisitorExpected::_E;
  };
  virtual void doVisitCallExpr(CallExpr& self) override {
    ret_val_e = visitCallExpr(self);
    expected = AstVisitorExpected::_E;
  };
  virtual void doVisitVariableExpr(VariableExpr& self) override {
    ret_val_e = visitVariableExpr(self);
    expected = AstVisitorExpected::_E;
  };
  virtual void doVisitDotExpr(DotExpr& self) override {
    ret_val_e = visitDotExpr(self);
    expected = AstVisitorExpected::_E;
  };
  virtual void doVisitArrayAccessExpr(ArrayAccessExpr& self) override {
    ret_val_e = visitArrayAccessExpr(self);
    expected = AstVisitorExpected::_E;
  };
  virtual void doVisitFloatLiteral(FloatLiteral& self) override {
    ret_val_e = visitFloatLiteral(self);
    expected = AstVisitorExpected::_E;
  };
  virtual void doVisitStringLiteral(StringLiteral& self) override {
    ret_val_e = visitStringLiteral(self);
    expected = AstVisitorExpected::_E;
  };
  virtual void doVisitIntLiteral(IntLiteral& self) override {
    ret_val_e = visitIntLiteral(self);
    expected = AstVisitorExpected::_E;
  };
  virtual void doVisitStructLiteral(StructLiteral& self) override {
    ret_val_e = visitStructLiteral(self);
    expected = AstVisitorExpected::_E;
  };
  virtual void doVisitProtoFunc(ProtoFunc& self) override {
    ret_val_n = visitProtoFunc(self);
    expected = AstVisitorExpected::_N;
  };
  virtual void doVisitExternDecl(ExternDecl& self) override {
    ret_val_n = visitExternDecl(self);
    expected = AstVisitorExpected::_N;
  };
  virtual void doVisitFunctionDecl(FunctionDecl& self) override {
    ret_val_n = visitFunctionDecl(self);
    expected = AstVisitorExpected::_N;
  };
  virtual void doVisitStructProto(StructProto& self) override {
    ret_val_n = visitStructProto(self);
    expected = AstVisitorExpected::_N;
  };
  virtual void doVisitImportDecl(ImportDecl& self) override {
    ret_val_n = visitImportDecl(self);
    expected = AstVisitorExpected::_N;
  };
};

class AstVisitorAcceptor {
protected:
  virtual void doAccept(AstVisitorBase& v) = 0;

public:
  template <typename T, typename N, typename S, typename E> E accept_e(AstVisitor<T, N, S, E>& v) {
    doAccept(v);
    assert(v.expected == AstVisitorExpected::_E);
    return v.ret_val_e;
  };
  template <typename T, typename N, typename S, typename E> N accept_n(AstVisitor<T, N, S, E>& v) {
    doAccept(v);
    assert(v.expected == AstVisitorExpected::_N);
    return v.ret_val_n;
  };
  template <typename T, typename N, typename S, typename E> S accept_s(AstVisitor<T, N, S, E>& v) {
    doAccept(v);
    assert(v.expected == AstVisitorExpected::_S || v.expected == AstVisitorExpected::_E);
    if (v.expected == AstVisitorExpected::_E) {
      return v.ret_val_e;
    }
    return v.ret_val_s;
  };
  template <typename T, typename N, typename S, typename E> T accept_t(AstVisitor<T, N, S, E>& v) {
    doAccept(v);
    assert(v.expected == AstVisitorExpected::_T);
    return v.ret_val_t;
  };
};

class Type : public AstVisitorAcceptor {
public:
  enum TypeKind { TK_PTR_TYPE, TK_INT_TYPE, TK_FLOAT_TYPE, TK_STRUCT_TYPE, TK_VOID_TYPE };
  const TypeKind kind;
  Type(TypeKind kind, std::shared_ptr<State> state) : kind(kind), state(state){};
  Type() = delete;
  std::shared_ptr<State> state;
  virtual ~Type(){};
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
  virtual Span span() const override;
  virtual std::string str() const override;
  virtual bool operator==(const Type& rhs) const override;
  static bool classof(const Type* s) { return s->kind == TK_PTR_TYPE; };
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitPtrType(*this); };
};

class IntType : public Type {
public:
  uint8_t size;
  Span name;
  IntType(std::shared_ptr<State> state, uint8_t size, Span name)
      : Type(Type::TypeKind::TK_INT_TYPE, state), size(size), name(name){};
  IntType() = delete;
  virtual Span span() const override;
  virtual std::string str() const override;
  virtual bool operator==(const Type& rhs) const override;
  static bool classof(const Type* s) { return s->kind == TK_INT_TYPE; };
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitIntType(*this); };
};

class FloatType : public Type {
public:
  uint8_t size;
  Span name;
  FloatType(std::shared_ptr<State> state, uint8_t size, Span name)
      : Type(Type::TypeKind::TK_FLOAT_TYPE, state), size(size), name(name){};
  FloatType() = delete;
  virtual Span span() const override;
  virtual std::string str() const override;
  virtual bool operator==(const Type& rhs) const override;
  static bool classof(const Type* s) { return s->kind == TK_FLOAT_TYPE; };
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitFloatType(*this); };
};

class StructType : public Type {
public:
  Span name;
  StructType(std::shared_ptr<State> state, Span name)
      : Type(Type::TypeKind::TK_STRUCT_TYPE, state), name(name){};
  StructType() = delete;
  virtual Span span() const override;
  virtual std::string str() const override;
  virtual bool operator==(const Type& rhs) const override;
  static bool classof(const Type* s) { return s->kind == TK_STRUCT_TYPE; };
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitStructType(*this); };
};

class VoidType : public Type {
public:
  Span name;
  VoidType(std::shared_ptr<State> state, Span name)
      : Type(Type::TypeKind::TK_VOID_TYPE, state), name(name){};
  VoidType() = delete;
  virtual Span span() const override;
  virtual std::string str() const override;
  virtual bool operator==(const Type& rhs) const override;
  static bool classof(const Type* s) { return s->kind == TK_VOID_TYPE; };
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitVoidType(*this); };
};

// Should have done a visitor pattern (╥﹏╥)
class Node : public AstVisitorAcceptor {
public:
  Node(std::shared_ptr<State> state) : state(state){};
  Node() = delete;
  std::shared_ptr<State> state;
  virtual ~Node(){};
  virtual void print(std::ostream& target) const = 0;
  virtual Span span() const = 0;
};

class Document : public Node {
public:
  std::vector<std::unique_ptr<Node>> data;
  Document(std::shared_ptr<State> state) : Node(state){};
  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitDocument(*this); };
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

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitBlock(*this); };
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

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitLetStatement(*this); };
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

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitReturnStatement(*this); };
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

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitIfStatement(*this); };
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

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitWhileStatement(*this); };
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

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitForStatement(*this); };
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

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitAssignExpr(*this); };
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

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitBinaryExpr(*this); };
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

  virtual std::unique_ptr<Expression> clone() const override;
  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitUnaryExpr(*this); };
};

class CallExpr : public Expression {
public:
  Span target;
  std::vector<std::unique_ptr<Expression>> args;

  CallExpr(std::shared_ptr<State> state, Span target, std::vector<std::unique_ptr<Expression>> args)
      : Expression(state), target(target), args(std::move(args)){};

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitCallExpr(*this); };
};

class VariableExpr : public Expression {
public:
  Span value;

  VariableExpr(std::shared_ptr<State> state, Span value) : Expression(state), value(value){};
  virtual std::unique_ptr<Expression> clone() const override;
  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitVariableExpr(*this); };
};

class DotExpr : public Expression {
public:
  std::unique_ptr<Expression> lhs;
  Span rhs;

  DotExpr(std::shared_ptr<State> state, std::unique_ptr<Expression> lhs, Span rhs)
      : Expression(state), lhs(std::move(lhs)), rhs(rhs){};
  DotExpr(std::unique_ptr<Expression> lhs, Span rhs)
      : Expression(lhs->state), lhs(std::move(lhs)), rhs(rhs){};

  virtual std::unique_ptr<Expression> clone() const override;
  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitDotExpr(*this); };
};

class ArrayAccessExpr : public Expression {
public:
  std::unique_ptr<Expression> target;
  std::unique_ptr<Expression> index;

  ArrayAccessExpr(std::shared_ptr<State> state, std::unique_ptr<Expression> target,
                  std::unique_ptr<Expression> index)
      : Expression(state), target(std::move(target)), index(std::move(index)){};

  virtual std::unique_ptr<Expression> clone() const override;
  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitArrayAccessExpr(*this); };
};

class FloatLiteral : public Expression {
public:
  Span value;

  FloatLiteral(std::shared_ptr<State> state, Span value) : Expression(state), value(value){};
  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitFloatLiteral(*this); };
};

class StringLiteral : public Expression {
public:
  Span value;

  StringLiteral(std::shared_ptr<State> state, Span value) : Expression(state), value(value){};
  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitStringLiteral(*this); };
};

class IntLiteral : public Expression {
public:
  Span value;

  IntLiteral(std::shared_ptr<State> state, Span value) : Expression(state), value(value){};
  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitIntLiteral(*this); };
};

class StructLiteral : public Expression {
public:
  std::vector<std::pair<Span, std::unique_ptr<Expression>>> fields;
  Span name;

  StructLiteral(std::shared_ptr<State> state, Span name) : Expression(state), name(name){};
  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitStructLiteral(*this); };
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

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitProtoFunc(*this); };
};

class ExternDecl : public Node {
public:
  std::unique_ptr<ProtoFunc> proto;
  ExternDecl(std::shared_ptr<State> state, std::unique_ptr<ProtoFunc> proto)
      : Node(state), proto(std::move(proto)) {}
  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitExternDecl(*this); };
};

class FunctionDecl : public Node {
public:
  std::unique_ptr<ProtoFunc> proto;
  Block body;

  FunctionDecl(std::shared_ptr<State> state, std::unique_ptr<ProtoFunc> proto, Block body)
      : Node(state), proto(std::move(proto)), body(std::move(body)) {}

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitFunctionDecl(*this); };
};

class StructProto : public Node {
public:
  Span ident;
  Span name;
  std::vector<std::pair<Span, std::shared_ptr<Type>>> fields;
  StructProto(std::shared_ptr<State> state, Span ident, Span name)
      : Node(state), ident(ident), name(name){};

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitStructProto(*this); };
};

class ImportDecl : public Node {
public:
  Span file_name;
  std::shared_ptr<Source> file_src;
  std::unique_ptr<Document> data;

  ImportDecl(std::shared_ptr<State> state, Span file_name, std::shared_ptr<Source> file_src)
      : Node(state), file_name(file_name), file_src(file_src){};

  virtual Span span() const override;
  virtual void print(std::ostream& target) const override;
  virtual void doAccept(AstVisitorBase& v) override { v.doVisitImportDecl(*this); };
};

} // namespace llang

std::ostream& operator<<(std::ostream& out, const llang::Node& node);
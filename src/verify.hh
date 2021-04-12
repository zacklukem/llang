
#pragma once

#include "ast.hh"

namespace llang {

class VerifyVisitor : public AstVisitor<std::shared_ptr<Type>, std::shared_ptr<Type>, std::shared_ptr<Type>, std::shared_ptr<Type>> {
protected:
  std::shared_ptr<State> state;
  virtual std::shared_ptr<Type> visitPtrType(PtrType& self) override;
  virtual std::shared_ptr<Type> visitIntType(IntType& self) override;
  virtual std::shared_ptr<Type> visitFloatType(FloatType& self) override;
  virtual std::shared_ptr<Type> visitStructType(StructType& self) override;
  virtual std::shared_ptr<Type> visitVoidType(VoidType& self) override;
  virtual std::shared_ptr<Type> visitDocument(Document& self) override;
  virtual std::shared_ptr<Type> visitBlock(Block& self) override;
  virtual std::shared_ptr<Type> visitLetStatement(LetStatement& self) override;
  virtual std::shared_ptr<Type> visitReturnStatement(ReturnStatement& self) override;
  virtual std::shared_ptr<Type> visitIfStatement(IfStatement& self) override;
  virtual std::shared_ptr<Type> visitWhileStatement(WhileStatement& self) override;
  virtual std::shared_ptr<Type> visitForStatement(ForStatement& self) override;
  virtual std::shared_ptr<Type> visitAssignExpr(AssignExpr& self) override;
  virtual std::shared_ptr<Type> visitBinaryExpr(BinaryExpr& self) override;
  virtual std::shared_ptr<Type> visitUnaryExpr(UnaryExpr& self) override;
  virtual std::shared_ptr<Type> visitCallExpr(CallExpr& self) override;
  virtual std::shared_ptr<Type> visitVariableExpr(VariableExpr& self) override;
  virtual std::shared_ptr<Type> visitDotExpr(DotExpr& self) override;
  virtual std::shared_ptr<Type> visitArrayAccessExpr(ArrayAccessExpr& self) override;
  virtual std::shared_ptr<Type> visitFloatLiteral(FloatLiteral& self) override;
  virtual std::shared_ptr<Type> visitStringLiteral(StringLiteral& self) override;
  virtual std::shared_ptr<Type> visitIntLiteral(IntLiteral& self) override;
  virtual std::shared_ptr<Type> visitStructLiteral(StructLiteral& self) override;
  virtual std::shared_ptr<Type> visitProtoFunc(ProtoFunc& self) override;
  virtual std::shared_ptr<Type> visitExternDecl(ExternDecl& self) override;
  virtual std::shared_ptr<Type> visitFunctionDecl(FunctionDecl& self) override;
  virtual std::shared_ptr<Type> visitStructProto(StructProto& self) override;
  virtual std::shared_ptr<Type> visitImportDecl(ImportDecl& self) override;

public:
  VerifyVisitor(std::shared_ptr<State> state) : state(state){};
};

} // namespace llang
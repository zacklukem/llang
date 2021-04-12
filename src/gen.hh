#pragma once

#include "ast.hh"

namespace llang {

class GenVisitor : public AstVisitor<llvm::Type*, llvm::Value*, llvm::Value*, llvm::Value*> {
protected:
  std::shared_ptr<State> state;
  virtual llvm::Type* visitPtrType(PtrType& self) override;
  virtual llvm::Type* visitIntType(IntType& self) override;
  virtual llvm::Type* visitFloatType(FloatType& self) override;
  virtual llvm::Type* visitStructType(StructType& self) override;
  virtual llvm::Type* visitVoidType(VoidType& self) override;
  virtual llvm::Value* visitDocument(Document& self) override;
  virtual llvm::Value* visitBlock(Block& self) override;
  virtual llvm::Value* visitLetStatement(LetStatement& self) override;
  virtual llvm::Value* visitReturnStatement(ReturnStatement& self) override;
  virtual llvm::Value* visitIfStatement(IfStatement& self) override;
  virtual llvm::Value* visitWhileStatement(WhileStatement& self) override;
  virtual llvm::Value* visitForStatement(ForStatement& self) override;
  virtual llvm::Value* visitAssignExpr(AssignExpr& self) override;
  virtual llvm::Value* visitBinaryExpr(BinaryExpr& self) override;
  virtual llvm::Value* visitUnaryExpr(UnaryExpr& self) override;
  virtual llvm::Value* visitCallExpr(CallExpr& self) override;
  virtual llvm::Value* visitVariableExpr(VariableExpr& self) override;
  virtual llvm::Value* visitDotExpr(DotExpr& self) override;
  virtual llvm::Value* visitArrayAccessExpr(ArrayAccessExpr& self) override;
  virtual llvm::Value* visitFloatLiteral(FloatLiteral& self) override;
  virtual llvm::Value* visitStringLiteral(StringLiteral& self) override;
  virtual llvm::Value* visitIntLiteral(IntLiteral& self) override;
  virtual llvm::Value* visitStructLiteral(StructLiteral& self) override;
  virtual llvm::Value* visitProtoFunc(ProtoFunc& self) override;
  virtual llvm::Value* visitExternDecl(ExternDecl& self) override;
  virtual llvm::Value* visitFunctionDecl(FunctionDecl& self) override;
  virtual llvm::Value* visitStructProto(StructProto& self) override;
  virtual llvm::Value* visitImportDecl(ImportDecl& self) override;

public:
  GenVisitor(std::shared_ptr<State> state) : state(state){};

private:
  llvm::AllocaInst* createAlloca(llvm::Function* f, llvm::Type* t, llvm::StringRef name);
  llvm::AllocaInst* createAlloca(llvm::Function* f, std::weak_ptr<Type> t, llvm::StringRef name);
};

} // namespace llang
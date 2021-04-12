#include "verify.hh"
#include "ast.hh"

using namespace llang;

inline std::string type_mis(std::shared_ptr<Type> expected, std::shared_ptr<Type> got) {
  return "unmatched types, expected " + expected->str() + " and got " + got->str();
}

std::shared_ptr<Type> VerifyVisitor::visitPtrType(PtrType& self) { return nullptr; };
std::shared_ptr<Type> VerifyVisitor::visitIntType(IntType& self) { return nullptr; };
std::shared_ptr<Type> VerifyVisitor::visitFloatType(FloatType& self) { return nullptr; };
std::shared_ptr<Type> VerifyVisitor::visitStructType(StructType& self) { return nullptr; };
std::shared_ptr<Type> VerifyVisitor::visitVoidType(VoidType& self) { return nullptr; };

std::shared_ptr<Type> VerifyVisitor::visitDocument(Document& self) {
  for (auto& node : self.data) {
    node->accept_n(*this);
  }
  return nullptr;
}

std::shared_ptr<Type> VerifyVisitor::visitBlock(Block& self) {
  std::shared_ptr<Type> last;
  for (auto& stmt : self.stmts) {
    last = stmt->accept_s(*this);
  }
  return last;
}

std::shared_ptr<Type> VerifyVisitor::visitLetStatement(LetStatement& self) {
  if (self.rhs.has_value()) {
    auto v = self.rhs.value()->accept_e(*this);
    if (!v)
      return nullptr;
    if (*v != *self.type)
      self.rhs.value()->span().fail() << type_mis(self.type, v);
  }
  return state->v_named_values[self.name.str()] = self.type;
}

std::shared_ptr<Type> VerifyVisitor::visitReturnStatement(ReturnStatement& self) {
  if (self.rhs.has_value()) {
    auto right = self.rhs.value()->accept_e(*this);
    if (!right)
      return nullptr;
    if (*right != *state->ret_type)
      self.rhs.value()->span().fail() << type_mis(state->ret_type, right);
  }
  return state->ret_type;
}

std::shared_ptr<Type> VerifyVisitor::visitIfStatement(IfStatement& self) {
  for (auto& block : self.pairs) {
    auto cond_v = block.first->accept_e(*this);
    if (!cond_v)
      return nullptr;
    if (!llvm::isa<IntType>(cond_v.get())) {
      block.first->span().fail() << "expected int type, got " + cond_v->str();
    }
    block.second.accept_n(*this);
  }
  if (self.else_block.has_value()) {
    self.else_block.value().accept_n(*this);
  }
  return nullptr;
}

std::shared_ptr<Type> VerifyVisitor::visitWhileStatement(WhileStatement& self) {
  auto cond_v = self.cond->accept_e(*this);
  if (!cond_v)
    return nullptr;
  if (!llvm::isa<IntType>(cond_v.get())) {
    self.cond->span().fail() << "expected int type, got " + cond_v->str();
  }
  return nullptr;
}

std::shared_ptr<Type> VerifyVisitor::visitForStatement(ForStatement& self) {
  self.init->accept_s(*this);
  auto cond_v = self.cond->accept_e(*this);
  if (!cond_v)
    return nullptr;
  if (!llvm::isa<IntType>(cond_v.get())) {
    self.cond->span().fail() << "expected int type, got " + cond_v->str();
  }
  self.body.accept_n(*this);
  self.inc->accept_e(*this);
  return nullptr;
}

std::shared_ptr<Type> VerifyVisitor::visitAssignExpr(AssignExpr& self) {
  auto left = self.lhs->accept_e(*this);
  if (!left)
    return nullptr;
  auto right = self.rhs->accept_e(*this);
  if (!right)
    return nullptr;
  if (*left != *right) {
    self.span().fail() << type_mis(left, right);
  }
  return left;
}

std::shared_ptr<Type> VerifyVisitor::visitBinaryExpr(BinaryExpr& self) {
  auto lhs_t = self.lhs->accept_e(*this);
  if (!lhs_t)
    return nullptr;
  auto rhs_t = self.rhs->accept_e(*this);
  if (!rhs_t)
    return nullptr;
  if (*lhs_t != *self.rhs->accept_e(*this)) {
    self.span().fail() << type_mis(lhs_t, rhs_t);
    return nullptr;
  }
  switch (self.oper) {
  case BinaryExpr::Op::ADD:
  case BinaryExpr::Op::SUB:
  case BinaryExpr::Op::MUL:
  case BinaryExpr::Op::DIV:
    return lhs_t;
  case BinaryExpr::Op::LT:
  case BinaryExpr::Op::GT:
  case BinaryExpr::Op::GTE:
  case BinaryExpr::Op::LTE:
  case BinaryExpr::Op::EQ:
    return std::make_shared<IntType>(state, 1, self.span());
  }
}

std::shared_ptr<Type> VerifyVisitor::visitUnaryExpr(UnaryExpr& self) {
  auto rhs_t = self.rhs->accept_e(*this);
  if (!rhs_t)
    return nullptr;
  if (self.oper == UnaryExpr::Op::DEREF) {
    if (!llvm::isa<PtrType>(rhs_t.get())) {
      self.rhs->span().fail() << "cannot dereference non-pointer type (" + rhs_t->str() + ")";
      return nullptr;
    }
    auto rhs_ptr = llvm::cast<PtrType>(rhs_t.get());
    return rhs_ptr->type;
  }
  if (self.oper == UnaryExpr::Op::REF) {
    return std::make_shared<PtrType>(state, rhs_t, self.span());
  }
  return rhs_t;
}

std::shared_ptr<Type> VerifyVisitor::visitCallExpr(CallExpr& self) {
  if (!state->v_funcs.count(self.target.str())) {
    self.target.fail() << "function name not in scope";
    return nullptr;
  }
  auto targ = state->v_funcs[self.target.str()];
  for (int i = 0; i < self.args.size(); i++) {
    if (i >= targ->args.size()) {
      self.args.at(i)->span().fail() << "extra argument";
      break;
    }
    auto arg = self.args.at(i)->accept_e(*this);
    if (!arg)
      continue;
    if (*arg != *targ->args.at(i).first) {
      self.args.at(i)->span().fail() << type_mis(targ->args.at(i).first, arg);
    }
  }
  return targ->return_type;
}

std::shared_ptr<Type> VerifyVisitor::visitVariableExpr(VariableExpr& self) {
  if (!state->v_named_values.count(self.value.str())) {
    self.value.fail() << "variable name not in scope";
    return nullptr;
  }
  return state->v_named_values.at(self.value.str());
}

std::shared_ptr<Type> VerifyVisitor::visitDotExpr(DotExpr& self) {
  auto left = self.lhs->accept_e(*this);
  if (!left)
    return nullptr;
  if (!llvm::isa<StructType>(left.get())) {
    self.lhs->span().fail() << "must be struct for dot operator (" + left->str() + ")";
    if (llvm::isa<PtrType>(left.get())) {
      self.lhs->span().fail(MessageType::NOTE) << "did you mean to use '->'?";
    }
    return nullptr;
  }
  auto as_struct = llvm::cast<StructType>(left.get());
  bool contains = false;
  for (auto& field : state->v_structures[as_struct->name.str()]->fields) {
    contains = (field.first.str() == self.rhs.str()) ? true : contains;
  }
  if (!contains) {
    self.lhs->span().fail() << "field not found in structure";
    return nullptr;
  }
  for (auto& field : state->v_structures[as_struct->name.str()]->fields) {
    if (field.first.str() == self.rhs.str()) {
      return field.second;
    }
  }
  return nullptr;
}

std::shared_ptr<Type> VerifyVisitor::visitArrayAccessExpr(ArrayAccessExpr& self) {
  auto target_v = self.target->accept_e(*this);
  auto index_v = self.index->accept_e(*this);
  if (!target_v)
    return nullptr;
  if (!index_v)
    return nullptr;
  if (!llvm::isa<PtrType>(target_v.get())) {
    self.target->span().fail() << "must be pointer to access values (" + target_v->str() + ")";
  }
  if (!llvm::isa<IntType>(index_v.get())) {
    self.index->span().fail() << "must be an integer (" + index_v->str() + ")";
  }
  return llvm::cast<PtrType>(*target_v).type;
}

std::shared_ptr<Type> VerifyVisitor::visitFloatLiteral(FloatLiteral& self) {
  return std::make_shared<FloatType>(state, 64, self.value);
}

std::shared_ptr<Type> VerifyVisitor::visitStringLiteral(StringLiteral& self) {
  return std::make_shared<PtrType>(state, std::make_shared<IntType>(state, 8, self.value),
                                   self.value);
}

std::shared_ptr<Type> VerifyVisitor::visitIntLiteral(IntLiteral& self) {
  return std::make_shared<IntType>(state, 64, self.value);
}

std::shared_ptr<Type> VerifyVisitor::visitStructLiteral(StructLiteral& self) {
  return std::make_shared<StructType>(state, self.name);
}

std::shared_ptr<Type> VerifyVisitor::visitProtoFunc(ProtoFunc& self) {
  for (auto& arg : self.args) {
    state->v_named_values[arg.second.str()] = arg.first;
  }
  state->v_funcs[self.name.str()] = &self;
  state->ret_type = self.return_type;
  return nullptr;
}

std::shared_ptr<Type> VerifyVisitor::visitExternDecl(ExternDecl& self) {
  self.proto->accept_n(*this);
  return nullptr;
}

std::shared_ptr<Type> VerifyVisitor::visitFunctionDecl(FunctionDecl& self) {
  self.proto->accept_n(*this);
  self.body.accept_n(*this);
  return nullptr;
}

std::shared_ptr<Type> VerifyVisitor::visitStructProto(StructProto& self) {
  state->v_structures[self.name.str()] = &self;
  return nullptr;
}

std::shared_ptr<Type> VerifyVisitor::visitImportDecl(ImportDecl& self) {
  if (self.file_src->debugPrintMessages()) { // TODO: probably not great for future to print here
    self.file_name.fail() << "errors in included file";
    return nullptr;
  }

  for (auto& tls : self.data->data) {
    if (auto proto = dynamic_cast<FunctionDecl*>(tls.get())) {
      proto->proto->accept_n(*this);
    } else {
      tls->accept_n(*this);
    }
  }

  return nullptr;
}

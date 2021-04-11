#include "ast.hh"

using namespace llang;

inline std::string type_mis(std::shared_ptr<Type> expected, std::shared_ptr<Type> got) {
  return "unmatched types, expected " + expected->str() + " and got " + got->str();
}

std::shared_ptr<Type> Document::verify() {
  for (auto& node : data) {
    node->verify();
  }
  return nullptr;
}

std::shared_ptr<Type> Block::verify() {
  std::shared_ptr<Type> last;
  for (auto& stmt : stmts) {
    last = stmt->verify();
  }
  return last;
}

std::shared_ptr<Type> LetStatement::verify() {
  if (rhs.has_value()) {
    auto v = rhs.value()->verify();
    if (!v)
      return nullptr;
    if (*v != *type)
      rhs.value()->span().fail() << type_mis(type, v);
  }
  return state->v_named_values[name.str()] = type;
}

std::shared_ptr<Type> ReturnStatement::verify() {
  if (rhs.has_value()) {
    auto right = rhs.value()->verify();
    if (!right)
      return nullptr;
    if (*right != *state->ret_type)
      rhs.value()->span().fail() << type_mis(state->ret_type, right);
  }
  return state->ret_type;
}

std::shared_ptr<Type> IfStatement::verify() {
  for (auto& block : this->pairs) {
    auto cond_v = block.first->verify();
    if (!cond_v)
      return nullptr;
    if (!llvm::isa<IntType>(cond_v.get())) {
      block.first->span().fail() << "expected int type, got " + cond_v->str();
      ;
    }
  }
  if (else_block.has_value()) {
    else_block.value().verify();
  }
  return nullptr;
}

std::shared_ptr<Type> WhileStatement::verify() {
  auto cond_v = cond->verify();
  if (!cond_v)
    return nullptr;
  if (!llvm::isa<IntType>(cond_v.get())) {
    cond->span().fail() << "expected int type, got " + cond_v->str();
  }
  return nullptr;
}

std::shared_ptr<Type> ForStatement::verify() {
  init->verify();
  auto cond_v = cond->verify();
  if (!cond_v)
    return nullptr;
  if (!llvm::isa<IntType>(cond_v.get())) {
    cond->span().fail() << "expected int type, got " + cond_v->str();
  }
  inc->verify();
  return nullptr;
}

std::shared_ptr<Type> AssignExpr::verify() {
  auto left = lhs->verify();
  if (!left)
    return nullptr;
  auto right = rhs->verify();
  if (!right)
    return nullptr;
  if (*left != *right) {
    span().fail() << type_mis(left, right);
  }
  return left;
}

std::shared_ptr<Type> BinaryExpr::verify() {
  auto lhs_t = lhs->verify();
  if (!lhs_t)
    return nullptr;
  auto rhs_t = rhs->verify();
  if (!rhs_t)
    return nullptr;
  if (*lhs_t != *rhs->verify()) {
    span().fail() << type_mis(lhs_t, rhs_t);
    return nullptr;
  }
  switch (oper) {
  case ADD:
  case SUB:
  case MUL:
  case DIV:
    return lhs_t;
  case LT:
  case GT:
  case GTE:
  case LTE:
  case EQ:
    return std::make_shared<IntType>(state, 1, span());
  }
}

std::shared_ptr<Type> UnaryExpr::verify() {
  auto rhs_t = rhs->verify();
  if (!rhs_t)
    return nullptr;
  if (oper == DEREF) {
    if (!llvm::isa<PtrType>(rhs_t.get())) {
      rhs->span().fail() << "cannot dereference non-pointer type (" + rhs_t->str() + ")";
      return nullptr;
    }
    auto rhs_ptr = llvm::cast<PtrType>(rhs_t.get());
    return rhs_ptr->type;
  }
  if (oper == REF) {
    return std::make_shared<PtrType>(state, rhs_t, span());
  }
  return rhs_t;
}

std::shared_ptr<Type> CallExpr::verify() {
  if (!state->v_funcs.count(target.str())) {
    target.fail() << "function name not in scope";
    return nullptr;
  }
  auto targ = state->v_funcs[target.str()];
  for (int i = 0; i < args.size(); i++) {
    if (i >= targ->args.size()) {
      args.at(i)->span().fail() << "extra argument";
      break;
    }
    auto arg = args.at(i)->verify();
    if (!arg)
      continue;
    if (*arg != *targ->args.at(i).first) {
      args.at(i)->span().fail() << type_mis(targ->args.at(i).first, arg);
    }
  }
  return targ->return_type;
}

std::shared_ptr<Type> VariableExpr::verify() {
  if (!state->v_named_values.count(value.str())) {
    value.fail() << "variable name not in scope";
    return nullptr;
  }
  return state->v_named_values.at(value.str());
}

std::shared_ptr<Type> DotExpr::verify() {
  auto left = lhs->verify();
  if (!left)
    return nullptr;
  if (!llvm::isa<StructType>(left.get())) {
    lhs->span().fail() << "must be struct for dot operator (" + left->str() + ")";
    if (llvm::isa<PtrType>(left.get())) {
      lhs->span().fail(MessageType::NOTE) << "did you mean to use '->'?";
    }
    return nullptr;
  }
  auto as_struct = llvm::cast<StructType>(left.get());
  bool contains = false;
  for (auto& field : state->v_structures[as_struct->name.str()]->fields) {
    contains = (field.first.str() == rhs.str()) ? true : contains;
  }
  if (!contains) {
    lhs->span().fail() << "field not found in structure";
    return nullptr;
  }
  for (auto& field : state->v_structures[as_struct->name.str()]->fields) {
    if (field.first.str() == rhs.str()) {
      return field.second;
    }
  }
  return nullptr;
}

std::shared_ptr<Type> ArrayAccessExpr::verify() {
  auto target_v = target->verify();
  auto index_v = index->verify();
  if (!target_v)
    return nullptr;
  if (!index_v)
    return nullptr;
  if (!llvm::isa<PtrType>(target_v.get())) {
    target->span().fail() << "must be pointer to access values (" + target_v->str() + ")";
  }
  if (!llvm::isa<IntType>(index_v.get())) {
    index->span().fail() << "must be an integer (" + index_v->str() + ")";
  }
  return llvm::cast<PtrType>(*target_v).type;
}

std::shared_ptr<Type> FloatLiteral::verify() {
  return std::make_shared<FloatType>(state, 64, value);
}

std::shared_ptr<Type> StringLiteral::verify() {
  return std::make_shared<PtrType>(state, std::make_shared<IntType>(state, 8, value), value);
}

std::shared_ptr<Type> IntLiteral::verify() { return std::make_shared<IntType>(state, 64, value); }

std::shared_ptr<Type> StructLiteral::verify() { return std::make_shared<StructType>(state, name); }

std::shared_ptr<Type> ProtoFunc::verify() {
  for (auto& arg : this->args) {
    state->v_named_values[arg.second.str()] = arg.first;
  }
  state->v_funcs[name.str()] = this;
  state->ret_type = return_type;
  return nullptr;
}

std::shared_ptr<Type> ExternDecl::verify() {
  proto->verify();
  return nullptr;
}

std::shared_ptr<Type> FunctionDecl::verify() {
  proto->verify();
  body.verify();
  return nullptr;
}

std::shared_ptr<Type> StructProto::verify() {
  state->v_structures[name.str()] = this;
  return nullptr;
}

std::shared_ptr<Type> ImportDecl::verify() { return nullptr; }

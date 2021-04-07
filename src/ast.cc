// Copyright 2020 Zachary Mayhew

#include "ast.hh"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include <iterator>

using namespace llang;

llvm::Type* toLLVMType(std::shared_ptr<State> state, Type type) {
  switch (type) {
  case NUMBER:
    return llvm::Type::getDoubleTy(*state->ctx);
  }
}

llvm::Value* BinaryExpr::codegen() {
  llvm::Value* l_val = lhs->codegen();
  llvm::Value* r_val = rhs->codegen();

  switch (oper) {
  case ADD:
    return state->builder->CreateFAdd(l_val, r_val);
  case SUB:
    return state->builder->CreateFSub(l_val, r_val);
  case MUL:
    return state->builder->CreateFMul(l_val, r_val);
  case DIV:
    return state->builder->CreateFDiv(l_val, r_val);
  case EQ:
    return state->builder->CreateFCmpUEQ(l_val, r_val);
  case LT:
    return state->builder->CreateFCmpULT(l_val, r_val);
  case GT:
    return state->builder->CreateFCmpUGT(l_val, r_val);
  case LTE:
    return state->builder->CreateFCmpULE(l_val, r_val);
  case GTE:
    return state->builder->CreateFCmpUGE(l_val, r_val);
  }

  return nullptr;
}

llvm::Value* UnaryExpr::codegen() {
  llvm::Value* r_val = rhs->codegen();
  switch (oper) {
  case NEG:
    return state->builder->CreateFNeg(r_val);
    break;
  }

  return nullptr;
}

llvm::Value* CallExpr::codegen() {
  auto callee = state->mod->getFunction(target);
  std::vector<llvm::Value*> args_val;
  std::transform(args.begin(), args.end(), std::back_inserter(args_val), [](auto& arg) { return arg->codegen(); });
  return state->builder->CreateCall(callee, args_val);
}

llvm::Value* VariableExpr::codegen() {
  auto v = state->named_values[value];
  if (!v)
    throw "bad stuff";
  return v;
}

llvm::Value* NumberLiteral::codegen() { return llvm::ConstantFP::get(*state->ctx, llvm::APFloat(value)); }

llvm::Value* ReturnStatement::codegen() {
  auto r_val = rhs->codegen();
  return state->builder->CreateRet(r_val);
}

llvm::Value* Block::codegen() {
  llvm::Value* ret_val = nullptr;
  for (auto& stmt : stmts)
    ret_val = stmt->codegen();
  return ret_val;
}

llvm::Value* IfStatement::codegen() {
  auto parent = state->builder->GetInsertBlock()->getParent();

  auto cont_bb = llvm::BasicBlock::Create(*state->ctx, "");

  for (auto& pair : pairs) {
    auto cond = pair.first->codegen();
    auto then_bb = llvm::BasicBlock::Create(*state->ctx, "");
    auto next_bb = llvm::BasicBlock::Create(*state->ctx, "");
    state->builder->CreateCondBr(cond, then_bb, next_bb);
    state->builder->SetInsertPoint(then_bb);
    pair.second.codegen();
    state->builder->CreateBr(cont_bb);
    parent->getBasicBlockList().push_back(then_bb);
    parent->getBasicBlockList().push_back(next_bb);
    state->builder->SetInsertPoint(next_bb);
  }

  if (else_block) {
    else_block.value().codegen();
    state->builder->CreateBr(cont_bb);
  } else {
    parent->getBasicBlockList().pop_back();
  }

  parent->getBasicBlockList().push_back(cont_bb);
  state->builder->SetInsertPoint(cont_bb);

  return nullptr;
}

llvm::Function* ProtoFunc::proto_codegen() {
  std::vector<llvm::Type*> args_type;
  std::transform(args.begin(), args.end(), std::back_inserter(args_type),
                 [this](auto& arg) { return toLLVMType(state, arg.first); });

  auto t = llvm::FunctionType::get(toLLVMType(state, return_type), args_type, false);
  auto f = llvm::Function::Create(t, llvm::Function::ExternalLinkage, name, *state->mod);

  auto i = 0;
  for (auto& arg : f->args())
    arg.setName(args.at(i++).second);

  return f;
}

llvm::Value* FunctionDecl::codegen() {
  auto f = state->mod->getFunction(proto->name);
  if (!f)
    f = proto->proto_codegen();

  auto bb = llvm::BasicBlock::Create(*state->ctx, "", f);
  state->builder->SetInsertPoint(bb);

  // Record the function arguments in the NamedValues map.
  state->named_values.clear();
  for (auto& arg : f->args())
    state->named_values[std::string(arg.getName())] = &arg;

  body.codegen();

  llvm::verifyFunction(*f);

  return f;
}

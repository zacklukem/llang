// Copyright 2020 Zachary Mayhew

#include "ast.hh"
#include <iostream>
#include <iterator>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <string>

using namespace llang;

llvm::Value* Document::codegen() {
  for (auto& d : data) {
    d->codegen();
  }
  return nullptr;
}

llvm::Type* PtrType::codegen() { return type->codegen()->getPointerTo(); }
llvm::Type* IntType::codegen() { return llvm::IntegerType::get(*state->ctx, size); }
llvm::Type* FloatType::codegen() {
  if (size == 32) {
    return llvm::Type::getFloatTy(*state->ctx);
  } else if (size == 64) {
    return llvm::Type::getDoubleTy(*state->ctx);
  }
  return nullptr;
}
llvm::Type* VoidType::codegen() { return llvm::Type::getVoidTy(*state->ctx); }

llvm::Type* StructType::codegen() { return state->structures[name].first; }

llvm::AllocaInst* createAlloca(std::shared_ptr<State> state, llvm::Function* f,
                               llvm::Type* t, llvm::StringRef name) {
  llvm::IRBuilder<> builder(&f->getEntryBlock(), f->getEntryBlock().begin());
  return builder.CreateAlloca(t, 0, name);
}

llvm::AllocaInst* createAlloca(std::shared_ptr<State> state, llvm::Function* f,
                               std::weak_ptr<Type> t, llvm::StringRef name) {
  if (auto ty = t.lock()) {
    return createAlloca(state, f, ty->codegen(), name);
  } else {
    throw "bad stuff";
  }
}

llvm::Value* AssignExpr::codegen() {
  auto val = rhs->codegen();
  auto c = state->setAddrMode();
  auto var = lhs->codegen();
  state->unsetAddrMode(c);
  return state->builder->CreateStore(val, var);
}

llvm::Value* BinaryExpr::codegen() {
  auto l_val = lhs->codegen();
  auto r_val = rhs->codegen();

  if (l_val->getType()->isFloatingPointTy()) {
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
  } else {
    switch (oper) {
    case ADD:
      return state->builder->CreateAdd(l_val, r_val);
    case SUB:
      return state->builder->CreateSub(l_val, r_val);
    case MUL:
      return state->builder->CreateMul(l_val, r_val);
    case DIV:
      return state->builder->CreateSDiv(l_val, r_val);
    case EQ:
      return state->builder->CreateICmpEQ(l_val, r_val);
    case LT:
      return state->builder->CreateICmpULT(l_val, r_val);
    case GT:
      return state->builder->CreateICmpUGT(l_val, r_val);
    case LTE:
      return state->builder->CreateICmpULE(l_val, r_val);
    case GTE:
      return state->builder->CreateICmpUGE(l_val, r_val);
    }
  }

  return nullptr;
}

llvm::Value* UnaryExpr::codegen() {
  if (oper == REF) {
    auto c = state->setAddrMode();
    auto o = rhs->codegen();
    state->unsetAddrMode(c);
    return o;
  }
  llvm::Value* r_val = rhs->codegen();
  switch (oper) {
  case NEG:
    return state->builder->CreateFNeg(r_val);
    break;
  case DEREF:
    return state->builder->CreateLoad(r_val);
    break;
  default:
    return r_val;
  }
}

llvm::Value* DotExpr::codegen() {
  auto c = state->setAddrMode();
  auto left = lhs->codegen();
  state->unsetAddrMode(c);
  auto name = left->getType()->getPointerElementType()->getStructName().str();
  auto gep = state->builder->CreateStructGEP(left, state->structures[name].second[rhs]);
  if (state->getAddrMode()) {
    return gep;
  }
  return state->builder->CreateLoad(gep);
}

llvm::Value* CallExpr::codegen() {
  auto callee = state->mod->getFunction(target);
  std::vector<llvm::Value*> args_val;
  std::transform(args.begin(), args.end(), std::back_inserter(args_val),
                 [](auto& arg) { return arg->codegen(); });
  return state->builder->CreateCall(callee, args_val);
}

llvm::Value* ArrayAccessExpr::codegen() {
  auto c = state->setAddrMode();
  auto target_val = target->codegen();
  state->unsetAddrMode(c);
  auto index_val = index->codegen();
  auto gep = state->builder->CreateGEP(target_val, index_val);
  return state->builder->CreateLoad(gep);
}

llvm::Value* VariableExpr::codegen() {
  auto v = state->named_values[value];
  if (state->getAddrMode()) {
    return v.first;
  }
  return state->builder->CreateLoad(v.first);
}

llvm::Value* FloatLiteral::codegen() {
  return llvm::ConstantFP::get(*state->ctx, llvm::APFloat(value));
}

llvm::Value* StringLiteral::codegen() { return state->builder->CreateGlobalString(value); }

llvm::Value* IntLiteral::codegen() {
  return llvm::ConstantInt::get(*state->ctx, llvm::APInt(64, value, true));
}

llvm::Value* StructLiteral::codegen() {
  auto parent = state->builder->GetInsertBlock()->getParent();
  auto str = state->structures[name];
  auto alloca = createAlloca(state, parent, str.first, name);
  for (auto& field : fields) {
    auto gep = state->builder->CreateStructGEP(alloca, str.second[field.first]);
    state->builder->CreateStore(field.second->codegen(), gep);
  }
  return state->builder->CreateLoad(alloca);
}

llvm::Value* LetStatement::codegen() {
  auto parent = state->builder->GetInsertBlock()->getParent();
  auto alloca_inst = createAlloca(state, parent, type, name);

  state->named_values[name] = std::make_pair(alloca_inst, type->codegen());

  if (rhs.has_value()) {
    auto val = rhs.value()->codegen();
    return state->builder->CreateStore(val, alloca_inst);
  }

  return alloca_inst;
}

llvm::Value* ReturnStatement::codegen() {
  if (rhs.has_value()) {
    auto r_val = rhs.value()->codegen();
    return state->builder->CreateRet(r_val);
  }
  return state->builder->CreateRetVoid();
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
    auto then_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
    auto next_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
    state->builder->CreateCondBr(cond, then_bb, next_bb);
    state->builder->SetInsertPoint(then_bb);
    auto last = pair.second.codegen();
    if (!last || !llvm::isa<llvm::ReturnInst>(last)) {
      state->builder->CreateBr(cont_bb);
    }
    state->builder->SetInsertPoint(next_bb);
  }

  if (else_block.has_value()) {
    else_block.value().codegen();
  }

  state->builder->CreateBr(cont_bb);

  parent->getBasicBlockList().push_back(cont_bb);
  state->builder->SetInsertPoint(cont_bb);

  return nullptr;
}

llvm::Value* WhileStatement::codegen() {
  auto parent = state->builder->GetInsertBlock()->getParent();

  auto cont_bb = llvm::BasicBlock::Create(*state->ctx, "");
  auto loop_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
  auto then_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
  state->builder->CreateBr(loop_bb);
  state->builder->SetInsertPoint(loop_bb);

  auto cond_l = cond->codegen();
  state->builder->CreateCondBr(cond_l, then_bb, cont_bb);

  state->builder->SetInsertPoint(then_bb);
  auto last = body.codegen();
  if (!last || !llvm::isa<llvm::ReturnInst>(last)) {
    state->builder->CreateBr(loop_bb);
  }

  parent->getBasicBlockList().push_back(cont_bb);
  state->builder->SetInsertPoint(cont_bb);

  return nullptr;
}

llvm::Value* ForStatement::codegen() {
  auto parent = state->builder->GetInsertBlock()->getParent();

  auto cont_bb = llvm::BasicBlock::Create(*state->ctx, "");
  auto loop_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
  auto then_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
  init->codegen();
  state->builder->CreateBr(loop_bb);
  state->builder->SetInsertPoint(loop_bb);

  auto cond_l = cond->codegen();
  state->builder->CreateCondBr(cond_l, then_bb, cont_bb);

  state->builder->SetInsertPoint(then_bb);
  auto last = body.codegen();
  if (!last || !llvm::isa<llvm::ReturnInst>(last)) {
    inc->codegen();
    state->builder->CreateBr(loop_bb);
  }

  parent->getBasicBlockList().push_back(cont_bb);
  state->builder->SetInsertPoint(cont_bb);

  return nullptr;
}

llvm::Function* ProtoFunc::proto_codegen() {
  std::vector<llvm::Type*> args_type;
  std::transform(args.begin(), args.end(), std::back_inserter(args_type),
                 [](auto& arg) { return arg.first->codegen(); });

  auto t = llvm::FunctionType::get(return_type->codegen(), args_type, false);
  auto f = llvm::Function::Create(t, llvm::Function::ExternalLinkage, name, *state->mod);

  auto i = 0;
  for (auto& arg : f->args())
    arg.setName(args.at(i++).second);

  return f;
}

llvm::Value* ExternDecl::codegen() { return proto->codegen(); }

llvm::Value* FunctionDecl::codegen() {
  auto f = state->mod->getFunction(proto->name);
  if (!f)
    f = proto->proto_codegen();

  auto bb = llvm::BasicBlock::Create(*state->ctx, "", f);
  state->builder->SetInsertPoint(bb);

  // Record the function arguments in the NamedValues map.
  state->named_values.clear();
  int i = 0;
  for (auto& arg : f->args()) {
    // TODO: fix the type to reference prototype
    auto alloca_inst = createAlloca(state, f, proto->args.at(i).first, arg.getName());

    state->builder->CreateStore(&arg, alloca_inst);

    state->named_values[std::string(arg.getName())] =
        std::make_pair(alloca_inst, proto->args.at(i).first->codegen());
    i++;
  }

  body.codegen();

  llvm::verifyFunction(*f);

  return f;
}

llvm::Value* StructProto::codegen() {
  std::vector<llvm::Type*> elements;

  std::transform(fields.begin(), fields.end(), std::back_inserter(elements),
                 [](auto& t) { return t.second->codegen(); });

  auto str = llvm::StructType::create(elements, name);
  std::map<std::string, size_t> mm;

  int i = 0;
  for (auto& field : fields) {
    mm[field.first] = i;
    i++;
  }

  state->structures[name] = std::make_pair(str, mm);
  return nullptr;
}
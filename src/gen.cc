// Copyright 2020 Zachary Mayhew
#include "gen.hh"
#include "ast.hh"
#include <iostream>
#include <iterator>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <string>

using namespace llang;

llvm::Value* GenVisitor::visitDocument(Document& self) {
  for (auto& d : self.data) {
    d->accept_n(*this);
  }
  return nullptr;
}

llvm::Type* GenVisitor::visitPtrType(PtrType& self) {
  return self.type->accept_t(*this)->getPointerTo();
}
llvm::Type* GenVisitor::visitIntType(IntType& self) {
  return llvm::IntegerType::get(*state->ctx, self.size);
}
llvm::Type* GenVisitor::visitFloatType(FloatType& self) {
  if (self.size == 32) {
    return llvm::Type::getFloatTy(*state->ctx);
  } else if (self.size == 64) {
    return llvm::Type::getDoubleTy(*state->ctx);
  }
  return nullptr;
}
llvm::Type* GenVisitor::visitVoidType(VoidType& self) { return llvm::Type::getVoidTy(*state->ctx); }

llvm::Type* GenVisitor::visitStructType(StructType& self) {
  return state->structures[self.name.str()].first;
}

llvm::AllocaInst* GenVisitor::createAlloca(llvm::Function* f, llvm::Type* t, llvm::StringRef name) {
  llvm::IRBuilder<> builder(&f->getEntryBlock(), f->getEntryBlock().begin());
  return builder.CreateAlloca(t, 0, name);
}

llvm::AllocaInst* GenVisitor::createAlloca(llvm::Function* f, std::weak_ptr<Type> t,
                                           llvm::StringRef name) {
  if (auto ty = t.lock()) {
    return createAlloca(f, ty->accept_t(*this), name);
  } else {
    throw "bad stuff";
  }
}

llvm::Value* GenVisitor::visitAssignExpr(AssignExpr& self) {
  auto val = self.rhs->accept_e(*this);
  auto c = state->setAddrMode();
  auto var = self.lhs->accept_e(*this);
  state->unsetAddrMode(c);
  return state->builder->CreateStore(val, var);
}

llvm::Value* GenVisitor::visitBinaryExpr(BinaryExpr& self) {
  auto l_val = self.lhs->accept_e(*this);
  auto r_val = self.rhs->accept_e(*this);

  if (l_val->getType()->isFloatingPointTy()) {
    switch (self.oper) {
    case BinaryExpr::Op::ADD:
      return state->builder->CreateFAdd(l_val, r_val);
    case BinaryExpr::Op::SUB:
      return state->builder->CreateFSub(l_val, r_val);
    case BinaryExpr::Op::MUL:
      return state->builder->CreateFMul(l_val, r_val);
    case BinaryExpr::Op::DIV:
      return state->builder->CreateFDiv(l_val, r_val);
    case BinaryExpr::Op::EQ:
      return state->builder->CreateFCmpUEQ(l_val, r_val);
    case BinaryExpr::Op::LT:
      return state->builder->CreateFCmpULT(l_val, r_val);
    case BinaryExpr::Op::GT:
      return state->builder->CreateFCmpUGT(l_val, r_val);
    case BinaryExpr::Op::LTE:
      return state->builder->CreateFCmpULE(l_val, r_val);
    case BinaryExpr::Op::GTE:
      return state->builder->CreateFCmpUGE(l_val, r_val);
    }
  } else {
    switch (self.oper) {
    case BinaryExpr::Op::ADD:
      return state->builder->CreateAdd(l_val, r_val);
    case BinaryExpr::Op::SUB:
      return state->builder->CreateSub(l_val, r_val);
    case BinaryExpr::Op::MUL:
      return state->builder->CreateMul(l_val, r_val);
    case BinaryExpr::Op::DIV:
      return state->builder->CreateSDiv(l_val, r_val);
    case BinaryExpr::Op::EQ:
      return state->builder->CreateICmpEQ(l_val, r_val);
    case BinaryExpr::Op::LT:
      return state->builder->CreateICmpULT(l_val, r_val);
    case BinaryExpr::Op::GT:
      return state->builder->CreateICmpUGT(l_val, r_val);
    case BinaryExpr::Op::LTE:
      return state->builder->CreateICmpULE(l_val, r_val);
    case BinaryExpr::Op::GTE:
      return state->builder->CreateICmpUGE(l_val, r_val);
    }
  }

  return nullptr;
}

llvm::Value* GenVisitor::visitUnaryExpr(UnaryExpr& self) {
  if (self.oper == UnaryExpr::Op::REF) {
    auto c = state->setAddrMode();
    auto o = self.rhs->accept_e(*this);
    state->unsetAddrMode(c);
    return o;
  }
  llvm::Value* r_val = self.rhs->accept_e(*this);
  switch (self.oper) {
  case UnaryExpr::Op::NEG:
    return state->builder->CreateFNeg(r_val);
    break;
  case UnaryExpr::Op::DEREF:
    return state->builder->CreateLoad(r_val);
    break;
  default:
    return r_val;
  }
}

llvm::Value* GenVisitor::visitDotExpr(DotExpr& self) {
  auto c = state->setAddrMode();
  auto left = self.lhs->accept_e(*this);
  state->unsetAddrMode(c);
  auto name = left->getType()->getPointerElementType()->getStructName().str();
  auto gep = state->builder->CreateStructGEP(left, state->structures[name].second[self.rhs.str()]);
  if (state->getAddrMode()) {
    return gep;
  }
  return state->builder->CreateLoad(gep);
}

llvm::Value* GenVisitor::visitCallExpr(CallExpr& self) {
  auto callee = state->mod->getFunction(self.target.str());
  std::vector<llvm::Value*> args_val;
  std::transform(self.args.begin(), self.args.end(), std::back_inserter(args_val),
                 [this](auto& arg) { return arg->accept_e(*this); });
  return state->builder->CreateCall(callee, args_val);
}

llvm::Value* GenVisitor::visitArrayAccessExpr(ArrayAccessExpr& self) {
  auto c = state->setAddrMode();
  auto target_val = self.target->accept_e(*this);
  state->unsetAddrMode(c);
  auto index_val = self.index->accept_e(*this);
  auto gep = state->builder->CreateGEP(target_val, index_val);
  return state->builder->CreateLoad(gep);
}

llvm::Value* GenVisitor::visitVariableExpr(VariableExpr& self) {
  auto v = state->named_values[self.value.str()];
  if (state->getAddrMode()) {
    return v.first;
  }
  return state->builder->CreateLoad(v.first);
}

llvm::Value* GenVisitor::visitFloatLiteral(FloatLiteral& self) {
  auto val = std::stod(self.value.str());
  return llvm::ConstantFP::get(*state->ctx, llvm::APFloat(val));
}

llvm::Value* GenVisitor::visitStringLiteral(StringLiteral& self) {
  return state->builder->CreateGlobalString(self.value.str());
}

llvm::Value* GenVisitor::visitIntLiteral(IntLiteral& self) {
  auto val = std::stoll(self.value.str());
  return llvm::ConstantInt::get(*state->ctx, llvm::APInt(64, val, true));
}

llvm::Value* GenVisitor::visitStructLiteral(StructLiteral& self) {
  auto parent = state->builder->GetInsertBlock()->getParent();
  auto str = state->structures[self.name.str()];
  auto alloca = createAlloca(parent, str.first, self.name.str());
  for (auto& field : self.fields) {
    auto gep = state->builder->CreateStructGEP(alloca, str.second[field.first.str()]);
    state->builder->CreateStore(field.second->accept_e(*this), gep);
  }
  return state->builder->CreateLoad(alloca);
}

llvm::Value* GenVisitor::visitLetStatement(LetStatement& self) {
  auto parent = state->builder->GetInsertBlock()->getParent();
  auto alloca_inst = createAlloca(parent, self.type, self.name.str());

  state->named_values[self.name.str()] = std::make_pair(alloca_inst, self.type->accept_t(*this));

  if (self.rhs.has_value()) {
    auto val = self.rhs.value()->accept_e(*this);
    return state->builder->CreateStore(val, alloca_inst);
  }

  return alloca_inst;
}

llvm::Value* GenVisitor::visitReturnStatement(ReturnStatement& self) {
  if (self.rhs.has_value()) {
    auto r_val = self.rhs.value()->accept_e(*this);
    return state->builder->CreateRet(r_val);
  }
  return state->builder->CreateRetVoid();
}

llvm::Value* GenVisitor::visitBlock(Block& self) {
  llvm::Value* ret_val = nullptr;
  for (auto& stmt : self.stmts)
    ret_val = stmt->accept_s(*this);
  return ret_val;
}

llvm::Value* GenVisitor::visitIfStatement(IfStatement& self) {
  auto parent = state->builder->GetInsertBlock()->getParent();

  auto cont_bb = llvm::BasicBlock::Create(*state->ctx, "");

  for (auto& pair : self.pairs) {
    auto cond = pair.first->accept_e(*this);
    auto then_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
    auto next_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
    state->builder->CreateCondBr(cond, then_bb, next_bb);
    state->builder->SetInsertPoint(then_bb);
    auto last = pair.second.accept_n(*this);
    if (!last || !llvm::isa<llvm::ReturnInst>(last)) {
      state->builder->CreateBr(cont_bb);
    }
    state->builder->SetInsertPoint(next_bb);
  }

  if (self.else_block.has_value()) {
    self.else_block.value().accept_n(*this);
  }

  state->builder->CreateBr(cont_bb);

  parent->getBasicBlockList().push_back(cont_bb);
  state->builder->SetInsertPoint(cont_bb);

  return nullptr;
}

llvm::Value* GenVisitor::visitWhileStatement(WhileStatement& self) {
  auto parent = state->builder->GetInsertBlock()->getParent();

  auto cont_bb = llvm::BasicBlock::Create(*state->ctx, "");
  auto loop_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
  auto then_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
  state->builder->CreateBr(loop_bb);
  state->builder->SetInsertPoint(loop_bb);

  auto cond_l = self.cond->accept_e(*this);
  state->builder->CreateCondBr(cond_l, then_bb, cont_bb);

  state->builder->SetInsertPoint(then_bb);
  auto last = self.body.accept_n(*this);
  if (!last || !llvm::isa<llvm::ReturnInst>(last)) {
    state->builder->CreateBr(loop_bb);
  }

  parent->getBasicBlockList().push_back(cont_bb);
  state->builder->SetInsertPoint(cont_bb);

  return nullptr;
}

llvm::Value* GenVisitor::visitForStatement(ForStatement& self) {
  auto parent = state->builder->GetInsertBlock()->getParent();

  auto cont_bb = llvm::BasicBlock::Create(*state->ctx, "");
  auto loop_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
  auto then_bb = llvm::BasicBlock::Create(*state->ctx, "", parent);
  self.init->accept_s(*this);
  state->builder->CreateBr(loop_bb);
  state->builder->SetInsertPoint(loop_bb);

  auto cond_l = self.cond->accept_e(*this);
  state->builder->CreateCondBr(cond_l, then_bb, cont_bb);

  state->builder->SetInsertPoint(then_bb);
  auto last = self.body.accept_n(*this);
  if (!last || !llvm::isa<llvm::ReturnInst>(last)) {
    self.inc->accept_e(*this);
    state->builder->CreateBr(loop_bb);
  }

  parent->getBasicBlockList().push_back(cont_bb);
  state->builder->SetInsertPoint(cont_bb);

  return nullptr;
}

llvm::Value* GenVisitor::visitProtoFunc(ProtoFunc& self) {
  std::vector<llvm::Type*> args_type;
  std::transform(self.args.begin(), self.args.end(), std::back_inserter(args_type),
                 [this](auto& arg) { return arg.first->accept_t(*this); });

  auto t = llvm::FunctionType::get(self.return_type->accept_t(*this), args_type, false);
  auto f = llvm::Function::Create(t, llvm::Function::ExternalLinkage, self.name.str(), *state->mod);

  auto i = 0;
  for (auto& arg : f->args())
    arg.setName(self.args.at(i++).second.str());

  return f;
}

llvm::Value* GenVisitor::visitExternDecl(ExternDecl& self) { return self.proto->accept_n(*this); }

llvm::Value* GenVisitor::visitFunctionDecl(FunctionDecl& self) {
  auto f = state->mod->getFunction(self.proto->name.str());
  if (!f)
    f = llvm::cast<llvm::Function>(self.proto->accept_n(*this));

  auto bb = llvm::BasicBlock::Create(*state->ctx, "", f);
  state->builder->SetInsertPoint(bb);

  // Record the function arguments in the NamedValues map.
  state->named_values.clear();
  int i = 0;
  for (auto& arg : f->args()) {
    // TODO: fix the type to reference prototype
    auto alloca_inst = createAlloca(f, self.proto->args.at(i).first, arg.getName());

    state->builder->CreateStore(&arg, alloca_inst);

    state->named_values[std::string(arg.getName())] =
        std::make_pair(alloca_inst, self.proto->args.at(i).first->accept_t(*this));
    i++;
  }

  self.body.accept_n(*this);

  llvm::verifyFunction(*f);

  return f;
}

llvm::Value* GenVisitor::visitStructProto(StructProto& self) {
  std::vector<llvm::Type*> elements;

  std::transform(self.fields.begin(), self.fields.end(), std::back_inserter(elements),
                 [this](auto& t) { return t.second->accept_t(*this); });

  auto str = llvm::StructType::create(elements, self.name.str());
  std::map<std::string, size_t> mm;

  int i = 0;
  for (auto& field : self.fields) {
    mm[field.first.str()] = i;
    i++;
  }

  state->structures[self.name.str()] = std::make_pair(str, mm);
  return nullptr;
}

llvm::Value* GenVisitor::visitImportDecl(ImportDecl& self) {
  for (auto& tls : self.data->data) {
    if (auto proto = dynamic_cast<FunctionDecl*>(tls.get())) {
      proto->proto->accept_n(*this);
    } else {
      tls->accept_n(*this);
    }
  }
  return nullptr;
}
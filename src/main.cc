// Copyright 2020 Zachary Mayhew

#include "ast.hh"
#include "state.hh"
#include <cstddef>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <map>
#include <memory>
#include <string>

static std::map<std::string, llvm::Value*> named_values;

int main() {
  // Open a new context and module.
  auto ctx = std::make_unique<llvm::LLVMContext>();
  auto mod = std::make_unique<llvm::Module>("my cool jit", *ctx);

  // Create a new builder for the module.
  auto builder = std::make_unique<llvm::IRBuilder<>>(*ctx);

  auto state =
      std::make_shared<llang::State>(std::move(ctx), std::move(mod), std::move(builder), std::move(named_values));

  // clang-format off
  auto b = std::make_unique<llang::ReturnStatement>(std::make_unique<llang::BinaryExpr>(
      llang::BinaryExpr::Op::ADD,
      std::make_unique<llang::UnaryExpr>(
        llang::UnaryExpr::Op::NEG,
        std::make_unique<llang::VariableExpr>(state, "a")
      ),
      std::make_unique<llang::NumberLiteral>(state, 2.456)
    ));

  auto f_proto = std::make_unique<llang::ProtoFunc>(state, "add", llang::Type::NUMBER,
      std::vector<std::pair<llang::Type, std::string>> {
        std::make_pair(llang::Type::NUMBER, "a"),
        std::make_pair(llang::Type::NUMBER, "b")
      }
    );

  auto stl = llang::Block::StmtList();
  stl.push_back(std::move(b));
  auto block = llang::Block(state, std::move(stl));

  auto f = std::make_unique<llang::FunctionDecl>(state, std::move(f_proto), std::move(block));

  auto b2 = std::make_unique<llang::ReturnStatement>(std::make_unique<llang::BinaryExpr>(
      llang::BinaryExpr::Op::ADD,
      std::make_unique<llang::UnaryExpr>(
        llang::UnaryExpr::Op::NEG,
        std::make_unique<llang::VariableExpr>(state, "a")
      ),
      std::make_unique<llang::NumberLiteral>(state, 2.456)
    ));

  auto f2_proto = std::make_unique<llang::ProtoFunc>(state, "add2", llang::Type::NUMBER,
      std::vector<std::pair<llang::Type, std::string>> {
        std::make_pair(llang::Type::NUMBER, "a"),
        std::make_pair(llang::Type::NUMBER, "b")
      }
    );

  auto ifl = llang::Block::StmtList();

  //ifl.push_back(std::make_unique<llang::ReturnStatement>(std::make_unique<llang::NumberLiteral>(state, 3.1415)));
  std::vector<std::unique_ptr<llang::Expression>> vec;
  vec.push_back(std::make_unique<llang::NumberLiteral>(state, 3.1415));
  vec.push_back(std::make_unique<llang::NumberLiteral>(state, 3.1415));
  ifl.push_back(std::make_unique<llang::CallExpr>(
      state,
      "add",
      std::move(vec)
    ));


  auto if_bod = llang::Block(state, std::move(ifl));

  auto ifl2 = llang::Block::StmtList();

  // ifl2.push_back(std::make_unique<llang::ReturnStatement>(std::make_unique<llang::NumberLiteral>(state, 54321)));
  std::vector<std::unique_ptr<llang::Expression>> vec2;
  vec2.push_back(std::make_unique<llang::NumberLiteral>(state, 3.1415));
  vec2.push_back(std::make_unique<llang::NumberLiteral>(state, 3.1415));
  ifl2.push_back(std::make_unique<llang::CallExpr>(
      state,
      "add",
      std::move(vec2)
    ));

  auto if2_bod = llang::Block(state, std::move(ifl2));

  auto stl2 = llang::Block::StmtList();



  auto ifl3 = llang::Block::StmtList();

  // ifl2.push_back(std::make_unique<llang::ReturnStatement>(std::make_unique<llang::NumberLiteral>(state, 54321)));
  std::vector<std::unique_ptr<llang::Expression>> vec3;
  vec3.push_back(std::make_unique<llang::NumberLiteral>(state, 3.1415));
  vec3.push_back(std::make_unique<llang::NumberLiteral>(state, 3.1415));
  ifl3.push_back(std::make_unique<llang::CallExpr>(
      state,
      "add",
      std::move(vec3)
    ));

  auto if3_bod = llang::Block(state, std::move(ifl3));

  auto stl3 = llang::Block::StmtList();

  auto ifstmt = std::make_unique<llang::IfStatement>(state, std::move(if3_bod));

  ifstmt->pairs.push_back(llang::IfStatement::IfPair(
      std::make_unique<llang::BinaryExpr>(
        llang::BinaryExpr::Op::EQ,
        std::make_unique<llang::VariableExpr>(state, "a"),
        std::make_unique<llang::NumberLiteral>(state, 2.0)
      ),
      std::move(if_bod)
    ));

  ifstmt->pairs.push_back(llang::IfStatement::IfPair(
      std::make_unique<llang::BinaryExpr>(
        llang::BinaryExpr::Op::EQ,
        std::make_unique<llang::VariableExpr>(state, "b"),
        std::make_unique<llang::NumberLiteral>(state, 3.0)
      ),
      std::move(if2_bod)
    ));

  //ifstmt->pairs.push_back(llang::IfStatement::IfPair(
  //    std::make_unique<llang::BinaryExpr>(
  //      llang::BinaryExpr::Op::EQ,
  //      std::make_unique<llang::VariableExpr>(state, "b"),
  //      std::make_unique<llang::NumberLiteral>(state, 3.0)
  //    ),
  //    std::move(if2_bod)
  //  ));

  stl2.push_back(std::move(ifstmt));
  stl2.push_back(std::move(b2));
  auto block2 = llang::Block(state, std::move(stl2));

  auto f2 = std::make_unique<llang::FunctionDecl>(state, std::move(f2_proto), std::move(block2));

  // clang-format on

  f->codegen();
  f2->codegen();

  state->mod->print(llvm::errs(), nullptr);
  return 0;
}

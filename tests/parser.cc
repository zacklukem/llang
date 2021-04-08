#include <gtest/gtest.h>
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <map>
#include <parser.hh>

TEST(Parser, ParserImpl) {
  std::string str = R"(
    function main(argc: i64, argv: str*): i64 {
      let i: i64 = 0;
      while (i < argc) {
        puts(argv[i])
        i = i + 1
      }
      return 0
    }
  )";
  auto source = std::make_shared<llang::Source>(str);

  // Open a new context and module.
  auto ctx = std::make_unique<llvm::LLVMContext>();
  auto mod = std::make_unique<llvm::Module>("my cool jit", *ctx);
  std::map<std::string, std::pair<llvm::AllocaInst*, llvm::Type*>> named_values;

  // Create a new builder for the module.
  auto builder = std::make_unique<llvm::IRBuilder<>>(*ctx);

  auto state = std::make_shared<llang::State>(std::move(ctx), std::move(mod), std::move(builder),
                                              std::move(named_values));

  llang::Parser parser(source, state);

  std::vector<llvm::Type*> args{llvm::Type::getDoubleTy(*state->ctx)};
  llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getVoidTy(*state->ctx), args, false);

  llvm::Function* f =
      llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "putd", state->mod.get());

  std::vector<llvm::Type*> args2{llvm::Type::getInt8PtrTy(*state->ctx)};
  llvm::FunctionType* ft2 =
      llvm::FunctionType::get(llvm::Type::getVoidTy(*state->ctx), args2, false);

  llvm::Function* f2 =
      llvm::Function::Create(ft2, llvm::Function::ExternalLinkage, "puts", state->mod.get());

  auto func = parser.parseFunction();
  if (!source->debugPrintMessages()) {
    func->codegen();
  };

  state->mod->print(llvm::errs(), nullptr);
}
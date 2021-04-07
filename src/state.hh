#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>

namespace llang {

struct State {
  std::unique_ptr<llvm::LLVMContext> ctx;
  std::unique_ptr<llvm::Module> mod;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::map<std::string, llvm::Value*> named_values;
  State(std::unique_ptr<llvm::LLVMContext> ctx, std::unique_ptr<llvm::Module> mod,
        std::unique_ptr<llvm::IRBuilder<>> builder, std::map<std::string, llvm::Value*> named_values)
      : ctx(std::move(ctx)), mod(std::move(mod)), builder(std::move(builder)), named_values(std::move(named_values)){};
};

} // namespace llang

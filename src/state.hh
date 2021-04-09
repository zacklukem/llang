#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
namespace llang {

class State {
public:
  std::unique_ptr<llvm::LLVMContext> ctx;
  std::unique_ptr<llvm::Module> mod;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::map<std::string, std::pair<llvm::AllocaInst*, llvm::Type*>> named_values;
  std::map<std::string, std::pair<llvm::StructType*, std::map<std::string, size_t>>> structures;
  inline bool setAddrMode() {
    bool c = is_addr_mode;
    is_addr_mode = true;
    return c;
  };
  inline void unsetAddrMode(bool c) { is_addr_mode = c; };
  inline bool getAddrMode() { return is_addr_mode; };
  State(std::unique_ptr<llvm::LLVMContext> ctx, std::unique_ptr<llvm::Module> mod,
        std::unique_ptr<llvm::IRBuilder<>> builder,
        std::map<std::string, std::pair<llvm::AllocaInst*, llvm::Type*>> named_values)
      : ctx(std::move(ctx)), mod(std::move(mod)), builder(std::move(builder)),
        named_values(std::move(named_values)){};

private:
  bool is_addr_mode = false;
};

} // namespace llang

// Copyright 2020 Zachary Mayhew

#include "ast.hh"
#include "state.hh"
#include <cstddef>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <map>
#include <memory>
#include <string>

#include "parser.hh"

#include <cerrno>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <filesystem>

std::string get_file_contents(const char* filename) {
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in) {
    std::ostringstream contents;
    contents << in.rdbuf();
    in.close();
    return contents.str();
  }
  throw errno;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: llang <file>";
  }
  auto file_fs = std::filesystem::path(argv[1]);
  std::string str = get_file_contents(file_fs.c_str());
  auto source = std::make_shared<llang::Source>(str, file_fs.generic_string());

  // Open a new context and module.
  auto ctx = std::make_unique<llvm::LLVMContext>();
  auto mod = std::make_unique<llvm::Module>("my cool jit", *ctx);
  std::map<std::string, std::pair<llvm::AllocaInst*, llvm::Type*>> named_values;

  // Create a new builder for the module.
  auto builder = std::make_unique<llvm::IRBuilder<>>(*ctx);

  auto state = std::make_shared<llang::State>(std::move(ctx), std::move(mod), std::move(builder),
                                              std::move(named_values));

  llang::Parser parser(source, state);

  auto func = parser.parseDocument();
  func->verify();
  if (!source->debugPrintMessages()) {
    func->codegen();
  } else {
    return 0;
  }

  state->mod->print(llvm::errs(), nullptr);

  // Initialize the target registry etc.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  auto targetTriple = llvm::sys::getDefaultTargetTriple();
  state->mod->setTargetTriple(targetTriple);

  std::string error;
  auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (!target) {
    llvm::errs() << error;
    return 1;
  }

  auto cpu = "generic";
  auto features = "";

  llvm::TargetOptions opt;
  auto rm = llvm::Optional<llvm::Reloc::Model>();
  auto theTargetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, rm);

  state->mod->setDataLayout(theTargetMachine->createDataLayout());

  auto filename = file_fs.replace_extension(".o").generic_string();
  std::error_code ec;
  llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);

  if (ec) {
    llvm::errs() << "Could not open file: " << ec.message();
    return 1;
  }

  llvm::legacy::PassManager pass;
  auto fileType = llvm::CGFT_ObjectFile;

  if (theTargetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
    llvm::errs() << "TheTargetMachine can't emit a file of this type";
    return 1;
  }

  pass.run(*state->mod);
  dest.flush();

  llvm::outs() << "Wrote " << filename << "\n";
}

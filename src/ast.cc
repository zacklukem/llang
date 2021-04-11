#include "ast.hh"
#include <llvm/Support/Casting.h>

using namespace llang;

std::ostream& operator<<(std::ostream& out, const llang::Node& node) {
  node.print(out);
  return out;
}

std::unique_ptr<Expression> VariableExpr::clone() const {
  return std::make_unique<VariableExpr>(state, value);
}

std::unique_ptr<Expression> DotExpr::clone() const {
  return std::make_unique<DotExpr>(lhs->clone(), rhs);
}

std::unique_ptr<Expression> ArrayAccessExpr::clone() const {
  return std::make_unique<ArrayAccessExpr>(state, target->clone(), index->clone());
}

std::unique_ptr<Expression> UnaryExpr::clone() const {
  return std::make_unique<UnaryExpr>(oper, rhs->clone());
}

bool PtrType::operator==(const Type& rhs) const {
  // TODO: ptr el type
  if (!llvm::isa<PtrType>(rhs))
    return false;
  return true;
}

bool IntType::operator==(const Type& rhs) const {
  if (!llvm::isa<IntType>(rhs))
    return false;
  return true;
}

bool FloatType::operator==(const Type& rhs) const {
  if (!llvm::isa<FloatType>(rhs))
    return false;
  return true;
}

bool StructType::operator==(const Type& rhs) const {
  if (!llvm::isa<StructType>(rhs))
    return false;
  return true;
}

bool VoidType::operator==(const Type& rhs) const {
  if (!llvm::isa<VoidType>(rhs))
    return false;
  return true;
}

std::string PtrType::str() const { return type->str() + "*"; }

std::string IntType::str() const { return "i" + std::to_string(size); }

std::string FloatType::str() const { return "f" + std::to_string(size); }

std::string StructType::str() const { return name.str(); }

std::string VoidType::str() const { return "void"; }
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

void Document::print(std::ostream& target) const {}

void Block::print(std::ostream& target) const {}

void LetStatement::print(std::ostream& target) const {}

void ReturnStatement::print(std::ostream& target) const {}

void IfStatement::print(std::ostream& target) const {}

void WhileStatement::print(std::ostream& target) const {}

void ForStatement::print(std::ostream& target) const {}

void AssignExpr::print(std::ostream& target) const {
  target << "(= " << *lhs << ' ' << *rhs << ")";
}

void BinaryExpr::print(std::ostream& target) const {
  target << "(";
  switch (oper) {
  case ADD:
    target << '+';
    break;
  case SUB:
    target << '-';
    break;
  case MUL:
    target << '*';
    break;
  case DIV:
    target << '/';
    break;
  case EQ:
    target << "==";
    break;
  case LT:
    target << "<";
    break;
  case GT:
    target << ">";
    break;
  case GTE:
    target << ">=";
    break;
  case LTE:
    target << "<=";
    break;
  }
  target << ' ' << *lhs << ' ' << *rhs << ")";
}

void UnaryExpr::print(std::ostream& target) const {
  target << "("
         << "unary_temp" << ' ' << *rhs << ")";
}

void CallExpr::print(std::ostream& target) const {
  target << "(" << this->target.str();
  for (auto& arg : args) {
    target << ' ' << *arg;
  }
  target << ")";
}

void VariableExpr::print(std::ostream& target) const { target << value.str(); }

void DotExpr::print(std::ostream& target) const {
  target << "(. " << *lhs << ' ' << rhs.str() << ")";
}

void ArrayAccessExpr::print(std::ostream& target) const {
  target << "("
         << "[]" << ' ' << *this->target << ' ' << *index << ")";
}

void FloatLiteral::print(std::ostream& target) const { target << value.str(); }

void StringLiteral::print(std::ostream& target) const { target << value.str(); }

void IntLiteral::print(std::ostream& target) const { target << value.str(); }

void StructLiteral::print(std::ostream& target) const {
  target << "{" << name.str();
  for (auto& field : fields) {
    target << " (" << field.first.str() << ' ' << *field.second << ")";
  }
  target << "}";
}

void ProtoFunc::print(std::ostream& target) const {}

void ExternDecl::print(std::ostream& target) const {}

void FunctionDecl::print(std::ostream& target) const {}

void StructProto::print(std::ostream& target) const {}

void ImportDecl::print(std::ostream& target) const {
  target << "import " << file_name.str() << ";";
}

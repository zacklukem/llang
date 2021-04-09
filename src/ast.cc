#include "ast.hh"

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
#include "ast.hh"
#include "source.hh"
using namespace llang;

Span Document::span() const { return data.front()->span(); }
Span PtrType::span() const { return type->span() + mul; }
Span IntType::span() const { return name; }
Span FloatType::span() const { return name; }
Span StructType::span() const { return name; }
Span VoidType::span() const { return name; }
Span Block::span() const { return stmts.front()->span(); }
Span LetStatement::span() const { return ident; }
Span ReturnStatement::span() const { return ident; }
Span IfStatement::span() const { return ident; }
Span WhileStatement::span() const { return ident; }
Span ForStatement::span() const { return ident; }
Span AssignExpr::span() const { return lhs->span() + rhs->span(); }
Span BinaryExpr::span() const { return lhs->span() + rhs->span(); }
Span UnaryExpr::span() const { return rhs->span(); }
Span CallExpr::span() const { return target; }
Span VariableExpr::span() const { return value; }
Span DotExpr::span() const { return lhs->span() + rhs; }
Span ArrayAccessExpr::span() const { return target->span(); }
Span FloatLiteral::span() const { return value; }
Span StringLiteral::span() const { return value; }
Span IntLiteral::span() const { return value; }
Span StructLiteral::span() const { return name; }
Span ProtoFunc::span() const { return ident; }
Span ExternDecl::span() const { return proto->span(); }
Span FunctionDecl::span() const { return proto->span(); }
Span StructProto::span() const { return ident; }
Span ImportDecl::span() const { return file_name; }
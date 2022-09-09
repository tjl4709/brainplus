//
// Created by 7budd on 9/9/2022.
//
#include "ASTNodes.h"

std::string StatementNode::to_string() { return "Statement" + ASTNode::to_string(); }
std::string MultiStatementNode::to_string() {
    if (Statements->empty()) return "";
    std::string ret;
    for (StatementNode *st : *Statements)
        ret += '\n' + st->to_string();
    return ret.substr(1);
}
std::string NumberNode::to_string() { return std::to_string(Number); }
std::string CallNode::to_string() { return Id; }
std::string NullaryOperatorNode::to_string() { return EnumOps::OpToStr(Op); }
std::string UnaryOperatorNode::to_string() { return EnumOps::OpToStr(Op) + NodeOps::Parenthesize(RHS); }
std::string BinaryOperatorNode::to_string() { return NodeOps::Parenthesize(LHS) + UnaryOperatorNode::to_string(); }
std::string DoWhileNode::to_string() {
    if (Type == NodeType::While)
        return "while (" + Expression->to_string() + ") {" + (Body ? '\n' + Body->to_string() + '\n' : "") + '}';
    return "do {" + (Body ? '\n' + Body->to_string() + '\n' : "") + "} while (" + Expression->to_string() + ')';
}
std::string IfTernaryNode::to_string() {
    if (Type == NodeType::Ternary)
        return NodeOps::Parenthesize(Expression) + '?' + NodeOps::Parenthesize(Body) + ':' + NodeOps::Parenthesize(Else);
    std::string ret = "if (" + Expression->to_string() + ") {" + (Body ? '\n' + Body->to_string() + '\n' : "") + '}';
    if (Else != nullptr) {
        ret += " else ";
        if (Else->getType() != NodeType::If)
            ret += "{\n" + Else->to_string() + "\n}";
        else ret += Else->to_string();
    }
    return ret;
}
std::string ForNode::to_string() {
    return "for (" + (Start ? Start->to_string() : "") + "; " + Expression->to_string() + "; "
           + (Step ? Step->to_string() : "") + ") {" + (Body ? '\n' + Body->to_string() + '\n' : "") + '}';
}
std::string IncludeNode::to_string() { return "include \"" + Id + '"'; }
std::string DefineNode::to_string() {
    std::string str;
    for (const auto& tok : *Replacement)
        str += " " + tok->toString();
    return "define " + Id + ":" + str;
}
std::string FunctionNode::to_string() { return Id + " {\n" + Statement->to_string() + "\n}"; }

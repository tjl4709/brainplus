//
// Created by 7budd on 5/24/2022.
//
#ifndef BRAINPLUS_ASTNODES_H
#define BRAINPLUS_ASTNODES_H

#include <utility>
#include <vector>
#include "Lexer.h"

enum NodeType {
    Base,
    Statement,
    MultiStatement,
    Number,
    NullaryOperator,
    UnaryOperator,
    BinaryOperator,
    Do,
    While,
    If,
    Ternary,
    For,
    Include,
    Call,
    Define,
    Function
};
enum Operator {
    //nullary operators
    print,              // .
    read,               // ,

    //unary value operators
    addition,           // +
    subtraction,        // -
    multiplication,     // *
    division,           // /
    assignment,         // =
    //unary ptr operators
    ptr_addition,       // @+
    ptr_subtraction,    // @-
    ptr_multiplication, // @*
    ptr_division,       // @/
    ptr_assignment,     // @=

    //pointer operators
    ptr_store,          // #
    ptr_lookup,         // @
    ptr_lookupRelUp,    // @#
    ptr_lookupRelDown,  // @##

    //value comparison operators
    lessThan,           // <
    greaterThan,        // >
    lessOrEqual,        // <=
    greaterOrEqual,     // >=
    equalTo,            // ==
    notEqual,           // !=
    //ptr comparison operators
    ptr_lessThan,       // @<
    ptr_greaterThan,    // @>
    ptr_lessOrEqual,    // @<=
    ptr_greaterOrEqual, // @>=
    ptr_equalTo,        // @==
    ptr_notEqual,       // @!=

    //boolean binary operators
    bool_and,           // &&
    bool_or,            // ||
    bool_xor            // ^^
};
std::string OpToStr(Operator op) {
    switch (op) {
        case print:             return ".";
        case read:              return ",";
        case addition:          return "+";
        case subtraction:       return "-";
        case multiplication:    return "*";
        case division:          return "/";
        case assignment:        return "=";
        case ptr_addition:      return "@+";
        case ptr_subtraction:   return "@-";
        case ptr_multiplication:return "@*";
        case ptr_division:      return "@/";
        case ptr_assignment:    return "@=";
        case ptr_store:         return "#";
        case ptr_lookup:        return "@";
        case ptr_lookupRelUp:   return "@#";
        case ptr_lookupRelDown: return "@##";
        case lessThan:          return "<";
        case greaterThan:       return ">";
        case lessOrEqual:       return "<=";
        case greaterOrEqual:    return ">=";
        case equalTo:           return "==";
        case notEqual:          return "!=";
        case ptr_lessThan:      return "@<";
        case ptr_greaterThan:   return "@>";
        case ptr_lessOrEqual:   return "@<=";
        case ptr_greaterOrEqual:return "@>=";
        case ptr_equalTo:       return "@==";
        case ptr_notEqual:      return "@!=";
        case bool_and:          return "&&";
        case bool_or:           return "||";
        case bool_xor:          return "^^";
    }
}

//Base Abstract Syntax Tree Node class
class ASTNode {
    Location Loc;
protected:
    NodeType Type;
public:
    ASTNode(Location l, NodeType t) : Loc(l), Type(t) {}
    explicit ASTNode(Location l) : ASTNode(l, NodeType::Base) {}
    int getLine() const { return Loc.Line; }
    int getCol() const { return Loc.Col; }
    NodeType getType() { return Type; }
    virtual std::string toString() { return " at " + Loc.toString(); }
};

//Statement nodes
class StatementNode : public ASTNode {
protected:
    StatementNode(Location l, NodeType t) : ASTNode(l, t) {}
    explicit StatementNode(Location l) : StatementNode(l, NodeType::Statement) {}
public:
    std::string toString() override { return "Statement" + ASTNode::toString(); }
};
class MultiStatementNode : public StatementNode {
    std::vector<StatementNode*> *Statements;
public:
    MultiStatementNode(std::vector<StatementNode*> *statements, Location l) : StatementNode(l, NodeType::MultiStatement),
        Statements(statements == nullptr ? new std::vector<StatementNode*> : statements) {}
    explicit MultiStatementNode(Location l) : MultiStatementNode(nullptr, l) {}
    std::string toString() override {
        if (Statements->empty()) return "";
        std::string ret;
        for (StatementNode *st : *Statements)
            ret += '\n' + st->toString();
        return ret.substr(1);
    }

    void addStatement(StatementNode *statement) { Statements->push_back(statement); }
    bool insertStatement(StatementNode *statement, int index) {
        if (index < 0 || index > Statements->size())
            return false;
        Statements->insert(Statements->begin() + index, statement);
        return true;
    }
    bool removeStatement(int index) {
        if (index < 0 || index >= Statements->size())
            return false;
        Statements->erase(Statements->begin() + index);
        return true;
    }
    bool removeStatement(StatementNode *statement) {
        auto elem = std::find(Statements->begin(), Statements->end(), statement);
        if (elem == Statements->end()) return false;
        Statements->erase(elem);
        return true;
    }
    StatementNode* getStatement(int index) { return Statements->at(index); }
};
class NumberNode : public StatementNode {
    int Number;
public:
    NumberNode(int n, Location l) : StatementNode(l, NodeType::Number), Number(n) {}
    std::string toString() override { return "N:" + std::to_string(Number); }
};
//Operator nodes
class NullaryOperatorNode : public StatementNode {
protected:
    Operator Op;
public:
    NullaryOperatorNode(Operator op, Location l) : StatementNode(l, NodeType::NullaryOperator), Op(op) {}
    std::string toString() override { return OpToStr(Op); }
};
class UnaryOperatorNode : public NullaryOperatorNode {
protected:
    StatementNode *RHS; //Right-Hand Side = number, ptr lookup, or ternary
public:                 //if RHS = null, implied default number
    UnaryOperatorNode(Operator op, StatementNode *rhs, Location l) : NullaryOperatorNode(op, l),
        RHS(rhs) { Type = NodeType::UnaryOperator; }
    std::string toString() override { return OpToStr(Op) + ' ' + RHS->toString(); }
};
class BinaryOperatorNode : public UnaryOperatorNode {
    StatementNode *LHS; //Left-Hand Side = null if ptr op
public:                 //RHS = number or ptr lookup if comp op, else bool expr
    BinaryOperatorNode(Operator op, StatementNode *lhs, StatementNode *rhs, Location l) :
        UnaryOperatorNode(op, rhs, l), LHS(lhs) { Type = NodeType::BinaryOperator; }
    std::string toString() override { return LHS->toString() + ' ' + UnaryOperatorNode::toString(); }
};
//Control statement nodes
class DoWhileNode : public StatementNode {
protected:
    StatementNode *Expression, *Body;
public:
    DoWhileNode(StatementNode *expr, StatementNode *body, bool isWhile, Location l) :
        StatementNode(l), Expression(expr), Body(body) { Type = isWhile ? NodeType::While : NodeType::Do; }
    DoWhileNode(StatementNode *expr, StatementNode *body, Location l) :
        DoWhileNode(expr, body, false, l) {}
    std::string toString() override {
        if (Type == NodeType::While)
            return "while (" + Expression->toString() + ") {\n" + Body->toString() + "\n}";
        return "do {\n" + Body->toString() + "\n} while (" + Expression->toString() + ')';
    }
};
class IfTernaryNode : public DoWhileNode {
    StatementNode *Else;    //null if no else. Body and Else are number, ptr lookup, or ternary if this is a ternary
public:
    IfTernaryNode(StatementNode *expr, StatementNode *body, StatementNode *elseBody, bool isTernary, Location l) :
        DoWhileNode(expr, body, l), Else(elseBody) { Type = isTernary ? NodeType::Ternary : NodeType::If; }
    IfTernaryNode(StatementNode *expr, StatementNode *body, StatementNode *elseBody, Location l) :
            IfTernaryNode(expr, body, elseBody, false, l) {}
    std::string toString() override {
        if (Type == NodeType::Ternary) return Expression->toString() + " ? " + Body->toString() + " : " + Else->toString();
        std::string ret = "if (" + Expression->toString() + ") {\n" + Body->toString() + "\n}";
        if (Else != nullptr) {
            ret += " else ";
            if (Else->getType() != NodeType::If)
                ret += "{\n" + Else->toString() + "\n}";
            else ret += Else->toString();
        }
        return ret;
    }
};
class ForNode : public DoWhileNode {
    StatementNode *Start, *Step;
public:
    ForNode(StatementNode *start, StatementNode *expr, StatementNode *step, StatementNode *body, Location l) :
        DoWhileNode(expr, body, l), Start(start), Step(step) { Type = NodeType::For; }
    std::string toString() override {
        return "for (" + Start->toString() + "; " + Expression->toString() + "; "
            + Step->toString() + ") {\n" + Body->toString() + "\n}";
    }
};

//Include, define, and function definition and call nodes
class IncludeNode : public ASTNode {
protected:
    std::string Id;
public:
    IncludeNode(std::string id, Location l) : ASTNode(l, NodeType::Include), Id(std::move(id)) {}
    std::string toString() override { return "include " + Id; }
};
class CallNode : public IncludeNode {
public:
    CallNode(std::string id, Location l) : IncludeNode(std::move(id), l) { Type = NodeType::Call; }
    std::string toString() override { return Id; }
};
class DefineNode : public IncludeNode {
protected:
    StatementNode *Statement;
public:
    DefineNode(std::string id, StatementNode *statement, Location l) : IncludeNode(std::move(id), l),
        Statement(statement) { Type = NodeType::Define; }
    std::string toString() override { return "define " + Id + " " + Statement->toString(); }
};
class FunctionNode : public DefineNode {
public:
    FunctionNode(std::string id, StatementNode *statement, Location l) :
        DefineNode(std::move(id), statement, l) { Type = NodeType::Function; }
    std::string toString() override { return Id + " {\n" + Statement->toString() + "\n}"; }
};

#endif //BRAINPLUS_ASTNODES_H

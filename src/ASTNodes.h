//
// Created by 7budd on 5/24/2022.
//
#ifndef BRAINPLUS_ASTNODES_H
#define BRAINPLUS_ASTNODES_H

#include <utility>
#include "Lexer.h"

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

//Base Abstract Syntax Tree Node class
class ASTNode {
    Location Loc;
public:
    explicit ASTNode(Location l) : Loc(l) {}
    int getLine() const { return Loc.Line; }
    int getCol() const { return Loc.Col; }
    virtual std::string toString() { return " at " + Loc.toString(); }
};

//Statement nodes
class StatementNode : public ASTNode {
protected:
    explicit StatementNode(Location l) : ASTNode(l) {}
};
class NumberNode : public StatementNode {
    int Number;
public:
    NumberNode(int n, Location l) : StatementNode(l), Number(n) {}
};
//Operator nodes
class NullaryOperatorNode : public StatementNode {
protected:
    Operator Op;
public:
    NullaryOperatorNode(Operator op, Location l) : StatementNode(l), Op(op) {}
};
class UnaryOperatorNode : public NullaryOperatorNode {
protected:
    StatementNode RHS;  //Right-Hand Side = number, ptr lookup, or ternary
public:
    UnaryOperatorNode(Operator op, StatementNode rhs, Location l) : NullaryOperatorNode(op, l), RHS(std::move(rhs)) {}
};
class BinaryOperatorNode : public UnaryOperatorNode {
    StatementNode LHS;  //Left-Hand Side = null if ptr op
                        //RHS = number or ptr lookup if comp op, else bool expr
public:
    BinaryOperatorNode(Operator op, StatementNode lhs, StatementNode rhs, Location l) :
        UnaryOperatorNode(op, std::move(rhs), l), LHS(std::move(lhs)) {}
};
//Control statement nodes
class DoWhileNode : public StatementNode {
    bool IsWhile;
protected:
    StatementNode Expression, Body;
public:
    DoWhileNode(StatementNode expr, StatementNode body, bool isWhile, Location l) :
        StatementNode(l), Expression(std::move(expr)), Body(std::move(body)), IsWhile(isWhile) {}
    DoWhileNode(StatementNode expr, StatementNode body, Location l) :
        DoWhileNode(std::move(expr), std::move(body), false, l) {}
    bool isWhile() const { return IsWhile; }
};
class IfTernaryNode : public DoWhileNode {
    StatementNode Else; //null if no else, Body and Else are number, ptr lookup, or ternary if this is a ternary
public:
    IfTernaryNode(StatementNode expr, StatementNode body, StatementNode elseBody, bool isTernary, Location l) :
        DoWhileNode(std::move(expr), std::move(body), isTernary, l), Else(std::move(elseBody)) {}
    IfTernaryNode(StatementNode expr, StatementNode body, StatementNode elseBody, Location l) :
            IfTernaryNode(std::move(expr), std::move(body), std::move(elseBody), false, l) {}
    bool isTernary() const { return isWhile(); }
};
class ForNode : public DoWhileNode {
    StatementNode Start, Step;
public:
    ForNode(StatementNode start, StatementNode expr, StatementNode step, StatementNode body, Location l) :
        DoWhileNode(std::move(expr), std::move(body), l), Start(std::move(start)), Step(std::move(step)) {}
};

//Include, define, and function definition and call nodes
class IncludeNode : public ASTNode {
protected:
    std::string Id;
public:
    IncludeNode(std::string id, Location l) : ASTNode(l), Id(std::move(id)) {}
    std::string toString() override { return "include " + Id; }
};
class CallNode : public IncludeNode {
public:
    CallNode(std::string id, Location l) : IncludeNode(std::move(id), l) {}
    std::string toString() override { return Id; }
};
class DefineNode : public IncludeNode {
protected:
    StatementNode Statement;
public:
    DefineNode(std::string id, StatementNode statement, Location l) : IncludeNode(std::move(id), l), Statement(std::move(statement)) {}
    std::string toString() override { return "define " + Id + " " + Statement.toString(); }
};
class FunctionNode : public DefineNode {
public:
    FunctionNode(std::string id, StatementNode statement, Location l) : DefineNode(std::move(id), std::move(statement), l) {}
    std::string toString() override { return Id + " {\n" + Statement.toString() + "\n}"; }
};

#endif //BRAINPLUS_ASTNODES_H

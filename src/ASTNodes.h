//
// Created by 7budd on 5/24/2022.
//
#ifndef BRAINPLUS_ASTNODES_H
#define BRAINPLUS_ASTNODES_H

#include <utility>
#include <vector>

//Base Abstract Syntax Tree Node class
class ASTNode {
    Location Loc;
protected:
    NodeType Type;
public:
    ASTNode(Location l, NodeType t) : Loc(l), Type(t) {}
    explicit ASTNode(Location l) : ASTNode(l, NodeType::Base) {}
    virtual ~ASTNode() = default;

    int getLine() const { return Loc.Line; }
    int getCol() const { return Loc.Col; }
    std::string getLocString() { return Loc.toString(); }
    void setLocation(Location l) { Loc = l; }
    NodeType getType() { return Type; }
    virtual std::string toString() { return " at " + Loc.toString(); }
};

//Statement nodes
class StatementNode : public ASTNode {
protected:
    StatementNode(Location l, NodeType t) : ASTNode(l, t) {}
    explicit StatementNode(Location l) : StatementNode(l, NodeType::Statement) {}
public:
    ~StatementNode() override = default;
    std::string toString() override { return "Statement" + ASTNode::toString(); }
};
class MultiStatementNode : public StatementNode {
    std::vector<StatementNode*> *Statements;
public:
    MultiStatementNode(std::vector<StatementNode*> *statements, Location l) : StatementNode(l, NodeType::MultiStatement),
        Statements(statements == nullptr ? new std::vector<StatementNode*> : statements) {}
    explicit MultiStatementNode(Location l) : MultiStatementNode(nullptr, l) {}
    ~MultiStatementNode() override {
        while (!Statements->empty()) {
            delete Statements->back();
            Statements->pop_back();
        }
        delete Statements;
    }

    std::string toString() override {
        if (Statements->empty()) return "";
        std::string ret;
        for (StatementNode *st : *Statements)
            ret += '\n' + st->toString();
        return ret.substr(1);
    }

    void addStatement(StatementNode *statement) {
        if (statement->getType() == NodeType::MultiStatement)
            addAllStatements((MultiStatementNode*)statement);
        else Statements->push_back(statement);
    }
    void addAllStatements(MultiStatementNode *multi) {
        for (auto *node : *multi->Statements)
            addStatement(node);
    }
    bool insertStatement(StatementNode *statement, int index) {
        if (statement->getType() == NodeType::MultiStatement)
            return insertAllStatements((MultiStatementNode*)statement, index);
        if (index < 0 || index > Statements->size())
            return false;
        Statements->insert(Statements->begin() + index, statement);
        return true;
    }
    bool insertAllStatements(MultiStatementNode *multi, int index) {
        if (index < 0 || index > Statements->size())
            return false;
        Statements->insert(Statements->begin() + index, multi->Statements->begin(), multi->Statements->end());
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
    unsigned int getNumStatements() { return Statements->size(); }
    StatementNode* getStatement(int index) { return Statements->at(index); }
};
class NumberNode : public StatementNode {
    int Number;
public:
    NumberNode(int n, Location l) : StatementNode(l, NodeType::Number), Number(n) {}
    ~NumberNode() override = default;
    std::string toString() override { return "N:" + std::to_string(Number); }
};
class CallNode : public StatementNode {
    std::string Id;
public:
    CallNode(std::string id, Location l) : StatementNode(l), Id(std::move(id)) { Type = NodeType::Call; }
    ~CallNode() override = default;
    std::string getId() { return Id; }
    std::string toString() override { return Id; }
};
//Operator nodes
class NullaryOperatorNode : public StatementNode {
protected:
    Operator Op;
public:
    NullaryOperatorNode(Operator op, Location l) : StatementNode(l, NodeType::NullaryOperator), Op(op) {}
    ~NullaryOperatorNode() override = default;
    std::string toString() override { return EnumOps::OpToStr(Op); }
};
class UnaryOperatorNode : public NullaryOperatorNode {
protected:
    StatementNode *RHS; //Right-Hand Side = number, ptr lookup, or ternary
public:                 //if RHS = null, implied default number
    UnaryOperatorNode(Operator op, StatementNode *rhs, Location l) : NullaryOperatorNode(op, l),
        RHS(rhs) { Type = NodeType::UnaryOperator; }
    ~UnaryOperatorNode() override { delete RHS; }
    std::string toString() override { return EnumOps::OpToStr(Op) + ' ' + RHS->toString(); }
};
class BinaryOperatorNode : public UnaryOperatorNode {
    StatementNode *LHS; //Left-Hand Side = null if ptr op
public:                 //RHS = number or ptr lookup if comp op, else bool expr
    BinaryOperatorNode(Operator op, StatementNode *lhs, StatementNode *rhs, Location l) :
        UnaryOperatorNode(op, rhs, l), LHS(lhs) { Type = NodeType::BinaryOperator; }
    ~BinaryOperatorNode() override { delete LHS; }
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
    ~DoWhileNode() override { delete Expression; delete Body; }
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
    ~IfTernaryNode() override { delete Else; }
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
    ~ForNode() override { delete Start; delete Step; }
    std::string toString() override {
        return "for (" + Start->toString() + "; " + Expression->toString() + "; "
            + Step->toString() + ") {\n" + Body->toString() + "\n}";
    }
};

//Include, define, and function definition nodes
class IncludeNode : public ASTNode {
protected:
    std::string Id;
public:
    IncludeNode(std::string id, Location l) : ASTNode(l, NodeType::Include), Id(std::move(id)) {}
    ~IncludeNode() override = default;
    std::string getFname() { return Id.substr(Id.rfind('\\')+1); }
    std::string getDir() { unsigned int i; return (i = Id.rfind('\\')) == -1 ? "." : Id.substr(0, i+1); }
    std::string getId() { return Id; }
    std::string toString() override { return "include \"" + Id + '"'; }
};
class DefineNode : public IncludeNode {
protected:
    std::vector<Token*> *Replacement;
public:
    DefineNode(std::string id, std::vector<Token*> *replacement, Location l) : IncludeNode(std::move(id), l),
        Replacement(replacement) { Type = NodeType::Define; }
    ~DefineNode() override {
        while (!Replacement->empty()) {
            delete Replacement->back();
            Replacement->pop_back();
        }
        delete Replacement;
    }

    std::vector<Token*> *getReplacements() { return Replacement; }
    unsigned int getNumReplacements() { return Replacement->size(); }
    Token *getReplacement(int i) { return Replacement->at(i); }
    void setReplacement(std::vector<Token*> *rep, int i) {
        Replacement->erase(Replacement->begin() + i);
        Replacement->insert(Replacement->begin() + i, rep->begin(), rep->end());
    }
    std::string toString() override {
        std::string str;
        for (const auto& tok : *Replacement)
            str += " " + tok->toString();
        return "define " + Id + ":" + str;
    }
};
class FunctionNode : public IncludeNode {
protected:
    StatementNode *Statement;
public:
    FunctionNode(std::string id, StatementNode *statement, Location l) :
        IncludeNode(std::move(id), l), Statement(statement) { Type = NodeType::Function; }
    ~FunctionNode() override { delete Statement; }
    std::string toString() override { return Id + " {\n" + Statement->toString() + "\n}"; }
};

class NodeOps {
public:
    static UnaryOperatorNode *CurrentValLookup(Location l) {
        return new UnaryOperatorNode(Operator::ptr_lookupRelUp, new NumberNode(0, l), l);
    }
};

#endif //BRAINPLUS_ASTNODES_H

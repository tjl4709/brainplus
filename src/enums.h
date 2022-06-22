//
// Created by 7budd on 6/22/2022.
//

#ifndef BRAINPLUS_ENUMS_H
#define BRAINPLUS_ENUMS_H


enum TokenType {
    t_eof       =  -1,

    //keywords
    t_include   =  -2,
    t_define    =  -3,
    t_if        =  -4,
    t_else      =  -5,
    t_for       =  -6,
    t_while     =  -7,
    t_do        =  -8,

    //primary
    t_number    =  -9,
    t_identifier= -10,
    t_string    = -11,
    t_op        = -12
};
std::string TypeToString(TokenType t) {
    switch(t) {
        case t_eof:     return "EOF";
        case t_include: return "include";
        case t_define:  return "define";
        case t_if:      return "if";
        case t_else:    return "else";
        case t_for:     return "for";
        case t_while:   return "while";
        case t_do:      return "do";
        case t_number:  return "number";
        case t_identifier:return "identifier";
        case t_string:  return "string";
        default: return {1, (char)t};
    }
}

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
    null = NULL,
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

    //value bitwise operators
    bit_not,            // !
    bit_and,            // &
    bit_or,             // |
    bit_xor,            // ^
    //ptr bitwise operators
    ptr_not,            // @!
    ptr_and,            // @&
    ptr_or,             // @|
    ptr_xor,            // @^
    //boolean operators
    bool_not,           // !!
    bool_and,           // &&
    bool_or,            // ||
    bool_xor            // ^^
};
std::string OpToStr(Operator op) {
    switch (op) {
        default:                return "NULL";
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
        case bit_not:           return "!";
        case bit_and:           return "&";
        case bit_or:            return "|";
        case bit_xor:           return "^";
        case ptr_not:           return "@!";
        case ptr_and:           return "@&";
        case ptr_or:            return "@|";
        case ptr_xor:           return "@^";
        case bool_not:          return "!!";
        case bool_and:          return "&&";
        case bool_or:           return "||";
        case bool_xor:          return "^^";
    }
}


#endif //BRAINPLUS_ENUMS_H

#pragma once

#include <bits/stdc++.h>
using namespace std;

enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    STRING,
    OPERATOR,
    COMMA,
    LPAREN,
    RPAREN,
    END_OF_FILE,
    UNKNOWN,
    STAR,
    DOT
};

struct Token {
    TokenType type;
    string_view lexeme;

    friend ostream& operator<<(ostream& os, const Token& token) {
        os << "Token{type: "
           << static_cast<int>(token.type)
           << ", lexeme: \""
           << token.lexeme
           << "\"}";
        return os;
    }
};
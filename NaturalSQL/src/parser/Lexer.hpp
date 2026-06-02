#pragma once

#include "Token.hpp"
#include <bits/stdc++.h>
using namespace std;

class Lexer {
public:
    explicit Lexer(string_view source);

    vector<Token> tokenize();

private:
    string_view source_;
    size_t cursor_{0};

    char peek() const;
    char advance();
    bool is_at_end() const;

    void skip_whitespace();

    Token handle_identifier_or_keyword();
    Token handle_number();
    Token handle_string();
};
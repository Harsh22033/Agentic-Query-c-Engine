#pragma once
#include "Token.hpp"
#include "AST.hpp"
#include <bits/stdc++.h>

using namespace std;

class Parser {
public:
    explicit Parser(const vector<Token>& tokens);
    unique_ptr<SelectStatement> parse();

private:
    const vector<Token>& tokens_;
    size_t current_{0};

    const Token& peek() const;
    const Token& previous() const;
    bool is_at_end() const;
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type) const;
    void consume(TokenType type, const string& message);

    unique_ptr<Expr> parse_expression();
    unique_ptr<Expr> parse_primary();
};
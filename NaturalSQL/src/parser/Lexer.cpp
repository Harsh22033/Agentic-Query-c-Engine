#include "Lexer.hpp"
#include <bits/stdc++.h>

using namespace std;

Lexer::Lexer(string_view source) : source_(source) {}

char Lexer::peek() const {
    if (is_at_end()) return '\0';
    return source_[cursor_];
}

char Lexer::advance() {
    return source_[cursor_++];
}

bool Lexer::is_at_end() const {
    return cursor_ >= source_.length();
}

void Lexer::skip_whitespace() {
    while (!is_at_end() && isspace(peek())) {
        advance();
    }
}

vector<Token> Lexer::tokenize() {
    vector<Token> tokens;

    while (!is_at_end()) {
        skip_whitespace();
        if (is_at_end()) break;

        size_t start = cursor_;
        char c = advance();

        // Single-character punctuations
        if (c == ',') {
            tokens.push_back({TokenType::COMMA, source_.substr(start, 1)});
            continue;
        }
        if (c == '(') {
            tokens.push_back({TokenType::LPAREN, source_.substr(start, 1)});
            continue;
        }
        if (c == ')') {
            tokens.push_back({TokenType::RPAREN, source_.substr(start, 1)});
            continue;
        }

        if (c == '*') {
            tokens.push_back({TokenType::STAR, source_.substr(start, 1)});
            continue;
        }
        if (c == '.') {
            tokens.push_back(
            {TokenType::DOT,
            source_.substr(start, 1)}
            );
            continue;
        }

        // Operators
        if (c == '=' || c == '<' || c == '>') {
            // Support simple two-character operators like <= or >=
            if (c == '<' && peek() == '=') { advance(); }
            if (c == '>' && peek() == '=') { advance(); }
            tokens.push_back({TokenType::OPERATOR, source_.substr(start, cursor_ - start)});
            continue;
        }

        // Strings literals (Single quotes)
        if (c == '\'') {
            tokens.push_back(handle_string());
            continue;
        }

        // Numbers
        if (isdigit(c)) {
            tokens.push_back(handle_number());
            continue;
        }

        // Identifiers or Keywords
        if (isalpha(c) || c == '_') {
            tokens.push_back(handle_identifier_or_keyword());
            continue;
        }

        // Handle fallback error token
        tokens.push_back({TokenType::UNKNOWN, source_.substr(start, 1)});
    }

    tokens.push_back({TokenType::END_OF_FILE, ""});
    return tokens;
}

Token Lexer::handle_identifier_or_keyword() {
    size_t start = cursor_ - 1;
    while (!is_at_end() && (isalnum(peek()) || peek() == '_')) {
        advance();
    }
    string_view lexeme = source_.substr(start, cursor_ - start);

    // Convert to uppercase for standardizing SQL keyword checking
    string upper;
    upper.reserve(lexeme.size());
    for (char ch : lexeme) upper += static_cast<char>(toupper(ch));

    if (upper == "SELECT" || upper == "FROM" || upper == "WHERE" || 
        upper == "ORDER" || upper == "BY" || upper == "LIMIT" ||
        upper == "ASC" || upper == "DESC" || upper == "COUNT" ||
        upper == "JOIN" || upper == "ON"){
        return {TokenType::KEYWORD, lexeme};
    }

    return {TokenType::IDENTIFIER, lexeme};
}

Token Lexer::handle_number() {
    size_t start = cursor_ - 1;
    while (!is_at_end() && isdigit(peek())) {
        advance();
    }
    return {TokenType::NUMBER, source_.substr(start, cursor_ - start)};
}

Token Lexer::handle_string() {
    size_t start = cursor_; // Exclude opening quote
    while (!is_at_end() && peek() != '\'') {
        advance();
    }
    string_view lexeme = source_.substr(start, cursor_ - start);
    if (!is_at_end()) advance(); // Consume closing quote
    return {TokenType::STRING, lexeme};
}
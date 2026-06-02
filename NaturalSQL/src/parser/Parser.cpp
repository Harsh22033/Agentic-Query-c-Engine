#include "Parser.hpp"
using namespace std;

Parser::Parser(const vector<Token>& tokens) : tokens_(tokens) {}

const Token& Parser::peek() const { return tokens_[current_]; }
const Token& Parser::previous() const { return tokens_[current_ - 1]; }
bool Parser::is_at_end() const { return peek().type == TokenType::END_OF_FILE; }

Token Parser::advance() {
    if (!is_at_end()) current_++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

void Parser::consume(TokenType type, const string& message) {
    if (check(type)) {
        advance();
        return;
    }
    throw runtime_error(message + " Found: " + string(peek().lexeme));
}

// Parses: SELECT ... FROM ... WHERE ... LIMIT ...
unique_ptr<SelectStatement> Parser::parse() {
    auto stmt = make_unique<SelectStatement>();

    // 1. SELECT
    consume(TokenType::KEYWORD, "Expected 'SELECT'");
    
    // Parse columns
    do {
        if (match(TokenType::OPERATOR) && previous().lexeme == "*") {
            stmt->columns.push_back("*");
        } else {
            consume(TokenType::IDENTIFIER, "Expected column name");
            stmt->columns.push_back(string(previous().lexeme));
        }
    } while (match(TokenType::COMMA));

    // 2. FROM
    consume(TokenType::KEYWORD, "Expected 'FROM'");
    consume(TokenType::IDENTIFIER, "Expected table name");
    stmt->table_name = string(previous().lexeme);

    // 3. WHERE
    if (match(TokenType::KEYWORD)) {
        // Simple case-insensitive check (assuming lexer handled it, but being safe)
        string kw = string(previous().lexeme);
        if (kw == "WHERE" || kw == "where") {
            stmt->where_clause = parse_expression();
        }
    }

    // 4. Order By

    if (!is_at_end() &&
        check(TokenType::KEYWORD) &&
        string(peek().lexeme) == "ORDER"){

        advance();

        consume(
            TokenType::KEYWORD,
            "Expected BY after ORDER"
        );

        consume(
            TokenType::IDENTIFIER,
            "Expected column after ORDER BY"
        );

        stmt->order_by_column =
            string(previous().lexeme);

        if (!is_at_end() &&
            check(TokenType::KEYWORD))
        {
            string dir =
                string(peek().lexeme);

            if (dir == "DESC") {
                stmt->order_desc = true;
                advance();
            }
            else if (dir == "ASC") {
                advance();
            }
        }
    }

    // 5. LIMIT 
    if (match(TokenType::KEYWORD)) {
        string kw = string(previous().lexeme);
        if (kw == "LIMIT" || kw == "limit") {
            consume(TokenType::NUMBER, "Expected number after LIMIT");
            stmt->limit = stoi(string(previous().lexeme));
        }
    }

    return stmt;
}

// Very basic expression parsing (Left OP Right)
unique_ptr<Expr> Parser::parse_expression() {
    auto left = parse_primary();

    if (match(TokenType::OPERATOR)) {
        string op = string(previous().lexeme);
        auto right = parse_primary();
        return make_unique<BinaryExpr>(move(left), op, move(right));
    }

    return left;
}

unique_ptr<Expr> Parser::parse_primary() {
    if (match(TokenType::NUMBER)) {
        return make_unique<LiteralExpr>(stoi(string(previous().lexeme)));
    }
    if (match(TokenType::STRING)) {
        return make_unique<LiteralExpr>(string(previous().lexeme));
    }
    if (match(TokenType::IDENTIFIER)) {
        return make_unique<IdentifierExpr>(string(previous().lexeme));
    }
    throw runtime_error("Expected expression.");
}
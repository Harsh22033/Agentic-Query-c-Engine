#pragma once
#include <bits/stdc++.h>

using namespace std;

// A SQL Value can be an integer, a double, or a string
using SqlValue = variant<int, double, string>;

// Base Expression Node
struct Expr {
    virtual ~Expr() = default;
};

// Represents a literal value (e.g., 5, 'active')
struct LiteralExpr : public Expr {
    SqlValue value;
    explicit LiteralExpr(SqlValue val) : value(move(val)) {}
};

// Represents a column name (e.g., cell_id, voltage)
struct IdentifierExpr : public Expr {
    string name;
    explicit IdentifierExpr(string n) : name(move(n)) {}
};

// Represents a binary operation (e.g., voltage > 1)
struct BinaryExpr : public Expr {
    unique_ptr<Expr> left;
    string op;
    unique_ptr<Expr> right;
    
    BinaryExpr(unique_ptr<Expr> l, string o, unique_ptr<Expr> r)
        : left(move(l)), op(move(o)), right(move(r)) {}
};

// Represents the entire SELECT query
struct SelectStatement {
    vector<string> columns; // "*" or ["cell_id", "voltage"]
    string table_name;
    unique_ptr<Expr> where_clause; // Can be null if no WHERE
    int limit = -1; // -1 means no limit
    string order_by_column;
    bool order_desc = false; // default to ASC
};
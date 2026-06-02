#pragma once
#include "../parser/AST.hpp"
#include "../storage/Table.hpp"
#include <bits/stdc++.h>

using namespace std;

class Executor {
public:
    // Main entry point to run the query and print results
    static void execute(const unique_ptr<SelectStatement>& ast, const Table& table);

private:
    // Evaluates a generic expression to a concrete value for a specific row
    static SqlValue evaluate_expr(const Expr* expr, const Table& table, size_t row_idx);
    
    // Checks if a specific row passes the WHERE condition
    static bool evaluate_where(const Expr* expr, const Table& table, size_t row_idx);
};
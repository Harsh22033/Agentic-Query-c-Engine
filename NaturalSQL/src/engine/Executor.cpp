#include "Executor.hpp"
#include <bits/stdc++.h>

using namespace std;

void Executor::execute(const unique_ptr<SelectStatement>& ast, const Table& table) {
    if (ast->table_name != table.name) {
        throw runtime_error("Table name mismatch: " + ast->table_name);
    }

    // Resolve which columns we actually need to print
    vector<string> target_columns;
    if (ast->columns.size() == 1 && ast->columns[0] == "*") {
        target_columns = table.column_names;
    } else {
        target_columns = ast->columns;
    }

    // Print Header
    for (const auto& col : target_columns) {
        cout << left << setw(15) << col;
    }
    cout << "\n";
    for (size_t i = 0; i < target_columns.size(); ++i) cout << "---------------";
    cout << "\n";

    // Execution Loop: Scan rows and apply WHERE & LIMIT
    int limit = ast->limit;
    int rows_yielded = 0;

    for (size_t row_idx = 0; row_idx < table.row_count; ++row_idx) {
        // Stop if we hit the limit
        if (limit != -1 && rows_yielded >= limit) {
            break;
        }

        // Check WHERE clause
        bool passes_where = true;
        if (ast->where_clause) {
            passes_where = evaluate_where(ast->where_clause.get(), table, row_idx);
        }

        // If it passes, print the selected columns
        if (passes_where) {
            for (const auto& col : target_columns) {
                const auto& value = table.columns.at(col)[row_idx];
                
                // std::visit is used to extract the value from our std::variant
                visit([](auto&& arg) { cout << left << setw(15) << arg; }, value);
            }
            cout << "\n";
            rows_yielded++;
        }
    }
    cout << "(" << rows_yielded << " rows returned)\n";
}

SqlValue Executor::evaluate_expr(const Expr* expr, const Table& table, size_t row_idx) {
    // If it's a literal value (e.g., 20), just return it
    if (auto lit = dynamic_cast<const LiteralExpr*>(expr)) {
        return lit->value;
    }
    // If it's a column name (e.g., "age"), fetch the value from the column-store for this row
    if (auto id = dynamic_cast<const IdentifierExpr*>(expr)) {
        return table.columns.at(id->name)[row_idx];
    }
    throw runtime_error("Unsupported expression type in evaluator.");
}

bool Executor::evaluate_where(const Expr* expr, const Table& table, size_t row_idx) {
    auto bin_expr = dynamic_cast<const BinaryExpr*>(expr);
    if (!bin_expr) {
        throw runtime_error("WHERE clause must be a binary expression (for now).");
    }

    SqlValue left_val = evaluate_expr(bin_expr->left.get(), table, row_idx);
    SqlValue right_val = evaluate_expr(bin_expr->right.get(), table, row_idx);

    // Simplistic integer comparison for Week 1 (e.g. age > 20)
    if (holds_alternative<int>(left_val) && holds_alternative<int>(right_val)) {
        int l = get<int>(left_val);
        int r = get<int>(right_val);

        if (bin_expr->op == ">") return l > r;
        if (bin_expr->op == "<") return l < r;
        if (bin_expr->op == "=" || bin_expr->op == "==") return l == r;
    }
    
    // Simplistic string comparison
    if (holds_alternative<string>(left_val) && holds_alternative<string>(right_val)) {
        string l = get<string>(left_val);
        string r = get<string>(right_val);
        
        if (bin_expr->op == "=" || bin_expr->op == "==") return l == r;
    }

    return false;
}
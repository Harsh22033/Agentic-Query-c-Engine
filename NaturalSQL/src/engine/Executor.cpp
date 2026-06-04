#include "Executor.hpp"
#include <bits/stdc++.h>

using namespace std;

void Executor::execute(const unique_ptr<SelectStatement>& ast, const Database& db) {
    if (db.find(ast->table_name) == db.end()) {
        throw runtime_error("Table not found: " + ast->table_name);
    }

    const Table& table = db.at(ast->table_name);

    // 1. Resolve which columns we actually need to print
    vector<string> target_columns;
    if (ast->columns.size() == 1 && ast->columns[0] == "*") {
        target_columns = table.column_names;
    } else {
        target_columns = ast->columns;
    }

    // 2. Filter Rows (WHERE clause)
    vector<size_t> row_ids;
    for (size_t row_idx = 0; row_idx < table.row_count; ++row_idx) {
        bool passes_where = true;

        if (ast->where_clause) {
            passes_where = evaluate_where(ast->where_clause.get(), table, row_idx);
        }

        if (passes_where) {
            row_ids.push_back(row_idx);
        }
    }

    // COUNT(*) fast path

    if (ast->is_count_star)
    {
        cout << left
            << setw(15)
            << "COUNT(*)"
            << "\n";

        cout << "---------------\n";

        cout << left
            << setw(15)
            << row_ids.size()
            << "\n";

        return;
    }

    // 3. Sort Rows (ORDER BY clause)
    if (!ast->order_by_column.empty()) {
        const auto& sort_col = table.columns.at(ast->order_by_column);
        
        sort(row_ids.begin(), row_ids.end(), [&](size_t a, size_t b) {
            if (ast->order_desc) {
                return sort_col[a] > sort_col[b]; 
            } else {
                return sort_col[a] < sort_col[b];
            }
        });
    }

    // 4. Limit Rows (LIMIT clause) - Must happen AFTER sorting
    if (ast->limit > 0 && ast->limit < row_ids.size()) {
        row_ids.resize(ast->limit);
    }

    // 5. Print Header
    for (const auto& col : target_columns) {
        cout << left << setw(15) << col;
    }
    cout << "\n";
    for (size_t i = 0; i < target_columns.size(); ++i) cout << "---------------";
    cout << "\n";

    // 6. Print the final Result Set
    for (size_t row_idx : row_ids) {
        for (const auto& col_name : target_columns) {
            const auto& val = table.columns.at(col_name)[row_idx];
            
            if (holds_alternative<int>(val)) {
                cout << left << setw(15) << get<int>(val);
            } else if (holds_alternative<string>(val)) {
                cout << left << setw(15) << get<string>(val);
            }
        }
        cout << "\n";
    }

    // 7. Print the total row count
    cout << "(" << row_ids.size() << " rows returned)\n";
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
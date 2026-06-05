#include "Executor.hpp"
#include <bits/stdc++.h>

using namespace std;

void Executor::execute(const unique_ptr<SelectStatement>& ast, const Database& db) {
    if (db.find(ast->table_name) == db.end()) {
        throw runtime_error("Table not found: " + ast->table_name);
    }

    const Table& table = db.at(ast->table_name);

    if (ast->join_clause) {
        execute_join(ast, db);
        return;
    }

    // 1. Resolve which columns we actually need to print
    vector<string> target_columns;
    if (ast->columns.size() == 1 && ast->columns[0] == "*") {
        target_columns = table.column_names;
    } else {
        target_columns = ast->columns;
    }

    // 2. Filter Rows (WHERE clause) - with INDEX OPTIMIZATION
    vector<size_t> row_ids;
    
    // Try to use index for WHERE clause optimization
    bool used_index = false;
    if (ast->where_clause) {
        auto bin_expr = dynamic_cast<const BinaryExpr*>(ast->where_clause.get());
        if (bin_expr && bin_expr->op == "=") {
            // Check if this is a "column = value" pattern we can optimize
            auto left_id = dynamic_cast<const IdentifierExpr*>(bin_expr->left.get());
            auto right_lit = dynamic_cast<const LiteralExpr*>(bin_expr->right.get());
            
            if (left_id && right_lit && holds_alternative<int>(right_lit->value)) {
                string column_name = left_id->name;
                
                // Check if we have an index for this column
                if (table.indexes.find(column_name) != table.indexes.end()) {
                    int search_value = get<int>(right_lit->value);
                    row_ids = table.indexes.at(column_name).find(search_value);
                    used_index = true;
                    
                    cout << "🚀 INDEX OPTIMIZATION: Used index on '" << column_name 
                         << "' for value " << search_value << " (found " << row_ids.size() << " rows)\n";
                }
            }
        }
    }
    
    // Fallback to full table scan if no index was used
    if (!used_index) {
        if (ast->where_clause) {
            cout << "⚠️  FULL TABLE SCAN: No suitable index found, scanning all " << table.row_count << " rows\n";
        }
        
        for (size_t row_idx = 0; row_idx < table.row_count; ++row_idx) {
            bool passes_where = true;

            if (ast->where_clause) {
                passes_where = evaluate_where(ast->where_clause.get(), table, row_idx);
            }

            if (passes_where) {
                row_ids.push_back(row_idx);
            }
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
void Executor::execute_join(
    const unique_ptr<SelectStatement>& ast,
    const Database& db) {
    
    const Table& left_table = db.at(ast->table_name);
    const Table& right_table = db.at(ast->join_clause->table_name);

    // Extract join column names (users.dept_id -> dept_id)
    string left_col = ast->join_clause->left_col.substr(
        ast->join_clause->left_col.find('.') + 1
    );
    string right_col = ast->join_clause->right_col.substr(
        ast->join_clause->right_col.find('.') + 1
    );

    // 1. Resolve target columns dynamically
    vector<pair<string, string>> target_columns; // (table_name, column_name)
    
    for (const string& col : ast->columns) {
        if (col == "*") {
            // Add all columns from both tables with table prefix
            for (const string& left_col_name : left_table.column_names) {
                target_columns.push_back({ast->table_name, left_col_name});
            }
            for (const string& right_col_name : right_table.column_names) {
                target_columns.push_back({ast->join_clause->table_name, right_col_name});
            }
        } else if (col.find('.') != string::npos) {
            // Qualified column name (e.g., users.name)
            string table_name = col.substr(0, col.find('.'));
            string column_name = col.substr(col.find('.') + 1);
            target_columns.push_back({table_name, column_name});
        } else {
            // Unqualified column name - search in both tables
            if (find(left_table.column_names.begin(), left_table.column_names.end(), col) != left_table.column_names.end()) {
                target_columns.push_back({ast->table_name, col});
            } else if (find(right_table.column_names.begin(), right_table.column_names.end(), col) != right_table.column_names.end()) {
                target_columns.push_back({ast->join_clause->table_name, col});
            } else {
                throw runtime_error("Column not found: " + col);
            }
        }
    }

    //--------------------------------------------------
    // BUILD PHASE - Create hash index on right table
    //--------------------------------------------------
    unordered_map<int, size_t> hash_index;
    const auto& right_column = right_table.columns.at(right_col);

    for (size_t row = 0; row < right_table.row_count; ++row) {
        if (holds_alternative<int>(right_column[row])) {
            hash_index[get<int>(right_column[row])] = row;
        }
    }

    //--------------------------------------------------
    // PROBE PHASE - Print header and results
    //--------------------------------------------------
    const auto& left_column = left_table.columns.at(left_col);

    // Print dynamic header
    for (const auto& [table_name, column_name] : target_columns) {
        cout << left << setw(15) << (table_name + "." + column_name);
    }
    cout << "\n";
    
    // Print separator
    for (size_t i = 0; i < target_columns.size(); ++i) {
        cout << "---------------";
    }
    cout << "\n";

    size_t rows_returned = 0;

    for (size_t left_row = 0; left_row < left_table.row_count; ++left_row) {
        if (!holds_alternative<int>(left_column[left_row])) {
            continue;
        }

        int key = get<int>(left_column[left_row]);
        auto it = hash_index.find(key);

        if (it == hash_index.end()) {
            continue;
        }

        size_t right_row = it->second;

        // Print each target column value
        for (const auto& [table_name, column_name] : target_columns) {
            if (table_name == ast->table_name) {
                // Value from left table
                const auto& val = left_table.columns.at(column_name)[left_row];
                if (holds_alternative<int>(val)) {
                    cout << left << setw(15) << get<int>(val);
                } else if (holds_alternative<string>(val)) {
                    cout << left << setw(15) << get<string>(val);
                }
            } else {
                // Value from right table
                const auto& val = right_table.columns.at(column_name)[right_row];
                if (holds_alternative<int>(val)) {
                    cout << left << setw(15) << get<int>(val);
                } else if (holds_alternative<string>(val)) {
                    cout << left << setw(15) << get<string>(val);
                }
            }
        }
        cout << "\n";
        rows_returned++;
    }

    cout << "(" << rows_returned << " rows returned)\n";
}
#pragma once
#include "../parser/AST.hpp"
#include "../index/TreeIndex.hpp"
#include <bits/stdc++.h>
using namespace std;

class Table {
public:
    string name;
    vector<string> column_names;
    
    // The core of the Column-Store: 
    // column_name -> flat vector of values
    unordered_map<string, vector<SqlValue>> columns;
    size_t row_count{0};

    // Index support
    unordered_map<string, TreeIndex> indexes;

    explicit Table(string table_name) : name(move(table_name)) {}

    void add_column(const string& col_name) {
        column_names.push_back(col_name);
        columns[col_name] = vector<SqlValue>();
    }

    // Takes a parsed row from CSV and transposes it into the column vectors
    void append_row(const vector<SqlValue>& row_values) {
        if (row_values.size() != column_names.size()) {
            throw runtime_error("Row size mismatch during insert.");
        }
        for (size_t i = 0; i < row_values.size(); ++i) {
            columns[column_names[i]].push_back(row_values[i]);
        }
        row_count++;
    }

    void print_schema() const {
        cout << "Table: " << name << " (" << row_count << " rows)\nColumns: ";
        for (const auto& col : column_names) cout << col << " | ";
        cout << "\n";
    }

    void build_index(const string& column_name) {
        TreeIndex index;
        
        const auto& column = columns.at(column_name);
        
        for (size_t row = 0; row < row_count; ++row) {
            if (holds_alternative<int>(column[row])) {
                index.insert(get<int>(column[row]), row);
            }
        }
        
        indexes[column_name] = move(index);
    }
};
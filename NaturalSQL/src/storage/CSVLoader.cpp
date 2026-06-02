#include "CSVLoader.hpp"
#include <bits/stdc++.h>

using namespace std;

// Helper to determine if a string is a pure integer
bool is_integer(const string& s) {
    if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;
    char* p;
    strtol(s.c_str(), &p, 10);
    return (*p == 0);
}

Table CSVLoader::load(const string& table_name, const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        throw runtime_error("Failed to open file: " + filepath);
    }

    Table table(table_name);
    string line;

    // 1. Read Headers
    if (getline(file, line)) {
        stringstream ss(line);
        string col_name;
        while (getline(ss, col_name, ',')) {
            // Trim whitespace
            col_name.erase(0, col_name.find_first_not_of(" \r\n\t"));
            col_name.erase(col_name.find_last_not_of(" \r\n\t") + 1);
            table.add_column(col_name);
        }
    }

    // 2. Read Data Rows
    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string cell;
        vector<SqlValue> row_values;

        while (getline(ss, cell, ',')) {
            // Trim whitespace
            cell.erase(0, cell.find_first_not_of(" \r\n\t"));
            cell.erase(cell.find_last_not_of(" \r\n\t") + 1);

            // Infer type: if it's an int, store as int. Else, store as string.
            if (is_integer(cell)) {
                row_values.push_back(stoi(cell));
            } else {
                row_values.push_back(cell);
            }
        }
        
        // Pad row if it has missing trailing commas
        while (row_values.size() < table.column_names.size()) {
            row_values.push_back(string("")); 
        }

        table.append_row(row_values);
    }

    return table;
}
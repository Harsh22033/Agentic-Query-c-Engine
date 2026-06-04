#include "parser/Lexer.hpp"
#include "parser/Parser.hpp"
#include "storage/CSVLoader.hpp"
#include "storage/Database.hpp"
#include "engine/Executor.hpp"

#include <bits/stdc++.h>

using namespace std;

int main() {
    cout << "--- NaturalSQL Engine Starting ---\n";
    
    try {
        cout << "\nLoading data...\n";
        
        // Load both tables
        Table users_table = CSVLoader::load("users", "data/users.csv");
        Table departments_table = CSVLoader::load("departments", "data/departments.csv");
        
        users_table.print_schema();
        cout << "\n";

        // Create Database
        Database db;
        db.emplace("users", std::move(users_table));
        db.emplace("departments", std::move(departments_table));

        string_view query = "SELECT COUNT(*) FROM users WHERE age > 20";

        Lexer lexer(query);
        auto tokens = lexer.tokenize();
        cout << "Tokenization Complete\n";

        Parser parser(tokens);
        auto ast = parser.parse();
        
        cout << "Parsing Complete.\n\n";

        // Execute the query
        Executor executor;
        executor.execute(ast, db);

        }
        catch (const exception& e) {
            cerr << "\nError: " << e.what() << "\n";
            return 1;
        }

        return 0;
    }
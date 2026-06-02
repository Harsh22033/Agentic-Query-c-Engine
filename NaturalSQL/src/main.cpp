#include "parser/Lexer.hpp"
#include "parser/Parser.hpp"
#include "storage/CSVLoader.hpp"
#include "engine/Executor.hpp" // <-- 1. Add the Executor include

#include <bits/stdc++.h>

using namespace std;

int main() {
    cout << "--- NaturalSQL Engine Starting ---\n";

    try {
        cout << "\nLoading data...\n";
        
        // Note: keeping your "data/users.csv" path
        Table users_table = CSVLoader::load("users", "data/users.csv");
        users_table.print_schema();
        cout << "\n";

        string_view query = "SELECT name, age FROM users WHERE age > 20 LIMIT 5";
        cout << "Query: " << query << "\n\n";

        Lexer lexer(query);
        auto tokens = lexer.tokenize();
        cout << "Tokenization Complete\n";

        Parser parser(tokens);
        auto ast = parser.parse();
        cout << "Parsing Complete. Executing...\n\n";

        // <-- 2. The grand finale: Execute the query!
        Executor::execute(ast, users_table);

    }
    catch (const exception& e) {
        cerr << "\nError: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
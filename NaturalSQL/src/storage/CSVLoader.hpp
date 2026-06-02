#pragma once
#include "Table.hpp"
#include <bits/stdc++.h>

using namespace std;

class CSVLoader {
public:
    // Factory method that creates and populates a Table from a file
    static Table load(const string& table_name, const string& filepath);
};
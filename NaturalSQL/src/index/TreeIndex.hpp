#pragma once

#include "Index.hpp"
#include <map>

using namespace std;

class TreeIndex : public Index {
private:
    map<int, vector<size_t>> index_;

public:
    void insert(int key, size_t row_id);
    
    vector<size_t> find(int key) const override;
};
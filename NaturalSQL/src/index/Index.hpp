#pragma once

#include <vector>

using namespace std;

class Index {
public:
    virtual ~Index() = default;
    
    virtual vector<size_t> find(int key) const = 0;
};
#include "TreeIndex.hpp"

void TreeIndex::insert(int key, size_t row_id) {
    index_[key].push_back(row_id);
}

vector<size_t> TreeIndex::find(int key) const {
    auto it = index_.find(key);
    
    if (it == index_.end()) {
        return {};
    }
    
    return it->second;
}
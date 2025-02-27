//
// Created by Alberto Santini on 06/06/2024.
//

#include "ColumnPool.h"
#include <algorithm>

namespace kpgf {

    bool ColumnPool::is_duplicate(const Column& col) const {
        if(col.dummy) {
            return false;
        }

        return std::any_of(columns.begin(), columns.end(), [&](const Column& existing) -> bool {
            if(existing.dummy) {
                return false;
            }

            if(existing.items_class != col.items_class) {
                return false;
            }

            return existing.items == col.items;
        });
    }

}// namespace kpgf
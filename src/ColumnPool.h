//
// Created by Alberto Santini on 06/06/2024.
//

#ifndef KPGF_COLUMNPOOL_H
#define KPGF_COLUMNPOOL_H

#include "Column.h"
#include <vector>

namespace kpgf {
    struct ColumnPool {
        std::vector<Column> columns;

        ColumnPool() = default;

        bool is_duplicate(const Column& col) const;
    };
}// namespace kpgf

#endif//KPGF_COLUMNPOOL_H

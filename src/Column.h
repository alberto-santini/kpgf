//
// Created by Alberto Santini on 05/06/2024.
//

#ifndef KPGF_COLUMN_H
#define KPGF_COLUMN_H

#include "BranchingRule.h"
#include "Instance.h"
#include <vector>

namespace kpgf {
    struct Column {
        bool dummy;
        double profit;
        double weight;
        std::size_t items_class;
        std::vector<size_t> items;

        Column(const Instance& i, std::vector<size_t> items);
        Column() = default;

        double cover_coefficient(std::size_t k) const;
        bool is_compatible_with(const BranchingRule& br) const;

        static Column make_dummy();
    };
}// namespace kpgf

#endif//KPGF_COLUMN_H

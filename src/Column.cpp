//
// Created by Alberto Santini on 05/06/2024.
//

#include "Column.h"
#include <cassert>
#include <exception>

namespace kpgf {
    Column::Column(const Instance& i, std::vector<size_t> items) : dummy{false} {
        if(items.empty()) {
            throw std::logic_error("Cannot create a column from an empty list of items");
        }

        profit = 0.0;
        weight = 0.0;
        items_class = i.class_of(items[0u]);

        for(const auto j : items) {
            assert(i.class_of(j) == items_class);
            profit += (double) i.profit[j];
            weight += (double) i.weight[j];
        }

        this->items = std::move(items);
    }

    double Column::cover_coefficient(std::size_t k) const {
        return (dummy or k == items_class) ? 1.0 : 0.0;
    }

    bool Column::is_compatible_with(const BranchingRule& br) const {
        if(dummy) {
            return true;
        }

        if(items_class != br.item_class) {
            return true;
        }

        const auto contains_j = std::find(items.begin(), items.end(), br.item) != items.end();

        if(contains_j and br.status == ItemBranchingStatus::FORCE_PACK) {
            return true;
        }

        if((not contains_j) and br.status == ItemBranchingStatus::FORCE_NO_PACK) {
            return true;
        }

        return false;
    }

    Column Column::make_dummy() {
        Column c;

        c.dummy = true;
        c.profit = -9999.0;
        c.weight = 0.0;
        c.items_class = 0u;
        c.items = {};

        return c;
    }
}// namespace kpgf
//
// Created by alberto on 02/12/24.
//

#ifndef CLASSCARDINALITYUBSOLUTION_H
#define CLASSCARDINALITYUBSOLUTION_H

#include "Column.h"
#include "Instance.h"
#include <cstddef>
#include <vector>

namespace kpgf {

    struct ClassCardinalityUbSolution {
        const Instance& i;

        bool feasible;
        bool primal_available;
        size_t card_lb;
        size_t card_ub;

        std::vector<std::vector<size_t>> feasible_packings;

        explicit ClassCardinalityUbSolution(const Instance& i) : i{i} {
        }

        std::vector<Column> to_columns() const;

        static ClassCardinalityUbSolution for_infeasible(const Instance& i);
        static ClassCardinalityUbSolution for_ub_only(const Instance& i, double card_ub);
        static ClassCardinalityUbSolution for_feasible(
            const Instance& i, double card_lb, double card_ub, std::vector<std::vector<size_t>> feasible_packings
        );
    };

}// namespace kpgf

#endif//CLASSCARDINALITYUBSOLUTION_H

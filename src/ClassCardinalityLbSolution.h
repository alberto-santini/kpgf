//
// Created by alberto on 02/12/24.
//

#ifndef CLASSCARDINALITYLBSOLUTION_H
#define CLASSCARDINALITYLBSOLUTION_H

#include "Column.h"
#include "Instance.h"
#include <vector>

namespace kpgf {

    struct ClassCardinalityLbSolution {
        const Instance& i;

        bool feasible;
        bool primal_available;
        size_t card_lb;
        size_t card_ub;

        std::vector<std::vector<size_t>> feasible_packings;

        explicit ClassCardinalityLbSolution(const Instance& i) : i{i} {
        }

        std::vector<Column> to_columns() const;

        static ClassCardinalityLbSolution for_infeasible(const Instance& i);
        static ClassCardinalityLbSolution for_lb_only(const Instance& i, double card_lb);
        static ClassCardinalityLbSolution for_feasible(
            const Instance& i, double card_lb, double card_ub, std::vector<std::vector<size_t>> feasible_packings
        );
    };

}// namespace kpgf

#endif//CLASSCARDINALITYLBSOLUTION_H

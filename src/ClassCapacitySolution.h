//
// Created by Alberto Santini on 05/06/2024.
//

#ifndef KPGF_MINKPMINFILLINGSOLUTION_H
#define KPGF_MINKPMINFILLINGSOLUTION_H

#include "Column.h"
#include "Instance.h"
#include <cstddef>
#include <vector>

namespace kpgf {
    struct ClassCapacitySolution {
        const Instance& i;

        bool feasible;
        bool primal_available;
        size_t weight_lb;
        size_t weight_ub;

        std::vector<std::vector<size_t>> feasible_packings;

        explicit ClassCapacitySolution(const Instance& i) : i{i} {
        }

        std::vector<Column> to_columns() const;

        static ClassCapacitySolution for_infeasible(const Instance& i);
        static ClassCapacitySolution for_lb_only(const Instance& i, double weight_lb);
        static ClassCapacitySolution for_feasible(
            const Instance& i, double weight_lb, double weight_ub, std::vector<std::vector<size_t>> feasible_packings
        );
    };
}// namespace kpgf

#endif//KPGF_MINKPMINFILLINGSOLUTION_H

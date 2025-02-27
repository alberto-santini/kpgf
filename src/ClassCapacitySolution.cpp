//
// Created by Alberto Santini on 05/06/2024.
//

#include "ClassCapacitySolution.h"
#include <cmath>

namespace kpgf {
    ClassCapacitySolution ClassCapacitySolution::for_infeasible(const Instance& i) {
        ClassCapacitySolution sol{i};
        sol.feasible = false;
        sol.primal_available = false;
        return sol;
    }

    ClassCapacitySolution ClassCapacitySolution::for_feasible(
        const Instance& i, double weight_lb, double weight_ub, std::vector<std::vector<size_t>> feasible_packings
    ) {
        ClassCapacitySolution sol{i};
        sol.feasible = true;
        sol.primal_available = true;
        sol.weight_lb = (std::size_t) std::ceil(weight_lb);
        sol.weight_ub = (std::size_t) std::floor(weight_ub);
        sol.feasible_packings = std::move(feasible_packings);
        return sol;
    }

    ClassCapacitySolution ClassCapacitySolution::for_lb_only(const Instance& i, double weight_lb) {
        ClassCapacitySolution sol{i};
        sol.feasible = true;
        sol.primal_available = false;
        sol.weight_lb = (std::size_t) std::ceil(weight_lb);
        return sol;
    }

    std::vector<Column> ClassCapacitySolution::to_columns() const {
        auto cols = std::vector<Column>{};
        cols.reserve(feasible_packings.size());

        for(const auto& packing : feasible_packings) {
            cols.emplace_back(i, packing);
        }

        return cols;
    }
}// namespace kpgf
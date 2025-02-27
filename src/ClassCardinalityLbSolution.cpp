//
// Created by alberto on 02/12/24.
//

#include "ClassCardinalityLbSolution.h"

namespace kpgf {
    ClassCardinalityLbSolution ClassCardinalityLbSolution::for_infeasible(const Instance& i) {
        ClassCardinalityLbSolution sol{i};
        sol.feasible = false;
        sol.primal_available = false;
        return sol;
    }

    ClassCardinalityLbSolution ClassCardinalityLbSolution::for_feasible(
        const Instance& i, double card_lb, double card_ub, std::vector<std::vector<size_t>> feasible_packings
    ) {
        ClassCardinalityLbSolution sol{i};
        sol.feasible = true;
        sol.primal_available = true;
        sol.card_lb = (std::size_t) std::ceil(card_lb);
        sol.card_ub = (std::size_t) std::floor(card_ub);
        sol.feasible_packings = std::move(feasible_packings);
        return sol;
    }

    ClassCardinalityLbSolution ClassCardinalityLbSolution::for_lb_only(const Instance& i, double card_lb) {
        ClassCardinalityLbSolution sol{i};
        sol.feasible = true;
        sol.primal_available = false;
        sol.card_lb = (std::size_t) std::ceil(card_lb);
        return sol;
    }

    std::vector<Column> ClassCardinalityLbSolution::to_columns() const {
        auto cols = std::vector<Column>{};
        cols.reserve(feasible_packings.size());

        for(const auto& packing : feasible_packings) {
            cols.emplace_back(i, packing);
        }

        return cols;
    }
}// namespace kpgf
//
// Created by Paolo on 16/09/2024.
//

#ifndef KPGF_MCKPSOLVER_H
#define KPGF_MCKPSOLVER_H

#include "Instance.h"
#include "MCKPSolverResults.h"
#include "Solution.h"
#include <cstddef>
#include <vector>

namespace kpgf {
    struct MaximalPacking {
        std::vector<std::size_t> items;
        std::size_t profit{0};
        std::size_t weight{0};
        MaximalPacking(const std::vector<std::size_t>& items_input, std::size_t profit_input, std::size_t weight_input)
            : items{items_input}, profit{profit_input}, weight{weight_input} {
        }
    };

    struct MCKPSolver {
        const Instance& i;
        std::vector<std::vector<MaximalPacking>> max_packings;
        explicit MCKPSolver(const Instance& i);
        void enumerate_packings(std::size_t k);
        MCKPSolverResults solve();

    private:
        Solution get_solution(const std::vector<std::pair<int, int>>& my_solution) const;
    };
}// namespace kpgf

#endif//KPGF_MCKPSOLVER_H

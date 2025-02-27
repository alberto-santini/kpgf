//
// Created by Alberto Santini on 08/06/2024.
//

#ifndef KPGF_SUBPROBLEMSOLUTION_H
#define KPGF_SUBPROBLEMSOLUTION_H

#include <cstddef>
#include <vector>

namespace kpgf {
    enum class SPFeasibleStatus { FEASIBLE, INFEASIBLE };

    struct SubProblemSolution {
        SPFeasibleStatus status;
        double time_elapsed;
        std::vector<std::vector<std::size_t>> packings;

        SubProblemSolution(SPFeasibleStatus status, double time_elapsed, std::vector<std::vector<std::size_t>> packings)
            : status{status}, time_elapsed{time_elapsed}, packings{std::move(packings)} {
        }
    };
}// namespace kpgf

#endif//KPGF_SUBPROBLEMSOLUTION_H

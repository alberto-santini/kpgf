//
// Created by Alberto Santini on 17/08/2024.
//

#ifndef KPGF_SUBPROBLEMSOLVER_H
#define KPGF_SUBPROBLEMSOLVER_H

#include <cstdlib>
#include <stdexcept>
#include <string>

#include "BranchingRule.h"
#include "ExtendedSolverSolution.h"
#include "Instance.h"
#include "SubProblemSolution.h"

namespace kpgf {
    enum class SubProblemSolverType { MIP, DP };

    inline std::string sps_str(const SubProblemSolverType& t) {
        if(t == SubProblemSolverType::MIP) {
            return "mip";
        } else if(t == SubProblemSolverType::DP) {
            return "dp";
        }

        throw std::logic_error("No known conversion to string for the given exploration strategy");
    }

    struct SubProblemSolver {
        const Instance& i;
        std::size_t k;

        SubProblemSolver(const Instance& i, std::size_t k) : i{i}, k{k} {
        }

        virtual SubProblemSolution
        solve(const ExtendedSolverSolution& lp_sol, const std::vector<BranchingRule>& active_br) = 0;
        virtual ~SubProblemSolver() = default;
    };
}// namespace kpgf

#endif
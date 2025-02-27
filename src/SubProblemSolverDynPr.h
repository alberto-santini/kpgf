//
// Created by Paolo on 25/07/2024.
//

#ifndef KPGF_SUBPROBLEMSOLVERDYNPR_H
#define KPGF_SUBPROBLEMSOLVERDYNPR_H

#include "BranchingRule.h"
#include "ExtendedSolverSolution.h"
#include "Instance.h"
#include "SubProblemSolution.h"
#include "SubProblemSolver.h"
#include <cstddef>
#include <gurobi_c++.h>
#include <map>
#include <memory>
#include <set>

namespace kpgf {

    struct SubProblemSolverDynPr : public SubProblemSolver {
        std::size_t maxH;
        std::size_t maxW;

        std::map<std::size_t, double> profit_to_consider;
        double fixed_z;
        std::size_t fixed_h;
        std::size_t fixed_w;
        std::vector<size_t> item_to_include;
        std::vector<std::vector<std::vector<size_t>>> item_sets;
        std::vector<std::vector<size_t>> r_item_sets;
        std::vector<std::vector<double>> zeta;
        std::vector<double> r_zeta;

        SubProblemSolverDynPr(const Instance& i, std::size_t k);
        SubProblemSolution
        solve(const ExtendedSolverSolution& lp_sol, const std::vector<BranchingRule>& active_br) override;

    private:
        void apply_branching_rules(const std::vector<BranchingRule>& brs, const double capacity_dual);
    };
}// namespace kpgf

#endif//KPGF_SUBPROBLEMSOLVERDYNPR_H

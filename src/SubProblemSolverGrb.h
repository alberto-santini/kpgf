//
// Created by Alberto Santini on 08/06/2024.
//

#ifndef KPGF_SUBPROBLEMSOLVERGRB_H
#define KPGF_SUBPROBLEMSOLVERGRB_H

#include "BranchingRule.h"
#include "ExtendedSolverSolution.h"
#include "Instance.h"
#include "SubProblemSolution.h"
#include "SubProblemSolver.h"
#include <cstddef>
#include <gurobi_c++.h>
#include <map>
#include <memory>

namespace kpgf {
    struct SubProblemSolverGrb : public SubProblemSolver {
        GRBEnv env;
        std::unique_ptr<GRBModel> m;

        std::map<std::size_t, GRBVar> delta;
        GRBConstr min_filling;
        GRBConstr capacity;
        GRBConstr rc_lb;
        GRBConstr rc_ub;
        GRBConstr card_lb;
        GRBConstr card_ub;

        SubProblemSolverGrb(const Instance& i, std::size_t k);
        SubProblemSolution
        solve(const ExtendedSolverSolution& lp_sol, const std::vector<BranchingRule>& active_br) override;

    private:
        void add_variables();
        void add_constraints();
        void reset_variables();
        void reset_constraints();
        void apply_branching_rules(const std::vector<BranchingRule>& brs);
        std::vector<std::vector<size_t>> get_all_solutions(double cover_dual);
    };
}// namespace kpgf

#endif//KPGF_SUBPROBLEMSOLVERGRB_H

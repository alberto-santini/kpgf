//
// Created by Alberto Santini on 06/06/2024.
//

#ifndef KPGF_EXTENDEDSOLVER_H
#define KPGF_EXTENDEDSOLVER_H

#include "BranchingRule.h"
#include "ColumnPool.h"
#include "ExtendedSolverSolution.h"
#include "Instance.h"
#include <gurobi_c++.h>
#include <memory>
#include <vector>

namespace kpgf {
    struct ExtendedSolver {
        const Instance& i;
        ColumnPool& cpool;

        GRBEnv env;
        std::unique_ptr<GRBModel> m;
        std::vector<GRBVar> eta;
        GRBConstr capacity;
        std::vector<GRBConstr> cover;

        ExtendedSolver(const Instance& i, ColumnPool& cpool);

        void add_column_to_pool(Column col);
        void apply_branching_rules(const std::vector<BranchingRule>& brules);
        void make_all_variables_continuous();
        void make_all_variables_binary();

        ExtendedSolverSolution solve();
        ExtendedSolverSolution solve_mip();

    private:
        void add_constraints();
        void add_variables();
        void add_variable_for_column(const Column& col);
    };
}// namespace kpgf

#endif//KPGF_EXTENDEDSOLVER_H

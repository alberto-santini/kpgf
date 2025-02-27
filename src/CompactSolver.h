//
// Created by Alberto Santini on 28/05/2024.
//

#ifndef KPGF_COMPACTSOLVER_H
#define KPGF_COMPACTSOLVER_H

#include "CompactSolverResults.h"
#include "CompactSolverRootCallback.h"
#include "Instance.h"
#include "Solution.h"
#include <gurobi_c++.h>
#include <memory>
#include <vector>

namespace kpgf {
    struct CompactSolver {
        const Instance& i;
        CompactSolverRootCallback cbk;

        GRBEnv env;

        // By experience, it is better to use a pointer instead
        // of directly a GRBModel object, in case one uses multi-thread
        // with multiple models active at the same time.
        std::unique_ptr<GRBModel> m;

        std::vector<GRBVar> x;
        GRBConstr capacity;
        std::vector<GRBConstr> rc_lb;
        std::vector<GRBConstr> rc_ub;

        explicit CompactSolver(const Instance& i);

        void load_initial_solution(const Solution& sol);
        void fix_solution(const Solution& sol);
        CompactSolverResults solve();

    private:
        void add_variables();
        void add_constraints();
        Solution get_solution() const;
    };
}// namespace kpgf

#endif//KPGF_COMPACTSOLVER_H

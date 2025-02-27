//
// Created by alberto on 02/12/24.
//

#ifndef CLASSCARDINALITYUBSOLVER_H
#define CLASSCARDINALITYUBSOLVER_H

#include "ClassCardinalityUbSolution.h"
#include "Instance.h"
#include <cstddef>
#include <gurobi_c++.h>
#include <memory>
#include <vector>

namespace kpgf {

    struct ClassCardinalityUbSolver {
        const Instance& i;
        size_t k;// Class index this problem refers to

        GRBEnv env;

        // By experience, it is better to use a pointer instead
        // of directly a GRBModel object, in case one uses multi-thread
        // with multiple models active at the same time.
        std::unique_ptr<GRBModel> m;

        std::map<size_t, GRBVar> y;
        GRBConstr min_rc;
        GRBConstr max_rc;
        GRBConstr max_capacity;

        ClassCardinalityUbSolver(const Instance& i, size_t k);
        ClassCardinalityUbSolution solve();

    private:
        void add_variables();
        void add_constraints();
        std::vector<std::vector<size_t>> get_all_solutions();
    };

}// namespace kpgf

#endif//CLASSCARDINALITYUBSOLVER_H

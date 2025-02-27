//
// Created by alberto on 02/12/24.
//

#ifndef CLASSCARDINALITYLBSOLVER_H
#define CLASSCARDINALITYLBSOLVER_H

#include "ClassCardinalityLbSolution.h"
#include "Instance.h"
#include <gurobi_c++.h>

namespace kpgf {

    struct ClassCardinalityLbSolver {
        const Instance& i;
        size_t k;// Class index this problem refers to

        GRBEnv env;

        // By experience, it is better to use a pointer instead
        // of directly a GRBModel object, in case one uses multi-thread
        // with multiple models active at the same time.
        std::unique_ptr<GRBModel> m;

        std::map<size_t, GRBVar> y;
        GRBConstr min_rc;
        GRBConstr max_capacity;

        ClassCardinalityLbSolver(const Instance& i, size_t k);
        ClassCardinalityLbSolution solve();

    private:
        void add_variables();
        void add_constraints();
        std::vector<std::vector<size_t>> get_all_solutions();
    };

}// namespace kpgf

#endif//CLASSCARDINALITYLBSOLVER_H

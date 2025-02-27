//
// Created by Alberto Santini on 05/06/2024.
//

#ifndef KPGF_MINKPMINFILLINGSOLVER_H
#define KPGF_MINKPMINFILLINGSOLVER_H

#include "ClassCapacitySolution.h"
#include "Instance.h"
#include <cstddef>
#include <gurobi_c++.h>
#include <map>
#include <memory>
#include <vector>

namespace kpgf {
    struct ClassCapacitySolver {
        const Instance& i;
        size_t k;// Class index this problem refers to

        GRBEnv env;

        // By experience, it is better to use a pointer instead
        // of directly a GRBModel object, in case one uses multi-thread
        // with multiple models active at the same time.
        std::unique_ptr<GRBModel> m;

        std::map<size_t, GRBVar> y;
        GRBConstr min_rc_filling;
        GRBConstr max_rc_filling;

        ClassCapacitySolver(const Instance& i, size_t k);
        ClassCapacitySolution solve();

    private:
        void add_variables();
        void add_constraints();
        std::vector<std::vector<size_t>> get_all_solutions();
    };
}// namespace kpgf

#endif//KPGF_MINKPMINFILLINGSOLVER_H

//
// Created by Alberto Santini on 05/06/2024.
//

#include "ClassCapacitySolver.h"
#include <gurobi_c++.h>
#include <sstream>

namespace kpgf {
    ClassCapacitySolver::ClassCapacitySolver(const Instance& i, size_t k) : i{i}, k{k}, env{true} {
        if(i.params.gurobi_silence_output) {
            env.set(GRB_IntParam_OutputFlag, 0);
            env.set(GRB_IntParam_LogToConsole, 0);
        }

        env.start();

        m = std::make_unique<GRBModel>(env);
        add_variables();
        add_constraints();
        m->set(GRB_IntParam_PoolSearchMode, 0);// Keep visited integer solutions in the pool
        m->set(
            GRB_DoubleParam_Cutoff, (double) i.capacity
        );// Not interested in solutions that occupy more than the capacity
        m->set(
            GRB_DoubleParam_BestBdStop, (double) i.capacity - Instance::eps
        );// As soon as the lower bound is >= capacity, the entire KPGF is infeasible
        m->set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
    }

    void ClassCapacitySolver::add_variables() {
        std::stringstream ss;

        for(const auto j : i.items_in_class(k)) {
            ss.str({});
            ss << "y[" << j << "]";

            y[j] = m->addVar(
                0.0,                 // LB
                1.0,                 // UB
                (double) i.weight[j],// Obj
                GRB_BINARY,          // Type
                ss.str()             // Name
            );
        }
    }

    void ClassCapacitySolver::add_constraints() {
        GRBLinExpr expr = 0;

        for(const auto j : i.items_in_class(k)) {
            expr += (double) i.rc[j] * y[j];
        }

        min_rc_filling = m->addConstr(expr >= (double) i.rc_lb[k], "min_filling");
        max_rc_filling = m->addConstr(expr <= (double) i.rc_ub[k], "max_capacity");
    }

    ClassCapacitySolution ClassCapacitySolver::solve() {
        m->set(GRB_IntParam_Threads, static_cast<int>(i.params.gurobi_n_threads));
        m->set(GRB_DoubleParam_TimeLimit, i.params.preprocessing_mips_gurobi_timelimit_s);
        m->optimize();

        const auto status = m->get(GRB_IntAttr_Status);

        if(status == GRB_INFEASIBLE or status == GRB_CUTOFF or status == GRB_USER_OBJ_LIMIT) {
            return ClassCapacitySolution::for_infeasible(i);
        }

        const auto lb = m->get(GRB_DoubleAttr_ObjBound);
        if(m->get(GRB_IntAttr_SolCount) == 0) {
            return ClassCapacitySolution::for_lb_only(i, lb);
        }

        const auto ub = m->get(GRB_DoubleAttr_ObjVal);
        return ClassCapacitySolution::for_feasible(i, lb, ub, get_all_solutions());
    }

    std::vector<std::vector<size_t>> ClassCapacitySolver::get_all_solutions() {
        const auto howmany = m->get(GRB_IntAttr_SolCount);
        auto solutions = std::vector<std::vector<size_t>>{};

        solutions.reserve(howmany);

        for(auto sol_n = 0; sol_n < howmany; ++sol_n) {
            m->set(GRB_IntParam_SolutionNumber, sol_n);
            const auto weight = m->get(GRB_DoubleAttr_PoolObjVal);

            if(weight <= (double) i.capacity) {
                std::vector<size_t> solution;

                for(const auto j : i.items_in_class(k)) {
                    if(y[j].get(GRB_DoubleAttr_Xn) > 1 - Instance::eps) {
                        solution.push_back(j);
                    }
                }

                if(not solution.empty()) {
                    solutions.push_back(std::move(solution));
                }
            }
        }

        return solutions;
    }
}// namespace kpgf
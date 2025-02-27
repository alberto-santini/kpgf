//
// Created by alberto on 02/12/24.
//

#include "ClassCardinalityUbSolver.h"
#include <sstream>
#include <utility>

namespace kpgf {
    ClassCardinalityUbSolver::ClassCardinalityUbSolver(const Instance& i, size_t k) : i{i}, k{k}, env{true} {
        if(i.params.gurobi_silence_output) {
            env.set(GRB_IntParam_OutputFlag, 0);
            env.set(GRB_IntParam_LogToConsole, 0);
        }

        env.start();

        m = std::make_unique<GRBModel>(env);
        add_variables();
        add_constraints();
        m->set(GRB_IntParam_PoolSearchMode, 0);// Keep visited integer solutions in the pool
        m->set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);
    }

    void ClassCardinalityUbSolver::add_variables() {
        std::stringstream ss;

        for(const auto j : i.items_in_class(k)) {
            ss.str({});
            ss << "y[" << j << "]";

            y[j] = m->addVar(
                0.0,       // LB
                1.0,       // UB
                1.0,       // Obj
                GRB_BINARY,// Type
                ss.str()   // Name
            );
        }
    }

    void ClassCardinalityUbSolver::add_constraints() {
        {
            GRBLinExpr expr = 0;

            for(const auto j : i.items_in_class(k)) {
                expr += (double) i.rc[j] * y[j];
            }

            min_rc = m->addConstr(expr >= (double) i.rc_lb[k], "min_rc");
            max_rc = m->addConstr(expr <= (double) i.rc_ub[k], "max_rc");
        }

        {
            GRBLinExpr expr = 0;

            for(const auto j : i.items_in_class(k)) {
                expr += (double) i.weight[j] * y[j];
            }

            max_capacity = m->addConstr(expr <= (double) i.class_capacity(k), "max_capacity");
        }
    }

    ClassCardinalityUbSolution ClassCardinalityUbSolver::solve() {
        m->set(GRB_IntParam_Threads, static_cast<int>(i.params.gurobi_n_threads));
        m->set(GRB_DoubleParam_TimeLimit, i.params.preprocessing_mips_gurobi_timelimit_s);
        m->optimize();

        const auto status = m->get(GRB_IntAttr_Status);

        if(status == GRB_INFEASIBLE) {
            return ClassCardinalityUbSolution::for_infeasible(i);
        }

        const auto ub = m->get(GRB_DoubleAttr_ObjBound);
        if(m->get(GRB_IntAttr_SolCount) == 0) {
            return ClassCardinalityUbSolution::for_ub_only(i, ub);
        }

        const auto lb = m->get(GRB_DoubleAttr_ObjVal);
        return ClassCardinalityUbSolution::for_feasible(i, lb, ub, get_all_solutions());
    }

    std::vector<std::vector<size_t>> ClassCardinalityUbSolver::get_all_solutions() {
        const auto howmany = m->get(GRB_IntAttr_SolCount);
        auto solutions = std::vector<std::vector<size_t>>{};

        solutions.reserve(howmany);

        for(auto sol_n = 0; sol_n < howmany; ++sol_n) {
            m->set(GRB_IntParam_SolutionNumber, sol_n);
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

        return solutions;
    }
}// namespace kpgf
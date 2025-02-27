//
// Created by Alberto Santini on 28/05/2024.
//

#include "CompactSolver.h"
#include <memory>
#include <sstream>

namespace kpgf {
    CompactSolver::CompactSolver(const Instance& i) : i{i}, cbk{}, env{true} {
        if(i.params.gurobi_silence_output) {
            env.set(GRB_IntParam_OutputFlag, 0);
            env.set(GRB_IntParam_LogToConsole, 0);
        }

        env.start();

        m = std::make_unique<GRBModel>(env);
        add_variables();
        add_constraints();
        m->set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);
    }

    void CompactSolver::add_variables() {
        std::stringstream ss;

        for(const auto j : i.items()) {
            ss.str({});
            ss << "x[" << j << "]";

            x.push_back(m->addVar(
                0.0,                 // LB
                1.0,                 // UB
                (double) i.profit[j],// Obj
                GRB_BINARY,          // Type
                ss.str()             // Name
            ));
        }
    }

    void CompactSolver::add_constraints() {
        // Capacity constraint:
        {
            GRBLinExpr lhs;

            for(const auto j : i.items()) {
                lhs += (double) i.weight[j] * x[j];
            }

            capacity = m->addConstr(lhs <= (double) i.capacity, "capacity");
        }

        // Resource consumption constraints:
        std::stringstream ss;
        for(const auto k : i.classes()) {
            GRBLinExpr expr;

            for(const auto j : i.items_in_class(k)) {
                expr += (double) i.rc[j] * x[j];
            }

            ss.str({});
            ss << "rc_lb[" << k << "]";

            rc_lb.push_back(m->addConstr(expr >= (double) i.rc_lb[k], ss.str()));

            ss.str({});
            ss << "rc_ub[" << k << "]";

            rc_ub.push_back(m->addConstr(expr <= (double) i.rc_ub[k], ss.str()));
        }
    }

    CompactSolverResults CompactSolver::solve() {
        m->set(GRB_DoubleParam_TimeLimit, i.params.compact_gurobi_timelimit_s);
        m->set(GRB_DoubleParam_IntFeasTol, Instance::eps);
        m->set(GRB_DoubleParam_MIPGap, Instance::eps);
        m->set(GRB_IntParam_Threads, static_cast<int>(i.params.gurobi_n_threads));
        m->setCallback(&cbk);
        m->optimize();

        const auto time_elapsed = m->get(GRB_DoubleAttr_Runtime);
        const auto status = m->get(GRB_IntAttr_Status);

        if(status == GRB_INFEASIBLE) {
            return CompactSolverResults::for_infeasible(i, time_elapsed);
        } else {
            const auto dual_ub = m->get(GRB_DoubleAttr_ObjBound);
            if(m->get(GRB_IntAttr_SolCount) == 0) {
                return CompactSolverResults::for_unknown(i, dual_ub, cbk.root_node_dual_ub, time_elapsed);
            } else {
                const auto primal_lb = m->get(GRB_DoubleAttr_ObjVal);
                return CompactSolverResults::for_feasible(
                    i, get_solution(), primal_lb, dual_ub, cbk.root_node_dual_ub, time_elapsed
                );
            }
        }
    }

    Solution CompactSolver::get_solution() const {
        std::vector<size_t> items_packed;

        for(const auto j : i.items()) {
            if(x[j].get(GRB_DoubleAttr_X) > 1.0 - Instance::eps) {
                items_packed.push_back(j);
            }
        }

        return Solution{i, items_packed};
    }

    void CompactSolver::load_initial_solution(const Solution& sol) {
        for(const auto j : sol.items_packed) {
            x[j].set(GRB_DoubleAttr_Start, 1.0);
        }
    }

    void CompactSolver::fix_solution(const Solution& sol) {
        for(const auto j : sol.items_packed) {
            x[j].set(GRB_DoubleAttr_LB, 1.0);
        }
    }
}// namespace kpgf
//
// Created by Alberto Santini on 08/06/2024.
//

#include "SubProblemSolverGrb.h"
#include <sstream>

namespace kpgf {
    SubProblemSolverGrb::SubProblemSolverGrb(const Instance& i, std::size_t k) : SubProblemSolver{i, k} {
        if(i.params.gurobi_silence_output) {
            env.set(GRB_IntParam_OutputFlag, 0);
            env.set(GRB_IntParam_LogToConsole, 0);
        }

        env.start();

        m = std::make_unique<GRBModel>(env);
        add_variables();
        add_constraints();
        m->set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);
        m->set(GRB_IntParam_PoolSearchMode, 0);// Keep visited integer solutions in the pool
    }

    void SubProblemSolverGrb::add_variables() {
        std::stringstream ss;

        for(const auto& j : i.items_in_class(k)) {
            ss.str({});
            ss << "delta[" << j << "]";

            delta[j] = m->addVar(
                0.0,       // LB
                1.0,       // UB
                0.0,       // Temp obj coefficient
                GRB_BINARY,// Type
                ss.str()   // Name
            );
        }
    }

    void SubProblemSolverGrb::add_constraints() {
        {
            GRBLinExpr expr;

            for(const auto& j : i.items_in_class(k)) {
                expr += (double) i.weight[j] * delta[j];
            }

            min_filling = m->addConstr(expr >= (double) i.class_min_filling(k), "min_filling");
            capacity = m->addConstr(expr <= (double) i.class_capacity(k), "capacity");
        }

        {
            GRBLinExpr expr;

            for(const auto& j : i.items_in_class(k)) {
                expr += (double) i.rc[j] * delta[j];
            }

            rc_lb = m->addConstr(expr >= (double) i.rc_lb[k], "rc_lb");
            rc_ub = m->addConstr(expr <= (double) i.rc_ub[k], "rc_ub");
        }

        {
            GRBLinExpr expr;

            for(const auto& j : i.items_in_class(k)) {
                expr += delta[j];
            }

            card_lb = m->addConstr(expr >= (double) i.class_cardinality_lb(k), "card_lb");
            card_ub = m->addConstr(expr <= (double) i.class_cardinality_ub((k)), "card_ub");
        }
    }

    void SubProblemSolverGrb::reset_variables() {
        for(const auto& j : i.items_in_class(k)) {
            delta[j].set(GRB_DoubleAttr_LB, 0.0);
            delta[j].set(GRB_DoubleAttr_UB, 1.0);
        }
    }

    void SubProblemSolverGrb::reset_constraints() {
        min_filling.set(GRB_DoubleAttr_RHS, (double) i.class_min_filling(k));
        capacity.set(GRB_DoubleAttr_RHS, (double) i.class_capacity(k));
        rc_lb.set(GRB_DoubleAttr_RHS, (double) i.rc_lb[k]);
        rc_ub.set(GRB_DoubleAttr_RHS, (double) i.rc_ub[k]);
        card_lb.set(GRB_DoubleAttr_RHS, (double) i.class_cardinality_lb(k));
        card_ub.set(GRB_DoubleAttr_RHS, (double) i.class_cardinality_ub(k));
    }

    void SubProblemSolverGrb::apply_branching_rules(const std::vector<BranchingRule>& brs) {
        reset_variables();

        // No need to reset constraints because I model items "forced" in the KP
        // by fixing the corresponding delta variable to 1.0.
        // An alternative approach would be to remove the item (i.e., fix the delta
        // variable to 0.0), and decrease the constraints' RHS appropriately. In
        // that case, I would have to reset the constraints here.
        // reset_constraints();

        for(const auto& br : brs) {
            if(br.item_class != k) {
                continue;
            }

            const auto j = br.item;

            if(br.status == ItemBranchingStatus::FORCE_NO_PACK) {
                delta[j].set(GRB_DoubleAttr_UB, 0.0);
            } else if(br.status == ItemBranchingStatus::FORCE_PACK) {
                delta[j].set(GRB_DoubleAttr_LB, 1.0);
            }
        }
    }

    SubProblemSolution
    SubProblemSolverGrb::solve(const ExtendedSolverSolution& lp_sol, const std::vector<BranchingRule>& active_br) {
        m->set(GRB_IntParam_Threads, static_cast<int>(i.params.gurobi_n_threads));

        if(not lp_sol.capacity_dual or not lp_sol.cover_duals) {
            throw std::logic_error{
                "Trying to solve the subproblem but the master problem LP solution does not have duals!"
            };
        }

        // Adjust parameters based on dual values:
        const auto cover_dual = (*lp_sol.cover_duals)[k];
        m->set(GRB_DoubleParam_Cutoff, cover_dual + Instance::eps);// Only interested in negative reduced cost columns
        for(const auto& j : i.items_in_class(k)) {
            delta[j].set(GRB_DoubleAttr_Obj, (double) i.profit[j] - (*lp_sol.capacity_dual) * (double) i.weight[j]);
        }

        apply_branching_rules(active_br);

        m->optimize();

        const auto status = m->get(GRB_IntAttr_Status);
        const auto time_elapsed = m->get(GRB_DoubleAttr_Runtime);

        if(status == GRB_INFEASIBLE) {
            return SubProblemSolution{SPFeasibleStatus::INFEASIBLE, time_elapsed, {}};
        } else if(status == GRB_CUTOFF) {
            return SubProblemSolution{SPFeasibleStatus::FEASIBLE, time_elapsed, {}};
        }

        return SubProblemSolution(SPFeasibleStatus::FEASIBLE, time_elapsed, get_all_solutions(cover_dual));
    }

    std::vector<std::vector<size_t>> SubProblemSolverGrb::get_all_solutions(double cover_dual) {
        const auto howmany = m->get(GRB_IntAttr_SolCount);
        auto solutions = std::vector<std::vector<size_t>>{};

        solutions.reserve(howmany);

        for(auto sol_n = 0; sol_n < howmany; ++sol_n) {
            m->set(GRB_IntParam_SolutionNumber, sol_n);
            const auto red_cost = m->get(GRB_DoubleAttr_PoolObjVal);

            if(red_cost >= cover_dual + Instance::eps) {
                std::vector<size_t> solution;

                for(const auto j : i.items_in_class(k)) {
                    if(delta[j].get(GRB_DoubleAttr_Xn) > 1 - Instance::eps) {
                        solution.push_back(j);
                    }
                }

                solutions.push_back(std::move(solution));
            }
        }

        return solutions;
    }
}// namespace kpgf
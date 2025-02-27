//
// Created by Alberto Santini on 06/06/2024.
//

#include "BBNode.h"
#include "Column.h"
#include "SubProblemSolver.h"
#include <cassert>
#include <chrono>

namespace kpgf {

    bool BBNode::can_contain_better_solution() const {
        if(father_dual_ub and res->primal_lb and *father_dual_ub <= *(res->primal_lb)) {
            return false;
        }

        return true;
    }

    void BBNode::update_sp_time_elapsed(double time_s) {
        if(i->params.bp_subproblem_solver == SubProblemSolverType::MIP) {
            res->time_elapsed_sp_gurobi += time_s;
        } else if(i->params.bp_subproblem_solver == SubProblemSolverType::DP) {
            res->time_elapsed_sp_dp += time_s;
        } else {
            throw std::logic_error{"Cannot record elapsed time for the current subproblem solver"};
        }
    }

    BBNodeSolution BBNode::solve() {
        const auto start_time = std::chrono::steady_clock::now();

        s->apply_branching_rules(active_br);

        auto pricing_round = 0u;

        while(true) {
            auto lp_sol = s->solve();
            res->time_elapsed_mp_lp += lp_sol.time_elapsed;

            assert(lp_sol.optimal);// LP solved to optimality

            if(current_primal_lb and lp_sol.obj_value < *current_primal_lb - Instance::eps) {
                return BBNodeSolution::with_dual_bound_only(
                    BBNodeStatus::PRUNED, lp_sol.obj_value, lp_sol.time_elapsed, lp_sol
                );
            }

            pricing_round += 1;

            if(i->params.verbose) {
                std::cout << "\tCurrent LP value at node " << node_number << ": " << lp_sol.obj_value << "\n";
            }

            std::vector<std::vector<std::size_t>> new_packings;
            for(const auto& k : i->classes()) {
                auto sol = sp_solvers->at(k)->solve(lp_sol, active_br);

                if(sol.status == SPFeasibleStatus::INFEASIBLE and i->params.verbose) {
                    std::cout << "\tSubproblem for class " << k << " infeasible at this node\n";
                }

                for(const auto& packing : sol.packings) {
                    new_packings.push_back(packing);
                }
            }

            if(i->params.verbose) {
                std::cout << "\tPricing round " << pricing_round << " ended at node " << node_number << ": ";
                if(new_packings.size() == 1u) {
                    std::cout << "1 new column found\n";
                } else if(new_packings.empty()) {
                    std::cout << "no new column found, pricing ends\n";
                } else {
                    std::cout << new_packings.size() << " new columns found\n";
                }
            }

            if(new_packings.empty()) {
                const auto end_time = std::chrono::steady_clock::now();
                const auto elapsed = std::chrono::duration<double>(end_time - start_time).count();
                update_sp_time_elapsed(elapsed);

                if(not lp_sol.feasible) {
                    return BBNodeSolution::from_infeasible_lp(elapsed, lp_sol);
                }

                if(lp_sol.integer) {
                    return BBNodeSolution::with_primal_and_dual_bounds(
                        BBNodeStatus::CLOSED, lp_sol.obj_value, lp_sol.obj_value, elapsed, lp_sol
                    );
                }

                return BBNodeSolution::with_dual_bound_only(BBNodeStatus::OPEN, lp_sol.obj_value, elapsed, lp_sol);
            }

            for(const auto& packing : new_packings) {
                s->add_column_to_pool(Column{*i, packing});
            }

            if(pricing_round >= 1'000'000u) {
                throw std::logic_error{"More than 1 million pricing rounds at the same BB Node? Smells like a bug!"};
            }
        }
    }
}// namespace kpgf
//
// Created by Alberto Santini on 06/06/2024.
//

#ifndef KPGF_BBNODESOLUTION_H
#define KPGF_BBNODESOLUTION_H

#include "ExtendedSolverSolution.h"
#include <cassert>
#include <cmath>
#include <optional>

namespace kpgf {
    enum class BBNodeStatus { CLOSED, OPEN, PRUNED, INFEASIBLE };

    struct BBNodeSolution {
        BBNodeStatus status;
        std::optional<double> primal_lb;
        std::optional<double> dual_ub;
        double time_elapsed;
        ExtendedSolverSolution es_sol;

        BBNodeSolution(
            BBNodeStatus status, std::optional<double> primal_lb, std::optional<double> dual_ub, double time_elapsed,
            ExtendedSolverSolution es_sol
        )
            : status{status}, primal_lb{primal_lb}, dual_ub{dual_ub}, time_elapsed{time_elapsed},
              es_sol{std::move(es_sol)} {
            if(dual_ub) {
                dual_ub = std::floor(*dual_ub);
            }
        }

        // Feasible LP solution with both primal LB and dual UB.
        // Typically, when hitting a leaf node where the solution is integer.
        // (In this case, LB == UB).
        // But it can also be used if a node runs a primal heuristic.
        static BBNodeSolution with_primal_and_dual_bounds(
            BBNodeStatus status, double primal_lb, double dual_ub, double time_elapsed, ExtendedSolverSolution es_sol
        ) {
            dual_ub = std::floor(dual_ub);
            return BBNodeSolution{status, primal_lb, dual_ub, time_elapsed, std::move(es_sol)};
        }

        // Feasible LP solution with dual UB but no primal LB.
        // Typical in BB Nodes whose optimum is fractional and branching must occur.
        static BBNodeSolution
        with_dual_bound_only(BBNodeStatus status, double dual_ub, double time_elapsed, ExtendedSolverSolution es_sol) {
            dual_ub = std::floor(dual_ub);
            return BBNodeSolution{status, std::nullopt, dual_ub, time_elapsed, std::move(es_sol)};
        }

        // Infeasible LP solution with neither primal nor dual bounds.
        // Branching rules can make some LP infeasible.
        static BBNodeSolution from_infeasible_lp(double time_elapsed, ExtendedSolverSolution es_sol) {
            return BBNodeSolution{
                BBNodeStatus::INFEASIBLE, std::nullopt, std::nullopt, time_elapsed, std::move(es_sol)
            };
        }
    };
}// namespace kpgf

#endif//KPGF_BBNODESOLUTION_H

//
// Created by Alberto Santini on 28/05/2024.
//

#ifndef KPGF_COMPACTSOLVERRESULTS_H
#define KPGF_COMPACTSOLVERRESULTS_H

#include "FeasibleStatus.h"
#include "Instance.h"
#include "Solution.h"
#include <nlohmann/json.hpp>
#include <optional>

namespace kpgf {
    struct CompactSolverResults {
        const Instance& i;

        FeasibleStatus status;
        std::optional<Solution> sol;
        std::optional<double> primal_lb;
        double dual_ub;
        std::optional<double> root_node_dual_ub;
        double time_elapsed;

        explicit CompactSolverResults(const Instance& i)
            : i{i}, status{FeasibleStatus::UNKNOWN}, sol{std::nullopt}, primal_lb{std::nullopt}, dual_ub{0.0},
              root_node_dual_ub{std::nullopt}, time_elapsed{0.0} {
        }

        nlohmann::json to_json() const;
        void to_file() const;

        static CompactSolverResults for_feasible(
            const Instance& i, const Solution& sol, double primal_lb, double dual_ub,
            std::optional<double> root_node_dual_ub, double time_elapsed
        );
        static CompactSolverResults for_infeasible(const Instance& i, double time_elapsed);
        static CompactSolverResults
        for_unknown(const Instance& i, double dual_ub, std::optional<double> root_node_dual_ub, double time_elapsed);
    };
}// namespace kpgf

#endif//KPGF_COMPACTSOLVERRESULTS_H

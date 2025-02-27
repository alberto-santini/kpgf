//
// Created by Paolo on 18/09/2024.
//

#ifndef KPGF_MCKPSOLVERRESULTS_H
#define KPGF_MCKPSOLVERRESULTS_H

#include "FeasibleStatus.h"
#include "Instance.h"
#include "Solution.h"
#include <nlohmann/json.hpp>
#include <optional>

namespace kpgf {
    struct MCKPSolverResults {
        const Instance& i;

        FeasibleStatus status;
        std::optional<Solution> sol;
        std::optional<double> primal_lb;
        double dual_ub;
        double build_time_elapsed;
        double solve_time_elapsed;
        std::size_t n_var;

        explicit MCKPSolverResults(const Instance& i)
            : i{i}, status{FeasibleStatus::UNKNOWN}, sol{std::nullopt}, primal_lb{std::nullopt}, dual_ub{0.0},
              build_time_elapsed{0.0}, solve_time_elapsed{0.0}, n_var{0u} {
        }

        nlohmann::json to_json() const;
        void to_file() const;

        static MCKPSolverResults for_feasible(
            const Instance& i, std::size_t v_n_var, const Solution& sol, double primal_lb, double dual_ub,
            double b_time_elapsed, double s_time_elapsed
        );
        static MCKPSolverResults for_infeasible(const Instance& i, double b_time_elapsed, double s_time_elapsed);
        static MCKPSolverResults
        for_unknown(const Instance& i, double dual_ub, double b_time_elapsed, double s_time_elapsed);
    };
}// namespace kpgf

#endif//KPGF_MCKPSOLVERRESULTS_H

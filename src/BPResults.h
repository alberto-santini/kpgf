//
// Created by alberto on 10/06/24.
//

#ifndef BPRESULTS_H
#define BPRESULTS_H

#include "FeasibleStatus.h"
#include "Solution.h"
#include <nlohmann/json.hpp>
#include <optional>

namespace kpgf {

    struct BPResults {
        const Instance& i;

        FeasibleStatus status;

        double time_elapsed;
        double time_elapsed_mp_lp;
        double time_elapsed_sp_gurobi;
        double time_elapsed_sp_dp;

        std::size_t nodes_created;
        std::size_t nodes_explored;
        std::size_t column_pool_sz;

        std::optional<double> primal_lb;
        std::optional<double> dual_ub;
        std::optional<double> dual_ub_at_root;
        std::optional<Solution> best_solution;

        BPResults(const Instance& i)
            : i{i}, status{FeasibleStatus::INFEASIBLE}, time_elapsed{0.0}, time_elapsed_mp_lp{0.0},
              time_elapsed_sp_gurobi{0.0}, time_elapsed_sp_dp{0.0}, nodes_created{0u}, nodes_explored{0u},
              column_pool_sz{0u}, primal_lb{std::nullopt}, dual_ub{std::nullopt}, dual_ub_at_root{std::nullopt},
              best_solution{std::nullopt} {
        }

        nlohmann::json to_json() const;
        void to_file() const;
    };

}// namespace kpgf

#endif//BPRESULTS_H

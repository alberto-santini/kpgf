//
// Created by Alberto Santini on 28/05/2024.
//

#include "CompactSolverResults.h"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace kpgf {
    nlohmann::json CompactSolverResults::to_json() const {
        nlohmann::json data;

        data["algorithm"] = "compact";
        data["params"] = i.params.to_json();
        data["instance"] = i.instance_name();
        data["time_elapsed"] = time_elapsed;

        if(status == FeasibleStatus::FEASIBLE) {
            assert(primal_lb);
            assert(sol);

            data["status"] = "feasible";
            data["dual_ub"] = dual_ub;
            data["primal_lb"] = *primal_lb;
            data["dual_ub_at_root"] = root_node_dual_ub.value_or(i.sum_profits());
            data["gap_pct"] = 100.0 * (dual_ub - *primal_lb) / dual_ub;
            data["solution"] = sol->to_json();
        } else if(status == FeasibleStatus::INFEASIBLE) {
            assert(not primal_lb);
            assert(not sol);

            data["status"] = "infeasible";
        } else if(status == FeasibleStatus::UNKNOWN) {
            assert(not primal_lb);
            assert(not sol);

            data["status"] = "unknown";
            data["dual_ub"] = dual_ub;
            data["dual_ub_at_root"] = root_node_dual_ub.value_or(i.sum_profits());
        }

        return data;
    }

    void CompactSolverResults::to_file() const {
        const auto filename =
            i.params.output_file.value_or(i.params.output_dir / ("results-" + i.instance_name() + "-compact.json"));
        std::ofstream ofs{filename};

        if(ofs.fail()) {
            std::cerr << "Cannot write to output file " << filename << "\n";
            std::exit(EXIT_FAILURE);
        }

        ofs << to_json().dump(2);
    }

    CompactSolverResults CompactSolverResults::for_feasible(
        const Instance& i, const Solution& sol, double primal_lb, double dual_ub,
        std::optional<double> root_node_dual_ub, double time_elapsed
    ) {
        auto res = CompactSolverResults{i};

        res.status = FeasibleStatus::FEASIBLE;
        res.sol.emplace(sol);
        res.dual_ub = dual_ub;
        res.primal_lb = primal_lb;
        res.root_node_dual_ub = root_node_dual_ub;
        res.time_elapsed = time_elapsed;

        return res;
    }

    CompactSolverResults CompactSolverResults::for_infeasible(const Instance& i, double time_elapsed) {
        auto res = CompactSolverResults{i};

        res.status = FeasibleStatus::INFEASIBLE;
        res.time_elapsed = time_elapsed;

        return res;
    }

    CompactSolverResults CompactSolverResults::for_unknown(
        const Instance& i, double dual_ub, std::optional<double> root_node_dual_ub, double time_elapsed
    ) {
        auto res = CompactSolverResults{i};

        res.status = FeasibleStatus::UNKNOWN;
        res.dual_ub = dual_ub;
        res.root_node_dual_ub = root_node_dual_ub;
        res.time_elapsed = time_elapsed;

        return res;
    }
}// namespace kpgf
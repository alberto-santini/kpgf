//
// Created by Paolo on 18/09/2024.
//

#include "MCKPSolverResults.h"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace kpgf {
    nlohmann::json MCKPSolverResults::to_json() const {
        nlohmann::json data;

        data["algorithm"] = "mckp";
        data["params"] = i.params.to_json();
        data["instance"] = i.instance_name();
        data["time_elapsed_build"] = build_time_elapsed;
        data["time_elapsed_solve"] = solve_time_elapsed;
        data["time_elapsed"] = build_time_elapsed + solve_time_elapsed;

        if(status == FeasibleStatus::FEASIBLE) {
            assert(primal_lb);
            assert(sol);

            data["status"] = "feasible";
            data["dual_ub"] = dual_ub;
            data["primal_lb"] = *primal_lb;
            data["gap_pct"] = 100.0 * (dual_ub - *primal_lb) / dual_ub;
            data["solution"] = sol->to_json();
            data["n_var"] = n_var;

        } else if(status == FeasibleStatus::INFEASIBLE) {
            assert(not primal_lb);
            assert(not sol);

            data["status"] = "infeasible";
        } else if(status == FeasibleStatus::UNKNOWN) {
            assert(not primal_lb);
            assert(not sol);

            data["status"] = "unknown";
            data["dual_ub"] = dual_ub;
        }

        return data;
    }

    void MCKPSolverResults::to_file() const {
        const auto filename =
            i.params.output_file.value_or(i.params.output_dir / ("results-" + i.instance_name() + "-mckp.json"));
        std::ofstream ofs{filename};

        if(ofs.fail()) {
            std::cerr << "Cannot write to output file " << filename << "\n";
            std::exit(EXIT_FAILURE);
        }

        ofs << to_json().dump(2);
    }

    MCKPSolverResults MCKPSolverResults::for_feasible(
        const Instance& i, std::size_t v_n_var, const Solution& sol, double primal_lb, double dual_ub,
        double b_time_elapsed, double s_time_elapsed
    ) {
        auto res = MCKPSolverResults{i};

        res.status = FeasibleStatus::FEASIBLE;
        res.sol.emplace(sol);
        res.dual_ub = dual_ub;
        res.primal_lb = primal_lb;
        res.build_time_elapsed = b_time_elapsed;
        res.solve_time_elapsed = s_time_elapsed;
        res.n_var = v_n_var;

        return res;
    }

    MCKPSolverResults
    MCKPSolverResults::for_infeasible(const Instance& i, double b_time_elapsed, double s_time_elapsed) {
        auto res = MCKPSolverResults{i};

        res.status = FeasibleStatus::INFEASIBLE;
        res.build_time_elapsed = b_time_elapsed;
        res.solve_time_elapsed = s_time_elapsed;

        return res;
    }

    MCKPSolverResults
    MCKPSolverResults::for_unknown(const Instance& i, double dual_ub, double b_time_elapsed, double s_time_elapsed) {
        auto res = MCKPSolverResults{i};

        res.status = FeasibleStatus::UNKNOWN;
        res.dual_ub = dual_ub;
        res.build_time_elapsed = b_time_elapsed;
        res.solve_time_elapsed = s_time_elapsed;

        return res;
    }
}// namespace kpgf
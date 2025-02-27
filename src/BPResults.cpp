//
// Created by alberto on 10/06/24.
//

#include "BPResults.h"
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace kpgf {
    nlohmann::json BPResults::to_json() const {
        nlohmann::json data;

        assert(nodes_created >= nodes_explored);

        data["algorithm"] = "bp";
        data["params"] = i.params.to_json();
        data["instance"] = i.instance_name();
        data["time_elapsed"] = time_elapsed;
        data["time_elapsed_mp_lp"] = time_elapsed_mp_lp;
        data["time_elapsed_sp_gurobi"] = time_elapsed_sp_gurobi;
        data["time_elapsed_sp_dp"] = time_elapsed_sp_dp;
        data["nodes_created"] = nodes_created;
        data["nodes_explored"] = nodes_explored;
        data["nodes_open"] = nodes_created - nodes_explored;
        data["column_pool_sz"] = column_pool_sz;

        if(status == FeasibleStatus::FEASIBLE) {
            assert(primal_lb);
            assert(dual_ub);
            assert(dual_ub_at_root);
            assert(best_solution);

            data["status"] = "feasible";
            data["dual_ub"] = *dual_ub;
            data["dual_ub_at_root"] = *dual_ub_at_root;
            data["primal_lb"] = *primal_lb;
            data["gap_pct"] = 100.0 * (*dual_ub - *primal_lb) / *dual_ub;
            data["solution"] = best_solution->to_json();
        } else if(status == FeasibleStatus::INFEASIBLE) {
            data["status"] = "infeasible";
        } else if(status == FeasibleStatus::UNKNOWN) {
            data["status"] = "unknown";

            if(dual_ub) {
                data["dual_ub"] = *dual_ub;
            }
            if(dual_ub_at_root) {
                data["ub_at_root"] = *dual_ub_at_root;
            }
        }

        return data;
    }

    void BPResults::to_file() const {
        const auto filename = i.params.output_file.value_or(
            i.params.output_dir / ("results-" + i.instance_name() + "-branch_price.json")
        );
        std::ofstream ofs{filename};

        if(ofs.fail()) {
            std::cerr << "Cannot write to output file " << filename << "\n";
            std::exit(EXIT_FAILURE);
        }

        ofs << to_json().dump(2);
    }
}// namespace kpgf
//
// Created by Alberto Santini on 28/05/2024.
//

#include "Params.h"
#include "BBTreeExplorationStrategy.h"
#include "SubProblemSolver.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace kpgf {
    namespace {
        bool read_param(auto& var, const std::string& key, const auto& data) {
            try {
                if(data.contains(key)) {
                    var = data[key];
                    return true;
                }

                return false;
            } catch(...) {
                return false;
            }
        }
    }// namespace

    Params::Params(const std::filesystem::path& params_file) {
        using json = nlohmann::json;

        if(not exists(params_file)) {
            std::cerr << "Params file " << params_file << " not found!\n";
            std::exit(EXIT_FAILURE);
        }

        std::ifstream ifs{params_file};

        const auto data = json::parse(ifs);

        read_param(verbose, "verbose", data);
        read_param(check_duplicate_cols, "check_duplicate_cols", data);
        read_param(gurobi_n_threads, "gurobi_n_threads", data);
        read_param(gurobi_silence_output, "gurobi_silence_output", data);
        read_param(compact_gurobi_timelimit_s, "compact_gurobi_timelimit_s", data);
        read_param(mckp_pisinger_timelimit_s, "mckp_pisinger_timelimit_s", data);
        read_param(mp_continuous_gurobi_timelimit_s, "mp_continuous_gurobi_timelimit_s", data);
        read_param(mp_integer_gurobi_timelimit_s, "mp_integer_gurobi_timelimit_s", data);
        read_param(preprocessing_mips_gurobi_timelimit_s, "preprocessing_mips_gurobi_timelimit_s", data);
        read_param(aggressive_preprocessing, "aggressive_preprocessing", data);
        read_param(bp_solve_mip, "bp_solve_mip", data);
        read_param(bp_solve_mip_each_n_nodes, "bp_solve_mip_each_n_nodes", data);
        read_param(bp_dp_state_space_relaxation, "bp_dp_state_space_relaxation", data);
        read_param(bp_overall_timelimit_s, "bp_overall_timelimit_s", data);

        if(data.contains("bp_change_exploration_strategy")) {
            bp_change_exploration_strategy = data["bp_change_exploration_strategy"].get<std::vector<std::size_t>>();
        } else {
            bp_change_exploration_strategy = {};
        }

        if(data.contains("output_dir")) {
            output_dir = data["output_dir"].get<std::string>();
        }

        if(data.contains("bp_starting_exploration_strategy")) {
            std::string s_bp_starting_exploration_strategy = data["bp_starting_exploration_strategy"];

            if(s_bp_starting_exploration_strategy == "depth_first") {
                bp_starting_exploration_strategy = BBTreeExplorationStrategy::DEPTH_FIRST;
            } else if(s_bp_starting_exploration_strategy == "best_first") {
                bp_starting_exploration_strategy = BBTreeExplorationStrategy::BEST_FIRST;
            } else {
                throw std::logic_error{
                    "Exploration strategy " + s_bp_starting_exploration_strategy + " not yet implemented"
                };
            }
        } else {
            bp_starting_exploration_strategy = BBTreeExplorationStrategy::DEPTH_FIRST;
        }

        if(data.contains("bp_subproblem_solver")) {
            std::string s_bp_subproblem_solver = data["bp_subproblem_solver"];

            if(s_bp_subproblem_solver == "mip") {
                bp_subproblem_solver = SubProblemSolverType::MIP;
            } else if(s_bp_subproblem_solver == "dp") {
                bp_subproblem_solver = SubProblemSolverType::DP;
            } else {
                throw std::logic_error{"SubProblem solver type " + s_bp_subproblem_solver + " not yet implmented"};
            }
        } else {
            bp_subproblem_solver = SubProblemSolverType::DP;
        }

        if(not exists(output_dir)) {
            std::filesystem::create_directories(output_dir);
        }

        if(data.contains("output_file")) {
            output_file = data["output_file"].get<std::string>();
            std::filesystem::create_directories(*output_file);
        }

        mp_continuous_gurobi_timelimit_s = std::min(mp_continuous_gurobi_timelimit_s, bp_overall_timelimit_s);
        mp_integer_gurobi_timelimit_s = std::min(mp_integer_gurobi_timelimit_s, bp_overall_timelimit_s);
    }

    nlohmann::json Params::to_json() const {
        nlohmann::json data;

        data["verbose"] = verbose;
        data["check_duplicate_cols"] = check_duplicate_cols;
        data["gurobi_n_threads"] = gurobi_n_threads;
        data["gurobi_silence_output"] = gurobi_silence_output;
        data["compact_gurobi_timelimit_s"] = compact_gurobi_timelimit_s;
        data["mckp_pisinger_timelimit_s"] = mckp_pisinger_timelimit_s;
        data["mp_continuous_gurobi_timelimit_s"] = mp_continuous_gurobi_timelimit_s;
        data["mp_integer_gurobi_timelimit_s"] = mp_integer_gurobi_timelimit_s;
        data["preprocessing_mips_gurobi_timelimit_s"] = preprocessing_mips_gurobi_timelimit_s;
        data["aggressive_preprocessing"] = aggressive_preprocessing;
        data["bp_solve_mip"] = bp_solve_mip;
        data["bp_solve_mip_each_n_nodes"] = bp_solve_mip_each_n_nodes;
        data["bp_starting_exploration_strategy"] = es_str(bp_starting_exploration_strategy);
        data["bp_change_exploration_strategy"] = bp_change_exploration_strategy;
        data["bp_subproblem_solver"] = sps_str(bp_subproblem_solver);
        data["bp_dp_state_space_relaxation"] = bp_dp_state_space_relaxation;
        data["bp_overall_timelimit_s"] = bp_overall_timelimit_s;

        return data;
    }

    void Params::print() const {
        std::cout << "Parameters:\n";
        std::cout << "\tVerbose: " << verbose << "\n";
        std::cout << "\tCheck duplicate cols: " << check_duplicate_cols << "\n";
        std::cout << "\tGurobi num threads: " << gurobi_n_threads << "\n";
        std::cout << "\tGurobi silence output: " << gurobi_silence_output << "\n";
        std::cout << "\tCompact Model Gurobi timelimit: " << compact_gurobi_timelimit_s << " seconds\n";
        std::cout << "\tPisinger MCKP timelimit: " << mckp_pisinger_timelimit_s << " seconds\n";
        std::cout << "\tMaster Problem LP Gurobi timelimit: " << mp_continuous_gurobi_timelimit_s << " seconds\n";
        std::cout << "\tMaster Problem MIP Gurobi timelimit: " << mp_integer_gurobi_timelimit_s << " sedonds\n";
        std::cout << "\tMinKP with Min Filling Gurobi timelimit: " << preprocessing_mips_gurobi_timelimit_s
                  << " seconds\n";
        std::cout << "\tAggressive preprocessing enabled: " << aggressive_preprocessing << "\n";
        std::cout << "\tBP solve MIP from time to time: " << bp_solve_mip << "\n";
        std::cout << "\tBP solve MIP each " << bp_solve_mip_each_n_nodes << " nodes\n";
        std::cout << "\tBP starting expl etrategy: " << es_str(bp_starting_exploration_strategy) << "\n";
        std::cout << "\tBP change expl strategy: at nodes ";
        for(const auto& n : bp_change_exploration_strategy) {
            std::cout << n << " ";
        }
        std::cout << "\n";
        std::cout << "\tBP subproblem solver: " << sps_str(bp_subproblem_solver) << "\n";
        if(bp_subproblem_solver == SubProblemSolverType::DP) {
            std::cout << "\tBP subproblem dp state space relaxation: " << bp_dp_state_space_relaxation << "\n";
        }
        std::cout << "\tBP overall timelimit: " << bp_overall_timelimit_s << "\n";
        std::cout << "\tOutput dir: " << output_dir << "\n";
    }
}// namespace kpgf
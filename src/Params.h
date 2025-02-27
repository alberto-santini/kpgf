//
// Created by Alberto Santini on 28/05/2024.
//

#ifndef KPGF_PARAMS_H
#define KPGF_PARAMS_H

#include <cstddef>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

namespace kpgf {
    enum class BBTreeExplorationStrategy;// Forward declaration
    enum class SubProblemSolverType;     // Forward declaration

    struct Params {
        bool verbose = true;
        bool check_duplicate_cols = true;

        std::size_t gurobi_n_threads = 1u;
        bool gurobi_silence_output = true;

        double compact_gurobi_timelimit_s = 3600.0;
        double mckp_pisinger_timelimit_s = 3600.0;
        double mp_continuous_gurobi_timelimit_s = 3600.0;
        double mp_integer_gurobi_timelimit_s = 3600.0;
        double preprocessing_mips_gurobi_timelimit_s = 3600.0;

        bool aggressive_preprocessing = true;

        bool bp_solve_mip = true;
        std::size_t bp_solve_mip_each_n_nodes = 1000u;
        BBTreeExplorationStrategy bp_starting_exploration_strategy;
        std::vector<std::size_t> bp_change_exploration_strategy;
        SubProblemSolverType bp_subproblem_solver;
        bool bp_dp_state_space_relaxation = true;
        double bp_overall_timelimit_s = 3600.0;

        std::filesystem::path output_dir = ".";
        std::optional<std::filesystem::path> output_file = std::nullopt;

        explicit Params(const std::filesystem::path& params_file);

        void print() const;
        nlohmann::json to_json() const;
    };
}// namespace kpgf

#endif//KPGF_PARAMS_H

//
// Created by Alberto Santini on 28/05/2024.
//

#ifndef KPGF_SOLUTION_H
#define KPGF_SOLUTION_H

#include "Instance.h"
#include <cstddef>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace kpgf {
    struct Solution {
        const Instance& i;

        std::vector<std::size_t> items_packed;

        std::size_t profit;
        std::size_t capacity_used;

        std::vector<std::size_t> weight_collected;
        std::vector<std::size_t> profit_collected;
        std::vector<std::size_t> rc_collected;

        Solution(const Instance& i, std::vector<std::size_t> items_packed);

        nlohmann::json to_json() const;
        static Solution from_json(const nlohmann::json& data, const Instance& i);
        static Solution from_file(const std::filesystem::path& solution_file, const Instance& i);
    };
}// namespace kpgf

#endif//KPGF_SOLUTION_H

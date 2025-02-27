//
// Created by Alberto Santini on 28/05/2024.
//

#include "Solution.h"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>

namespace kpgf {
    Solution::Solution(const Instance& i, std::vector<std::size_t> items_packed)
        : i{i}, items_packed{std::move(items_packed)}, profit{0u}, capacity_used{0u} {
        weight_collected.resize(i.n_classes);
        profit_collected.resize(i.n_classes);
        rc_collected.resize(i.n_classes);

        for(const auto j : this->items_packed) {
            profit += i.profit[j];
            capacity_used += i.weight[j];

            const auto k = i.class_of(j);

            weight_collected[k] += i.weight[j];
            profit_collected[k] += i.profit[j];
            rc_collected[k] += i.rc[j];
        }
    }

    nlohmann::json Solution::to_json() const {
        nlohmann::json data;

        const auto profit_available = std::reduce(i.profit.begin(), i.profit.end());

        data["instance"] = i.instance_name();
        data["profit_collected"] = profit;
        data["profit_available"] = profit_available;
        data["profit_collected_pct"] = 100.0 * (double) profit / (double) profit_available;
        data["capacity_used"] = capacity_used;
        data["capacity_available"] = i.capacity;
        data["capacity_used_pct"] = 100.0 * (double) capacity_used / (double) i.capacity;

        data["items_packed"] = nlohmann::json{};
        for(const auto j : items_packed) {
            nlohmann::json jdata;

            jdata["item"] = j;
            jdata["weight"] = i.weight[j];
            jdata["profit"] = i.profit[j];
            jdata["rc"] = i.rc[j];
            jdata["class"] = i.class_of(j);

            data["items_packed"].push_back(jdata);
        }

        data["classes"] = nlohmann::json{};
        for(const auto k : i.classes()) {
            nlohmann::json kdata;

            kdata["class"] = k;
            kdata["weight_collected"] = weight_collected[k];
            kdata["profit_collected"] = profit_collected[k];
            kdata["rc_collected"] = rc_collected[k];
            kdata["rc_lb"] = i.rc_lb[k];
            kdata["rc_ub"] = i.rc_ub[k];
            kdata["rc_pct_in_range"] =
                100.0 * (double) (rc_collected[k] - i.rc_lb[k]) / (double) (i.rc_ub[k] - i.rc_lb[k]);

            std::vector<std::size_t> packed;

            for(const auto j : i.items_in_class(k)) {
                if(std::find(items_packed.begin(), items_packed.end(), j) != items_packed.end()) {
                    packed.push_back(j);
                }
            }

            kdata["items_packed"] = packed;

            data["classes"].push_back(kdata);
        }

        return data;
    }

    Solution Solution::from_json(const nlohmann::json& data, const Instance& i) {
        std::vector<std::size_t> items_packed;

        for(const auto& jdata : data["items_packed"]) {
            items_packed.push_back(jdata["item"]);
        }

        return Solution{i, items_packed};
    }

    Solution Solution::from_file(const std::filesystem::path& solution_file, const Instance& i) {
        std::ifstream ifs{solution_file};

        if(ifs.fail()) {
            std::cerr << "Cannot read solution from file " << solution_file << "\n";
            std::exit(EXIT_FAILURE);
        }

        nlohmann::json data;
        ifs >> data;

        return from_json(data, i);
    }
}// namespace kpgf
//
// Created by Alberto Santini on 27/05/2024.
//

#ifndef KPGF_INSTANCE_H
#define KPGF_INSTANCE_H

#include "Params.h"
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace kpgf {

    struct Instance {
        static constexpr double eps = 1e-6;

        std::filesystem::path instance_file;

        Params params;

        std::size_t n_items = 0u;
        std::size_t n_classes = 0u;
        std::size_t capacity = 0u;

        // Data about classes
        std::vector<std::size_t> class_sz;
        std::vector<std::size_t> class_start;
        std::vector<std::size_t> class_end;
        std::vector<std::size_t> rc_lb;                               // Resource Consumption LB
        std::vector<std::size_t> rc_ub;                               // Resource Consumption UB
        std::optional<std::vector<std::size_t>> _class_min_filling;   // Minimum weight to pack
        std::optional<std::vector<std::size_t>> _class_capacity;      // Maximum weight to pack
        std::optional<std::vector<std::size_t>> _class_cardinality_lb;// Min number of objects to take
        std::optional<std::vector<std::size_t>> _class_cardinality_ub;// Max number of objects to take

        // Data about items
        std::vector<std::size_t> profit;
        std::vector<std::size_t> weight;
        std::vector<std::size_t> rc;// Resource Consumption

        Instance(const std::filesystem::path& instance_file, const std::filesystem::path& params_file);

        void print() const;
        std::string instance_name() const;

        // Utilities

        auto items() const {
            return std::ranges::views::iota(0u, n_items);
        }

        auto classes() const {
            return std::ranges::views::iota(0u, n_classes);
        }

        auto items_in_class(std::size_t k) const {
            assert(k < n_classes);

            using namespace std::ranges::views;
            return iota(class_start[k], class_end[k] + 1u);
        }

        auto class_of(std::size_t j) const {
            for(const auto k : classes()) {
                if(class_start[k] <= j and j <= class_end[k]) {
                    return k;
                }
            }
            assert(false);
            throw std::runtime_error("Cannot determine the class of item " + std::to_string(j));
        }

        auto sum_profits() const {
            return std::accumulate(profit.begin(), profit.end(), std::size_t{0u});
        }

        auto class_min_filling(std::size_t k) const {
            if(_class_min_filling) {
                return _class_min_filling->at(k);
            }
            return (std::size_t) 0u;
        }

        void class_min_filling(std::size_t k, std::size_t min_filling) {
            if(not _class_min_filling) {
                _class_min_filling = std::vector<std::size_t>(n_classes, 0u);
            }
            _class_min_filling->at(k) = min_filling;
        }

        auto class_capacity(std::size_t k) const {
            if(_class_capacity) {
                return _class_capacity->at(k);
            }
            return capacity;
        }

        void class_capacity(std::size_t k, std::size_t reduced_capacity) {
            if(not _class_capacity) {
                _class_capacity = std::vector<std::size_t>(n_classes, capacity);
            }
            _class_capacity->at(k) = reduced_capacity;
        }

        auto class_cardinality_lb(std::size_t k) const {
            if(_class_cardinality_lb) {
                return _class_cardinality_lb->at(k);
            }
            return (std::size_t) 0u;
        }

        void class_cardinality_lb(std::size_t k, std::size_t lb) {
            if(not _class_cardinality_lb) {
                _class_cardinality_lb = std::vector<std::size_t>(n_classes, 0u);
            }
            _class_cardinality_lb->at(k) = lb;
        }

        auto class_cardinality_ub(std::size_t k) const {
            if(_class_cardinality_ub) {
                return _class_cardinality_ub->at(k);
            }
            return class_sz.at(k);
        }

        void class_cardinality_ub(std::size_t k, std::size_t ub) {
            if(not _class_cardinality_ub) {
                _class_cardinality_ub = class_sz;
            }
            _class_cardinality_ub->at(k) = ub;
        }

    private:
        void tighten_resource_lower_bounds();
    };
}// namespace kpgf

#endif//KPGF_INSTANCE_H

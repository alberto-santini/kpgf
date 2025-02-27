//
// Created by Alberto Santini on 27/05/2024.
//

#include "Instance.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace kpgf {
    Instance::Instance(const std::filesystem::path& instance_file, const std::filesystem::path& params_file)
        : instance_file{instance_file}, params{params_file} {
        if(not exists(instance_file)) {
            std::cerr << "Instance file " << instance_file << " not found!\n";
            std::exit(EXIT_FAILURE);
        }

        std::ifstream ifs{instance_file};

        ifs >> n_items >> n_classes >> capacity;

        class_sz.resize(n_classes);
        class_start.resize(n_classes);
        class_end.resize(n_classes);
        rc_lb.resize(n_classes);
        rc_ub.resize(n_classes);

        size_t tot_items = 0u;

        for(const auto k : classes()) {
            size_t sz;
            ifs >> sz;

            class_sz[k] = sz;
            class_start[k] = tot_items;
            tot_items += sz;
            class_end[k] = tot_items - 1u;

            ifs >> rc_lb[k] >> rc_ub[k];

            if(rc_lb[k] > rc_ub[k]) {
                std::cerr << "Class " << k << ", RC LB (" << rc_lb[k] << ") > RC UB (" << rc_ub[k] << "). Invalid!\n";
                std::exit(EXIT_FAILURE);
            }
        }

        if(tot_items != n_items) {
            std::cerr << "Total items defined in classes (" << tot_items << ") != number of items (" << n_items
                      << "). Invalid!\n";
            std::exit(EXIT_FAILURE);
        }

        profit.resize(n_items);
        weight.resize(n_items);
        rc.resize(n_items);

        for(const auto j : items()) {
            ifs >> profit[j] >> weight[j] >> rc[j];
        }

        for(const auto j : items()) {
            if(rc[j] > rc_ub[class_of(j)]) {
                std::cerr << "Item " << j << " has RC = " << rc[j] << " but it belongs to class ";
                std::cerr << class_of(j) << " with RC UB = " << rc_ub[class_of(j)] << ". Invalid!\n";
                std::exit(EXIT_FAILURE);
            }
        }

        tighten_resource_lower_bounds();
    }

    void Instance::tighten_resource_lower_bounds() {
        using std::ranges::min_element;

        for(const auto k : classes()) {
            if(rc_lb[k] == 0u) {
                continue;
            }

            const auto min_rc = rc[*min_element(items_in_class(k), {}, [&](auto j) { return rc[j]; })];
            if(rc_lb[k] < min_rc) {
                if(params.verbose) {
                    std::cout << "Tightened RC LB of class " << k << ": " << rc_lb[k] << " -> " << min_rc << "\n";
                    rc_lb[k] = min_rc;
                }
            }
        }
    }

    void Instance::print() const {
        std::cout << "Instance " << instance_file << "\n";
        for(const auto k : classes()) {
            std::cout << "== Class " << k << " ==\n";
            std::cout << "RC bounds: [" << rc_lb[k] << ", " << rc_ub[k] << "]\n";
            std::cout << "Size: " << class_sz[k] << "\n";
            std::cout << "Items:\n";

            for(const auto j : items_in_class(k)) {
                std::cout << "\tItem " << j << ", p: " << profit[j] << ", w: " << weight[j] << ", h:" << rc[j] << "\n";
            }
        }
        params.print();
    }

    std::string Instance::instance_name() const {
        return instance_file.stem();
    }
}// namespace kpgf
//
// Created by Alberto Santini on 06/06/2024.
//

#ifndef KPGF_EXTENDEDSOLVERSOLUTION_H
#define KPGF_EXTENDEDSOLVERSOLUTION_H

#include "ColumnPool.h"
#include <cstddef>
#include <optional>
#include <vector>

namespace kpgf {
    struct ExtendedSolver;

    struct ExtendedSolverSolution {
        const Instance& i;
        bool feasible;
        bool integer;
        bool optimal;
        double obj_value;
        double time_elapsed;
        std::vector<double> eta_values;
        std::vector<double> item_values;

        std::optional<double> capacity_dual;
        std::optional<std::vector<double>> cover_duals;

        explicit ExtendedSolverSolution(const ExtendedSolver& solver);

        std::vector<std::size_t> items_packed() const;
    };
}// namespace kpgf

#endif//KPGF_EXTENDEDSOLVERSOLUTION_H

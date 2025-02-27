//
// Created by Alberto Santini on 06/06/2024.
//

#include "ExtendedSolverSolution.h"
#include "ExtendedSolver.h"
#include <cmath>
#include <ranges>

namespace kpgf {
    ExtendedSolverSolution::ExtendedSolverSolution(const ExtendedSolver& solver)
        : i{solver.i}, feasible{true}, integer{true}, optimal{solver.m->get(GRB_IntAttr_Status) == GRB_OPTIMAL},
          obj_value{0.0}, time_elapsed{solver.m->get(GRB_DoubleAttr_Runtime)}, eta_values(solver.eta.size(), 0.0),
          item_values(solver.i.n_items, 0.0), capacity_dual{std::nullopt}, cover_duals{std::nullopt} {
        const auto status = solver.m->get(GRB_IntAttr_Status);

        if(status == GRB_INFEASIBLE) {
            throw std::logic_error(
                "Gurobi MP returned Infeasible. This should not happen, the dummy column should be selected instead."
            );
        }

        assert(solver.m->get(GRB_IntAttr_SolCount) > 0);
        assert(solver.cpool.columns.at(0u).dummy);

        // Dummy column selected
        if(solver.eta.at(0).get(GRB_DoubleAttr_X) > Instance::eps) {
            feasible = false;
        }

        auto is_zero_one = [](const double coeff) {
            return std::fabs(coeff) < Instance::eps or std::fabs(1.0 - coeff) < Instance::eps;
        };

        for(auto col_idx = 0u; col_idx < solver.eta.size(); ++col_idx) {
            const auto& etavar = solver.eta[col_idx];
            const auto& col = solver.cpool.columns[col_idx];
            const auto cval = etavar.get(GRB_DoubleAttr_X);

            if(integer and not is_zero_one(cval)) {
                integer = false;
            }

            obj_value += cval * col.profit;
            eta_values[col_idx] = cval;

            for(const auto& j : col.items) {
                item_values[j] += cval;
            }
        }

        // If we solved the continuous relaxation, get the duals
        if(solver.eta[0].get(GRB_CharAttr_VType) == GRB_CONTINUOUS) {
            capacity_dual = solver.capacity.get(GRB_DoubleAttr_Pi);
            cover_duals = std::vector<double>(i.n_classes, 0.0);

            for(const auto& k : i.classes()) {
                (*cover_duals)[k] = solver.cover[k].get(GRB_DoubleAttr_Pi);
            }
        }
    }

    std::vector<std::size_t> ExtendedSolverSolution::items_packed() const {
        if(not integer) {
            throw std::logic_error{"Trying to extract the list of items from a fractional solution!"};
        }

        auto items = std::vector<std::size_t>();

        for(const auto& j : i.items()) {
            if(item_values[j] > 1.0 - Instance::eps) {
                items.push_back(j);
            }
        }

        return items;
    }
}// namespace kpgf
//
// Created by Paolo on 16/09/2024.
//

#include "MCKPSolver.h"
#include "mcknap.h"

#include <chrono>
#include <cstddef>
#include <optional>

namespace kpgf {
    MCKPSolver::MCKPSolver(const Instance& i) : i{i} {
        max_packings.resize(i.n_classes);
    }

    void MCKPSolver::enumerate_packings(std::size_t k) {
        std::size_t maxH = 0u;
        std::size_t maxW = 0u;

        for(auto j : i.items_in_class(k)) {
            maxH += i.rc[j];
            maxW += i.weight[j];
        }

        maxW = std::min(maxW, i.capacity);
        maxH = std::min(maxH, i.rc_ub[k]);

        std::vector<std::vector<int>> zeta(maxW + 1, std::vector<int>(maxH + 1));

        for(auto& row : zeta) {
            std::fill(row.begin(), row.end(), std::numeric_limits<int>::min());
        }

        zeta[0][0] = 0;

        std::vector<std::vector<std::vector<std::size_t>>> item_sets(
            maxW + 1, std::vector<std::vector<std::size_t>>(maxH + 1)
        );

        for(const auto j : i.items_in_class(k)) {
            auto profit = i.profit[j];

            for(auto w = maxW; w >= i.weight[j]; --w) {
                for(auto r = maxH; r >= i.rc[j]; --r) {
                    if(zeta[w - i.weight[j]][r - i.rc[j]] != std::numeric_limits<int>::min()) {
                        int newValue = zeta[w - i.weight[j]][r - i.rc[j]] + (int) profit;

                        if(newValue > zeta[w][r]) {
                            zeta[w][r] = newValue;
                            item_sets[w][r] = item_sets[w - i.weight[j]][r - i.rc[j]];
                            item_sets[w][r].push_back(j);
                        }
                    }
                }
            }
        }

        for(auto w = 0u; w <= maxW; ++w) {
            int best_profit = -1;
            int best_r = -1;

            for(auto r = i.rc_lb[k]; r <= maxH; ++r) {
                if(r >= i.rc_lb[k] && r <= i.rc_ub[k] && zeta[w][r] > best_profit) {
                    best_profit = zeta[w][r];
                    best_r = (int) r;
                }
            }

            if(best_profit > 0) {
                max_packings[k].emplace_back(item_sets[w][best_r], best_profit, w);
            }
        }
    }

    Solution MCKPSolver::get_solution(const std::vector<std::pair<int, int>>& my_solution) const {
        std::vector<std::size_t> items_packed;

        for(auto class_pack : my_solution) {
            for(auto j : max_packings[class_pack.first][class_pack.second].items) {
                items_packed.push_back(j);
            }
        }

        return Solution{i, items_packed};
    }

    MCKPSolverResults MCKPSolver::solve() {
        using namespace std::chrono;
        auto b_start_time = steady_clock::now();

        for(const auto& k : i.classes()) {
            enumerate_packings(k);

            auto end_time = steady_clock::now();
            const auto time_elapsed = std::chrono::duration<double>(end_time - b_start_time).count();
            if(time_elapsed > i.params.mckp_pisinger_timelimit_s)
                return MCKPSolverResults::for_unknown(i, 0.0, time_elapsed, 0.0);

            if(max_packings[k].empty()) {
                auto b_end_time = steady_clock::now();
                const auto time_elapsed = duration<double>(b_end_time - b_start_time).count();
                return MCKPSolverResults::for_infeasible(i, time_elapsed, 0.0);
            }
        }

        auto b_end_time = steady_clock::now();
        const auto b_time_elapsed = duration<double>(b_end_time - b_start_time).count();

        auto s_start_time = steady_clock::now();
        auto solution_pair = minmcknap(max_packings, i);
        auto s_end_time = steady_clock::now();
        const auto s_time_elapsed = duration<double>(s_end_time - s_start_time).count();

        if(solution_pair.second.empty()) {
            return MCKPSolverResults::for_unknown(
                i, 0.0, b_time_elapsed, s_time_elapsed
            );
        }
        auto n_var = 0u;

        for(const auto& packs : max_packings) {
            n_var += packs.size();
        }

        return MCKPSolverResults::for_feasible(
            i, n_var, get_solution(solution_pair.second), (double) solution_pair.first.zstar,
            0.0, b_time_elapsed, s_time_elapsed
        );
    }
}// namespace kpgf

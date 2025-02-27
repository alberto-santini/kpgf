//
// Created by Paolo on 25/07/2024.
//

#include "SubProblemSolverDynPr.h"

namespace kpgf {

    SubProblemSolverDynPr::SubProblemSolverDynPr(const Instance& i, std::size_t k)
        : SubProblemSolver{i, k}, maxH{0u}, maxW{0u}, fixed_z{0.0}, fixed_h{0u}, fixed_w{0u} {
        for(auto j : i.items_in_class(k)) {
            maxH += i.rc[j];
            maxW += i.weight[j];
        }
        maxW = std::min(maxW, i.capacity);
        maxH = std::min(maxH, i.rc_ub[k]);

        item_sets.resize(maxW + 1, std::vector<std::vector<size_t>>(maxH + 1));
        r_item_sets.resize(maxH + 1);

        zeta.resize(maxW + 1, std::vector<double>(maxH + 1));
        r_zeta.resize(maxH + 1);
    }

    namespace {
        bool compare(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) {
            return a.second > b.second;
        }
    }// namespace

    void
    SubProblemSolverDynPr::apply_branching_rules(const std::vector<BranchingRule>& brs, const double capacity_dual) {

        profit_to_consider.clear();
        for(auto j : i.items_in_class(k)) {
            profit_to_consider[j] = (double) i.profit[j] - capacity_dual * (double) i.weight[j];
        }

        fixed_z = 0;
        fixed_h = 0;
        fixed_w = 0;
        item_to_include.clear();

        for(const auto& br : brs) {
            if(br.item_class == k) {
                const auto j = br.item;
                profit_to_consider.erase(j);

                if(br.status == ItemBranchingStatus::FORCE_PACK) {
                    item_to_include.push_back(j);
                    fixed_z += (double) i.profit[j] - capacity_dual * (double) i.weight[j];
                    fixed_h += i.rc[j];
                    fixed_w += i.weight[j];
                }
            }
        }

        for(auto& row : zeta) {
            std::fill(row.begin(), row.end(), -std::numeric_limits<double>::infinity());
        }
        zeta[0][0] = 0.0;

        std::fill(r_zeta.begin(), r_zeta.end(), -std::numeric_limits<double>::infinity());
        r_zeta[0] = 0.0;
    }

    SubProblemSolution
    SubProblemSolverDynPr::solve(const ExtendedSolverSolution& lp_sol, const std::vector<BranchingRule>& active_br) {
        auto start_time = std::chrono::steady_clock::now();

        if(not lp_sol.capacity_dual or not lp_sol.cover_duals) {
            throw std::logic_error{
                "Trying to solve the subproblem but the master problem LP solution does not have duals!"
            };
        }

        apply_branching_rules(active_br, *lp_sol.capacity_dual);

        const auto cover_dual_eps = (*lp_sol.cover_duals)[k] + Instance::eps;
        auto solutions = std::vector<std::vector<size_t>>{};

        double z_rel_lp = 0;
        if(i.params.bp_dp_state_space_relaxation) {

            std::vector<std::pair<size_t, double>> scores;
            scores.reserve(profit_to_consider.size());
            for(const auto& [j, profit] : profit_to_consider)
                scores.emplace_back(j, profit / ((double) i.rc[j]));
            std::sort(scores.begin(), scores.end(), compare);
            auto z_feas_lp = fixed_z;
            z_rel_lp += fixed_z;
            auto totR = fixed_h;
            auto totW = fixed_w;
            std::vector<size_t> solution;
            bool before_critical = true;
            for(const auto& [j, score] : scores) {
                if(score > 0) {
                    if(totR + i.rc[j] <= i.rc_ub[k]) {
                        totR += i.rc[j];
                        totW += i.weight[j];
                        if(before_critical)
                            z_rel_lp += profit_to_consider[j];
                        solution.push_back(j);
                        z_feas_lp += profit_to_consider[j];
                    } else {
                        double frac = ((double) i.rc_ub[k] - (double) totR) / ((double) i.rc[j]);
                        z_rel_lp += profit_to_consider[j] * frac;
                        before_critical = false;
                        if(z_rel_lp <= cover_dual_eps)
                            break;
                    }
                }
            }
            if(z_feas_lp > cover_dual_eps) {
                while(totW > i.capacity) {
                    auto j = solution.back();
                    totW -= i.weight[j];
                    totR -= i.rc[j];
                    solution.pop_back();
                    z_feas_lp -= profit_to_consider[j];
                }
                if(z_feas_lp > cover_dual_eps && totR >= i.rc_lb[k])
                    solutions.push_back(solution);
            }
        }


        if(solutions.empty() and (z_rel_lp > cover_dual_eps || !i.params.bp_dp_state_space_relaxation)) {

            double z_rel = 0;
            if(i.params.bp_dp_state_space_relaxation) {
                for(const auto& [j, profit] : profit_to_consider) {
                    for(size_t r = maxH; r >= i.rc[j]; --r) {
                        if(r_zeta[r - i.rc[j]] != -std::numeric_limits<double>::infinity()) {
                            double newValue = r_zeta[r - i.rc[j]] + profit;

                            if(newValue > r_zeta[r]) {
                                r_zeta[r] = newValue;
                                r_item_sets[r] = r_item_sets[r - i.rc[j]];
                                r_item_sets[r].push_back(j);
                            }
                        }
                    }
                }

                double z_feas = cover_dual_eps;
                for(size_t r = 0; r <= maxH; ++r) {
                    if(r + fixed_h >= i.rc_lb[k] and r + fixed_h <= i.rc_ub[k]) {
                        if(r_zeta[r] + fixed_z > z_rel)
                            z_rel = r_zeta[r] + fixed_z;

                        size_t check_w = fixed_w;

                        for(auto j : r_item_sets[r])
                            check_w += i.weight[j];

                        if(check_w < i.capacity && r_zeta[r] + fixed_z > z_feas) {
                            solutions.clear();
                            solutions.push_back(r_item_sets[r]);
                            z_feas = r_zeta[r] + fixed_z;
                        }
                    }
                }
            }

            if(solutions.empty() and (z_rel > cover_dual_eps || !i.params.bp_dp_state_space_relaxation)) {
                for(const auto& [j, profit] : profit_to_consider) {
                    for(size_t w = maxW; w >= i.weight[j]; --w) {
                        for(size_t r = maxH; r >= i.rc[j]; --r) {
                            if(zeta[w - i.weight[j]][r - i.rc[j]] != -std::numeric_limits<double>::infinity()) {
                                double newValue = zeta[w - i.weight[j]][r - i.rc[j]] + profit;

                                if(newValue > zeta[w][r]) {
                                    zeta[w][r] = newValue;
                                    item_sets[w][r] = item_sets[w - i.weight[j]][r - i.rc[j]];
                                    item_sets[w][r].push_back(j);
                                }
                            }
                        }
                    }
                }

                double z_k = cover_dual_eps;
                for(size_t w = 0; w <= maxW - fixed_w; ++w) {
                    for(size_t r = i.rc_lb[k]; r <= maxH; ++r) {
                        if(fixed_z + zeta[w][r] > z_k && r + fixed_h >= i.rc_lb[k] && r + fixed_h <= i.rc_ub[k]) {
                            solutions.clear();
                            solutions.push_back(item_sets[w][r]);
                            z_k = fixed_z + zeta[w][r];
                        }
                    }
                }
            }
        }

        if(!item_to_include.empty()) {
            for(auto& solution : solutions) {
                solution.insert(solution.end(), item_to_include.begin(), item_to_include.end());
            }
        }

        auto end_time = std::chrono::steady_clock::now();
        const auto time_elapsed = std::chrono::duration<double>(end_time - start_time).count();

        return SubProblemSolution(SPFeasibleStatus::FEASIBLE, time_elapsed, solutions);
    }

}// namespace kpgf
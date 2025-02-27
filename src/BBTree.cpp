//
// Created by Alberto Santini on 08/06/2024.
//

#include "BBTree.h"
#include "ClassCapacitySolver.h"
#include "ClassCardinalityLbSolver.h"
#include "ClassCardinalityUbSolver.h"
#include "SubProblemSolver.h"
#include "SubProblemSolverDynPr.h"
#include "SubProblemSolverGrb.h"
#include "combo.h"
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <ranges>
#include <stdexcept>
#include <string>

namespace kpgf {
    BBTree::BBTree(Instance& i)
        : i{i}, res{i}, cpool{}, es{i, cpool}, sp_solvers{}, node_num{0u},
          current_exploration_strategy{i.params.bp_starting_exploration_strategy} {
        sp_solvers_init();
    }

    void BBTree::sp_solvers_init() {
        for(const auto& k : i.classes()) {
            if(i.params.bp_subproblem_solver == SubProblemSolverType::MIP) {
                sp_solvers.emplace_back(std::make_unique<SubProblemSolverGrb>(i, k));
            } else if(i.params.bp_subproblem_solver == SubProblemSolverType::DP) {
                sp_solvers.emplace_back(std::make_unique<SubProblemSolverDynPr>(i, k));
            } else {
                throw std::logic_error("Only MIP and DP subproblem solvers implemented!");
            }
        }
    }

    void BBTree::compute_class_cardinality_bounds() {
        const auto& verbose = i.params.verbose;
        auto packings = std::vector<std::vector<std::size_t>>();

        if(verbose) {
            std::cout << "Computing class cardinality lower bounds\n";
        }

        for(const auto k : i.classes()) {
            if(verbose) {
                std::cout << "=> Class " << k << "\n";
            }

            auto ccls = ClassCardinalityLbSolver{i, k};
            const auto ccls_sol = ccls.solve();

            if(ccls_sol.feasible) {
                if(verbose and ccls_sol.card_lb > 0) {
                    std::cout << "\tTightened cardinality LB from 0 to " << ccls_sol.card_lb << "\n";
                }

                i.class_cardinality_lb(k, ccls_sol.card_lb);
                for(const auto& packing : ccls_sol.feasible_packings) {
                    packings.push_back(packing);
                }
            } else {
                throw ProblemInfeasibleDuringPresolve{};
            }
        }

        if(verbose) {
            std::cout << "Computing class cardinality upper bounds\n";
        }

        for(const auto k : i.classes()) {
            if(verbose) {
                std::cout << "=> Class " << k << "\n";
            }

            auto ccus = ClassCardinalityUbSolver{i, k};
            const auto ccus_sol = ccus.solve();

            if(ccus_sol.feasible) {
                if(verbose and ccus_sol.card_ub < i.class_sz[k]) {
                    std::cout << "\tTightened cardinality UB from " << i.class_sz[k] << " to " << ccus_sol.card_ub
                              << "\n";
                }

                i.class_cardinality_ub(k, ccus_sol.card_ub);
                for(const auto& packing : ccus_sol.feasible_packings) {
                    packings.push_back(packing);
                }
            } else {
                throw ProblemInfeasibleDuringPresolve{};
            }
        }

        for(const auto& packing : packings) {
            const auto col = Column{i, packing};

            if(not cpool.is_duplicate(col)) {
                es.add_column_to_pool(col);
            }
        }
    }

    void BBTree::compute_tighter_resource_ubs() {
        if(i.params.verbose) {
            std::cout << "Solving Knapsack Problems to tighten RC upper bounds\n";
        }

        for(const auto k : i.classes()) {
            std::vector<item> cmb_items(i.class_sz[k]);
            auto tot_rc = 0u;
            auto tot_weight = 0u;
            auto solution = 0u;

#ifdef __cpp_lib_ranges_enumerate
            for(const auto [idx, j] : i.items_in_class(k) | std::views::enumerate) {
                cmb_items[idx] = {
                    static_cast<itype>(i.rc[j]), static_cast<itype>(i.weight[j]), static_cast<boolean>(false)
                };
                tot_rc += i.rc[j];
                tot_weight += i.weight[j];
            }
#else
            for(auto idx = 0u; const auto& j : i.items_in_class(k)) {
                cmb_items[idx] = {
                    static_cast<itype>(i.rc[j]), static_cast<itype>(i.weight[j]), static_cast<boolean>(false)
                };
                tot_rc += i.rc[j];
                tot_weight += i.weight[j];
                ++idx;
            }
#endif

            if(tot_weight < i.class_capacity(k)) {
                // All items fit!
                solution = tot_rc;
            } else {
                const auto capacity = static_cast<stype>(i.class_capacity(k));
                solution = combo(
                    &cmb_items.front(), &cmb_items.back(), capacity, static_cast<stype>(0),
                    static_cast<stype>(i.rc_ub[k]), static_cast<boolean>(true), static_cast<boolean>(false)
                );
            }

            if(solution < static_cast<stype>(i.rc_lb[k])) {
                std::cerr << "\tThere is no way to pack items of class " << k
                          << " while respecting all constraints. KPGF Infeasible!\n";
                throw ProblemInfeasibleDuringPresolve{};
            }

            if(solution < static_cast<stype>(i.rc_ub[k])) {
                std::vector<std::size_t> packing{};

#ifdef __cpp_lib_ranges_enumerate
                for(const auto [idx, j] : i.items_in_class(k) | std::views::enumerate) {
                    if(cmb_items[idx].x) {
                        packing.push_back(j);
                    }
                }
#else
                for(auto idx = 0u; const auto& j : i.items_in_class(k)) {
                    if(cmb_items[idx].x) {
                        packing.push_back(j);
                    }
                    ++idx;
                }
#endif

                if(packing.empty()) {
                    std::cerr << "Combo produced an empty packing for class " << std::to_string(k);
                    std::cerr << " in instance " << i.instance_name() << "\n";
                    std::cerr << "Class capacity: " << i.class_capacity(k) << ", ";
                    std::cerr << "UB used for combo is RC_UB: " << i.rc_ub[k] << "\n";
                    std::cerr << "Item resources: ";
                    for(const auto& j : i.items_in_class(k)) {
                        std::cerr << i.rc[j] << " ";
                    }
                    std::cerr << "\n";
                    std::cerr << "Item weights: ";
                    for(const auto& j : i.items_in_class(k)) {
                        std::cerr << i.weight[j] << " ";
                    }
                    std::cerr << "\n";
                } else {
                    if(i.params.verbose) {
                        std::cout << "Tightened RC UB for class " << k << ": " << i.rc_ub[k] << " -> " << solution << "\n";
                    }
                    i.rc_ub[k] = static_cast<std::size_t>(solution);

                    const auto col = Column{i, packing};

                    if(not cpool.is_duplicate(col)) {
                        es.add_column_to_pool(col);
                    }
                }
            }
        }
    }

    void BBTree::compute_reduced_class_capacities() {
        const auto& verbose = i.params.verbose;

        if(verbose) {
            std::cout << "Solving Min-Knapsack Problems with Minimum Filling Constraints\n";
            std::cout << "This will tighten the definition of a feasible packing and produce initial columns\n";
        }

        auto min_weights = std::vector<size_t>(i.n_classes);
        auto new_capacities = std::vector<size_t>(i.n_classes);
        auto packings = std::vector<std::vector<std::size_t>>();

        for(const auto k : i.classes()) {
            if(verbose) {
                std::cout << "=> Class " << k << "\n";
            }

            auto ccs = ClassCapacitySolver{i, k};
            const auto ccs_sol = ccs.solve();

            if(ccs_sol.feasible) {
                if(verbose) {
                    std::cout << "\tMin weight for items of class " << k << ": " << ccs_sol.weight_lb << "\n";
                }
                min_weights[k] = ccs_sol.weight_lb;
                i.class_min_filling(k, ccs_sol.weight_lb);

                for(const auto& packing : ccs_sol.feasible_packings) {
                    packings.push_back(packing);
                }

                if(verbose) {
                    std::cout << "\tAlso added " << ccs_sol.feasible_packings.size() << " packings of class " << k
                              << "\n";
                }
            } else {
                std::cerr << "\tThere is no way to pack items of class " << k
                          << " while respecting all constraints. KPGF Infeasible!\n";
                std::cerr << "\tKnapsack capacity: " << i.capacity << "\n";
                std::cerr << "\tClass " << k << " resource bounds: [" << i.rc_lb[k] << "," << i.rc_ub[k] << "]\n";
                std::cerr << "\tClass total weight: ";
#ifdef __cpp_lib_ranges_fold
                std::cerr << std::ranges::fold_left(i.items_in_class(k), 0u, [&](auto sum, auto j) {
                    return sum + i.weight[j];
                });
#else
                auto total_w = 0u;
                for(const auto& j : i.items_in_class(k)) {
                    total_w += i.weight[j];
                }
                std::cerr << total_w;
#endif
                std::cerr << "\n";
                std::cerr << "\tClass total RC: ";
#ifdef __cpp_lib_ranges_fold
                std::cerr << std::ranges::fold_left(i.items_in_class(k), 0u, [&](auto sum, auto j) {
                    return sum + i.rc[j];
                });
#else
                auto total_rc = 0u;
                for(const auto& j : i.items_in_class(k)) {
                    total_rc += i.rc[j];
                }
                std::cerr << total_rc;
#endif
                std::cerr << "\n";
                throw ProblemInfeasibleDuringPresolve{};
            }
        }

        for(const auto k : i.classes()) {
            auto sum_of_others = 0u;

            for(const auto kk : i.classes()) {
                if(kk == k) {
                    continue;
                }

                sum_of_others = min_weights[kk];
            }

            if(sum_of_others > i.capacity) {
                std::cerr << "There is no way to pack items of all classes in the given capacity!\n";
                std::cerr << "The minimum capacity should be " << min_weights[k] + sum_of_others
                          << " but the current capacity is " << i.capacity << "\n";

                throw ProblemInfeasibleDuringPresolve{};
            }

            if(sum_of_others == i.capacity) {
                if(verbose) {
                    std::cout << "Cannot tighten capacity for class " << k << "\n";
                }
            } else {
                if(verbose) {
                    std::cout << "Capacity for class " << k << " tightened: " << i.capacity << " -> "
                              << i.capacity - sum_of_others << "\n";
                }
                i.class_capacity(k, i.capacity - sum_of_others);
            }
        }

        for(const auto& packing : packings) {
            const auto col = Column{i, packing};

            if(not cpool.is_duplicate(col)) {
                es.add_column_to_pool(col);
            }
        }
    }

    void BBTree::cpool_init() {
        const auto& verbose = i.params.verbose;

        es.add_column_to_pool(Column::make_dummy());

        if(i.params.aggressive_preprocessing) {
            compute_reduced_class_capacities();
            compute_tighter_resource_ubs();
            compute_class_cardinality_bounds();
        }

        if(verbose) {
            std::cout << "Initial column pool size: " << cpool.columns.size() << "\n";
        }
    }

    void BBTree::print_output_line(const BBNode& node) const {
        if(i.params.verbose or (node.node_number % 100u == 0u)) {
            std::cout << "Node " << std::setw(6) << node.node_number << "; ";
            std::cout << "Nodes created: " << std::setw(6) << res.nodes_created << "; ";
            std::cout << "Open nodes: " << std::setw(6) << res.nodes_created - res.nodes_explored << "; ";
            std::cout << "Pool size: " << std::setw(6) << cpool.columns.size() << "; ";
            std::cout << "Active br rules: " << std::setw(6) << node.active_br.size() << "; ";
            std::cout << "Primal LB: ";

            if(res.primal_lb) {
                std::cout << std::setw(8) << *res.primal_lb;
            } else {
                std::cout << std::setw(8) << "???";
            }

            std::cout << "; Dual UB: ";

            if(res.dual_ub) {
                std::cout << std::setw(8) << *res.dual_ub;
            } else {
                std::cout << std::setw(8) << "???";
            }

            std::cout << "\n";
        }
    }

    void BBTree::add_root_node() {
        if(current_exploration_strategy == BBTreeExplorationStrategy::DEPTH_FIRST) {
            dfnodes.emplace(node_num, &i, &res, &es, &cpool, &sp_solvers, std::nullopt, res.primal_lb);
            return;
        }

        if(current_exploration_strategy == BBTreeExplorationStrategy::BEST_FIRST) {
            bfnodes.emplace(node_num, &i, &res, &es, &cpool, &sp_solvers, std::nullopt, res.primal_lb);
            return;
        }

        throw std::logic_error("Current tree exploration strategy not yet implemented");
    }

    void BBTree::potentially_solve_mip() {
        if(not i.params.bp_solve_mip) {
            return;
        }

        if(node_num % i.params.bp_solve_mip_each_n_nodes != 0u) {
            return;
        }

        if(cpool.columns.size() == last_cpool_sz_when_solving_mip) {
            return;
        }

        last_cpool_sz_when_solving_mip = cpool.columns.size();

        const auto solution = es.solve_mip();

        if(not solution.feasible) {
            return;
        }

        res.status = FeasibleStatus::FEASIBLE;

        if(not res.primal_lb or *res.primal_lb < solution.obj_value) {
            res.primal_lb = solution.obj_value;
            res.best_solution.emplace(i, solution.items_packed());
        }
    }

    void BBTree::potentially_change_exploration_strategy() {
        if(std::find(
               i.params.bp_change_exploration_strategy.begin(), i.params.bp_change_exploration_strategy.end(),
               res.nodes_explored
           ) != i.params.bp_change_exploration_strategy.end()) {
            if(current_exploration_strategy == BBTreeExplorationStrategy::DEPTH_FIRST) {
                current_exploration_strategy = BBTreeExplorationStrategy::BEST_FIRST;
                change_nodes_container(dfnodes, bfnodes);
                std::cout << "Changed exploration strategy to Best First\n";
            } else if(current_exploration_strategy == BBTreeExplorationStrategy::BEST_FIRST) {
                current_exploration_strategy = BBTreeExplorationStrategy::DEPTH_FIRST;
                change_nodes_container(bfnodes, dfnodes);
                std::cout << "Changed exploration strategy to Depth First\n";
            }
        }
    }

    bool BBTree::there_are_open_nodes() const {
        if(current_exploration_strategy == BBTreeExplorationStrategy::DEPTH_FIRST) {
            return not dfnodes.empty();
        }

        if(current_exploration_strategy == BBTreeExplorationStrategy::BEST_FIRST) {
            return not bfnodes.empty();
        }

        throw std::logic_error("Current tree exploration strategy not yet implemented");
    }

    BBNode BBTree::next_node() {
        if(current_exploration_strategy == BBTreeExplorationStrategy::DEPTH_FIRST) {
            assert(not dfnodes.empty());
            auto last_node_handle = dfnodes.extract(dfnodes.begin());
            return std::move(last_node_handle.value());
        }

        if(current_exploration_strategy == BBTreeExplorationStrategy::BEST_FIRST) {
            assert(not bfnodes.empty());
            auto last_node_handle = bfnodes.extract(bfnodes.begin());
            return std::move(last_node_handle.value());
        }

        throw std::logic_error("Current tree exploration strategy not yet implemented");
    }

    bool BBTree::time_is_over() const {
        const auto current_time = std::chrono::steady_clock::now();
        const auto time_elapsed = std::chrono::duration<double>(current_time - start_time).count();

        return time_elapsed > i.params.bp_overall_timelimit_s;
    }

    BPResults BBTree::solve() {
        start_time = std::chrono::steady_clock::now();

        try {
            cpool_init();
        } catch(const ProblemInfeasibleDuringPresolve&) {
            collect_results();
            return res;
        }

        res.nodes_created++;
        add_root_node();

        while(there_are_open_nodes()) {
            auto node = next_node();

            potentially_change_exploration_strategy();

            if(not node.can_contain_better_solution()) {
                res.nodes_explored++;
                continue;
            }

            if(time_is_over()) {
                break;
            }

            print_output_line(node);

            const auto sol = node.solve();
            res.nodes_explored++;

            if(node_num == 0u) {
                res.dual_ub_at_root = sol.dual_ub;
            }

            potentially_solve_mip();

            if(sol.status == BBNodeStatus::PRUNED or sol.status == BBNodeStatus::INFEASIBLE) {
                continue;
            }

            if(sol.status == BBNodeStatus::CLOSED) {
                process_closed_node_sol(sol);
                continue;
            }

            branch(sol, node);

            try {
                res.dual_ub = get_best_dual_ub();
            } catch(std::logic_error&) {
            }
        }

        std::cout << "Pool size: " << std::setw(6) << cpool.columns.size() << "; " << std::endl;
        collect_results();

        return res;
    }

    void BBTree::collect_results() {
        end_time = std::chrono::steady_clock::now();
        res.time_elapsed = std::chrono::duration<double>(end_time - start_time).count();
        res.column_pool_sz = cpool.columns.size();

        if(not res.best_solution) {
            if(not there_are_open_nodes()) {
                // No open node remains. Infeasible instance.
                return;
            }

            // There are still open nodes, but no primal solution up to now.
            // The instance could be feasible or infeasible.
            res.status = FeasibleStatus::UNKNOWN;
            res.dual_ub = get_best_dual_ub();
            return;
        }

        // Feasible instance.

        assert(res.primal_lb);
        assert(res.best_solution);

        res.status = FeasibleStatus::FEASIBLE;
        res.dual_ub = get_best_dual_ub();

        if(res.dual_ub == res.primal_lb) {
            // Optimal instance
            if(res.nodes_created == 1u and res.nodes_explored == 1u) {
                // Instance closed at the root node
                res.dual_ub_at_root = res.dual_ub;
            }
        }
    }

    void BBTree::process_closed_node_sol(const BBNodeSolution& sol) {
        assert(sol.primal_lb);
        res.status = FeasibleStatus::FEASIBLE;

        if(not res.primal_lb or *res.primal_lb < *sol.primal_lb) {
            res.primal_lb = *sol.primal_lb;
            res.best_solution.emplace(i, sol.es_sol.items_packed());
            prune_open_nodes();
        }
    }

    void BBTree::prune_open_nodes() {
        if(not res.primal_lb) {
            return;
        }

        auto check = [&](const BBNode& node) -> bool {
            return node.father_dual_ub and *node.father_dual_ub < *res.primal_lb;
        };

        if(current_exploration_strategy == BBTreeExplorationStrategy::DEPTH_FIRST) {
            std::erase_if(dfnodes, check);
            return;
        }

        if(current_exploration_strategy == BBTreeExplorationStrategy::BEST_FIRST) {
            std::erase_if(bfnodes, check);
            return;
        }

        throw std::logic_error("Current tree exploration strategy not yet implemented");
    }

    void BBTree::branch(const BBNodeSolution& sol, const BBNode& node) {
        const auto j = find_fractional_item(sol);
        const auto k = i.class_of(j);

        auto node_pack = BBNode{++node_num, &i, &res, &es, &cpool, &sp_solvers, sol.dual_ub, res.primal_lb};
        node_pack.active_br = node.active_br;
        node_pack.active_br.emplace_back(ItemBranchingStatus::FORCE_PACK, k, j);
        insert_node(std::move(node_pack));
        res.nodes_created++;

        auto node_nopack = BBNode{++node_num, &i, &res, &es, &cpool, &sp_solvers, sol.dual_ub, res.primal_lb};
        node_nopack.active_br = node.active_br;
        node_nopack.active_br.emplace_back(ItemBranchingStatus::FORCE_NO_PACK, k, j);
        insert_node(std::move(node_nopack));
        res.nodes_created++;
    }

    void BBTree::insert_node(BBNode&& node) {
        if(current_exploration_strategy == BBTreeExplorationStrategy::DEPTH_FIRST) {
            dfnodes.insert(std::move(node));
            return;
        }

        if(current_exploration_strategy == BBTreeExplorationStrategy::BEST_FIRST) {
            bfnodes.insert(std::move(node));
            return;
        }

        throw std::logic_error("Current tree exploration strategy not yet implemented");
    }

    bool BBTree::no_node_has_a_father_ub() const {
        if(current_exploration_strategy == BBTreeExplorationStrategy::DEPTH_FIRST) {
            return std::ranges::none_of(dfnodes, [](const auto& node) { return node.father_dual_ub.has_value(); });
        }

        if(current_exploration_strategy == BBTreeExplorationStrategy::BEST_FIRST) {
            return std::ranges::none_of(bfnodes, [](const auto& node) { return node.father_dual_ub.has_value(); });
        }

        throw std::logic_error("Current tree exploration strategy not yet implemented");
    }

    double BBTree::get_best_dual_ub() const {
        if(not res.best_solution and not there_are_open_nodes()) {
            throw std::logic_error("Asking for the dual bound of an infeasible instance!");
        }

        // Optimal solution found:
        if(res.best_solution and not there_are_open_nodes()) {
            assert(res.primal_lb);
            return *res.primal_lb;
        }

        // Otherwise, get the highest father dual UB of all open nodes.

        if(no_node_has_a_father_ub()) {
            throw std::logic_error("None of the open nodes has a valid dual UB from the father!");
        }

        if(current_exploration_strategy == BBTreeExplorationStrategy::DEPTH_FIRST) {
            return std::ranges::max_element(
                       dfnodes, [](const auto& n1, const auto& n2
                                ) { return n1.father_dual_ub.value_or(0.0) < n2.father_dual_ub.value_or(0.0); }
            )->father_dual_ub.value_or(0.0);
        }

        if(current_exploration_strategy == BBTreeExplorationStrategy::BEST_FIRST) {
            return bfnodes.begin()->father_dual_ub.value_or(0.0);
        }

        throw std::logic_error("Current tree exploration strategy not yet implemented");
    }

    std::size_t BBTree::find_fractional_item(const BBNodeSolution& sol) const {
        const auto& s = sol.es_sol;

        auto most_frac_item = std::optional<std::size_t>{std::nullopt};
        auto max_fractional = 0.0;

        for(const auto& j : i.items()) {
            const auto vj = s.item_values[j];
            const auto frac0 = std::fabs(vj);
            const auto frac1 = std::fabs(vj - 1.0);

            if(frac0 > Instance::eps and frac1 > Instance::eps) {
                const auto frac = std::min(frac0, frac1);

                if(frac > max_fractional) {
                    most_frac_item = j;
                }
            }
        }

        if(not most_frac_item) {
            throw std::logic_error{"No fractional item found in supposedly fractional solution!"};
        }

        return *most_frac_item;
    }
}// namespace kpgf
//
// Created by Alberto Santini on 08/06/2024.
//

#ifndef KPGF_BBTREE_H
#define KPGF_BBTREE_H

#include "BBNode.h"
#include "BBNodeSolution.h"
#include "BBTreeExplorationStrategy.h"
#include "BPResults.h"
#include "ColumnPool.h"
#include "ExtendedSolver.h"
#include "Instance.h"
#include "Solution.h"
#include "SubProblemSolver.h"
#include <cassert>
#include <chrono>
#include <cstddef>
#include <exception>
#include <memory>
#include <optional>
#include <vector>

namespace kpgf {
    struct ProblemInfeasibleDuringPresolve : std::exception {};

    struct BBTree {
        Instance& i;
        BPResults res;
        ColumnPool cpool;
        ExtendedSolver es;
        std::vector<std::unique_ptr<SubProblemSolver>> sp_solvers;

        std::size_t node_num;
        std::chrono::time_point<std::chrono::steady_clock> start_time, end_time;
        std::size_t last_cpool_sz_when_solving_mip = 1u;

        BBTreeExplorationStrategy current_exploration_strategy;
        DepthFirstNodeContainer dfnodes;
        BestFirstNodeContainer bfnodes;

        explicit BBTree(Instance& i);

        BPResults solve();
        void sp_solvers_init();

    private:
        void cpool_init();
        void compute_reduced_class_capacities();
        void compute_tighter_resource_ubs();
        void compute_class_cardinality_bounds();
        double get_best_dual_ub() const;
        void add_root_node();
        void process_closed_node_sol(const BBNodeSolution& sol);
        std::size_t find_fractional_item(const BBNodeSolution& sol) const;
        void branch(const BBNodeSolution& sol, const BBNode& node);
        void print_output_line(const BBNode& node) const;
        void collect_results();
        void potentially_solve_mip();
        void potentially_change_exploration_strategy();
        void prune_open_nodes();
        bool there_are_open_nodes() const;
        bool no_node_has_a_father_ub() const;
        BBNode next_node();
        void insert_node(BBNode&& node);
        bool time_is_over() const;

        template<typename OrderO, typename OrderD>
        void change_nodes_container(BBNodeContainer<OrderO>& origin, BBNodeContainer<OrderD>& dest) {
            // for(auto it = origin.begin(); it != origin.end();) {
            //   dest.insert(std::move(origin.extract(it++)));
            // }

            [[maybe_unused]] const auto initial_size = origin.size();
            auto it = origin.begin();

            while(it != origin.end()) {
                auto tmp = it;
                it++;
                dest.insert(std::move(origin.extract(tmp)));
            }

            assert(origin.size() == 0u);
            assert(dest.size() == initial_size);
        }
    };
}// namespace kpgf

#endif//KPGF_BBTREE_H

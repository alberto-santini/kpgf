//
// Created by Alberto Santini on 06/06/2024.
//

#ifndef KPGF_BBNODE_H
#define KPGF_BBNODE_H

#include "BBNodeSolution.h"
#include "BPResults.h"
#include "ColumnPool.h"
#include "ExtendedSolver.h"
#include "Instance.h"
#include "SubProblemSolver.h"
#include <vector>

namespace kpgf {
    struct BBNode {
        std::size_t node_number;

        const Instance* const i;
        BPResults* res;
        ExtendedSolver* const s;
        ColumnPool* const cpool;
        std::vector<std::unique_ptr<SubProblemSolver>>* const sp_solvers;

        std::vector<BranchingRule> active_br;
        std::optional<double> father_dual_ub;
        std::optional<double> current_primal_lb;

        BBNode(
            std::size_t node_number, const Instance* const i, BPResults* res, ExtendedSolver* const s,
            ColumnPool* const cpool, std::vector<std::unique_ptr<SubProblemSolver>>* const sp_solvers,
            std::optional<double> father_dual_ub, std::optional<double> current_primal_lb
        )
            : node_number{node_number}, i{i}, res{res}, s{s}, cpool{cpool}, sp_solvers{sp_solvers}, active_br{},
              father_dual_ub{father_dual_ub}, current_primal_lb{current_primal_lb} {
        }

        BBNodeSolution solve();
        bool can_contain_better_solution() const;

    private:
        void update_sp_time_elapsed(double time_s);
    };
}// namespace kpgf

#endif//KPGF_BBNODE_H

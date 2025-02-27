//
// Created by Alberto Santini on 06/06/2024.
//

#include "ExtendedSolver.h"
#include <algorithm>
#include <cassert>
#include <ranges>
#include <sstream>
#include <version>

namespace kpgf {
    ExtendedSolver::ExtendedSolver(const Instance& i, ColumnPool& cpool) : i{i}, cpool{cpool}, env{true} {
        if(i.params.gurobi_silence_output) {
            env.set(GRB_IntParam_OutputFlag, 0);
            env.set(GRB_IntParam_LogToConsole, 0);
        }

        env.start();

        m = std::make_unique<GRBModel>(env);
        add_constraints();
        add_variables();
        m->set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);
    }

    void ExtendedSolver::add_constraints() {
        capacity = m->addConstr(GRBLinExpr{} <= (double) i.capacity, "capacity");

        std::stringstream ss;

        for(const auto& k : i.classes()) {
            ss.str({});
            ss << "cover[" << k << "]";

            cover.push_back(m->addConstr(GRBLinExpr{} == 1, ss.str()));
        }
    }

    void ExtendedSolver::add_variables() {
        for(const auto& col : cpool.columns) {
            add_variable_for_column(col);
        }
    }

    void ExtendedSolver::add_variable_for_column(const kpgf::Column& col) {
        GRBColumn gcol;

        gcol.addTerm(col.weight, capacity);

        for(const auto& k : i.classes()) {
            gcol.addTerm(col.cover_coefficient(k), cover[k]);
        }

        std::stringstream ss;
        ss << "eta[" << eta.size() << "," << col.items_class << "]";

        eta.push_back(m->addVar(
            0.0,           // LB
            GRB_INFINITY,  // UB
            col.profit,    // Obj
            GRB_CONTINUOUS,// Type
            gcol,          // Column
            ss.str()       // Name
        ));
    }

    void ExtendedSolver::apply_branching_rules(const std::vector<BranchingRule>& brules) {
        assert(eta.size() == cpool.columns.size());

#ifdef __cpp_lib_ranges_zip
        for(auto&& [etavar, col] : std::views::zip(eta, cpool.columns)) {
#else
        for(auto idx = 0u; idx < eta.size(); ++idx) {
            auto& etavar = eta.at(idx);
            auto& col = cpool.columns.at(idx);
#endif
            if(std::all_of(brules.begin(), brules.end(), [&](const auto& br) { return col.is_compatible_with(br); })) {
                etavar.set(GRB_DoubleAttr_UB, GRB_INFINITY);// Allow in MP
            } else {
                etavar.set(GRB_DoubleAttr_UB, 0.0);// Remove from MP
            }
        }
    }

    void ExtendedSolver::make_all_variables_continuous() {
        for(auto& etavar : eta) {
            etavar.set(GRB_DoubleAttr_LB, 0.0);
            etavar.set(GRB_DoubleAttr_UB, GRB_INFINITY);
            etavar.set(GRB_CharAttr_VType, GRB_CONTINUOUS);
        }
    }

    void ExtendedSolver::make_all_variables_binary() {
        for(auto& etavar : eta) {
            etavar.set(GRB_DoubleAttr_LB, 0.0);
            etavar.set(GRB_DoubleAttr_UB, 1.0);
            etavar.set(GRB_CharAttr_VType, GRB_BINARY);
        }
    }

    ExtendedSolverSolution ExtendedSolver::solve() {
        m->set(GRB_DoubleParam_TimeLimit, i.params.mp_continuous_gurobi_timelimit_s);
        m->set(GRB_IntParam_Threads, static_cast<int>(i.params.gurobi_n_threads));
        m->optimize();

        return ExtendedSolverSolution(*this);
    }

    ExtendedSolverSolution ExtendedSolver::solve_mip() {
        make_all_variables_binary();

        m->set(GRB_DoubleParam_TimeLimit, i.params.mp_integer_gurobi_timelimit_s);
        m->set(GRB_IntParam_Threads, static_cast<int>(i.params.gurobi_n_threads));
        m->optimize();

        auto solution = ExtendedSolverSolution(*this);

        make_all_variables_continuous();

        return solution;
    }

    void ExtendedSolver::add_column_to_pool(Column col) {
        if(i.params.check_duplicate_cols and cpool.is_duplicate(col)) {
            std::cerr << "Found a duplicate column for class " << col.items_class << "!\nItems: ";
            for(const auto& j : col.items) {
                std::cerr << j << " ";
            }
            std::cerr << "\n";

            throw std::logic_error("Duplicate column found!");
        }

        add_variable_for_column(col);
        cpool.columns.push_back(std::move(col));
    }
}// namespace kpgf
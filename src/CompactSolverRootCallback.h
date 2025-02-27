//
// Created by alberto on 04/12/24.
//

#ifndef KPGF_COMPACTSOLVERROOTCALLBACK_H
#define KPGF_COMPACTSOLVERROOTCALLBACK_H

#include <cmath>
#include <cstddef>
#include <gurobi_c++.h>
#include <optional>

namespace kpgf {
    struct CompactSolverRootCallback : public GRBCallback {
        std::optional<double> root_node_dual_ub;

        explicit CompactSolverRootCallback() : root_node_dual_ub{std::nullopt} {
        }

    protected:
        void callback() override;
    };
}// namespace kpgf

#endif//KPGF_COMPACTSOLVERROOTCALLBACK_H

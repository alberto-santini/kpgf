//
// Created by alberto on 04/12/24.
//

#include "CompactSolverRootCallback.h"

namespace kpgf {
    void CompactSolverRootCallback::callback() {
        try {
            if (where != GRB_CB_MIPNODE) {
                return;
            }

            const auto node_n = getDoubleInfo(GRB_CB_MIPNODE_NODCNT);

            if ((int)node_n != 0) {
                return;
            }

            const auto status = getIntInfo(GRB_CB_MIPNODE_STATUS);

            if (status != GRB_OPTIMAL) {
                return;
            }

            const auto dual_obj = getDoubleInfo(GRB_CB_MIPNODE_OBJBND);

            root_node_dual_ub = std::floor(dual_obj);
        } catch(const GRBException& e) {
            std::cerr << "Gurobi error: " << e.getErrorCode() << " - " << e.getMessage() << "\n";
            throw;
        }
    }
}// namespace kpgf
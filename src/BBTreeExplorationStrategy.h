//
// Created by Alberto Santini on 30/07/2024.
//

#ifndef KPGF_BBTREEEXPLORATIONSTRATEGY_H
#define KPGF_BBTREEEXPLORATIONSTRATEGY_H

#include "BBNode.h"
#include <set>
#include <string>

namespace kpgf {
    enum class BBTreeExplorationStrategy { DEPTH_FIRST, BEST_FIRST };

    inline std::string es_str(const BBTreeExplorationStrategy& s) {
        if(s == BBTreeExplorationStrategy::DEPTH_FIRST) {
            return "depth_first";
        } else if(s == BBTreeExplorationStrategy::BEST_FIRST) {
            return "best_first";
        }

        throw std::logic_error("No known conversion to string for the given exploration strategy");
    }

    template<typename OrderF>
    using BBNodeContainer = std::set<BBNode, OrderF>;

    struct DepthFirstExploration {
        bool operator()(const BBNode& n1, const BBNode& n2) const {
            return n1.node_number > n2.node_number;
        }
    };

    struct BestFirstExploration {
        bool operator()(const BBNode& n1, const BBNode& n2) const {
            if((not n1.father_dual_ub) and (not n2.father_dual_ub)) {
                return n1.node_number < n2.node_number;
            }

            if(not n1.father_dual_ub) {
                return true;
            }

            if(not n2.father_dual_ub) {
                return false;
            }

            if(*n1.father_dual_ub == *n2.father_dual_ub) {
                return n1.node_number < n2.node_number;
            }

            return *n1.father_dual_ub > *n2.father_dual_ub;
        }
    };

    using DepthFirstNodeContainer = BBNodeContainer<DepthFirstExploration>;
    using BestFirstNodeContainer = BBNodeContainer<BestFirstExploration>;
}// namespace kpgf

#endif
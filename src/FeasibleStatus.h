//
// Created by alberto on 10/06/24.
//

#ifndef FEASIBLESTATUS_H
#define FEASIBLESTATUS_H

namespace kpgf {
    enum class FeasibleStatus {
        FEASIBLE,  // At least one solution produced within time limit
        INFEASIBLE,// Proven infeasible within time limit
        UNKNOWN    // No solution produced and not proven infeasible within time limit
    };
}

#endif//FEASIBLESTATUS_H

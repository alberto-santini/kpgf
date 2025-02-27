//
// Created by Alberto Santini on 05/06/2024.
//

#ifndef KPGF_BRANCHINGRULE_H
#define KPGF_BRANCHINGRULE_H

#include <cstddef>

namespace kpgf {
    enum class ItemBranchingStatus { FORCE_PACK, FORCE_NO_PACK };

    struct BranchingRule {
        ItemBranchingStatus status;
        std::size_t item_class;
        std::size_t item;

        BranchingRule(ItemBranchingStatus status, std::size_t item_class, std::size_t item)
            : status{status}, item_class{item_class}, item{item} {
        }
    };
}// namespace kpgf

#endif//KPGF_BRANCHINGRULE_H

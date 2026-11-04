#pragma once

#include "algos/FixedVec2.hpp"

namespace onyx::contracts::doomlike
{

struct Snapshot
{
    algos::FixedVec2 pos_;
    algos::FixedVec2 dir_;
    algos::FixedVec2 plane_;
};

} // namespace onyx::contracts::doomlike

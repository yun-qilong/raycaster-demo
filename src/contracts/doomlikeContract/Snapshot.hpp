#pragma once

#include "infra/FixedVec2.hpp"

namespace onyx::contracts::doomlike
{

struct Snapshot
{
    onyx::infra::FixedVec2 pos_;
    onyx::infra::FixedVec2 dir_;
    onyx::infra::FixedVec2 plane_;
};

} // namespace onyx::contracts::doomlike

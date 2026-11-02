#pragma once

#include "base/Contract.hpp"
#include "contracts/doomlikeContract/Input.hpp"
#include "contracts/doomlikeContract/Snapshot.hpp"

namespace onyx::contracts::doomlike
{

using Contract = onyx::base::ContractT<Input, Snapshot>;

} // namespace onyx::contracts::doomlike

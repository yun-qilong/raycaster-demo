#pragma once

namespace onyx::base
{

template <typename InputT, typename SnapshotT>
struct ContractT
{
    using Input = InputT;
    using Snapshot = SnapshotT;
};

} // namespace onyx::base

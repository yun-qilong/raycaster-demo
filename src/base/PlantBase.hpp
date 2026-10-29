#pragma once

#include "base/FrameBuffer.hpp"
#include "base/Resolution.hpp"
#include "utils/CrtpBase.hpp"

#include <cstddef>

namespace onyx::base
{

class PlantBaseTag
{
};

template <typename Derived, typename ContractT, typename PixelT, Resolution SelectedResolutionT>
class PlantBase : public ray::utils::CrtpBase<Derived>, public PlantBaseTag
{
    static_assert(static_cast<std::size_t>(SelectedResolutionT) < kResolutionTable.size(),
                  "selected resolution out of range");

  public:
    using Contract = ContractT;
    using Pixel = PixelT;
    using FrameBuffer = FrameBufferT<PixelT, SelectedResolutionT>;

    PlantBase() = default;
    PlantBase(const PlantBase &) = delete;
    PlantBase(PlantBase &&) = delete;
    PlantBase &operator=(const PlantBase &) = delete;
    PlantBase &operator=(PlantBase &&) = delete;

    [[nodiscard]] typename ContractT::Input sampleInput()
    {
        return this->getImplementation().sampleInputImpl();
    }

    void putSnapshot(const typename ContractT::Snapshot &snapshot)
    {
        this->getImplementation().putSnapshotImpl(snapshot);
    }

    [[nodiscard]] const typename ContractT::Snapshot *getSnapshot()
    {
        return this->getImplementation().getSnapshotImpl();
    }

    [[nodiscard]] FrameBuffer requestFrameBuffer()
    {
        return FrameBuffer{this->getImplementation().pixelBufferImpl()};
    }

    void submitFrame()
    {
        this->getImplementation().submitFrameImpl();
    }
};

} // namespace onyx::base

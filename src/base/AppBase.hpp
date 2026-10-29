#pragma once

#include "base/PlantBase.hpp"
#include "utils/CrtpBase.hpp"

#include <type_traits>

namespace onyx::base
{

template <typename Derived, typename PlantT, typename ContractT>
class AppBase : public ray::utils::CrtpBase<Derived>
{
    static_assert(std::is_base_of_v<PlantBaseTag, PlantT>, "PlantT must derive from PlantBase");
    static_assert(std::is_same_v<ContractT, typename PlantT::Contract>,
                  "AppBase contract does not match the plant contract");

  public:
    explicit AppBase(PlantT &plant) noexcept : plant_(plant) {}

    AppBase(const AppBase &) = delete;
    AppBase &operator=(const AppBase &) = delete;
    AppBase(AppBase &&) = delete;
    AppBase &operator=(AppBase &&) = delete;

    void stepLogic()
    {
        const auto input = plant_.sampleInput();
        const auto &snapshot = this->getImplementation().advanceWorld(input);
        plant_.putSnapshot(snapshot);
    }

    void presentFrame()
    {
        const auto *snapshot = plant_.getSnapshot();
        if (snapshot == nullptr)
        {
            return;
        }

        auto frameBuffer = plant_.requestFrameBuffer();
        this->getImplementation().drawFrame(*snapshot, frameBuffer);
        plant_.submitFrame();
    }

  private:
    PlantT &plant_;
};

} // namespace onyx::base

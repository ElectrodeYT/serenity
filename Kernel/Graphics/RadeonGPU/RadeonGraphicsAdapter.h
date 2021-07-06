#pragma once

#include <Kernel/Graphics/RadeonGPU/RadeonGPU.h>
#include <Kernel/Graphics/VGACompatibleAdapter.h>

namespace Kernel::Graphics {

class RadeonGraphicsAdapter final : public VGACompatibleAdapter {
AK_MAKE_ETERNAL
public:
    static RefPtr<RadeonGraphicsAdapter> initialize(PCI::Address addr);

private:
    RadeonGraphicsAdapter(PCI::Address addr);

    RefPtr<RadeonGPUBase> m_gpu;
};

}
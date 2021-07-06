#include <Kernel/Graphics/RadeonGPU/RadeonGraphicsAdapter.h>
#include <Kernel/Graphics/RadeonGPU/Evergreen.h>

namespace Kernel::Graphics {

RefPtr<RadeonGraphicsAdapter> RadeonGraphicsAdapter::initialize(PCI::Address addr) {
    // Check if the GPu is supported
    PCI::ID id = PCI::get_id(addr);
    dbgln("RadeonGraphicsAdapter: pci id: {:x}", id.device_id);
    switch(static_cast<SupportedGPUDeviceIDs>(id.device_id)) {
    case BartsXT:
        break;
    default:
        return {};
    }
    return RefPtr<RadeonGraphicsAdapter>(RadeonGraphicsAdapter(addr));
}

RadeonGraphicsAdapter::RadeonGraphicsAdapter(PCI::Address addr)
    : VGACompatibleAdapter(addr) {
    // We need to assign the chip familiy and also figure out which Generation
    // Handler we need to create.
    PCI::ID id = PCI::get_id(addr);
    switch(static_cast<SupportedGPUDeviceIDs>(id.device_id)) {
    case BartsXT:
        m_gpu = EvergreenGPU::initialize(addr, GPUChipFamilies::CHIP_BARTS);
        break;
    default:
        TODO();
    }
    TODO();
}


}
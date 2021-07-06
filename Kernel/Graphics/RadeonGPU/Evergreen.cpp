#include <Kernel/Graphics/RadeonGPU/Evergreen.h>

namespace Kernel::Graphics {

EvergreenGPU::ResetState EvergreenGPU::getSoftResetState()
{
    u32 grbmStatus = mmioReadRegister(GRBM_STATUS);
    ResetState ret;
    if (grbmStatus & (PA_BUSY | SC_BUSY | SH_BUSY | SX_BUSY | TA_BUSY | VGT_BUSY | DB_BUSY | CB_BUSY | SPI_BUSY | VGT_BUSY_NO_DMA)) {
        ret.GFX = true;
        dbgln("EvergreenGPU: getSoftResetState(): grbmStatus GFX");
    }
    if (grbmStatus & (CF_RQ_PENDING | PF_RQ_PENDING)) {
        ret.CP = true;
        dbgln("EvergreenGPU: getSoftResetState(): grbmStatus CP");
    }
    if (grbmStatus & GRBM_EE_BUSY) {
        ret.GRBM = true;
        ret.GFX = true;
        ret.CP = true;
        dbgln("EvergreenGPU: getSoftResetState(): grbmStatus GRBM, GFX, CP");
    }

    u32 dmaStatusRegister = mmioReadRegister(DMA_STATUS_REG);
    if (!(dmaStatusRegister & DMA_IDLE)) {
        ret.DMA = true;
        dbgln("EvergreenGPU: getSoftResetState(): dmaStatusRegister DMA");
    }

    u32 srbmStatus2 = mmioReadRegister(SRBM_STATUS2);
    if (srbmStatus2 & DMA_BUSY) {
        ret.DMA = true;
        dbgln("EvergreenGPU: getSoftResetState(): srbmStatus2 DMA");
    }

    u32 srbmStatus = mmioReadRegister(SRBM_STATUS);
    if (srbmStatus & (RLC_RQ_PENDING | RLC_BUSY)) {
        ret.RLC = true;
        dbgln("EvergreenGPU: getSoftResetState(): srbmStatus RLC");
    }
    if (srbmStatus & IH_BUSY) {
        ret.IH = true;
        dbgln("EvergreenGPU: getSoftResetState(): srbmStatus IH");
    }
    if (srbmStatus & SEM_BUSY) {
        ret.SEM = true;
        dbgln("EvergreenGPU: getSoftResetState(): srbmStatus SEM");
    }
    if (srbmStatus & GRBM_RQ_PENDING) {
        ret.GRBM = true;
        dbgln("EvergreenGPU: getSoftResetState(): srbmStatus GRBM");
    }
    if (srbmStatus & VMC_BUSY) {
        ret.VMC = true;
        dbgln("EvergreenGPU: getSoftResetState(): srbmStatus VMC");
    }
    if (srbmStatus & (MCB_BUSY | MCB_NON_DISPLAY_BUSY | MCC_BUSY | MCD_BUSY)) {
        dbgln("EvergreenGPU: getSoftResetState(): MC?");
        // ret.MC = true;
    }

    if (displayHung()) {
        ret.Display = true;
        dbgln("EvergreenGPU: getSoftResetState(): Display");
    }

    u32 vmL2Status = mmioReadRegister(VM_L2_STATUS);
    if (vmL2Status & L2_BUSY) {
        ret.VMC = true;
        dbgln("EvergreenGPU: getSoftResetState(): L2_BUSY VMC");
    }

    return ret;
}

}
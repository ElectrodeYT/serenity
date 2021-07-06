#pragma once

#include <Kernel/Graphics/RadeonGPU/RadeonGPU.h>
#include <Kernel/Graphics/RadeonGPU/RadeonDriverHeaders/evergreend.h>
#include <Kernel/Graphics/RadeonGPU/RadeonDriverHeaders/evergreen_reg.h>
#include <Kernel/Graphics/RadeonGPU/RadeonDriverHeaders/avivod.h>
#include <Kernel/Bus/PCI/Access.h>

namespace Kernel::Graphics {

// Golden register array's for Evergreen.
// I honestly wish i knew what this did, but I do not.
// Taken from the radeon linux driver
static const u32 BartsGoldenRegisters[] = {
    0x5eb4, 0xffffffff, 0x00000002,
    0x5e78, 0x8f311ff1, 0x001000f0,
    0x3f90, 0xffff0000, 0xff000000,
    0x9148, 0xffff0000, 0xff000000,
    0x3f94, 0xffff0000, 0xff000000,
    0x914c, 0xffff0000, 0xff000000,
    0xc78, 0x00000080, 0x00000080,
    0xbd4, 0x70073777, 0x00010001,
    0xd02c, 0xbfffff1f, 0x08421000,
    0xd0b8, 0x03773777, 0x02011003,
    0x5bc0, 0x00200000, 0x50100000,
    0x98f8, 0x33773777, 0x02011003,
    0x98fc, 0xffffffff, 0x76543210,
    0x7030, 0x31000311, 0x00000011,
    0x2f48, 0x00000007, 0x02011003,
    0x6b28, 0x00000010, 0x00000012,
    0x7728, 0x00000010, 0x00000012,
    0x10328, 0x00000010, 0x00000012,
    0x10f28, 0x00000010, 0x00000012,
    0x11b28, 0x00000010, 0x00000012,
    0x12728, 0x00000010, 0x00000012,
    0x240c, 0x000007ff, 0x00000380,
    0x8a14, 0xf000001f, 0x00000007,
    0x8b24, 0x3fff3fff, 0x00ff0fff,
    0x8b10, 0x0000ff0f, 0x00000000,
    0x28a4c, 0x07ffffff, 0x06000000,
    0x10c, 0x00000001, 0x00010003,
    0xa02c, 0xffffffff, 0x0000009b,
    0x913c, 0x0000000f, 0x0100000a,
    0x8d00, 0xffff7f7f, 0x100e4848,
    0x8d04, 0x00ffffff, 0x00164745,
    0x8c00, 0xfffc0003, 0xe4000003,
    0x8c04, 0xf8ff00ff, 0x40600060,
    0x8c08, 0x00ff00ff, 0x001c001c,
    0x8cf0, 0x1fff1fff, 0x08e00620,
    0x8c20, 0x0fff0fff, 0x00800080,
    0x8c24, 0x0fff0fff, 0x00800080,
    0x8c18, 0xffffffff, 0x20202078,
    0x8c1c, 0x0000ffff, 0x00001010,
    0x28350, 0x00000f01, 0x00000000,
    0x9508, 0x3700001f, 0x00000002,
    0x960c, 0xffffffff, 0x54763210,
    0x88c4, 0x001f3ae3, 0x000000c2,
    0x88d4, 0x0000001f, 0x00000010,
    0x8974, 0xffffffff, 0x00000000
};

// Evergreen GPU class.
// A lot of this is almost a direct copy from various functions in the radeon code.
// If a function is a reimplementation from the radeon driver, I have commented
// the function it is based on
// TODO: get rid of evergreend.h
class EvergreenGPU final : public RadeonGPUBase {
public:
    static RefPtr<EvergreenGPU> initialize(PCI::Address addr, GPUChipFamilies family)
    {
        return RefPtr<EvergreenGPU>(EvergreenGPU(addr, family));
    }

    // evergreen_is_display_hung
    bool displayHung()
    {
        u32 crtcHungState = 0;
        u32 crtcBeginHV[6];

        // Check if the CRTC is active and if it is, get current HV count
        for (size_t i = 0; i < m_crtcCount; i++) {
            if (mmioReadRegister(EVERGREEN_CRTC_CONTROL + m_crtcRegisterOffsets[i] + EVERGREEN_CRTC_MASTER_EN)) {
                crtcBeginHV[i] = mmioReadRegister(EVERGREEN_CRTC_STATUS_HV_COUNT + m_crtcRegisterOffsets[i]);
                crtcHungState |= 1 << i;
            }
        }

        // Loop through them a bit and for all active CRTCs check if their HV value has changed
        for (size_t loops = 0; loops < 10; loops++) {
            for (size_t i = 0; i < m_crtcCount; i++) {
                if (crtcHungState & (1 << i)) {
                    u32 currentHV = mmioReadRegister(EVERGREEN_CRTC_STATUS_HV_COUNT + m_crtcRegisterOffsets[i]);
                    if (currentHV != crtcBeginHV[i]) {
                        crtcHungState &= ~(1 << i);
                    }
                }
            }
            // If crtcHungState is 0, then we can return false as no crtc's are hung
            if (crtcHungState == 0) {
                return false;
            }
            IO::delay(100);
        }
        return true;
    }

    // evergreen_gpu_check_soft_reset
    virtual ResetState getSoftResetState() override;

    struct savedMCRegisters {
        u32 vgaRenderControl;
        u32 vgaHDPControl;
        // TODO: RADEON_MAX_CRTCS
        bool crtcEnabled[6];
    };

    // dce4_is_in_vblank
    virtual bool vblank(size_t crtc) override {
        return mmioReadRegister(EVERGREEN_CRTC_STATUS + m_crtcRegisterOffsets[crtc]) & EVERGREEN_CRTC_V_BLANK;
    }

    // dce4_is_counter_moving
    // TODO: same todo as blockUntilVBlank
    virtual bool isVBlankCounterChanging(size_t crtc) override {
        u32 counter_1 = mmioReadRegister(EVERGREEN_CRTC_STATUS_POSITION + m_crtcRegisterOffsets[crtc]);
        IO::delay(1); // TODO: not in driver, not sure if this could cause a bug
        u32 counter_2 = mmioReadRegister(EVERGREEN_CRTC_STATUS_POSITION + m_crtcRegisterOffsets[crtc]);
        if(counter_1 == counter_2) { dbgln("EvergreenGPU: crtc {:x} is not changing!", crtc); return false; }
        return true;
    }

    // dce4_wait_for_vblank
    // TODO: there are several versions of this and i think evergreen uses several of them, best way to do this
    virtual void blockUntilVBLank(size_t crtc) override {
        VERIFY(crtc < m_crtcCount);
        // Check if the output is enabled and return if it isnt
        if(mmioReadRegister(EVERGREEN_CRTC_CONTROL + m_crtcRegisterOffsets[crtc]) & EVERGREEN_CRTC_MASTER_EN) {
            for(size_t i = 0; vblank(crtc); i++) {
                if(i % 100) {
                    if(!isVBlankCounterChanging(crtc)) { break; }
                }
            }

            // We should be in vblank now, but if we were really, really close, it could have changed by now, so we check if the vblank state has already ended
            // If it did, we wait until its in vblank again
            for(size_t i = 0; !vblank(crtc); i++) {
                if(i % 100) {
                    if(!isVBlankCounterChanging(crtc)) { break; }
                }
            }
        }
    }

    // evergreen_get_vblank_counter
    // I have no idea why this is a seperate function in the radeon driver.
    // This is basically a VERIFY and a read register.
    // TODO: decide wether to inline it or to axe it
    virtual u32 getVBlankCounter(size_t crtc) override {
        VERIFY(crtc < m_crtcCount);
        return mmioReadRegister(CRTC_STATUS_FRAME_COUNT + m_crtcRegisterOffsets[crtc]);
    }

    // evergreen_mc_wait_for_idle
    // Returns true if it is idle, false if it timed out.
    bool blockUntilMCisIdle() {
        const u32 timeout = 1000;
        for(size_t i = 0; i < timeout; i++) {
            u32 srbmStatus = mmioReadRegister(SRBM_STATUS);
            // Check MC_STATUS
            // For some reason there is no define for this, the radeon driver also hard codes it
            if(!(srbmStatus & 0x1F00)) {
                return true;
            }
            IO::delay(1);
        }
        return false;
    }

    // evergreen_mc_stop
    // Stop the memory controller
    savedMCRegisters stopMC() {
        savedMCRegisters ret;
        if(!isNoDCEAsic()) {
            ret.vgaRenderControl = mmioReadRegister(VGA_RENDER_CONTROL);
            ret.vgaHDPControl = mmioReadRegister(VGA_HDP_CONTROL);
            // Disable VGA render
            mmioWriteRegister(VGA_RENDER_CONTROL, 0);
        }

        // Black display controllers
        for(size_t i = 0; i < m_crtcCount; i++) {
            if(mmioReadRegister(EVERGREEN_CRTC_CONTROL + m_crtcRegisterOffsets[i]) & EVERGREEN_CRTC_MASTER_EN) {
                ret.crtcEnabled[i] = true;
                if(isDCE6Asic()) {
                    u32 crtcBlankControl = mmioReadRegister(EVERGREEN_CRTC_BLANK_CONTROL + m_crtcRegisterOffsets[i]);
                    if(!(crtcBlankControl & EVERGREEN_CRTC_BLANK_DATA_EN)) {
                        blockUntilVBLank(i);
                        mmioWriteRegister(EVERGREEN_CRTC_UPDATE_LOCK + m_crtcRegisterOffsets[i], 1);
                        crtcBlankControl |= EVERGREEN_CRTC_BLANK_DATA_EN;
                        mmioWriteRegister(EVERGREEN_CRTC_BLANK_CONTROL + m_crtcRegisterOffsets[i], crtcBlankControl);
                        mmioWriteRegister(EVERGREEN_CRTC_UPDATE_LOCK + m_crtcRegisterOffsets[i], 0);
                    }
                } else {
                    u32 crtcControl = mmioReadRegister(EVERGREEN_CRTC_CONTROL + m_crtcRegisterOffsets[i]);
                    if(!(crtcControl & EVERGREEN_CRTC_DISP_READ_REQUEST_DISABLE)) {
                        blockUntilVBLank(i);
                        mmioWriteRegister(EVERGREEN_CRTC_UPDATE_LOCK + m_crtcRegisterOffsets[i], 1);
                        crtcControl |= EVERGREEN_CRTC_DISP_READ_REQUEST_DISABLE;
                        mmioWriteRegister(EVERGREEN_CRTC_CONTROL + m_crtcRegisterOffsets[i], crtcControl);
                        mmioWriteRegister(EVERGREEN_CRTC_UPDATE_LOCK + m_crtcRegisterOffsets[i], 0);
                    }
                }

                // Block until the next frame
                u32 current_frame_count = getVBlankCounter(i);
                const size_t vblank_counter_timeout = 1000;
                for(size_t timeout = 0; timeout < vblank_counter_timeout; timeout++) {
                    u32 new_frame_count = getVBlankCounter(i);
                    if(current_frame_count != new_frame_count) { break; }
                    IO::delay(1);
                }
                // TODO: the radeon driver does not crash or report a error here, should we?

                // TODO: blank display port outputs
            }
        }
        return ret;
    }

    // evergreen_asic_reset
    // Reset the ASIC.
    // The driver has a hard reset flag, but im not sure thats needed, and anyway, that
    // should be a separate function anyway
    virtual void doAsicReset() override {
        // Get current reset State
        ResetState softResetState = getSoftResetState();

        // Disable CP parsing and prefetching
        mmioWriteRegister(CP_ME_CNTL, CP_ME_HALT | CP_PFP_HALT);

        if(softResetState.DMA) {
            // Disable DMA
            mmioWriteRegister(DMA_RB_CNTL, mmioReadRegister(DMA_RB_CNTL) & ~DMA_RB_ENABLE);
        }
        IO::delay(50);

        savedMCRegisters saved_mc_registers = stopMC();
        (void)saved_mc_registers;
        VERIFY(blockUntilMCisIdle());

        // Oh nice ginglies we get to create a giagantic fucking wall of values
        u32 grbmSoftReset = 0;
        u32 srbmSoftReset = 0;

        // TODO: we never set nor do we have a Compute state, and the driver also does not get this, so dead code?
        if(softResetState.GFX) {
            grbmSoftReset |= SOFT_RESET_DB;
            grbmSoftReset |= SOFT_RESET_CB;
            grbmSoftReset |= SOFT_RESET_PA;
            grbmSoftReset |= SOFT_RESET_SC;
            grbmSoftReset |= SOFT_RESET_SPI;
            grbmSoftReset |= SOFT_RESET_SX;
            grbmSoftReset |= SOFT_RESET_SH;
            grbmSoftReset |= SOFT_RESET_TC;
            grbmSoftReset |= SOFT_RESET_TA;
            grbmSoftReset |= SOFT_RESET_VC;
            grbmSoftReset |= SOFT_RESET_VGT;
        }

        if(softResetState.CP) {
            grbmSoftReset |= SOFT_RESET_CP;
            grbmSoftReset |= SOFT_RESET_DB;

            srbmSoftReset |= SOFT_RESET_GRBM;
        }

        if(softResetState.DMA) {
            srbmSoftReset |= SOFT_RESET_DMA;
        }
        if(softResetState.Display) {
            srbmSoftReset |= SOFT_RESET_DC;
        }
        if(softResetState.RLC) {
            srbmSoftReset |= SOFT_RESET_RLC;
        }
        if(softResetState.SEM) {
            srbmSoftReset |= SOFT_RESET_SEM;
        }
        if(softResetState.IH) {
            srbmSoftReset |= SOFT_RESET_IH;
        }
        if(softResetState.GRBM) {
            srbmSoftReset |= SOFT_RESET_GRBM;
        }
        if(softResetState.VMC) {
            srbmSoftReset |= SOFT_RESET_VMC;
        }

        // Set the registers
        if(grbmSoftReset) {
            u32 new_grbmSoftReset = mmioReadRegister(GRBM_SOFT_RESET);
            new_grbmSoftReset |= grbmSoftReset;
            dbgln("EvergreenGPU: soft reset: new grbmSoftReset: {:x}", new_grbmSoftReset);
            mmioWriteRegister(GRBM_SOFT_RESET, new_grbmSoftReset);
            IO::delay(50);

            // Reset the registers
            new_grbmSoftReset &= ~grbmSoftReset;
            mmioWriteRegister(GRBM_SOFT_RESET, new_grbmSoftReset);
        }

        if(srbmSoftReset) {
            u32 new_srbmSoftReset = mmioReadRegister(SRBM_SOFT_RESET);
            new_srbmSoftReset |= srbmSoftReset;
            dbgln("EvergreenGPU: soft reset: new srbmSoftReset: {:x}", new_srbmSoftReset);
            mmioWriteRegister(SRBM_SOFT_RESET, new_srbmSoftReset);
            IO::delay(50);

            // Reset the registers
            new_srbmSoftReset &= ~srbmSoftReset;
            mmioWriteRegister(SRBM_SOFT_RESET, new_srbmSoftReset);
        }

        IO::delay(50);


    }

private:
    static constexpr u32 m_crtcRegisterOffsets[] = {
        EVERGREEN_CRTC0_REGISTER_OFFSET,
        EVERGREEN_CRTC1_REGISTER_OFFSET,
        EVERGREEN_CRTC2_REGISTER_OFFSET,
        EVERGREEN_CRTC3_REGISTER_OFFSET,
        EVERGREEN_CRTC4_REGISTER_OFFSET,
        EVERGREEN_CRTC5_REGISTER_OFFSET,
    };

    EvergreenGPU(PCI::Address addr, GPUChipFamilies family)
        : RadeonGPUBase(addr, family)
    {
        dbgln("EvergreenGPU: beginning Evergreen init");


            TODO();
        // Reset the chip now
        doAsicReset();
        // Init golden registers
        switch (family) {
        case GPUChipFamilies::CHIP_BARTS:
            loadGoldenRegisterArray(&(BartsGoldenRegisters[0]), sizeof(BartsGoldenRegisters));
            break;
        default:
            TODO();
        }
    }
};

}
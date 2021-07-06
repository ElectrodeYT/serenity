#pragma once

#include <Kernel/Devices/BlockDevice.h>
#include <Kernel/IO.h>
#include <Kernel/Bus/PCI/Definitions.h>
#include <Kernel/Bus/PCI/Access.h>
#include <Kernel/Graphics/RadeonGPU/RadeonDriverHeaders/atom.h>

namespace Kernel::Graphics {

// Supported GPU Device IDs
enum SupportedGPUDeviceIDs {
    BartsXT = 0x6738
};

// Chip families.
// List was copied from the linux radeon driver
// It must be in this order as several checks are performed for various
// things.
// TODO: figure out how many of these we need, rewrite them to be more c++-like
enum GPUChipFamilies {
    CHIP_R100 = 0,
    CHIP_RV100,
    CHIP_RS100,
    CHIP_RV200,
    CHIP_RS200,
    CHIP_R200,
    CHIP_RV250,
    CHIP_RS300,
    CHIP_RV280,
    CHIP_R300,
    CHIP_R350,
    CHIP_RV350,
    CHIP_RV380,
    CHIP_R420,
    CHIP_R423,
    CHIP_RV410,
    CHIP_RS400,
    CHIP_RS480,
    CHIP_RS600,
    CHIP_RS690,
    CHIP_RS740,
    CHIP_RV515,
    CHIP_R520,
    CHIP_RV530,
    CHIP_RV560,
    CHIP_RV570,
    CHIP_R580,
    CHIP_R600,
    CHIP_RV610,
    CHIP_RV630,
    CHIP_RV670,
    CHIP_RV620,
    CHIP_RV635,
    CHIP_RS780,
    CHIP_RS880,
    CHIP_RV770,
    CHIP_RV730,
    CHIP_RV710,
    CHIP_RV740,
    CHIP_CEDAR,
    CHIP_REDWOOD,
    CHIP_JUNIPER,
    CHIP_CYPRESS,
    CHIP_HEMLOCK,
    CHIP_PALM,
    CHIP_SUMO,
    CHIP_SUMO2,
    CHIP_BARTS,
    CHIP_TURKS,
    CHIP_CAICOS,
    CHIP_CAYMAN,
    CHIP_ARUBA,
    CHIP_TAHITI,
    CHIP_PITCAIRN,
    CHIP_VERDE,
    CHIP_OLAND,
    CHIP_HAINAN,
    CHIP_BONAIRE,
    CHIP_KAVERI,
    CHIP_KABINI,
    CHIP_HAWAII,
    CHIP_MULLINS,
    CHIP_LAST,
};

struct AtomExecutionContext {

    void ExecuteTable(u32* pointer);
};

class RadeonGPUBase : public RefCounted<RadeonGPUBase> {
public:
    virtual ~RadeonGPUBase() = default;

    const u32 MM_Index = 0x0;
    const u32 MM_Data = 0x4;

    struct ResetState {
        bool GFX : 1;
        bool CP : 1;
        bool GRBM : 1;
        bool DMA : 1;
        bool RLC : 1;
        bool IH : 1;
        bool SEM : 1;
        bool VMC : 1;
        bool MC : 1;
        bool Display : 1;
    };

    // CRTC stuff
    virtual bool vblank(size_t crtc) = 0;
    virtual bool isVBlankCounterChanging(size_t crtc) = 0;
    virtual void blockUntilVBLank(size_t crtc) = 0;
    virtual u32 getVBlankCounter(size_t crtc) = 0;

    // Reset stuff
    virtual ResetState getSoftResetState() = 0;
    virtual void doAsicReset() = 0;

    inline void mmioWriteRegister(u32 reg, u32 data)
    {
        *(reinterpret_cast<u32*>(m_rmmio_region->vaddr().as_ptr()) + MM_Index) = reg;
        full_memory_barrier();
        *(reinterpret_cast<u32*>(m_rmmio_region->vaddr().as_ptr()) + MM_Data) = data;
        full_memory_barrier();
    }
    inline u32 mmioReadRegister(u32 reg)
    {
        *(reinterpret_cast<u32*>(m_rmmio_region->vaddr().as_ptr()) + MM_Index) = reg;
        full_memory_barrier();
        u32 ret = *(reinterpret_cast<u32*>(m_rmmio_region->vaddr().as_ptr()) + MM_Data);
        full_memory_barrier();
        return ret;
    }

    inline void rioWriteRegister(u32 reg, u32 data)
    {
        if (reg < m_rio_size) {
            IO::out32(m_rio + reg, data);
        } else {
            TODO();
        }
    }
    inline u32 rioReadRegister(u32 reg)
    {
        if (reg < m_rio_size) {
            return IO::in32(m_rio + reg);
        } else {
            TODO();
        }
    }

    inline u8 biosRead8(u32 offset) {
        return *(reinterpret_cast<u8*>(m_option_rom->vaddr().as_ptr() + offset));
    }
    inline u16 biosRead16(u32 offset) {
        return biosRead8(offset) | (biosRead8(offset + 1) << 8);
    }
    inline u32 biosRead32(u32 offset) {
        return biosRead16(offset) | (biosRead16(offset + 2) << 16);
    }

    // Another Radeon ASIC thing i dont truly understand
    // These Defines are commented with "ASICs helpers." in the radeon source code.
    // They seme to define which ASIC is used for what GPU Family(s)
    inline bool isRN50Asic() { TODO(); return false; }
    inline bool isRV100Asic() { TODO(); return false; }
    inline bool isR300Asic() { TODO(); return false; }
    inline bool isX2Asic() { TODO(); return false; }
    inline bool isAvivoAsic() { TODO(); return false; }
    inline bool isDCE2Asic() { TODO(); return false; }
    inline bool isDCE3Asic() { TODO(); return false; }
    inline bool isDCE32Asic() { TODO(); return false; }
    inline bool isDCE4Asic() { TODO(); return false; }
    inline bool isDCE41Asic() { TODO(); return false; }
    inline bool isDCE5Asic() { TODO(); return false; }
    inline bool isDCE6Asic() { return m_familiy >= GPUChipFamilies::CHIP_ARUBA; }
    inline bool isDCE61Asic() { TODO(); return false; }
    inline bool isDCE64Asic() { TODO(); return false; }
    inline bool isNoDCEAsic() { return m_familiy == GPUChipFamilies::CHIP_HAINAN;  }
    inline bool isDCE8Asic() { TODO(); return false; }
    inline bool isDCE81Asic() { TODO(); return false; }
    inline bool isDCE82Asic() { TODO(); return false; }
    inline bool isDCE83Asic() { TODO(); return false; }
    inline bool isLombokAsic() { TODO(); return false; }


    RadeonGPUBase(PCI::Address addr, GPUChipFamilies family)
    : m_familiy(family)
    {
        // Set m_crtcCount
        switch (family) {
        case GPUChipFamilies::CHIP_BARTS:
        case GPUChipFamilies::CHIP_TURKS:
            m_crtcCount = 6;
            break;
        case GPUChipFamilies::CHIP_CAICOS:
            m_crtcCount = 4;
            break;
        default:
            TODO();
        }

        // Map the register region
        // GPUs before Bonaire used BAR2, but Bonaire and later use BAR5
        if (family >= GPUChipFamilies::CHIP_BONAIRE) {
            dbgln("RadeonGPUBase: Using BAR5 as RMMIO Region");
            m_rmmio_region = MM.allocate_kernel_region(PhysicalAddress(PCI::get_BAR5(addr) & ~0xF), PCI::get_BAR_space_size(addr, 5), "Radeon GPU MMIO Registers", Region::Access::Read | Region::Access::Write);
        } else {
            dbgln("RadeonGPUBase: Using BAR2 as RMMIO Region");
            m_rmmio_region = MM.allocate_kernel_region(PhysicalAddress(PCI::get_BAR2(addr) & ~0xF), PCI::get_BAR_space_size(addr, 2), "Radeon GPU MMIO Registers", Region::Access::Read | Region::Access::Write);
        }
        dbgln("RadeonGPUBase: mapped rmmio region, base {}, size {:x}", m_rmmio_region->physical_page(0)->paddr(), m_rmmio_region->size());
        // TODO: doorbell init
        // we dont support chips >= bonaire though
        // Find and map RIO region
        for (u8 i = 0; i < 5; i++) {
            u32 curr_bar = PCI::get_BAR(addr, i);
            if (curr_bar & 1) {
                // BAR maps into IO space, it is RIO
                m_rio = curr_bar & ~0xF;
                m_rio_size = PCI::get_BAR_space_size(addr, i);
                break;
            }
        }
        VERIFY(m_rio);
        VERIFY(m_rio_size);
        VERIFY(m_crtcCount);
        dbgln("RadeonGPUBase: found rio region; base {:x}, size {:x}", m_rio, m_rio_size);

        // Try to read the ATOMBios
        u32 option_rom_pointer = PCI::Access::the().read32_field(addr, 0x30);
        dbgln("RadeonGPUBase: Option ROM Pointer: {:x}", option_rom_pointer);
        // We now write to it to figure out the mapping size
        PCI::Access::the().write32_field(addr, 0x30, 0xFFFFF800);
        // Now we read it back
        u32 option_rom_pointer_after_write = PCI::Access::the().read32_field(addr, 0x30);
        dbgln("RadeonGPUBase: Option ROM Pointer after write: {:x}", option_rom_pointer_after_write);
        // We can now calculate the PCI window by ANDing FFFFF800 and the read value, then inverting it + 1
        u32 option_rom_mapping_size = ~(0xFFFFF800 & option_rom_pointer_after_write) + 1;
        // Just in case, we write the original value back
        PCI::Access::the().write32_field(addr, 0x30, option_rom_pointer | 1);
        dbgln("RadeonGPUBase: Option ROM PCI Space size: {:x}", option_rom_mapping_size);

        PCI::enable_memory_space(addr);

        // Now we can map it!
        m_option_rom = MM.allocate_kernel_region(PhysicalAddress(option_rom_pointer), page_round_up(option_rom_mapping_size), "Radeon GPU BIOS", Region::Access::Read);
        u8* rom_pointer = m_option_rom->vaddr().as_ptr();
        dbgln("RadeonGPUBase: First few bytes of rom: {:x} {:x} {:x} {:x}", rom_pointer[0], rom_pointer[1], rom_pointer[2], rom_pointer[3]);
        VERIFY(rom_pointer[0] == 0x55);
        VERIFY(rom_pointer[1] == 0xAA);

        // We can now try and find the PCI Data structure format
        u16 pci_data_structure_offset = rom_pointer[0x18] | (rom_pointer[0x19] << 8);
        dbgln("RadeonGPUBase: pci_data_structure_offset: {:x}", pci_data_structure_offset);
        u8* pci_data_structure = rom_pointer + pci_data_structure_offset;
        dbgln("RadeonGPUBase: Should be PCIR: {} {} {} {}", pci_data_structure[0], pci_data_structure[1], pci_data_structure[2], pci_data_structure[3]);
        dbgln("RadeonGPUBase: Should be PCIR: {:c} {:c} {:c} {:c}", pci_data_structure[0] & 0x7F, pci_data_structure[1] & 0x7F, pci_data_structure[2] & 0x7F, pci_data_structure[3] & 0x7F);
        VERIFY(pci_data_structure[0] == 'P');
        VERIFY(pci_data_structure[1] == 'C');
        VERIFY(pci_data_structure[2] == 'I');
        VERIFY(pci_data_structure[3] == 'R');

        // Check if the ROM is for x86
        VERIFY(biosRead8(biosRead16(0x18) + 0x14) == 0);

        // Get the BIOS Header start
        m_bios_header_start = biosRead16(0x48);
        VERIFY(m_bios_header_start);

        // Check if the ROM is a ATOMBios
        u32 bios_tag = biosRead32(m_bios_header_start + 4);
        VERIFY(bios_tag == 0x41544f4d || bios_tag == 0x4d4f5441); // ATOM or MOTA

        dbgln("RadeonGPUBase: Found ATOMBios header");
        dbgln("RadeonGPUBase: ATOMBios header offset: {:x}", m_bios_header_start);

        // Try and read the name string
        u32 atom_bios_name_pointer = biosRead16(m_bios_header_start + ATOM_ROM_MSG_PTR);
        u8 byte_skip_thing = biosRead8(atom_bios_name_pointer);
        // Uhhh... the driver does this and i have no idea why
        while(byte_skip_thing == '\n' || byte_skip_thing == '\r') {
            byte_skip_thing = biosRead8(++atom_bios_name_pointer);
        }

        // The string is not always 0 terminated, so we stop on the first non-text char
        // Read the name
        char name[512];
        size_t rom_name_size;
        for(rom_name_size = 0; rom_name_size < 511; rom_name_size++) {
            name[rom_name_size] = biosRead8(atom_bios_name_pointer + rom_name_size);
            if(name[rom_name_size] < '.' || name[rom_name_size] > 'z') { break; }
        }

        // We got the size, now to read it into the string
        m_atombios_name = AK::String(name, rom_name_size);
        dbgln("RadeonGPUBase: ATOMBios Name string: {}", m_atombios_name);
    }

    // Loads a "golden register" array
    // This is a almost 1 to 1 translation of the linux driver's radeon_program_register_sequence
    // Since i have no access to the register docs, i have quite literally no clue what this does
    // At this point im just hoping it works
    // TODO: figure out a way to not have to do this (pipe dream)
    void loadGoldenRegisterArray(const u32* array, size_t size)
    {
        VERIFY(size % 3 == 0);
        for (size_t i = 0; i < size; i += 3) {
            u32 reg = array[i + 0];
            u32 and_mask = array[i + 1];
            u32 or_mask = array[i + 2];
            if (and_mask == ~0u) {
                mmioWriteRegister(reg, or_mask);
            } else {
                mmioWriteRegister(reg, (mmioReadRegister(reg) & ~and_mask) | or_mask);
            }
        }
    }

protected:
    // Various multi-ASIC variables
    GPUChipFamilies m_familiy; // GPU Familiy
    u32 m_crtcCount { 0 }; // Amount of CRTC registers
    u32 m_internal_mc_mask { 0 }; // Internal MC address mask


    // Option ROM region
    OwnPtr<Region> m_option_rom;
    u32 m_bios_header_start { 0 };

    // ATOMBios stuff
    AK::String m_atombios_name { "" };

    // Register region
    OwnPtr<Region> m_rmmio_region;

    // RIO region
    u32 m_rio;
    u32 m_rio_size;

};

}
//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "rv32.h"
#include "rv32_isa.h"

//-----------------------------------------------------------------
// Defines:
//-----------------------------------------------------------------
#define LOG_INST            (1 << 0)
#define LOG_OPCODES         (1 << 1)
#define LOG_REGISTERS       (1 << 2)
#define LOG_MEM             (1 << 3)
#define LOG_MMU             (1 << 4)

#define DPRINTF(l,a)        do { if (m_trace & l) printf a; } while (0)
#define TRACE_ENABLED(l)    (m_trace & l)
#define INST_STAT(l)

//-----------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------
rv32::rv32(uint32_t baseAddr /*= 0*/, uint32_t len /*= 0*/): cpu()
{
    m_enable_unaligned   = false;
    m_enable_mem_errors  = true;
    m_compliant_csr      = false;
    m_enable_rvm         = true;
    m_enable_rvc         = true;
    m_enable_rva         = true;
    m_enable_mtimecmp    = false;
    m_enable_sbi         = false;

    // Some memory defined
    if (len != 0)
        create_memory(baseAddr, len);

    reset(baseAddr);
}
//-----------------------------------------------------------------
// set_pc: Set PC
//-----------------------------------------------------------------
void rv32::set_pc(uint32_t pc)
{
    m_pc        = pc;
    m_pc_x      = pc;
}
//-----------------------------------------------------------------
// set_register: Set register value
//-----------------------------------------------------------------
void rv32::set_register(int r, uint32_t val)
{
    if (r <= RISCV_REGNO_GPR31)   m_gpr[r] = val;
    else if (r == RISCV_REGNO_PC) m_pc     = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MEPC)) m_csr_mepc = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MCAUSE)) m_csr_mcause = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MSTATUS)) m_csr_msr = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTVEC)) m_csr_mevec = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTVAL)) m_csr_mtval = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIE)) m_csr_mie = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIP)) m_csr_mip = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTIME)) m_csr_mtime = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTIMECMP)) m_csr_mtimecmp = val; // Non-std
    else if (r == (RISCV_REGNO_CSR0 + CSR_MSCRATCH)) m_csr_mscratch = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIDELEG)) m_csr_mideleg = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MEDELEG)) m_csr_medeleg = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SEPC)) m_csr_sepc = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_STVEC)) m_csr_sevec = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SCAUSE)) m_csr_scause = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_STVAL)) m_csr_stval = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SATP)) m_csr_satp = val;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SSCRATCH)) m_csr_sscratch = val;
    else if (r == RISCV_REGNO_PRIV) m_csr_mpriv = val;
    
}
//-----------------------------------------------------------------
// get_register: Get register value
//-----------------------------------------------------------------
uint32_t rv32::get_register(int r)
{
    if (r <= RISCV_REGNO_GPR31)   return m_gpr[r];
    else if (r == RISCV_REGNO_PC) return m_pc;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MEPC)) return m_csr_mepc;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MCAUSE)) return m_csr_mcause;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MSTATUS)) return m_csr_msr;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTVEC)) return m_csr_mevec;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTVAL)) return m_csr_mtval;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIE)) return m_csr_mie;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIP)) return m_csr_mip;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MCYCLE)) return m_csr_mtime;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTIME)) return m_csr_mtime;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MTIMECMP)) return m_csr_mtimecmp; // Non-std
    else if (r == (RISCV_REGNO_CSR0 + CSR_MSCRATCH)) return m_csr_mscratch;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MIDELEG)) return m_csr_mideleg;
    else if (r == (RISCV_REGNO_CSR0 + CSR_MEDELEG)) return m_csr_medeleg;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SEPC)) return m_csr_sepc;
    else if (r == (RISCV_REGNO_CSR0 + CSR_STVEC)) return m_csr_sevec;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SCAUSE)) return m_csr_scause;
    else if (r == (RISCV_REGNO_CSR0 + CSR_STVAL)) return m_csr_stval;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SATP)) return m_csr_satp;
    else if (r == (RISCV_REGNO_CSR0 + CSR_SSCRATCH)) return m_csr_sscratch;
    else if (r == RISCV_REGNO_PRIV) return m_csr_mpriv;

    return 0;
}
//-----------------------------------------------------------------
// reset: Reset CPU state
//-----------------------------------------------------------------
void rv32::reset(uint32_t start_addr)
{
    m_pc        = start_addr;
    m_pc_x      = start_addr;
    m_load_res  = 0;

    for (int i=0;i<REGISTERS;i++)
        m_gpr[i] = 0;

    m_csr_mpriv    = PRIV_MACHINE;
    m_csr_msr      = 0;
    m_csr_mideleg  = 0;
    m_csr_medeleg  = 0;

    m_csr_mepc     = 0;
    m_csr_mie      = 0;
    m_csr_mip      = 0;
    m_csr_mcause   = 0;
    m_csr_mevec    = 0;
    m_csr_mtval    = 0;
    m_csr_mtime    = 0;
    m_csr_mtimecmp = 0;
    m_csr_mtime_ie = false;
    m_csr_mscratch = 0;

    m_csr_sepc     = 0;
    m_csr_sevec    = 0;
    m_csr_scause   = 0;
    m_csr_stval    = 0;
    m_csr_satp     = 0;
    m_csr_sscratch = 0;

    m_fault       = false;
    m_break       = false;
    m_trace       = 0;

    mmu_flush();

    stats_reset();
}
//-----------------------------------------------------------------
// get_opcode: Get instruction from address
//-----------------------------------------------------------------
uint32_t rv32::get_opcode(uint32_t address)
{
    // 2 byte aligned addresses are supported if RVC
    if (m_enable_rvc && (address & 2))
    {
        uint16_t opcode = ifetch16(address);
        return (ifetch16((address+2)) << 16) | opcode;
    }
    else
        return ifetch32(address);
}
//-----------------------------------------------------------------
// mmu_read_word: Read a word from memory
//-----------------------------------------------------------------
int rv32::mmu_read_word(uint32_t address, uint32_t *val)
{
    int m;
    *val = 0;

    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(address))
        {
            mem->read32(address, *val);
            return 1;
        }

    return 0;
}
//-----------------------------------------------------------------
// mmu_flush: Flush cached MMU TLBs
//-----------------------------------------------------------------
void rv32::mmu_flush(void)
{
    for (int i=0;i<MMU_TLB_ENTRIES;i++)
    {
        m_mmu_addr[i] = 0;
        m_mmu_pte[i]  = 0;
    }
}
//-----------------------------------------------------------------
// mmu_walk: Page table walker
//-----------------------------------------------------------------
uint32_t rv32::mmu_walk(uint32_t addr)
{
    int shift = 32 - MMU_VA_BITS;
    uint32_t pte = 0;

    DPRINTF(LOG_MMU, ("MMU: Walk %x\n", addr));

    // the address must be a canonical sign-extended VA_BITS-bit number    
    if (((int32_t)addr << shift >> shift) != (int32_t)addr)
    {
        pte = 0; // Bad alignment....

        error(false, "%08x: PTE Walk - Bad alignment %x\n", m_pc, addr);
    }
    else if ((m_csr_satp & SATP_MODE) == 0) // Bare mode
    {
        pte = PAGE_PRESENT | PAGE_READ | PAGE_WRITE | PAGE_EXEC | ((addr >> MMU_PGSHIFT) << MMU_PGSHIFT);
        DPRINTF(LOG_MMU, ("MMU: MMU not enabled\n"));
    }
    else
    {
        // Fast path lookup in TLBs
        uint32_t tlb_entry = (addr >> MMU_PGSHIFT) & (MMU_TLB_ENTRIES-1);
        uint32_t tlb_match = (addr >> MMU_PGSHIFT);
        if (m_mmu_addr[tlb_entry] == tlb_match && m_mmu_pte[tlb_entry] != 0)
            return m_mmu_pte[tlb_entry];

        uint32_t base = ((m_csr_satp >> SATP_PPN_SHIFT) & SATP_PPN_MASK) * PAGE_SIZE;
        uint32_t asid = ((m_csr_satp >> SATP_ASID_SHIFT) & SATP_ASID_MASK);

        DPRINTF(LOG_MMU, ("MMU: MMU enabled - base 0x%08x\n", base));

        uint32_t i;
        for (i=MMU_LEVELS-1; i >= 0; i--)
        {
            int      ptshift  = i * MMU_PTIDXBITS;
            uint32_t idx      = (addr >> (MMU_PGSHIFT+ptshift)) & ((1<<MMU_PTIDXBITS)-1);
            uint32_t pte_addr = base + (idx * MMU_PTESIZE);

            // Read PTE
            if (!mmu_read_word(pte_addr, &pte))
            {
                DPRINTF(LOG_MMU, ("MMU: Cannot read PTE entry %x\n", pte_addr));
                pte = 0;
                break;
            }

            DPRINTF(LOG_MMU, ("MMU: PTE value = 0x%08x @ 0x%08x\n", pte, pte_addr));

            uint32_t ppn = pte >> PAGE_PFN_SHIFT;

            // Invalid mapping
            if (!(pte & PAGE_PRESENT))
            {
                DPRINTF(LOG_MMU, ("MMU: Invalid mapping %x\n", pte_addr));
                pte = 0;
                break;
            }
            // Next level of page table
            else if (!(pte & (PAGE_READ | PAGE_WRITE | PAGE_EXEC)))
            {
                base = ppn << MMU_PGSHIFT;
                DPRINTF(LOG_MMU, ("MMU: Next level of page table %x\n", base));
            }
            // The actual PTE
            else
            {
                // Keep permission bits
                pte &= PAGE_FLAGS;

                // if this PTE is from a larger PT, fake a leaf
                // PTE so the TLB will work right
                uint64_t vpn   = addr >> MMU_PGSHIFT;
                uint64_t value = (ppn | (vpn & ((((uint64_t)1) << ptshift) - 1))) << MMU_PGSHIFT;

                // Add back in permission bits
                value |= pte;

                assert((value >> 32) == 0);
                pte   = value;

                uint32_t ptd_addr = ((pte >> MMU_PGSHIFT) << MMU_PGSHIFT);

                DPRINTF(LOG_MMU, ("MMU: PTE addr %x (%x)\n", ptd_addr, pte));

                // fault if physical addr is out of range
                if (valid_addr(ptd_addr))
                {
                    DPRINTF(LOG_MMU, ("MMU: PTE entry found %x\n", pte));
                }
                else
                {
                    DPRINTF(LOG_MMU, ("MMU: PTE access out of range %x\n", ((pte >> MMU_PGSHIFT) << MMU_PGSHIFT)));
                    pte = 0;
                    error(false, "%08x: PTE access out of range %x\n", m_pc, addr);
                }

                m_mmu_addr[tlb_entry] = tlb_match;
                m_mmu_pte[tlb_entry]  = pte;
                break;
            }
        }
    }

    return pte;
}
//-----------------------------------------------------------------
// mmu_i_translate: Translate instruction fetch
//-----------------------------------------------------------------
int rv32::mmu_i_translate(uint32_t addr, uint32_t *physical)
{
    bool page_fault = false;

    // Machine - no MMU
    if (m_csr_mpriv > PRIV_SUPER)
    {
        *physical = addr;
        return 1; 
    }
    
    uint32_t pte = mmu_walk(addr);

    // Reserved configurations
    if (((pte & (PAGE_EXEC | PAGE_READ | PAGE_WRITE)) == PAGE_WRITE) ||
        ((pte & (PAGE_EXEC | PAGE_READ | PAGE_WRITE)) == (PAGE_EXEC | PAGE_WRITE)))
    {
        page_fault = true;
    }
    // Supervisor mode
    else if (m_csr_mpriv == PRIV_SUPER)
    {
        // Supervisor attempts to execute user mode page
        if (pte & PAGE_USER)
        {
            error(false, "IMMU: Attempt to execute user page 0x%08x\n", addr);
            page_fault = true;
        }
        // Page not executable
        else if ((pte & (PAGE_EXEC)) != (PAGE_EXEC))
        {
            page_fault = true;
        }
    }
    // User mode
    else
    {
        // User mode page not executable
        if ((pte & (PAGE_EXEC | PAGE_USER)) != (PAGE_EXEC | PAGE_USER))
        {
            page_fault = true;
        }
    }

    if (page_fault)
    {
        *physical      = 0xFFFFFFFF;
        exception(MCAUSE_PAGE_FAULT_INST, addr, addr);
        return 0;
    }

    uint32_t pgoff  = addr & (MMU_PGSIZE-1);
    uint32_t pgbase = pte >> MMU_PGSHIFT << MMU_PGSHIFT;
    uint32_t paddr  = pgbase + pgoff;

    DPRINTF(LOG_MMU, ("IMMU: Lookup VA %x -> PA %x\n", addr, paddr));

    *physical = paddr;
    return 1; 
}
//-----------------------------------------------------------------
// mmu_d_translate: Translate load store
//-----------------------------------------------------------------
int rv32::mmu_d_translate(uint32_t pc, uint32_t addr, uint32_t *physical, int writeNotRead)
{
    bool page_fault = false;
    uint32_t priv = m_csr_mpriv;

    // Modify data access privilege level (allows machine mode to use MMU)
    if (m_csr_msr & SR_MPRV)
        priv = SR_GET_MPP(m_csr_msr);

    // Machine - no MMU
    if (priv > PRIV_SUPER)
    {
        *physical = addr;
        return 1; 
    }

    uint32_t pte = mmu_walk(addr);

    // MXR: Loads from pages marked either readable or executable (R=1 or X=1) will succeed.
    if ((m_csr_msr & SR_MXR) && (pte & PAGE_EXEC))
        pte |= PAGE_READ;

    // Reserved configurations
    if (((pte & (PAGE_EXEC | PAGE_READ | PAGE_WRITE)) == PAGE_WRITE) ||
        ((pte & (PAGE_EXEC | PAGE_READ | PAGE_WRITE)) == (PAGE_EXEC | PAGE_WRITE)))
    {
        page_fault = true;
    }
    // Supervisor mode
    else if (priv == PRIV_SUPER)
    {
        // User page access - super mode access not enabled
        if ((pte & PAGE_USER) && !(m_csr_msr & SR_SUM))
        {
            error(false, "MMU_D: PC=%08x Access %08x - User page access by super\n", pc, addr);
        }
        else if ((writeNotRead  && ((pte & (PAGE_WRITE)) != (PAGE_WRITE))) || 
                 (!writeNotRead && ((pte & (PAGE_READ))  != (PAGE_READ))))
        {
            page_fault = true;
        }
    }
    // User mode
    else
    {
        if ((writeNotRead  && ((pte & (PAGE_WRITE | PAGE_USER)) != (PAGE_WRITE | PAGE_USER))) || 
            (!writeNotRead && ((pte & (PAGE_READ | PAGE_USER))  != (PAGE_READ | PAGE_USER))))
        {
            page_fault = true;
        }
    }

    if (page_fault)
    {
        *physical      = 0xFFFFFFFF;
        exception(writeNotRead ? MCAUSE_PAGE_FAULT_STORE : MCAUSE_PAGE_FAULT_LOAD, pc, addr);
        return 0;
    }

    uint32_t pgoff  = addr & (MMU_PGSIZE-1);
    uint32_t pgbase = pte >> MMU_PGSHIFT << MMU_PGSHIFT;
    uint32_t paddr  = pgbase + pgoff;

    DPRINTF(LOG_MMU, ("DMMU: Lookup VA %x -> PA %x\n", addr, paddr));

    *physical = paddr;
    return 1; 
}
//-----------------------------------------------------------------
// load: Perform a load operation (with optional MMU lookup)
//-----------------------------------------------------------------
int rv32::load(uint32_t pc, uint32_t address, uint32_t *result, int width, bool signedLoad)
{
    uint32_t physical = address;

    // Translate addresses if required
    if (!mmu_d_translate(pc, address, &physical, 0))
        return 0;

    DPRINTF(LOG_MEM, ("LOAD: VA 0x%08x PA 0x%08x Width %d\n", address, physical, width));

    m_stats[STATS_LOADS]++;
    *result = 0;

    // Detect misaligned load
    if (((address & 3) != 0 && width == 4) || ((address & 1) != 0 && width == 2))
    {
        // Support for unaligned loads
        if (m_enable_unaligned)
        {
            int ok = 1;
            for (int j=0;j<width;j++)
            {
                uint32_t tmp_res = 0;
                if (!load(pc, address + j, &tmp_res, 1, false))
                {
                    ok = 0;
                    break;
                }

                *result |= (tmp_res << (8 * j));
            }

            if (width == 2 && signedLoad && ((*result) & (1 << 15)))
                 *result |= 0xFFFF0000;
            if (width == 1 && signedLoad && ((*result) & (1 << 7)))
                 *result |= 0xFFFFFF00;

            return ok;
        }

        exception(MCAUSE_MISALIGNED_LOAD, pc, address);
        return 0;
    }

    // Aligned loads
    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(physical))
        {
            switch (width)
            {
                case 4:
                    mem->read32(physical, *result);
                    break;
                case 2:
                {
                    uint16_t dh = 0;
                    mem->read16(physical, dh);
                    *result |= dh;

                    if (signedLoad && ((*result) & (1 << 15)))
                         *result |= 0xFFFF0000;
                }
                break;
                case 1:
                {
                    uint8_t db = 0;
                    mem->read8(physical + 0, db);
                    *result |= ((uint32_t)db << 0);

                    if (signedLoad && ((*result) & (1 << 7)))
                         *result |= 0xFFFFFF00;
                }
                break;
                default:
                    assert(!"Invalid");
                    break;
            }

            DPRINTF(LOG_MEM, ("LOAD_RESULT: 0x%08x\n",*result));
            return 1;
        }

    if (m_enable_mem_errors)
    {
        exception(MCAUSE_FAULT_LOAD, pc, address);
        return 0;
    }

    error(false, "%08x: Bad memory access 0x%x\n", pc, address);
    return 0;
}
//-----------------------------------------------------------------
// store: Perform a store operation (with optional MMU lookup)
//-----------------------------------------------------------------
int rv32::store(uint32_t pc, uint32_t address, uint32_t data, int width)
{
    uint32_t physical = address;

    // Translate addresses if required
    if (!mmu_d_translate(pc, address, &physical, 1))
        return 0;

    DPRINTF(LOG_MEM, ("STORE: VA 0x%08x PA 0x%08x Value 0x%08x Width %d\n", address, physical, data, width));
    m_stats[STATS_STORES]++;

    // Detect misaligned store
    if (((address & 3) != 0 && width == 4) || ((address & 1) != 0 && width == 2))
    {
        // Support for unaligned stores
        if (m_enable_unaligned)
        {
            int ok = 1;
            for (int j=0;j<width;j++)
            {
                if (!store(pc, address + j, (data >> (j*8)), 1))
                {
                    ok = 0;
                    break;
                }
            }
            return ok;
        }

        exception(MCAUSE_MISALIGNED_STORE, pc, address);
        return 0;
    }    

    // Aligned stores
    for (memory_base *mem = m_memories; mem != NULL; mem = mem->next)
        if (mem->valid_addr(physical))
        {
            switch (width)
            {
                case 4:
                    mem->write32(physical, data);
                    break;
                case 2:
                    mem->write16(physical, data & 0xFFFF);
                    break;
                case 1:
                    mem->write8(physical, data & 0xFF);
                    break;
                default:
                    assert(!"Invalid");
                    break;
            }
            return 1;
        }

    if (m_enable_mem_errors)
    {
        exception(MCAUSE_FAULT_STORE, pc, address);
        return 0;
    }

    error(false, "%08x: Bad memory access 0x%x\n", pc, address);
    return 0;
}
//-----------------------------------------------------------------
// access_csr: Perform CSR access
//-----------------------------------------------------------------
bool rv32::access_csr(uint32_t address, uint32_t data, bool set, bool clr, uint32_t &result)
{
    result = 0;

#define CSR_STD(name, var_name) \
    case CSR_ ##name: \
    { \
        data       &= CSR_ ##name## _MASK; \
        result      = var_name & CSR_ ##name## _MASK; \
        if (set && clr) \
            var_name  = data; \
        else if (set) \
            var_name |= data; \
        else if (clr) \
            var_name &= ~data; \
    } \
    break;

#define CSR_STDS(name, var_name) \
    case CSR_ ##name: \
    { \
        data       &= CSR_ ##name## _MASK; \
        result      = var_name & CSR_ ##name## _MASK; \
        if (set && clr) \
            var_name  = (var_name & ~CSR_ ##name## _MASK) | data; \
        else if (set) \
            var_name |= data; \
        else if (clr) \
            var_name &= ~data; \
    } \
    break;

#define CSR_CONST(name, value) \
    case CSR_ ##name: \
    { \
        result      = value; \
    } \
    break;

    // Apply CSR access permissions
    if (m_compliant_csr)
    {
        // Get permissions required inferred from the CSR address
        uint32_t csr_priv      = (address >> 8) & 0x3;
        uint32_t csr_read_only = ((address >> 10) & 0x3) == 3;

        if (((set || clr) && csr_read_only) || m_csr_mpriv < csr_priv)
        {
            DPRINTF(LOG_INST, ("-> CSR %08x access fault - permission level %d required %d\n", address & 0xFFF, m_csr_mpriv, csr_priv));
            return true;
        }
    }

    uint32_t misa_val = MISA_VALUE;
    misa_val |= m_enable_rvc ? MISA_RVC : 0;
    misa_val |= m_enable_rva ? MISA_RVA : 0;

    // SATP write - flush cached TLBs
    if (((address & 0xFFF) == CSR_SATP) && (set || clr))
        mmu_flush();

    switch (address & 0xFFF)
    {
        //--------------------------------------------------------
        // Simulation control
        //--------------------------------------------------------
        case CSR_DSCRATCH:
        case CSR_SIM_CTRL:
            switch (data & 0xFF000000)
            {
                case CSR_SIM_CTRL_EXIT:
                    stats_dump();
                    printf("Exit code = %d\n", (char)(data & 0xFF));
                    // Abnormal exit
                    if (data & 0xFF)
                        exit(data & 0xFF);
                    else
                        m_stopped = true;
                    break;
                case CSR_SIM_CTRL_PUTC:
                    if (m_console)
                        m_console->putchar(data & 0xFF);
                    else
                        fprintf(stderr, "%c", (data & 0xFF));
                    break;
                case CSR_SIM_CTRL_GETC:
                    if (m_console)
                        result = m_console->getchar();
                    else
                        result = 0;
                    break;
                case CSR_SIM_CTRL_TRACE:
                    enable_trace(data & 0xFF);
                    break;
                case CSR_SIM_PRINTF:
                {
                    uint32_t fmt_addr = m_gpr[10];
                    uint32_t arg1     = m_gpr[11];
                    uint32_t arg2     = m_gpr[12];
                    uint32_t arg3     = m_gpr[13];
                    uint32_t arg4     = m_gpr[14];

                    {
                        uint32_t pte = mmu_walk(fmt_addr);
                        uint32_t pgoff = fmt_addr & (MMU_PGSIZE-1);
                        uint32_t pgbase = pte >> MMU_PGSHIFT << MMU_PGSHIFT;
                        if (pte != 0) fmt_addr = pgbase + pgoff;
                    }

                    char fmt_str[1024];
                    int idx = 0;
                    while (idx < (sizeof(fmt_str)-1))
                    {
                        fmt_str[idx] = read(fmt_addr++);
                        if (fmt_str[idx] == 0)
                            break;
                        idx += 1;
                    }

                    char out_str[1024];
                    sprintf(out_str, fmt_str, arg1, arg2, arg3, arg4);
                    printf("%s",out_str);
                }
                break;
            }
         break;
        //--------------------------------------------------------
        // Standard - Machine
        //--------------------------------------------------------
        CSR_STD(MEPC,    m_csr_mepc)
        CSR_STD(MTVEC,   m_csr_mevec)
        CSR_STD(MTVAL,   m_csr_mtval)
        CSR_STD(MCAUSE,  m_csr_mcause)
        CSR_STD(MSTATUS, m_csr_msr)
        CSR_STD(MIP,     m_csr_mip)
        CSR_STD(MIE,     m_csr_mie)
        CSR_STD(MISA,    misa_val)
        CSR_STD(MIDELEG, m_csr_mideleg)
        CSR_STD(MEDELEG, m_csr_medeleg)
        CSR_STD(MSCRATCH,m_csr_mscratch)
        CSR_CONST(MHARTID,  MHARTID_VALUE)
        //--------------------------------------------------------
        // Standard - Supervisor
        //--------------------------------------------------------
        CSR_STD(SEPC,    m_csr_sepc)
        CSR_STD(STVEC,   m_csr_sevec)
        CSR_STD(SCAUSE,  m_csr_scause)
        CSR_STDS(SIP,    m_csr_mip)
        CSR_STDS(SIE,    m_csr_mie)
        CSR_STD(SATP,    m_csr_satp)
        CSR_STD(STVAL,   m_csr_stval)
        CSR_STD(SSCRATCH,m_csr_sscratch)
        CSR_STDS(SSTATUS, m_csr_msr)
        //--------------------------------------------------------
        // Extensions
        //-------------------------------------------------------- 
        CSR_CONST(PMPCFG0, 0)
        CSR_CONST(PMPCFG1, 0)
        CSR_CONST(PMPCFG2, 0)
        CSR_CONST(PMPADDR0, 0)
        case CSR_MTIME:
            result      = m_csr_mtime;
            break;
        case CSR_MTIMEH:
            result      = m_csr_mtime >> 32;
            break;
       case CSR_MCYCLE:
            result      = m_csr_mtime;
            break;

        // Non-std behaviour
        case CSR_MTIMECMP:
            data       &= CSR_MTIMECMP_MASK;
            result      = m_csr_mtimecmp & CSR_MTIMECMP_MASK;
            if (set && clr)
                m_csr_mtimecmp  = data;
            else if (set)
                m_csr_mtimecmp |= data;
            else if (clr)
                m_csr_mtimecmp &= ~data;

            // Enable mtimecmp interrupt
            if (set || clr)
                m_csr_mtime_ie = true;

            m_enable_mtimecmp = true;
            break;

        default:
            error(false, "*** CSR address not supported %08x [PC=%08x]\n", address, m_pc);
            break;
    }

    m_enable_rvc = (misa_val & MISA_RVC) ? true : false;
    m_enable_rva = (misa_val & MISA_RVA) ? true : false;

    return false;
}
//-----------------------------------------------------------------
// exception: Handle an exception or interrupt
//-----------------------------------------------------------------
void rv32::exception(uint32_t cause, uint32_t pc, uint32_t badaddr /*= 0*/)
{
    uint32_t deleg;
    uint32_t bit;

    // Interrupt
    if (cause >= MCAUSE_INTERRUPT)
    {
        deleg = m_csr_mideleg;
        bit   = 1 << (cause - MCAUSE_INTERRUPT);
    }
    // Exception
    else
    {
        deleg = m_csr_medeleg;
        bit   = 1 << cause;
    }

    // Exception delegated to supervisor mode
    if (m_csr_mpriv <= PRIV_SUPER && (deleg & bit))
    {
        uint32_t s = m_csr_msr;

        // Interrupt save and disable
        s &= ~SR_SPIE;
        s |= (s & SR_SIE) ? SR_SPIE : 0;
        s &= ~SR_SIE;

        // Record previous priviledge level
        s &= ~SR_SPP;
        s |= (m_csr_mpriv == PRIV_SUPER) ? SR_SPP : 0;

        // Raise priviledge to supervisor level
        m_csr_mpriv  = PRIV_SUPER;

        m_csr_msr    = s;
        m_csr_sepc   = pc;
        m_csr_scause = cause;
        m_csr_stval  = badaddr;

        log_exception(pc, m_csr_sevec, cause);

        // Set new PC
        m_pc = m_csr_sevec;
    }
    // Machine mode
    else
    {
        uint32_t s = m_csr_msr;

        // Interrupt save and disable
        s &= ~SR_MPIE;
        s |= (s & SR_MIE) ? SR_MPIE : 0;
        s &= ~SR_MIE;

        // Record previous priviledge level
        s &= ~SR_MPP;
        s |= (m_csr_mpriv << SR_MPP_SHIFT);

        // Raise priviledge to machine level
        m_csr_mpriv  = PRIV_MACHINE;

        m_csr_msr    = s;
        m_csr_mepc   = pc;
        m_csr_mcause = cause;
        m_csr_mtval  = badaddr;

        log_exception(pc, m_csr_mevec, cause);

        // Set new PC
        m_pc = m_csr_mevec;
    }
}
//-----------------------------------------------------------------
// execute: Instruction execution stage
//-----------------------------------------------------------------
bool rv32::execute(void)
{
    uint32_t phy_pc = m_pc;

    // Translate PC to physical address
    if (!mmu_i_translate(m_pc, &phy_pc))
        return false;

    // Misaligned PC
    if ((!m_enable_rvc && (m_pc & 3)) || (m_enable_rvc && (m_pc & 1)))
    {
        exception(MCAUSE_MISALIGNED_FETCH, m_pc, m_pc);
        return false;
    }

    // Get opcode at current PC
    uint32_t opcode = get_opcode(phy_pc);
    m_pc_x = m_pc;

    // Extract registers
    int rd          = (opcode & OPCODE_RD_MASK)  >> OPCODE_RD_SHIFT;
    int rs1         = (opcode & OPCODE_RS1_MASK) >> OPCODE_RS1_SHIFT;
    int rs2         = (opcode & OPCODE_RS2_MASK) >> OPCODE_RS2_SHIFT;

    // Extract immediates
    int typei_imm   = ((signed)(opcode & OPCODE_TYPEI_IMM_MASK)) >> OPCODE_TYPEI_IMM_SHIFT;
    int typeu_imm   = ((signed)(opcode & OPCODE_TYPEU_IMM_MASK)) >> OPCODE_TYPEU_IMM_SHIFT;
    int imm20       = typeu_imm << OPCODE_TYPEU_IMM_SHIFT;
    int imm12       = typei_imm;
    int bimm        = OPCODE_SBTYPE_IMM(opcode);
    int jimm20      = OPCODE_UJTYPE_IMM(opcode);
    int storeimm    = OPCODE_STYPE_IMM(opcode);
    int shamt       = ((signed)(opcode & OPCODE_SHAMT_MASK)) >> OPCODE_SHAMT_SHIFT;

    // Retrieve registers
    uint32_t reg_rd  = 0;
    uint32_t reg_rs1 = m_gpr[rs1];
    uint32_t reg_rs2 = m_gpr[rs2];
    uint32_t pc      = m_pc;

    bool take_exception = false;

    DPRINTF(LOG_OPCODES,( "%08x: %08x\n", pc, opcode));
    DPRINTF(LOG_OPCODES,( "        rd(%d) r%d = %d, r%d = %d\n", rd, rs1, reg_rs1, rs2, reg_rs2));

    // As RVC is not supported, fault on opcode which is all zeros
    if (opcode == 0)
    {
        error(false, "Bad instruction @ %x\n", pc);

        exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
        m_fault = true;
        take_exception = true;        
    }
    else if ((opcode & INST_ANDI_MASK) == INST_ANDI)
    {
        DPRINTF(LOG_INST,("%08x: andi r%d, r%d, %d\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_ANDI);
        reg_rd = reg_rs1 & imm12;
        pc += 4;
    }
    else if ((opcode & INST_ORI_MASK) == INST_ORI)
    {
        DPRINTF(LOG_INST,("%08x: ori r%d, r%d, %d\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_ORI);
        reg_rd = reg_rs1 | imm12;
        pc += 4;
    }
    else if ((opcode & INST_XORI_MASK) == INST_XORI)
    {
        DPRINTF(LOG_INST,("%08x: xori r%d, r%d, %d\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_XORI);
        reg_rd = reg_rs1 ^ imm12;
        pc += 4;
    }
    else if ((opcode & INST_ADDI_MASK) == INST_ADDI)
    {
        DPRINTF(LOG_INST,("%08x: addi r%d, r%d, %d\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_ADDI);
        reg_rd = reg_rs1 + imm12;
        pc += 4;
    }
    else if ((opcode & INST_SLTI_MASK) == INST_SLTI)
    {
        DPRINTF(LOG_INST,("%08x: slti r%d, r%d, %d\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_SLTI);
        reg_rd = (signed)reg_rs1 < (signed)imm12;
        pc += 4;
    }
    else if ((opcode & INST_SLTIU_MASK) == INST_SLTIU)
    {
        DPRINTF(LOG_INST,("%08x: sltiu r%d, r%d, %d\n", pc, rd, rs1, (unsigned)imm12));
        INST_STAT(ENUM_INST_SLTIU);
        reg_rd = (unsigned)reg_rs1 < (unsigned)imm12;
        pc += 4;
    }
    else if ((opcode & INST_SLLI_MASK) == INST_SLLI)
    {
        DPRINTF(LOG_INST,("%08x: slli r%d, r%d, %d\n", pc, rd, rs1, shamt));
        INST_STAT(ENUM_INST_SLLI);
        reg_rd = reg_rs1 << shamt;
        pc += 4;
    }
    else if ((opcode & INST_SRLI_MASK) == INST_SRLI)
    {
        DPRINTF(LOG_INST,("%08x: srli r%d, r%d, %d\n", pc, rd, rs1, shamt));
        INST_STAT(ENUM_INST_SRLI);
        reg_rd = (unsigned)reg_rs1 >> shamt;
        pc += 4;
    }
    else if ((opcode & INST_SRAI_MASK) == INST_SRAI)
    {
        DPRINTF(LOG_INST,("%08x: srai r%d, r%d, %d\n", pc, rd, rs1, shamt));
        INST_STAT(ENUM_INST_SRAI);
        reg_rd = (signed)reg_rs1 >> shamt;
        pc += 4;
    }
    else if ((opcode & INST_LUI_MASK) == INST_LUI)
    {
        DPRINTF(LOG_INST,("%08x: lui r%d, 0x%x\n", pc, rd, imm20));
        INST_STAT(ENUM_INST_LUI);
        reg_rd = imm20;
        pc += 4;
    }
    else if ((opcode & INST_AUIPC_MASK) == INST_AUIPC)
    {
        DPRINTF(LOG_INST,("%08x: auipc r%d, 0x%x\n", pc, rd, imm20));
        INST_STAT(ENUM_INST_AUIPC);
        reg_rd = imm20 + pc;
        pc += 4;
    }
    else if ((opcode & INST_ADD_MASK) == INST_ADD)
    {
        DPRINTF(LOG_INST,("%08x: add r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_ADD);
        reg_rd = reg_rs1 + reg_rs2;
        pc += 4;
    }
    else if ((opcode & INST_SUB_MASK) == INST_SUB)
    {
        DPRINTF(LOG_INST,("%08x: sub r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SUB);
        reg_rd = reg_rs1 - reg_rs2;
        pc += 4;
    }
    else if ((opcode & INST_SLT_MASK) == INST_SLT)
    {
        DPRINTF(LOG_INST,("%08x: slt r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SLT);
        reg_rd = (signed)reg_rs1 < (signed)reg_rs2;
        pc += 4;
    }
    else if ((opcode & INST_SLTU_MASK) == INST_SLTU)
    {
        DPRINTF(LOG_INST,("%08x: sltu r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SLTU);
        reg_rd = (unsigned)reg_rs1 < (unsigned)reg_rs2;
        pc += 4;
    }
    else if ((opcode & INST_XOR_MASK) == INST_XOR)
    {
        DPRINTF(LOG_INST,("%08x: xor r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_XOR);
        reg_rd = reg_rs1 ^ reg_rs2;
        pc += 4;
    }
    else if ((opcode & INST_OR_MASK) == INST_OR)
    {
        DPRINTF(LOG_INST,("%08x: or r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_OR);
        reg_rd = reg_rs1 | reg_rs2;
        pc += 4;
    }
    else if ((opcode & INST_AND_MASK) == INST_AND)
    {
        DPRINTF(LOG_INST,("%08x: and r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_AND);
        reg_rd = reg_rs1 & reg_rs2;
        pc += 4;
    }
    else if ((opcode & INST_SLL_MASK) == INST_SLL)
    {
        DPRINTF(LOG_INST,("%08x: sll r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SLL);
        reg_rd = reg_rs1 << reg_rs2;
        pc += 4;
    }
    else if ((opcode & INST_SRL_MASK) == INST_SRL)
    {
        DPRINTF(LOG_INST,("%08x: srl r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SRL);
        reg_rd = (unsigned)reg_rs1 >> reg_rs2;
        pc += 4;
    }
    else if ((opcode & INST_SRA_MASK) == INST_SRA)
    {
        DPRINTF(LOG_INST,("%08x: sra r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_SRA);
        reg_rd = (signed)reg_rs1 >> reg_rs2;
        pc += 4;
    }
    else if ((opcode & INST_JAL_MASK) == INST_JAL)
    {
        DPRINTF(LOG_INST,("%08x: jal r%d, %d\n", pc, rd, jimm20));
        INST_STAT(ENUM_INST_JAL);
        reg_rd = pc + 4;
        pc = pc + jimm20;

        if (rd == RISCV_REG_RA)
            log_branch_call(m_pc, pc);
        else
            log_branch_jump(m_pc, pc);

        m_stats[STATS_BRANCHES]++;
    }
    else if ((opcode & INST_JALR_MASK) == INST_JALR)
    {
        DPRINTF(LOG_INST,("%08x: jalr r%d, r%d\n", pc, rs1, imm12));
        INST_STAT(ENUM_INST_JALR);
        reg_rd = pc + 4;
        pc = (reg_rs1 + imm12) & ~1;

        if (rs1 == RISCV_REG_RA && imm12 == 0)
            log_branch_ret(m_pc, pc);
        else if (rd == RISCV_REG_RA)
            log_branch_call(m_pc, pc);
        else
            log_branch_jump(m_pc, pc);

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BEQ_MASK) == INST_BEQ)
    {
        DPRINTF(LOG_INST,("%08x: beq r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BEQ);

        bool take_branch = (reg_rs1 == reg_rs2);
        if (take_branch)
            pc = pc + bimm;
        else
            pc += 4;

        log_branch(m_pc, pc, take_branch);

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BNE_MASK) == INST_BNE)
    {
        DPRINTF(LOG_INST,("%08x: bne r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BNE);

        bool take_branch = (reg_rs1 != reg_rs2);
        if (take_branch)
            pc = pc + bimm;
        else
            pc += 4;

        log_branch(m_pc, pc, take_branch);

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BLT_MASK) == INST_BLT)
    {
        DPRINTF(LOG_INST,("%08x: blt r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BLT);

        bool take_branch = ((signed)reg_rs1 < (signed)reg_rs2);
        if (take_branch)
            pc = pc + bimm;
        else
            pc += 4;

        log_branch(m_pc, pc, take_branch);

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BGE_MASK) == INST_BGE)
    {
        DPRINTF(LOG_INST,("%08x: bge r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BGE);

        bool take_branch = ((signed)reg_rs1 >= (signed)reg_rs2);
        if (take_branch)
            pc = pc + bimm;
        else
            pc += 4;

        log_branch(m_pc, pc, take_branch);

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BLTU_MASK) == INST_BLTU)
    {
        DPRINTF(LOG_INST,("%08x: bltu r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BLTU);

        bool take_branch = ((unsigned)reg_rs1 < (unsigned)reg_rs2);
        if (take_branch)
            pc = pc + bimm;
        else
            pc += 4;

        log_branch(m_pc, pc, take_branch);

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_BGEU_MASK) == INST_BGEU)
    {
        DPRINTF(LOG_INST,("%08x: bgeu r%d, r%d, %d\n", pc, rs1, rs2, bimm));
        INST_STAT(ENUM_INST_BGEU);

        bool take_branch = ((unsigned)reg_rs1 >= (unsigned)reg_rs2);
        if (take_branch)
            pc = pc + bimm;
        else
            pc += 4;

        log_branch(m_pc, pc, take_branch);

        // No writeback
        rd = 0;

        m_stats[STATS_BRANCHES]++;        
    }
    else if ((opcode & INST_LB_MASK) == INST_LB)
    {
        DPRINTF(LOG_INST,("%08x: lb r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        INST_STAT(ENUM_INST_LB);
        if (load(pc, reg_rs1 + imm12, &reg_rd, 1, true))
            pc += 4;
        else
            return false;
    }
    else if ((opcode & INST_LH_MASK) == INST_LH)
    {
        DPRINTF(LOG_INST,("%08x: lh r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        INST_STAT(ENUM_INST_LH);
        if (load(pc, reg_rs1 + imm12, &reg_rd, 2, true))
            pc += 4;
        else
            return false;
    }
    else if ((opcode & INST_LW_MASK) == INST_LW)
    {
        INST_STAT(ENUM_INST_LW);
        DPRINTF(LOG_INST,("%08x: lw r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        if (load(pc, reg_rs1 + imm12, &reg_rd, 4, true))
            pc += 4;
        else
            return false;
    }
    else if ((opcode & INST_LBU_MASK) == INST_LBU)
    {
        DPRINTF(LOG_INST,("%08x: lbu r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        INST_STAT(ENUM_INST_LBU);
        if (load(pc, reg_rs1 + imm12, &reg_rd, 1, false))
            pc += 4;
        else
            return false;
    }
    else if ((opcode & INST_LHU_MASK) == INST_LHU)
    {
        DPRINTF(LOG_INST,("%08x: lhu r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        INST_STAT(ENUM_INST_LHU);
        if (load(pc, reg_rs1 + imm12, &reg_rd, 2, false))
            pc += 4;
        else
            return false;
    }
    else if ((opcode & INST_LWU_MASK) == INST_LWU)
    {
        DPRINTF(LOG_INST,("%08x: lwu r%d, %d(r%d)\n", pc, rd, imm12, rs1));
        INST_STAT(ENUM_INST_LWU);
        if (load(pc, reg_rs1 + imm12, &reg_rd, 4, false))
            pc += 4;
        else
            return false;
    }
    else if ((opcode & INST_SB_MASK) == INST_SB)
    {
        DPRINTF(LOG_INST,("%08x: sb %d(r%d), r%d\n", pc, storeimm, rs1, rs2));
        INST_STAT(ENUM_INST_SB);
        if (store(pc, reg_rs1 + storeimm, reg_rs2, 1))
            pc += 4;
        else
            return false;

        // No writeback
        rd = 0;
    }
    else if ((opcode & INST_SH_MASK) == INST_SH)
    {
        DPRINTF(LOG_INST,("%08x: sh %d(r%d), r%d\n", pc, storeimm, rs1, rs2));
        INST_STAT(ENUM_INST_SH);
        if (store(pc, reg_rs1 + storeimm, reg_rs2, 2))
            pc += 4;
        else
            return false;

        // No writeback
        rd = 0;
    }
    else if ((opcode & INST_SW_MASK) == INST_SW)
    {
        DPRINTF(LOG_INST,("%08x: sw %d(r%d), r%d\n", pc, storeimm, rs1, rs2));
        INST_STAT(ENUM_INST_SW);
        if (store(pc, reg_rs1 + storeimm, reg_rs2, 4))
            pc += 4;
        else
            return false;

        // No writeback
        rd = 0;
    }
    else if (m_enable_rvm && (opcode & INST_MUL_MASK) == INST_MUL)
    {
        DPRINTF(LOG_INST,("%08x: mul r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_MUL);
        m_stats[STATS_MUL]++;
        reg_rd = (signed)reg_rs1 * (signed)reg_rs2;
        pc += 4;
    }
    else if (m_enable_rvm && (opcode & INST_MULH_MASK) == INST_MULH)
    {
        long long res = ((long long) (int)reg_rs1) * ((long long)(int)reg_rs2);
        INST_STAT(ENUM_INST_MULH);
        m_stats[STATS_MUL]++;
        DPRINTF(LOG_INST,("%08x: mulh r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        reg_rd = (int)(res >> 32);
        pc += 4;
    }
    else if (m_enable_rvm && (opcode & INST_MULHSU_MASK) == INST_MULHSU)
    {
        long long res = ((long long) (int)reg_rs1) * ((unsigned long long)(unsigned)reg_rs2);
        INST_STAT(ENUM_INST_MULHSU);
        m_stats[STATS_MUL]++;
        DPRINTF(LOG_INST,("%08x: mulhsu r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        reg_rd = (int)(res >> 32);
        pc += 4;
    }
    else if (m_enable_rvm && (opcode & INST_MULHU_MASK) == INST_MULHU)
    {
        unsigned long long res = ((unsigned long long) (unsigned)reg_rs1) * ((unsigned long long)(unsigned)reg_rs2);
        INST_STAT(ENUM_INST_MULHU);
        m_stats[STATS_MUL]++;
        DPRINTF(LOG_INST,("%08x: mulhu r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        reg_rd = (int)(res >> 32);
        pc += 4;
    }
    else if (m_enable_rvm && (opcode & INST_DIV_MASK) == INST_DIV)
    {
        DPRINTF(LOG_INST,("%08x: div r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_DIV);
        m_stats[STATS_DIV]++;
        if ((signed)reg_rs1 == INT32_MIN && (signed)reg_rs2 == -1)
            reg_rd = reg_rs1;
        else if (reg_rs2 != 0)
            reg_rd = (signed)reg_rs1 / (signed)reg_rs2;
        else
            reg_rd = (unsigned)-1;
        pc += 4;
    }
    else if (m_enable_rvm && (opcode & INST_DIVU_MASK) == INST_DIVU)
    {
        DPRINTF(LOG_INST,("%08x: divu r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_DIVU);
        m_stats[STATS_DIV]++;
        if (reg_rs2 != 0)
            reg_rd = (unsigned)reg_rs1 / (unsigned)reg_rs2;
        else
            reg_rd = (unsigned)-1;
        pc += 4;
    }
    else if (m_enable_rvm && (opcode & INST_REM_MASK) == INST_REM)
    {
        DPRINTF(LOG_INST,("%08x: rem r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_REM);
        m_stats[STATS_DIV]++;

        if((signed)reg_rs1 == INT32_MIN && (signed)reg_rs2 == -1)
            reg_rd = 0;
        else if (reg_rs2 != 0)
            reg_rd = (signed)reg_rs1 % (signed)reg_rs2;
        else
            reg_rd = reg_rs1;
        pc += 4;
    }
    else if (m_enable_rvm && (opcode & INST_REMU_MASK) == INST_REMU)
    {
        DPRINTF(LOG_INST,("%08x: remu r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        INST_STAT(ENUM_INST_REMU);
        m_stats[STATS_DIV]++;
        if (reg_rs2 != 0)
            reg_rd = (unsigned)reg_rs1 % (unsigned)reg_rs2;
        else
            reg_rd = reg_rs1;
        pc += 4;
    }
    else if ((opcode & INST_ECALL_MASK) == INST_ECALL)
    {
        DPRINTF(LOG_INST,("%08x: ecall\n", pc));
        INST_STAT(ENUM_INST_ECALL);

        // Semi-hosted system call?
        if (syscall_handler())
            pc += 4;
        else
        {
            exception(MCAUSE_ECALL_U + m_csr_mpriv, pc);
            take_exception   = true;
        }
    }
    else if ((opcode & INST_EBREAK_MASK) == INST_EBREAK)
    {
        DPRINTF(LOG_INST,("%08x: ebreak\n", pc));
        INST_STAT(ENUM_INST_EBREAK);

        exception(MCAUSE_BREAKPOINT, pc);
        take_exception   = true;
        m_break          = true;
    }
    else if ((opcode & INST_MRET_MASK) == INST_MRET)
    {
        DPRINTF(LOG_INST,("%08x: mret\n", pc));
        INST_STAT(ENUM_INST_MRET);

        assert(m_csr_mpriv == PRIV_MACHINE);

        uint32_t s        = m_csr_msr;
        uint32_t prev_prv = SR_GET_MPP(m_csr_msr);

        // Interrupt enable pop
        s &= ~SR_MIE;
        s |= (s & SR_MPIE) ? SR_MIE : 0;
        s |= SR_MPIE;

        // Set next MPP to user mode
        s &= ~SR_MPP;
        s |=  SR_MPP_U;

        // Set privilege level to previous MPP
        m_csr_mpriv   = prev_prv;
        m_csr_msr     = s;

        // Return to EPC
        pc = m_csr_mepc;
    }
    else if ((opcode & INST_SRET_MASK) == INST_SRET)
    {
        DPRINTF(LOG_INST,("%08x: sret\n", pc));
        INST_STAT(ENUM_INST_SRET);

        assert(m_csr_mpriv == PRIV_SUPER);

        uint32_t s        = m_csr_msr;
        uint32_t prev_prv = (m_csr_msr & SR_SPP) ? PRIV_SUPER : PRIV_USER;

        // Interrupt enable pop
        s &= ~SR_SIE;
        s |= (s & SR_SPIE) ? SR_SIE : 0;
        s |= SR_SPIE;

        // Set next SPP to user mode
        s &= ~SR_SPP;

        // Set privilege level to previous MPP
        m_csr_mpriv   = prev_prv;
        m_csr_msr     = s;

        // Return to EPC
        pc = m_csr_sepc;
    }
    else if ( ((opcode & INST_SFENCE_MASK) == INST_SFENCE) ||
              ((opcode & INST_FENCE_MASK) == INST_FENCE) ||
              ((opcode & INST_IFENCE_MASK) == INST_IFENCE))
    {
        DPRINTF(LOG_INST,("%08x: fence\n", pc));
        INST_STAT(ENUM_INST_FENCE);

        // SFENCE.VMA
        if ((opcode & INST_SFENCE_MASK) == INST_SFENCE)
            mmu_flush();
        pc += 4;
    }
    else if ((opcode & INST_CSRRW_MASK) == INST_CSRRW)
    {
        DPRINTF(LOG_INST,("%08x: csrw r%d, r%d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRW);
        take_exception = access_csr(imm12, reg_rs1, true, true, reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
        else
            pc += 4;
    }    
    else if ((opcode & INST_CSRRS_MASK) == INST_CSRRS)
    {
        DPRINTF(LOG_INST,("%08x: csrs r%d, r%d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRS);
        take_exception = access_csr(imm12, reg_rs1, (rs1 != 0), false, reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
        else
            pc += 4;
    }
    else if ((opcode & INST_CSRRC_MASK) == INST_CSRRC)
    {
        DPRINTF(LOG_INST,("%08x: csrc r%d, r%d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRC);
        take_exception = access_csr(imm12, reg_rs1, false, (rs1 != 0), reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
        else
            pc += 4;
    }
    else if ((opcode & INST_CSRRWI_MASK) == INST_CSRRWI)
    {
        DPRINTF(LOG_INST,("%08x: csrwi r%d, %d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRWI);
        take_exception = access_csr(imm12, rs1, true, true, reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
        else
            pc += 4;
    }
    else if ((opcode & INST_CSRRSI_MASK) == INST_CSRRSI)
    {
        DPRINTF(LOG_INST,("%08x: csrsi r%d, %d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRSI);
        take_exception = access_csr(imm12, rs1, (rs1 != 0), false, reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
        else
            pc += 4;
    }
    else if ((opcode & INST_CSRRCI_MASK) == INST_CSRRCI)
    {
        DPRINTF(LOG_INST,("%08x: csrci r%d, %d, 0x%x\n", pc, rd, rs1, imm12));
        INST_STAT(ENUM_INST_CSRRCI);
        take_exception = access_csr(imm12, rs1, false, (rs1 != 0), reg_rd);
        if (take_exception)
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
        else
            pc += 4;
    }
    else if ((opcode & INST_WFI_MASK) == INST_WFI)
    {
        DPRINTF(LOG_INST,("%08x: wfi\n", pc));
        INST_STAT(ENUM_INST_WFI);
        pc += 4;
    }
    //-----------------------------------------------------------------
    // A Extension
    //-----------------------------------------------------------------
    else if (m_enable_rva && (opcode & INST_AMOADD_W_MASK) == INST_AMOADD_W)
    {
        DPRINTF(LOG_INST,("%08x: amoadd.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return false;

        // Modify
        uint32_t val = reg_rd + reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return false;

        INST_STAT(ENUM_INST_ADD);
        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOXOR_W_MASK) == INST_AMOXOR_W)
    {
        DPRINTF(LOG_INST,("%08x: amoxor.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return false;

        // Modify
        uint32_t val = reg_rd ^ reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return false;

        INST_STAT(ENUM_INST_XOR);
        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOOR_W_MASK) == INST_AMOOR_W)
    {
        DPRINTF(LOG_INST,("%08x: amoor.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return false;

        // Modify
        uint32_t val = reg_rd | reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return false;

        INST_STAT(ENUM_INST_OR);
        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOAND_W_MASK) == INST_AMOAND_W)
    {
        DPRINTF(LOG_INST,("%08x: amoand.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return false;

        // Modify
        uint32_t val = reg_rd & reg_rs2;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return false;

        INST_STAT(ENUM_INST_AND);
        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMIN_W_MASK) == INST_AMOMIN_W)
    {
        DPRINTF(LOG_INST,("%08x: amomin.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return false;

        // Modify
        uint32_t val = reg_rs2;
        if ((int32_t)reg_rd < (int32_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return false;

        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMAX_W_MASK) == INST_AMOMAX_W)
    {
        DPRINTF(LOG_INST,("%08x: amomax.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return false;

        // Modify
        uint32_t val = reg_rs2;
        if ((int32_t)reg_rd > (int32_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return false;

        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMINU_W_MASK) == INST_AMOMINU_W)
    {
        DPRINTF(LOG_INST,("%08x: amominu.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return false;

        // Modify
        uint32_t val = reg_rs2;
        if ((uint32_t)reg_rd < (uint32_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return false;

        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOMAXU_W_MASK) == INST_AMOMAXU_W)
    {
        DPRINTF(LOG_INST,("%08x: amomaxu.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return false;

        // Modify
        uint32_t val = reg_rs2;
        if ((uint32_t)reg_rd > (uint32_t)reg_rs2)
            val = reg_rd;

        // Write
        if (!store(pc, reg_rs1, val, 4))
            return false;

        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_AMOSWAP_W_MASK) == INST_AMOSWAP_W)
    {
        DPRINTF(LOG_INST,("%08x: amoswap.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));

        // Read
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return false;

        // Write
        if (!store(pc, reg_rs1, reg_rs2, 4))
            return false;

        INST_STAT(ENUM_INST_LW);
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_LR_W_MASK) == INST_LR_W)
    {
        DPRINTF(LOG_INST,("%08x: lr.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        if (!load(pc, reg_rs1, &reg_rd, 4, true))
            return false;

        // Record load address
        m_load_res = reg_rs1;

        INST_STAT(ENUM_INST_LW);
        pc += 4;
    }
    else if (m_enable_rva && (opcode & INST_SC_W_MASK) == INST_SC_W)
    {
        DPRINTF(LOG_INST,("%08x: sc.w r%d, r%d, r%d\n", pc, rd, rs1, rs2));
        if (m_load_res == reg_rs1)
        {
            // Write
            if (!store(pc, reg_rs1, reg_rs2, 4))
                return false;

            reg_rd = 0;
        }
        else
            reg_rd = 1;

        m_load_res = 0;
        INST_STAT(ENUM_INST_SW);
        pc += 4;
    }
    //-----------------------------------------------------------------
    // C Extension
    //-----------------------------------------------------------------
    // RVC - Quadrant 0
    else if (m_enable_rvc && ((opcode & 3) == 0))
    {
        opcode &= 0xFFFF;
        rvc_decode rvc(opcode);

        rs1     = rvc.rs1s();
        rs2     = rvc.rs2s();
        rd      = rs2;
        reg_rs1 = m_gpr[rs1];
        reg_rs2 = m_gpr[rs2];        

        // C.ADDI4SPN
        if ((opcode >> 13) == 0 && opcode != 0)
        {
            rs1     = RISCV_REG_SP;
            reg_rs1 = m_gpr[rs1];

            uint32_t imm = rvc.addi4spn_imm();
            DPRINTF(LOG_INST,("%08x: c.addi4spn r%d,r%d,%d\n", pc, rd, rs1, imm));
            INST_STAT(ENUM_INST_ADDI);
            reg_rd = reg_rs1 + imm;
            pc += 2;
        }
        // C.LW
        else if ((opcode >> 13) == 2)
        {
            uint32_t imm = rvc.lw_imm();
            DPRINTF(LOG_INST,("%08x: c.lw r%d, %d(r%d)\n", pc, rd, imm, rs1));
            INST_STAT(ENUM_INST_LW);
            if (load(pc, reg_rs1 + imm, &reg_rd, 4, true))
                pc += 2;
            else
                return false;
        }
        // C.LD
        else if ((opcode >> 13) == 3)
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
            m_fault        = true;
            take_exception = true;
        }
        // C.SW
        else if ((opcode >> 13) == 6)
        {
            uint32_t imm = rvc.lw_imm();
            DPRINTF(LOG_INST,("%08x: c.sw %d(r%d), r%d\n", pc, imm, rs1, rd));
            INST_STAT(ENUM_INST_SW);
            if (store(pc, reg_rs1 + imm, reg_rs2, 4))
                pc += 2;
            else
                return false;

            // No writeback
            rd = 0;
        }
        // C.SD
        else if ((opcode >> 13) == 7)
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
            m_fault        = true;
            take_exception = true;
        }
        // Illegal instruction
        else
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
            m_fault        = true;
            take_exception = true;
        }
    }
    // RVC - Quadrant 1 (top half - c.nop - c.lui)
    else if (m_enable_rvc && ((opcode & 3) == 1) && (((opcode & 0xFFFF) >> 13) < 4))
    {
        opcode &= 0xFFFF;
        rvc_decode rvc(opcode);

        rs1     = rvc.rs1();
        rs2     = rvc.rs2();
        rd      = rs1;
        reg_rs1 = m_gpr[rs1];
        reg_rs2 = m_gpr[rs2];

        // C.NOP
        if ((opcode >> 13) == 0 && opcode == 0)
        {
            DPRINTF(LOG_INST,("%08x: c.nop\n", pc));
            INST_STAT(ENUM_INST_ADDI);
            pc += 2;
        }
        // C.ADDI
        else if ((opcode >> 13) == 0)
        {
            int32_t imm = rvc.imm();
            DPRINTF(LOG_INST,("%08x: c.addi r%d, %d\n", pc, rs1, imm));
            INST_STAT(ENUM_INST_ADDI);
            reg_rd = reg_rs1 + imm;
            pc += 2;
        }
        // C.JAL // TODO: C.ADDIW on RV64
        else if ((opcode >> 13) == 1)
        {
            uint32_t imm = rvc.j_imm();
            DPRINTF(LOG_INST,("%08x: c.jal 0x%08x\n", pc, pc + imm));
            INST_STAT(ENUM_INST_JAL);
            rd     = RISCV_REG_RA;
            reg_rd = pc + 2;
            pc += imm;

            log_branch_call(m_pc, pc);
        }
        // C.LI
        else if ((opcode >> 13) == 2)
        {
            uint32_t imm = rvc.imm();
            DPRINTF(LOG_INST,("%08x: c.li r%d, %d\n", pc, rd, imm));
            INST_STAT(ENUM_INST_LUI);
            reg_rd = imm;
            pc += 2;
        }
        // C.ADDI16SP
        else if ((opcode >> 13) == 3 && ((opcode >> 7) & 0x1f) == 2)
        {
            uint32_t imm = rvc.addi16sp_imm();
            rd      = RISCV_REG_SP;
            rs1     = RISCV_REG_SP;
            reg_rs1 = m_gpr[rs1];
            DPRINTF(LOG_INST,("%08x: c.addi16sp r%d, r%d, %d\n", pc, rd, rs1, imm));
            INST_STAT(ENUM_INST_ADDI);
            reg_rd  = reg_rs1 + imm;
            pc += 2;
        }
        // C.LUI
        else if ((opcode >> 13) == 3)
        {
            uint32_t imm = rvc.imm() << 12;
            DPRINTF(LOG_INST,("%08x: c.lui r%d, 0x%x\n", pc, rd, imm));
            INST_STAT(ENUM_INST_LUI);
            reg_rd  = imm;
            pc += 2;
        }
        // Illegal instruction
        else
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
            m_fault        = true;
            take_exception = true;
        }
    }
    // RVC - Quadrant 1 (bottom half - c.srli -)
    else if (m_enable_rvc && ((opcode & 3) == 1))
    {
        opcode &= 0xFFFF;
        rvc_decode rvc(opcode);

        rs1     = rvc.rs1s();
        rs2     = rvc.rs2s();
        rd      = rs1;
        reg_rs1 = m_gpr[rs1];
        reg_rs2 = m_gpr[rs2];

        // C.SRLI
        if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x3) == 0)
        {
            uint32_t imm = rvc.zimm();
            DPRINTF(LOG_INST,("%08x: c.srli r%d, %d\n", pc, rd, imm));
            INST_STAT(ENUM_INST_SRLI);
            reg_rd = reg_rs1 >> imm;
            pc += 2;
        }
        // C.SRAI
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x3) == 1)
        {
            uint32_t imm = rvc.zimm();
            DPRINTF(LOG_INST,("%08x: c.srai r%d, %d\n", pc, rd, imm));
            INST_STAT(ENUM_INST_SRAI);
            reg_rd = (int32_t)reg_rs1 >> imm;
            pc += 2;
        }
        // C.ANDI
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x3) == 2)
        {
            uint32_t imm = rvc.imm();
            DPRINTF(LOG_INST,("%08x: c.andi r%d, 0x%08x\n", pc, rd, imm));
            INST_STAT(ENUM_INST_ANDI);
            reg_rd = reg_rs1 & imm;
            pc += 2;
        }
        // C.SUB
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 3 && ((opcode >> 5) & 0x3) == 0)
        {
            DPRINTF(LOG_INST,("%08x: c.sub r%d, r%d\n", pc, rs1, rs2));
            INST_STAT(ENUM_INST_SUB);
            reg_rd = (int32_t)(reg_rs1 - reg_rs2);
            pc += 2;
        }
        // C.XOR
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 3 && ((opcode >> 5) & 0x3) == 1)
        {
            DPRINTF(LOG_INST,("%08x: c.xor r%d, r%d\n", pc, rs1, rs2));
            INST_STAT(ENUM_INST_XOR);
            reg_rd = reg_rs1 ^ reg_rs2;
            pc += 2;
        }
        // C.OR
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 3 && ((opcode >> 5) & 0x3) == 2)
        {
            DPRINTF(LOG_INST,("%08x: c.or r%d, r%d\n", pc, rs1, rs2));
            INST_STAT(ENUM_INST_OR);
            reg_rd = reg_rs1 | reg_rs2;
            pc += 2;
        }
        // C.AND
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 3 && ((opcode >> 5) & 0x3) == 3)
        {
            DPRINTF(LOG_INST,("%08x: c.and r%d, r%d\n", pc, rs1, rs2));
            INST_STAT(ENUM_INST_AND);
            reg_rd = reg_rs1 & reg_rs2;
            pc += 2;
        }
        // C.SUBW
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 7 && ((opcode >> 5) & 0x3) == 0)
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
            m_fault        = true;
            take_exception = true;
        }
        // C.ADDW
        else if ((opcode >> 13) == 4 && ((opcode >> 10) & 0x7) == 7 && ((opcode >> 5) & 0x3) == 1)
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
            m_fault        = true;
            take_exception = true;
        }
        // C.J
        else if ((opcode >> 13) == 5)
        {
            uint32_t imm = rvc.j_imm();
            DPRINTF(LOG_INST,("%08x: c.j 0x%08x\n", pc, pc + imm));
            INST_STAT(ENUM_INST_J);
            pc += imm;
            rd = 0;
            log_branch_jump(m_pc, pc);
        }
        // C.BEQZ
        else if ((opcode >> 13) == 6)
        {
            uint32_t imm = rvc.b_imm();

            DPRINTF(LOG_INST,("%08x: c.beqz r%d, %d\n", pc, rs1, imm));
            INST_STAT(ENUM_INST_BEQ);
            bool take_branch = (reg_rs1 == 0);
            if (take_branch)
                pc += imm;
            else
                pc += 2;
            log_branch(m_pc, pc, take_branch);
            rd = 0;
        }
        // C.BNEZ
        else if ((opcode >> 13) == 7)
        {
            uint32_t imm = rvc.b_imm();
            DPRINTF(LOG_INST,("%08x: c.bnez r%d, %d\n", pc, rs1, imm));
            INST_STAT(ENUM_INST_BNE);
            bool take_branch = (reg_rs1 != 0);
            if (take_branch)
                pc += imm;
            else
                pc += 2;
            log_branch(m_pc, pc, take_branch);
            rd = 0;
        }
        // Illegal instruction
        else
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
            m_fault        = true;
            take_exception = true;
        }
    }
    // RVC - Quadrant 2
    else if (m_enable_rvc && ((opcode & 3) == 2))
    {
        opcode &= 0xFFFF;

        rvc_decode rvc(opcode);
        rs1     = rvc.rs1();
        rs2     = rvc.rs2();
        rd      = rs1;
        reg_rs1 = m_gpr[rs1];
        reg_rs2 = m_gpr[rs2];

        // C.SLLI
        if ((opcode >> 13) == 0)
        {
            uint32_t imm = rvc.zimm();
            DPRINTF(LOG_INST,("%08x: c.slli r%d, %d\n", pc, rs1, imm));
            INST_STAT(ENUM_INST_SLLI);
            reg_rd = reg_rs1 << imm;
            pc += 2;
        }
        // C.LWSP
        else if ((opcode >> 13) == 2)
        {
            uint32_t imm = rvc.lwsp_imm();
            rs1     = RISCV_REG_SP;
            reg_rs1 = m_gpr[rs1];

            DPRINTF(LOG_INST,("%08x: c.lwsp r%d, %d(r%d)\n", pc, rd, imm, rs1));
            INST_STAT(ENUM_INST_LW);
            if (load(pc, reg_rs1 + imm, &reg_rd, 4, true))
                pc += 2;
            else
                return false;
        }
        // C.LDSP
        else if ((opcode >> 13) == 3)
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
            m_fault        = true;
            take_exception = true;
        }
        // C.JR
        // C.MV
        // C.EBREAK
        // C.JALR
        // C.ADD
        else if ((opcode >> 13) == 4)
        {
            if (!(opcode & (1 << 12)))
            {
                // C.JR
                if (((opcode >> 2) & 0x1F) == 0)
                {
                    rd = 0;
                    DPRINTF(LOG_INST,("%08x: c.jr r%d\n", pc, rs1));
                    INST_STAT(ENUM_INST_J);
                    pc = reg_rs1 & ~1;
                    if (rs1 == RISCV_REG_RA)
                        log_branch_ret(m_pc, pc);
                    else
                        log_branch_jump(m_pc, pc);
                }
                // C.MV
                else
                {
                    DPRINTF(LOG_INST,("%08x: c.mv r%d, r%d\n", pc, rd, rs2));
                    INST_STAT(ENUM_INST_ADD);
                    pc += 2;
                    reg_rd = reg_rs2;
                }
            }
            else
            {
                // C.EBREAK
                if (((opcode >> 7) & 0x1F) == 0 && ((opcode >> 2) & 0x1F) == 0)
                {
                    rd = 0;
                    DPRINTF(LOG_INST,("%08x: c.ebreak\n", pc));
                    INST_STAT(ENUM_INST_EBREAK);

                    exception(MCAUSE_BREAKPOINT, pc);
                    take_exception   = true;
                    m_break          = true;
                }
                // C.JALR
                else if (((opcode >> 2) & 0x1F) == 0)
                {
                    rd = RISCV_REG_RA;
                    DPRINTF(LOG_INST,("%08x: c.jalr r%d, r%d\n", pc, rd, rs1));
                    INST_STAT(ENUM_INST_JALR);
                    reg_rd = pc + 2;
                    pc     = reg_rs1 & ~1;
                    if (rs1 == RISCV_REG_RA)
                        log_branch_ret(m_pc, pc);
                    else
                        log_branch_call(m_pc, pc);
                }
                // C.ADD
                else
                {
                    DPRINTF(LOG_INST,("%08x: c.add r%d, r%d, r%d\n", pc, rd, rs1, rs2));
                    INST_STAT(ENUM_INST_ADD);
                    reg_rd = reg_rs1 + reg_rs2;
                    pc += 2;
                }
            }
        }
        // C.SWSP
        else if ((opcode >> 13) == 6)
        {
            uint32_t uimm = rvc.swsp_imm();
            rs1     = RISCV_REG_SP;
            reg_rs1 = m_gpr[rs1];
            DPRINTF(LOG_INST,("%08x: c.swsp r%d, %d(r%d)\n", pc, rs2, uimm, rs1));
            INST_STAT(ENUM_INST_SW);

            if (store(pc, reg_rs1 + uimm, reg_rs2, 4))
                pc += 2;
            else
                return false;

            // No writeback
            rd = 0;            
        }
        // C.SDSP
        else if ((opcode >> 13) == 7)
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
            m_fault        = true;
            take_exception = true;
        }
        // Illegal instruction
        else
        {
            error(false, "Bad instruction @ %x (opcode %x)\n", pc, opcode);
            exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
            m_fault        = true;
            take_exception = true;
        }
    }
    else
    {
        exception(MCAUSE_ILLEGAL_INSTRUCTION, pc, opcode);
        take_exception = true;
    }

    if (rd != 0 && !take_exception)
        m_gpr[rd] = reg_rd;

    // Monitor executed instructions
    log_commit_pc(m_pc_x);

    // Pending interrupt
    if (!take_exception && (m_csr_mip & m_csr_mie))
    {
        uint32_t pending_interrupts = (m_csr_mip & m_csr_mie);
        uint32_t m_enabled          = m_csr_mpriv < PRIV_MACHINE || (m_csr_mpriv == PRIV_MACHINE && (m_csr_msr & SR_MIE));
        uint32_t s_enabled          = m_csr_mpriv < PRIV_SUPER   || (m_csr_mpriv == PRIV_SUPER   && (m_csr_msr & SR_SIE));
        uint32_t m_interrupts       = pending_interrupts & ~m_csr_mideleg & -m_enabled;
        uint32_t s_interrupts       = pending_interrupts & m_csr_mideleg & -s_enabled;
        uint32_t interrupts         = m_interrupts ? m_interrupts : s_interrupts;

        // Interrupt pending and mask enabled
        if (interrupts)
        {
            //printf("Take Interrupt...: %08x\n", interrupts);
            int i;

            for (i=IRQ_MIN;i<IRQ_MAX;i++)
            {
                if (interrupts & (1 << i))
                {
                    // Only service one interrupt per cycle
                    DPRINTF(LOG_INST,( "Interrupt%d taken...\n", i));
                    exception(MCAUSE_INTERRUPT + i, pc);
                    take_exception = true;
                    break;
                }
            }
        }
    }

    if (!take_exception)
        m_pc = pc;

    return true;
}
//-----------------------------------------------------------------
// step: Step through one instruction
//-----------------------------------------------------------------
void rv32::step(void)
{
    m_stats[STATS_INSTRUCTIONS]++;

    // Execute instruction at current PC
    int max_steps = 2;
    while (max_steps-- && !execute())
        ;

    // Increment timer counter
    m_csr_mtime++;

    // Non-std: Timer should generate an internal interrupt?
    if (m_enable_mtimecmp)
    {
        // Limited internal timer, truncate to 32-bits
        m_csr_mtime &= 0xFFFFFFFF;
        if (m_csr_mtime == m_csr_mtimecmp && m_csr_mtime_ie)
        {
            m_csr_mip     |= m_enable_sbi ? SR_IP_STIP : SR_IP_MTIP;
            m_csr_mtime_ie = false;
        }
    }

    // Dump state
    if (TRACE_ENABLED(LOG_REGISTERS))
    {
        // Register trace
        int i;
        for (i=0;i<REGISTERS;i+=4)
        {
            DPRINTF(LOG_REGISTERS,( " %d: ", i));
            DPRINTF(LOG_REGISTERS,( " %08x %08x %08x %08x\n", m_gpr[i+0], m_gpr[i+1], m_gpr[i+2], m_gpr[i+3]));
        }
    }

    cpu::step();
}
//-----------------------------------------------------------------
// set_interrupt: Register pending interrupt
//-----------------------------------------------------------------
void rv32::set_interrupt(int irq)
{
    assert(irq == IRQ_M_EXT || irq == IRQ_M_TIMER);
    if (irq == IRQ_M_EXT)
#ifdef CPU_INTERRUPT_MEIP_ONLY
        m_csr_mip |= (SR_IP_MEIP);
#else
        m_csr_mip |= (SR_IP_MEIP | SR_IP_SEIP);
#endif
    else if (!m_enable_mtimecmp)
        m_csr_mip |= SR_IP_MTIP;

#ifdef CPU_INTERRUPT_ON_SET
    // Pending interrupt
    if (m_csr_mip & m_csr_mie)
    {
        uint32_t pending_interrupts = (m_csr_mip & m_csr_mie);
        uint32_t m_enabled          = m_csr_mpriv < PRIV_MACHINE || (m_csr_mpriv == PRIV_MACHINE && (m_csr_msr & SR_MIE));
        uint32_t s_enabled          = m_csr_mpriv < PRIV_SUPER   || (m_csr_mpriv == PRIV_SUPER   && (m_csr_msr & SR_SIE));
        uint32_t m_interrupts       = pending_interrupts & ~m_csr_mideleg & -m_enabled;
        uint32_t s_interrupts       = pending_interrupts & m_csr_mideleg & -s_enabled;
        uint32_t interrupts         = m_interrupts ? m_interrupts : s_interrupts;

        // Interrupt pending and mask enabled
        if (interrupts)
        {
            for (int i=IRQ_MIN;i<IRQ_MAX;i++)
            {
                if (interrupts & (1 << i))
                {
                    // Only service one interrupt per cycle
                    DPRINTF(LOG_INST,( "Interrupt%d taken...\n", i));
                    exception(MCAUSE_INTERRUPT + i, m_pc);
                    break;
                }
            }
        }
    }
#endif
}
//-----------------------------------------------------------------
// clr_interrupt: Clear interrupt pending
//-----------------------------------------------------------------
void rv32::clr_interrupt(int irq)
{
    assert(irq == IRQ_M_TIMER || irq == IRQ_M_EXT);
    if (irq == IRQ_M_TIMER && !m_enable_mtimecmp)
        m_csr_mip &= ~SR_IP_MTIP;
    else if (irq == IRQ_M_EXT)
    {
#ifdef CPU_INTERRUPT_MEIP_ONLY
        m_csr_mip &= ~(SR_IP_MEIP);
#else
        m_csr_mip &= ~(SR_IP_MEIP | SR_IP_SEIP);
#endif
    }
}
//-----------------------------------------------------------------
// set_timer: Set built in timer interrupt
//-----------------------------------------------------------------
void rv32::set_timer(uint32_t value)
{
    m_enable_mtimecmp = true;
    m_csr_mtime_ie    = true;
    m_csr_mtimecmp    = value;
    m_csr_mip        &= ~SR_IP_STIP;
}
//-----------------------------------------------------------------
// in_super_mode: Is the CPU in SUPER mode
//-----------------------------------------------------------------
bool rv32::in_super_mode(void)
{
    return m_csr_mpriv == PRIV_SUPER;
}
//-----------------------------------------------------------------
// sbi_boot: Boot to super mode (linux boot - SBI emulation)
//-----------------------------------------------------------------
void rv32::sbi_boot(uint32_t boot_addr, uint32_t dtb_addr)
{
    m_csr_mpriv   = PRIV_SUPER;
    m_enable_sbi  = true;

    m_csr_mideleg = ~0;
    m_csr_medeleg = ~MCAUSE_ECALL_S;

    m_pc = m_pc_x = boot_addr;
    m_gpr[RISCV_REG_A0 + 0] = 0;
    m_gpr[RISCV_REG_A0 + 1] = dtb_addr;
}
//-----------------------------------------------------------------
// stats_reset: Reset runtime stats
//-----------------------------------------------------------------
void rv32::stats_reset(void)
{
    // Clear stats
    for (int i=STATS_MIN;i<STATS_MAX;i++)
        m_stats[i] = 0;
}
//-----------------------------------------------------------------
// stats_dump: Show execution stats
//-----------------------------------------------------------------
void rv32::stats_dump(void)
{  
    printf( "Runtime Stats:\n");
    printf( "- Total Instructions %d\n", m_stats[STATS_INSTRUCTIONS]);
    if (m_stats[STATS_INSTRUCTIONS] > 0)
    {
        printf( "- Loads                 %d (%d%%)\n", m_stats[STATS_LOADS],  (m_stats[STATS_LOADS] * 100)  / m_stats[STATS_INSTRUCTIONS]);
        printf( "- Stores                %d (%d%%)\n", m_stats[STATS_STORES], (m_stats[STATS_STORES] * 100) / m_stats[STATS_INSTRUCTIONS]);
        printf( "- Branch Operations     %d (%d%%)\n", m_stats[STATS_BRANCHES], (m_stats[STATS_BRANCHES] * 100)  / m_stats[STATS_INSTRUCTIONS]);
        printf( "- Multiply              %d (%d%%)\n", m_stats[STATS_MUL], (m_stats[STATS_MUL] * 100) / m_stats[STATS_INSTRUCTIONS]);
        printf( "- Division              %d (%d%%)\n", m_stats[STATS_DIV], (m_stats[STATS_DIV] * 100) / m_stats[STATS_INSTRUCTIONS]);
    }

    stats_reset();
}

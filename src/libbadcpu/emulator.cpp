#include "badcpu.h"
#include <cstdio>
#include <cstring>
#include <ucontext.h>

namespace badcpu {

static uint64_t* get_reg_ptr(ucontext_t* ctx, uint8_t reg) {
    auto& gregs = ctx->uc_mcontext.gregs;
    switch (reg) {
        case 0: return reinterpret_cast<uint64_t*>(&gregs[REG_RAX]);
        case 1: return reinterpret_cast<uint64_t*>(&gregs[REG_RCX]);
        case 2: return reinterpret_cast<uint64_t*>(&gregs[REG_RDX]);
        case 3: return reinterpret_cast<uint64_t*>(&gregs[REG_RBX]);
        case 4: return reinterpret_cast<uint64_t*>(&gregs[REG_RSP]);
        case 5: return reinterpret_cast<uint64_t*>(&gregs[REG_RBP]);
        case 6: return reinterpret_cast<uint64_t*>(&gregs[REG_RSI]);
        case 7: return reinterpret_cast<uint64_t*>(&gregs[REG_RDI]);
        default: return nullptr;
    }
}

static uint64_t get_reg(ucontext_t* ctx, uint8_t reg) {
    auto* p = get_reg_ptr(ctx, reg);
    return p ? *p : 0;
}

static uint64_t get_rm_value(ucontext_t* ctx, const DecodedInstruction& inst) {
    if ((inst.modrm >> 6) == 3) {
        return get_reg(ctx, inst.rm);
    }

    uint64_t addr = 0;
    if (inst.rm == 4 && inst.has_sib) {
        uint8_t scale = (inst.sib >> 6) & 0x03;
        uint8_t index = (inst.sib >> 3) & 0x07;
        uint8_t base = inst.sib & 0x07;

        uint64_t base_val = (base == 5 && (inst.modrm >> 6) == 0)
                                ? static_cast<uint64_t>(static_cast<int32_t>(inst.displacement))
                                : get_reg(ctx, base);
        uint64_t index_val = (index == 4) ? 0 : get_reg(ctx, index);
        addr = base_val + (index_val << scale);
    } else if (inst.rm == 5 && (inst.modrm >> 6) == 0) {
        addr = static_cast<uint64_t>(static_cast<int32_t>(inst.displacement));
    } else {
        addr = get_reg(ctx, inst.rm);
    }

    uint8_t mod = (inst.modrm >> 6) & 0x03;
    if (mod == 1) {
        addr += static_cast<int8_t>(inst.displacement);
    } else if (mod == 2) {
        addr += static_cast<int32_t>(inst.displacement);
    }

    return addr;
}

static EmulationResult emulate_popcnt(const uint8_t* ip, ucontext_t* ctx,
                                       const DecodedInstruction& inst) {
    uint64_t src = get_rm_value(ctx, inst);
    uint64_t count = __builtin_popcountll(src);
    *get_reg_ptr(ctx, inst.reg) = count;

    uint64_t& flags = *reinterpret_cast<uint64_t*>(&ctx->uc_mcontext.gregs[REG_EFL]);
    flags &= ~static_cast<uint64_t>(0x844); // clear ZF, SF, CF
    if (count == 0) flags |= 0x40; // ZF

    ctx->uc_mcontext.gregs[REG_RIP] += inst.len;
    return EmulationResult::Success;
}

static EmulationResult emulate_movbe(const uint8_t* ip, ucontext_t* ctx,
                                       const DecodedInstruction& inst) {
    if (inst.has_66) {
        uint64_t addr = get_rm_value(ctx, inst);
        if ((inst.modrm >> 6) == 3) {
            uint16_t val = static_cast<uint16_t>(get_reg(ctx, inst.rm));
            uint16_t swapped = __builtin_bswap16(val);
            *get_reg_ptr(ctx, inst.reg) = (*get_reg_ptr(ctx, inst.reg) & ~0xFFFFu) | swapped;
        } else {
            uint16_t val;
            memcpy(&val, reinterpret_cast<void*>(addr), 2);
            val = __builtin_bswap16(val);
            *get_reg_ptr(ctx, inst.reg) = (*get_reg_ptr(ctx, inst.reg) & ~0xFFFFu) | val;
        }
    } else {
        uint64_t addr = get_rm_value(ctx, inst);
        if ((inst.modrm >> 6) == 3) {
            uint32_t val = static_cast<uint32_t>(get_reg(ctx, inst.rm));
            uint32_t swapped = __builtin_bswap32(val);
            *get_reg_ptr(ctx, inst.reg) = swapped;
        } else {
            uint32_t val;
            memcpy(&val, reinterpret_cast<void*>(addr), 4);
            val = __builtin_bswap32(val);
            *get_reg_ptr(ctx, inst.reg) = val;
        }
    }

    ctx->uc_mcontext.gregs[REG_RIP] += inst.len;
    return EmulationResult::Success;
}

static EmulationResult emulate_lzcnt_tzcnt(const uint8_t* ip, ucontext_t* ctx,
                                              const DecodedInstruction& inst, bool is_lzcnt) {
    uint64_t src = get_rm_value(ctx, inst);
    uint64_t result;

    if (is_lzcnt) {
        if (src == 0) {
            result = inst.has_66 ? 16 : 32;
            uint64_t& flags = *reinterpret_cast<uint64_t*>(&ctx->uc_mcontext.gregs[REG_EFL]);
            flags |= 0x40; // ZF
        } else {
            result = inst.has_66 ? __builtin_clz(static_cast<uint16_t>(src)) - 16
                                  : __builtin_clz(static_cast<uint32_t>(src));
        }
    } else {
        if (src == 0) {
            result = inst.has_66 ? 16 : 32;
            uint64_t& flags = *reinterpret_cast<uint64_t*>(&ctx->uc_mcontext.gregs[REG_EFL]);
            flags |= 0x40; // ZF
        } else {
            result = inst.has_66 ? __builtin_ctz(static_cast<uint16_t>(src))
                                  : __builtin_ctz(static_cast<uint32_t>(src));
        }
    }

    *get_reg_ptr(ctx, inst.reg) = result;
    ctx->uc_mcontext.gregs[REG_RIP] += inst.len;
    return EmulationResult::Success;
}

static EmulationResult emulate_bmi(uint8_t op2, const uint8_t* ip, ucontext_t* ctx,
                                     const DecodedInstruction& inst) {
    uint64_t src1 = get_reg(ctx, inst.reg);
    uint64_t src2 = get_rm_value(ctx, inst);
    uint64_t result;

    switch (op2) {
        case 0xF0: result = src1 & ~src2; break; // ANDN
        case 0xF3: // BLSI
            result = src2 & (-src2);
            break;
        case 0xF2: // BLSMSK
            result = src2 ^ (src2 - 1);
            break;
        case 0xF5: // BLSR
            result = src2 & (src2 - 1);
            break;
        default:
            return EmulationResult::UnrecognizedInstruction;
    }

    *get_reg_ptr(ctx, inst.reg) = result;
    ctx->uc_mcontext.gregs[REG_RIP] += inst.len;
    return EmulationResult::Success;
}

EmulationResult emulate_instruction(const uint8_t* ip, const CpuFeatures& features,
                                      ucontext_t* ctx) {
    DecodedInstruction inst = decode_instruction(ip);

    if (inst.opcode[0] == 0x0F) {
        uint8_t op2 = inst.opcode[1];

        // POPCNT r, r/m (0F B8 /1)
        if (op2 == 0xB8 && inst.has_modrm && (inst.reg & 7) == 1) {
            if (!features.has_popcnt()) {
                return emulate_popcnt(ip, ctx, inst);
            }
        }

        // MOVBE (0F 38 F0/F1)
        if (op2 == 0x38) {
            uint8_t op3 = inst.opcode[2];
            if (op3 == 0xF0 || op3 == 0xF1) {
                return emulate_movbe(ip, ctx, inst);
            }
            // LZCNT (0F 38 F5)
            if (op3 == 0xF5 && inst.has_f3) {
                return emulate_lzcnt_tzcnt(ip, ctx, inst, true);
            }
        }

        // TZCNT (0F BC with F3 prefix)
        if (op2 == 0xBC && inst.has_f3) {
            return emulate_lzcnt_tzcnt(ip, ctx, inst, false);
        }
        // LZCNT (0F BD with F3 prefix)
        if (op2 == 0xBD && inst.has_f3) {
            return emulate_lzcnt_tzcnt(ip, ctx, inst, true);
        }

        // BMI1: ANDN (0F 38 F2), BLSI/BLSMSK/BLSR (0F 38 F3 with different reg)
        // BMI2: various 0F 38 ops
        if (op2 == 0x38 || op2 == 0x3A) {
            uint8_t op3 = inst.opcode[2];
            if (op3 >= 0xF0 && op3 <= 0xF7) {
                return emulate_bmi(op3, ip, ctx, inst);
            }
        }
    }

    return EmulationResult::UnrecognizedInstruction;
}

void fatal_unrecognized_instruction(const uint8_t* ip, size_t max_len) {
    fprintf(stderr, "---------------------------------------------------------\n");
    fprintf(stderr, "FATAL: Unrecognized Instruction\n");
    fprintf(stderr, "Instruction Bytes:");
    for (size_t i = 0; i < max_len && i < 16; i++) {
        fprintf(stderr, " %02x", ip[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, 
        "While your CPU supports SSE 4.1, illegal instruction errors can also "
        "be thrown due to the program encountering something it didn't expect "
        "as a way to quickly stop itself. Check your configuration, verify file "
        "integrity, and try again. If nothing works, you should probably report "
        "this as a bug.\n");
    _exit(1);
}

} // namespace badcpu
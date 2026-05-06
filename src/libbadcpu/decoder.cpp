#include "badcpu.h"
#include <cstring>

namespace badcpu {

static void decode_modrm(uint8_t modrm, uint8_t& mod, uint8_t& reg, uint8_t& rm) {
    mod = (modrm >> 6) & 0x03;
    reg = (modrm >> 3) & 0x07;
    rm = modrm & 0x07;
}

static void decode_sib(uint8_t sib, uint8_t& scale, uint8_t& index, uint8_t& base) {
    scale = (sib >> 6) & 0x03;
    index = (sib >> 3) & 0x07;
    base = sib & 0x07;
}

static int displacement_size(uint8_t mod, uint8_t rm, bool has_sib_byte) {
    if (mod == 0) {
        if (rm == 5 && !has_sib_byte) return 4;
        if (rm == 4 && has_sib_byte) return -1; // need sib.base check
        return 0;
    }
    if (mod == 1) return 1;
    if (mod == 2) return 4;
    return 0;
}

static int displacement_size_with_sib(uint8_t mod, uint8_t base) {
    if (mod == 0 && base == 5) return 4;
    if (mod == 1) return 1;
    if (mod == 2) return 4;
    return 0;
}

DecodedInstruction decode_instruction(const uint8_t* ip) {
    DecodedInstruction inst{};
    size_t pos = 0;

    // Legacy prefixes
    while (pos < 15) {
        uint8_t b = ip[pos];
        if (b == 0x66 || b == 0x67 || b == 0xF2 || b == 0xF3 ||
            b == 0x26 || b == 0x2E || b == 0x36 || b == 0x3E ||
            b == 0x64 || b == 0x65) {
            if (inst.prefix_count < 4) {
                inst.prefixes[inst.prefix_count++] = b;
            }
            pos++;
        } else {
            break;
        }
    }

    // REX prefix (0x40-0x4F) — must come after legacy prefixes, before opcode
    if (pos < 15 && (ip[pos] & 0xF0) == 0x40) {
        inst.rex = ip[pos];
        inst.has_rex = true;
        pos++;
    }

    for (uint8_t i = 0; i < inst.prefix_count; i++) {
        if (inst.prefixes[i] == 0x66) inst.has_66 = true;
        if (inst.prefixes[i] == 0xF2) inst.has_f2 = true;
        if (inst.prefixes[i] == 0xF3) inst.has_f3 = true;
    }

    uint8_t first_byte = ip[pos];
    uint8_t opcode_map = 0; // 0=1-byte, 1=0F, 2=0F38, 3=0F3A

    // VEX prefix
    if (first_byte == 0xC4 || first_byte == 0xC5) {
        inst.is_vex = true;
        pos++;
        uint8_t vex_w = 0, vex_r = 0, vex_x = 0, vex_b = 0;
        uint8_t vex_m = 1;
        uint8_t vex_pp = 0;

        if (first_byte == 0xC4) {
            uint8_t b1 = ip[pos++];
            vex_r = (b1 >> 7) & 1;
            vex_x = (b1 >> 6) & 1;
            vex_b = (b1 >> 5) & 1;
            vex_m = b1 & 0x1F;
            uint8_t b2 = ip[pos++];
            vex_w = (b2 >> 7) & 1;
            vex_pp = b2 & 0x03;
        } else {
            uint8_t b1 = ip[pos++];
            vex_r = (b1 >> 7) & 1;
            vex_pp = b1 & 0x03;
        }

        // VEX R/X/B/W are inverted; convert to normal REX form
        inst.rex = (vex_w << 3) | ((~vex_r & 1) << 2) | ((~vex_x & 1) << 1) | ((~vex_b & 1) << 0);
        inst.has_rex = true;

        if (vex_pp == 1) inst.has_66 = true;
        if (vex_pp == 2) inst.has_f2 = true;
        if (vex_pp == 3) inst.has_f3 = true;

        if (vex_m == 1) opcode_map = 1;
        else if (vex_m == 2) opcode_map = 2;
        else if (vex_m == 3) opcode_map = 3;

        first_byte = ip[pos++];
        inst.is_simd = true;
    }

    // Parse opcode bytes
    if (!inst.is_vex && first_byte == 0x0F) {
        opcode_map = 1;
        inst.opcode[0] = 0x0F;
        inst.opcode[1] = ip[pos + 1];
        inst.opcode_len = 2;
        pos += 2;
        uint8_t op2 = inst.opcode[1];
        if (op2 == 0x38 || op2 == 0x3A) {
            opcode_map = (op2 == 0x38) ? 2 : 3;
            inst.opcode[2] = ip[pos];
            inst.opcode_len = 3;
            pos++;
        }
    } else if (inst.is_vex) {
        if (opcode_map == 1) {
            inst.opcode[0] = 0x0F;
            inst.opcode[1] = first_byte;
            inst.opcode_len = 2;
        } else if (opcode_map == 2) {
            inst.opcode[0] = 0x0F;
            inst.opcode[1] = 0x38;
            inst.opcode[2] = first_byte;
            inst.opcode_len = 3;
        } else if (opcode_map == 3) {
            inst.opcode[0] = 0x0F;
            inst.opcode[1] = 0x3A;
            inst.opcode[2] = first_byte;
            inst.opcode_len = 3;
        } else {
            inst.opcode[0] = first_byte;
            inst.opcode_len = 1;
        }
    } else {
        inst.opcode[0] = first_byte;
        inst.opcode_len = 1;
    }

    // Determine if this opcode needs ModR/M and whether it's SIMD
    bool needs_modrm = false;
    bool is_simd = false;

    if (opcode_map == 1) {
        uint8_t op2 = inst.opcode[1];
        needs_modrm = (op2 < 0x06) || (op2 >= 0x10 && op2 <= 0x73) ||
                      (op2 >= 0x7C && op2 <= 0x7F) ||
                      (op2 >= 0x90 && op2 <= 0x9F) ||
                      (op2 >= 0xA3 && op2 <= 0xAF) ||
                      (op2 >= 0xB0 && op2 <= 0xBF) ||
                      (op2 >= 0xC0 && op2 <= 0xCF) ||
                      (op2 >= 0xD0 && op2 <= 0xDF) ||
                      (op2 >= 0xE0 && op2 <= 0xEF) ||
                      (op2 >= 0xF0);
        is_simd = (op2 >= 0x10 && op2 <= 0x7F) ||
                  (op2 >= 0x90 && op2 != 0xA0 && op2 != 0xA1 && op2 != 0xA2 && op2 != 0xA8 && op2 != 0xA9 && op2 != 0xB0 && op2 != 0xB1) ||
                  (op2 >= 0xC4 && op2 <= 0xCF) ||
                  (op2 >= 0xD0 && op2 <= 0xDF) ||
                  (op2 >= 0xE0 && op2 <= 0xEF);
    } else if (opcode_map == 2 || opcode_map == 3) {
        needs_modrm = true;
        is_simd = true;
    } else {
        // 1-byte opcode map
        static const bool one_byte_modrm[256] = {
            1,1,1,1, 0,0,0,0, 1,1,1,1, 0,0,0,0,
            1,1,1,1, 0,0,0,0, 1,1,1,1, 0,0,0,0,
            1,1,1,1, 0,0,0,0, 1,1,1,1, 0,0,0,0,
            1,1,1,1, 0,0,0,0, 1,1,1,1, 0,0,0,0,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
            0,0,1,1, 0,0,0,0, 0,1,0,1, 0,0,0,0,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
            1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
            1,1,0,0, 1,1,1,1, 0,0,0,0, 0,0,0,0,
            1,1,1,1, 0,0,0,0, 1,1,1,1, 1,1,1,1,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
            0,0,0,0, 0,0,1,1, 0,0,0,0, 0,0,1,1,
        };
        if (first_byte < 256) {
            needs_modrm = one_byte_modrm[first_byte];
        }
    }

    inst.is_simd = is_simd;

    // Parse ModR/M, SIB, displacement
    if (needs_modrm && pos < 15) {
        uint8_t modrm_byte = ip[pos];
        uint8_t mod, reg_field, rm_field;
        decode_modrm(modrm_byte, mod, reg_field, rm_field);
        inst.modrm = modrm_byte;
        inst.reg = reg_field | (((inst.rex >> 2) & 1) << 3);
        inst.rm = rm_field | (((inst.rex >> 0) & 1) << 3);
        inst.has_modrm = true;
        pos++;

        if (mod != 3) {
            if (rm_field == 4) {
                uint8_t sib_byte = ip[pos];
                uint8_t scale, index_field, base_field;
                decode_sib(sib_byte, scale, index_field, base_field);
                inst.sib = sib_byte;
                inst.has_sib = true;
                pos++;

                int disp_sz = displacement_size_with_sib(mod, base_field);
                if (disp_sz > 0 && pos + disp_sz <= 15) {
                    inst.has_displacement = true;
                    if (disp_sz == 4) {
                        memcpy(&inst.displacement, ip + pos, 4);
                    } else {
                        inst.displacement = static_cast<int8_t>(ip[pos]);
                    }
                    pos += disp_sz;
                }
            } else {
                int disp_sz = displacement_size(mod, rm_field, false);
                if (disp_sz > 0 && pos + disp_sz <= 15) {
                    inst.has_displacement = true;
                    if (disp_sz == 4) {
                        memcpy(&inst.displacement, ip + pos, 4);
                    } else {
                        inst.displacement = static_cast<int8_t>(ip[pos]);
                    }
                    pos += disp_sz;
                }
            }
        }
    }

    inst.operand_size = inst.has_66 ? 16 : 32;
    if ((inst.rex & 0x08) && !inst.has_66) {
        inst.operand_size = 64; // REX.W overrides to 64-bit
    }
    inst.len = static_cast<uint8_t>(pos);
    return inst;
}

} // namespace badcpu

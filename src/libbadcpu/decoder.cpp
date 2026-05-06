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

    for (uint8_t i = 0; i < inst.prefix_count; i++) {
        if (inst.prefixes[i] == 0x66) inst.has_66 = true;
        if (inst.prefixes[i] == 0xF2) inst.has_f2 = true;
        if (inst.prefixes[i] == 0xF3) inst.has_f3 = true;
    }

    uint8_t opcode = ip[pos];

    if (opcode == 0xC4 || opcode == 0xC5) {
        inst.is_vex = true;
        pos++;
        if (opcode == 0xC4) {
            pos++; // vex mmmmm
            pos++; // vex vvvv/L/pp
        } else {
            pos++; // vex vvvv/L/pp
        }
        inst.is_simd = true;
        inst.opcode[0] = opcode;
        inst.opcode_len = 1;
        inst.len = static_cast<uint8_t>(pos);
        return inst;
    }

    if (opcode == 0x0F) {
        inst.opcode[0] = opcode;
        inst.opcode[1] = ip[pos + 1];
        inst.opcode_len = 2;
        pos += 2;

        uint8_t op2 = inst.opcode[1];

        if (op2 == 0x38 || op2 == 0x3A) {
            inst.opcode[2] = ip[pos];
            pos++;
        }

        bool needs_modrm = (op2 < 0x06) || (op2 >= 0x10 && op2 <= 0x73) ||
                           (op2 >= 0x7C && op2 <= 0x7F) ||
                           (op2 >= 0x90 && op2 <= 0x9F) ||
                           (op2 >= 0xA3 && op2 <= 0xAF) ||
                           (op2 >= 0xB0 && op2 <= 0xBF) ||
                           (op2 >= 0xC0 && op2 <= 0xCF) ||
                           (op2 >= 0xD0 && op2 <= 0xDF) ||
                           (op2 >= 0xE0 && op2 <= 0xEF) ||
                           (op2 >= 0xF0);

        inst.is_simd = (op2 >= 0x10 && op2 <= 0x7F) ||
                       (op2 >= 0x90 && op2 != 0xA0 && op2 != 0xA1 && op2 != 0xA2 && op2 != 0xA8 && op2 != 0xA9 && op2 != 0xB0 && op2 != 0xB1) ||
                       (op2 >= 0xC4 && op2 <= 0xCF) ||
                       (op2 >= 0xD0 && op2 <= 0xDF) ||
                       (op2 >= 0xE0 && op2 <= 0xEF);

        if (needs_modrm && pos < 15) {
            uint8_t modrm_byte = ip[pos];
            uint8_t mod, reg_field, rm_field;
            decode_modrm(modrm_byte, mod, reg_field, rm_field);
            inst.modrm = modrm_byte;
            inst.reg = reg_field;
            inst.rm = rm_field;
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
                    if (disp_sz > 0) {
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
                    if (disp_sz > 0) {
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
    } else {
        inst.opcode[0] = opcode;
        inst.opcode_len = 1;
        pos++;

        static const bool one_byte_modrm[256] = {
            0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0,
            0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0,
            0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0,
            0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0,
            1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
            0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0,
            0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0,
            0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0,
            1,1,0,0, 1,1,1,1, 0,0,0,0, 0,0,0,0,
            1,1,0,0, 1,1,1,1, 0,0,0,0, 0,0,0,0,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
            1,1,1,1, 0,0,0,0, 1,1,1,1, 0,0,0,0,
            1,1,1,1, 0,0,0,0, 1,1,1,1, 0,0,0,0,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
            0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        };

        if (opcode < 256 && one_byte_modrm[opcode] && pos < 15) {
            uint8_t modrm_byte = ip[pos];
            uint8_t mod, reg_field, rm_field;
            decode_modrm(modrm_byte, mod, reg_field, rm_field);
            inst.modrm = modrm_byte;
            inst.reg = reg_field;
            inst.rm = rm_field;
            inst.has_modrm = true;
            pos++;

            if (mod != 3) {
                if (rm_field == 4 && pos < 15) {
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
    }

    inst.operand_size = inst.has_66 ? 16 : 32;
    inst.len = static_cast<uint8_t>(pos);
    return inst;
}

} // namespace badcpu
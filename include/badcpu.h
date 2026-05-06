#ifndef BADCPU_H
#define BADCPU_H

#include <cstdint>
#include <csignal>
#include <cstring>
#include <string>
#include <array>

namespace badcpu {

enum class CpuVendor : uint32_t {
    Intel = 0,
    AMD = 1,
    Hygon = 2,
    Unknown = 3,
};

struct CpuFeatures {
    CpuVendor vendor = CpuVendor::Unknown;
    uint32_t max_standard_leaf = 0;
    uint32_t feature_ecx = 0;
    uint32_t feature_edx = 0;
    uint32_t max_extended_leaf = 0;
    uint32_t ext_feature_ecx = 0;
    uint32_t ext_feature_edx = 0;

    bool has_sse3() const { return feature_ecx & (1u << 0); }
    bool has_ssse3() const { return feature_ecx & (1u << 9); }
    bool has_sse41() const { return feature_ecx & (1u << 19); }
    bool has_sse42() const { return feature_ecx & (1u << 20); }
    bool has_popcnt() const { return feature_ecx & (1u << 23); }
    bool has_avx() const { return feature_ecx & (1u << 28); }
    bool has_fma() const { return feature_ecx & (1u << 12); }
    bool has_avx2() const { return ext_feature_ecx & (1u << 5); }
    bool has_bmi1() const { return ext_feature_ecx & (1u << 3); }
    bool has_bmi2() const { return ext_feature_ecx & (1u << 8); }
};

struct X86Register {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip;
    uint64_t rflags;
};

struct DecodedInstruction {
    uint8_t len = 0;
    uint8_t prefix_count = 0;
    uint8_t prefixes[4] = {};
    uint8_t opcode[3] = {};
    uint8_t opcode_len = 0;
    uint8_t modrm = 0;
    uint8_t sib = 0;
    int32_t displacement = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint8_t operand_size = 32;
    bool has_modrm = false;
    bool has_sib = false;
    bool has_displacement = false;
    bool is_simd = false;
    bool is_vex = false;
    bool has_66 = false;
    bool has_f2 = false;
    bool has_f3 = false;
    uint8_t rex = 0;
    bool has_rex = false;
    uint8_t vex_vvvv = 0;
    bool vex_w = false;
};

enum class EmulationResult : uint8_t {
    Success = 0,
    UnrecognizedInstruction = 1,
    UnsupportedCpu = 2,
    HandlerError = 3,
};

CpuFeatures detect_cpu_features();
bool install_sigill_handler();
void remove_sigill_handler();

EmulationResult emulate_instruction(const uint8_t* ip, const CpuFeatures& features,
                                      ucontext_t* ctx);
DecodedInstruction decode_instruction(const uint8_t* ip);
void print_cpu_info(const CpuFeatures& features);
void fatal_unrecognized_instruction(const uint8_t* ip, size_t max_len);

} // namespace badcpu

#endif // BADCPU_H
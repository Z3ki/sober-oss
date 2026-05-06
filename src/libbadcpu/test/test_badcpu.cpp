#include "badcpu.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

static void test_cpu_features() {
    printf("=== CPU Feature Detection ===\n");
    auto features = badcpu::detect_cpu_features();
    const char* vendor = "Unknown";
    switch (features.vendor) {
        case badcpu::CpuVendor::Intel: vendor = "Intel"; break;
        case badcpu::CpuVendor::AMD: vendor = "AMD"; break;
        case badcpu::CpuVendor::Hygon: vendor = "Hygon"; break;
        default: break;
    }
    printf("CPU Vendor: %s\n", vendor);
    printf("SSE3:    %s\n", features.has_sse3() ? "yes" : "no");
    printf("SSSE3:   %s\n", features.has_ssse3() ? "yes" : "no");
    printf("SSE4.1:  %s\n", features.has_sse41() ? "yes" : "no");
    printf("SSE4.2:  %s\n", features.has_sse42() ? "yes" : "no");
    printf("POPCNT:  %s\n", features.has_popcnt() ? "yes" : "no");
    printf("AVX:     %s\n", features.has_avx() ? "yes" : "no");
    printf("AVX2:    %s\n", features.has_avx2() ? "yes" : "no");
    printf("\n");
}

static void test_instruction_decode() {
    printf("=== Instruction Decode ===\n");

    // POPCNT rax, rbx = F3 0F B8 C3
    {
        uint8_t popcnt[] = {0xF3, 0x0F, 0xB8, 0xC3};
        auto inst = badcpu::decode_instruction(popcnt);
        printf("POPCNT: len=%u, opcode=%02x %02x, modrm=%02x, "
               "has_modrm=%d, has_f3=%d, is_simd=%d\n",
               inst.len, inst.opcode[0], inst.opcode[1],
               inst.modrm, inst.has_modrm, inst.has_f3, inst.is_simd);
    }

    // MOVBE rax, [rbx] = 0F 38 F0 03
    {
        uint8_t movbe[] = {0x0F, 0x38, 0xF0, 0x03};
        auto inst = badcpu::decode_instruction(movbe);
        printf("MOVBE: len=%u, opcode=%02x %02x %02x, modrm=%02x, "
               "has_modrm=%d\n",
               inst.len, inst.opcode[0], inst.opcode[1], inst.opcode[2],
               inst.modrm, inst.has_modrm);
    }

    // Simple ADD rax, rbx = 48 01 D8
    {
        uint8_t add[] = {0x48, 0x01, 0xD8};
        auto inst = badcpu::decode_instruction(add);
        printf("ADD: len=%u, opcode=%02x, modrm=%02x, has_modrm=%d\n",
               inst.len, inst.opcode[0], inst.modrm, inst.has_modrm);
    }
    printf("\n");
}

static void test_sigill_handler() {
    printf("=== SIGILL Handler Test ===\n");

    if (!badcpu::install_sigill_handler()) {
        printf("FAIL: Could not install SIGILL handler (CPU may not support SSE4.1)\n");
        return;
    }
    printf("PASS: SIGILL handler installed successfully\n");

    badcpu::remove_sigill_handler();
    printf("PASS: SIGILL handler removed successfully\n\n");
}

int main() {
    printf("libbadcpu.so test suite\n");
    printf("========================\n\n");

    test_cpu_features();
    test_instruction_decode();
    test_sigill_handler();

    printf("All tests completed.\n");
    return 0;
}
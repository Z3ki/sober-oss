#include "badcpu.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ucontext.h>
#include <signal.h>
#include <unistd.h>

static int failures = 0;

static void expect_eq(const char* what, uint64_t got, uint64_t want) {
    if (got == want) {
        printf("  ok   %-26s 0x%llx\n", what, (unsigned long long)got);
    } else {
        printf("  FAIL %-26s got 0x%llx want 0x%llx\n", what,
               (unsigned long long)got, (unsigned long long)want);
        failures++;
    }
}

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
    printf("CPU Vendor: %s  SSE4.1:%d POPCNT:%d AVX:%d AVX2:%d\n\n", vendor,
           features.has_sse41(), features.has_popcnt(),
           features.has_avx(), features.has_avx2());
}

// helpers for the emulation tests
static void setreg(ucontext_t& c, int r, uint64_t v){ c.uc_mcontext.gregs[r]=(greg_t)v; }
static uint64_t getreg(ucontext_t& c, int r){ return (uint64_t)c.uc_mcontext.gregs[r]; }

static void test_emulation() {
    printf("=== Instruction Emulation (vs real x86 semantics) ===\n");
    badcpu::CpuFeatures nofeat{}; // has_popcnt()==false etc, forces emulation

    // POPCNT eax, ecx  (F3 0F B8 C1)
    {
        ucontext_t c; memset(&c, 0, sizeof c);
        setreg(c, REG_RCX, 0x0F0F); setreg(c, REG_RAX, 0xdead);
        uint8_t code[] = {0xF3,0x0F,0xB8,0xC1};
        setreg(c, REG_RIP, (uint64_t)code);
        badcpu::emulate_instruction(code, nofeat, &c);
        expect_eq("POPCNT", getreg(c, REG_RAX), 8);
    }
    // MOVBE eax, [rbx]  (0F 38 F0 03)
    {
        ucontext_t c; memset(&c, 0, sizeof c);
        uint8_t buf[4] = {0x11,0x22,0x33,0x44};
        setreg(c, REG_RBX, (uint64_t)buf);
        uint8_t code[] = {0x0F,0x38,0xF0,0x03};
        setreg(c, REG_RIP, (uint64_t)code);
        badcpu::emulate_instruction(code, nofeat, &c);
        expect_eq("MOVBE", getreg(c, REG_RAX) & 0xFFFFFFFF, 0x11223344);
    }
    // LZCNT eax, ecx  (F3 0F BD C1)
    {
        ucontext_t c; memset(&c, 0, sizeof c);
        setreg(c, REG_RCX, 0x0000FFFF);
        uint8_t code[] = {0xF3,0x0F,0xBD,0xC1};
        setreg(c, REG_RIP, (uint64_t)code);
        badcpu::emulate_instruction(code, nofeat, &c);
        expect_eq("LZCNT", getreg(c, REG_RAX), 16);
    }
    // TZCNT eax, ecx  (F3 0F BC C1)
    {
        ucontext_t c; memset(&c, 0, sizeof c);
        setreg(c, REG_RCX, 0xFFFF0000);
        uint8_t code[] = {0xF3,0x0F,0xBC,0xC1};
        setreg(c, REG_RIP, (uint64_t)code);
        badcpu::emulate_instruction(code, nofeat, &c);
        expect_eq("TZCNT", getreg(c, REG_RAX), 16);
    }
    // ANDN eax, ebx, ecx  (C4 E2 60 F2 C1)  -> eax = ~ebx & ecx
    {
        ucontext_t c; memset(&c, 0, sizeof c);
        setreg(c, REG_RAX, 0xAAAAAAAA);
        setreg(c, REG_RBX, 0x0F0F0F0F); // src1 via vvvv
        setreg(c, REG_RCX, 0xFFFFFFFF); // src2 via r/m
        uint8_t code[] = {0xC4,0xE2,0x60,0xF2,0xC1};
        setreg(c, REG_RIP, (uint64_t)code);
        badcpu::emulate_instruction(code, nofeat, &c);
        expect_eq("ANDN", getreg(c, REG_RAX) & 0xFFFFFFFF, 0xF0F0F0F0);
    }
    // BLSI eax, ecx  (C4 E2 78 F3 D9, /3)  -> eax = ecx & -ecx, ebx untouched
    {
        ucontext_t c; memset(&c, 0, sizeof c);
        setreg(c, REG_RCX, 0x0000000C);
        setreg(c, REG_RAX, 0xdead); setreg(c, REG_RBX, 0xbeef);
        uint8_t code[] = {0xC4,0xE2,0x78,0xF3,0xD9};
        setreg(c, REG_RIP, (uint64_t)code);
        badcpu::emulate_instruction(code, nofeat, &c);
        expect_eq("BLSI dest", getreg(c, REG_RAX) & 0xFFFFFFFF, 0x4);
        expect_eq("BLSI keeps ebx", getreg(c, REG_RBX), 0xbeef);
    }
    // BLSR eax, ecx  (C4 E2 78 F3 C9, /1)  -> eax = ecx & (ecx-1)
    {
        ucontext_t c; memset(&c, 0, sizeof c);
        setreg(c, REG_RCX, 0x0000000C); setreg(c, REG_RAX, 0xdead);
        uint8_t code[] = {0xC4,0xE2,0x78,0xF3,0xC9};
        setreg(c, REG_RIP, (uint64_t)code);
        badcpu::emulate_instruction(code, nofeat, &c);
        expect_eq("BLSR", getreg(c, REG_RAX) & 0xFFFFFFFF, 0x8);
    }
    // BLSMSK eax, ecx  (C4 E2 78 F3 D1, /2)  -> eax = ecx ^ (ecx-1)
    {
        ucontext_t c; memset(&c, 0, sizeof c);
        setreg(c, REG_RCX, 0x0000000C); setreg(c, REG_RAX, 0xdead);
        uint8_t code[] = {0xC4,0xE2,0x78,0xF3,0xD1};
        setreg(c, REG_RIP, (uint64_t)code);
        badcpu::emulate_instruction(code, nofeat, &c);
        expect_eq("BLSMSK", getreg(c, REG_RAX) & 0xFFFFFFFF, 0x7);
    }
    printf("\n");
}

static void test_decode_lengths() {
    printf("=== Decode Length Sanity ===\n");
    // REX.W ADD eax path: 48 01 D8 -> 3 bytes, modrm D8
    uint8_t add[] = {0x48,0x01,0xD8};
    auto inst = badcpu::decode_instruction(add);
    expect_eq("ADD len", inst.len, 3);
    expect_eq("ADD modrm", inst.modrm, 0xD8);
    printf("\n");
}

static void test_sigill_handler() {
    printf("=== SIGILL Handler ===\n");
    if (!badcpu::install_sigill_handler()) {
        printf("  skip (CPU lacks SSE4.1)\n\n");
        return;
    }
    badcpu::remove_sigill_handler();
    printf("  ok   install/remove\n\n");
}

int main() {
    printf("libbadcpu.so test suite\n=======================\n\n");
    test_cpu_features();
    test_decode_lengths();
    test_emulation();
    test_sigill_handler();

    if (failures) {
        printf("FAILED: %d check(s)\n", failures);
        return 1;
    }
    printf("All checks passed.\n");
    return 0;
}

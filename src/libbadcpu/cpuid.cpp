#include "badcpu.h"
#include <cpuid.h>
#include <cstdio>
#include <cstring>

namespace badcpu {

CpuFeatures detect_cpu_features() {
    CpuFeatures features{};
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    if (__get_cpuid(0, &eax, &ebx, &ecx, &edx)) {
        features.max_standard_leaf = eax;

        char vendor[13];
        memcpy(vendor, &ebx, 4);
        memcpy(vendor + 4, &edx, 4);
        memcpy(vendor + 8, &ecx, 4);
        vendor[12] = '\0';

        if (strcmp(vendor, "GenuineIntel") == 0) {
            features.vendor = CpuVendor::Intel;
        } else if (strcmp(vendor, "AuthenticAMD") == 0) {
            features.vendor = CpuVendor::AMD;
        } else if (strcmp(vendor, "HygonGenuine") == 0) {
            features.vendor = CpuVendor::Hygon;
        } else {
            features.vendor = CpuVendor::Unknown;
        }

        if (features.max_standard_leaf >= 1) {
            uint32_t feat_eax = 0, feat_ebx = 0, feat_ecx = 0, feat_edx = 0;
            if (__get_cpuid(1, &feat_eax, &feat_ebx, &feat_ecx, &feat_edx)) {
                features.feature_ecx = feat_ecx;
                features.feature_edx = feat_edx;
            }
        }
    }

    uint32_t ext_eax = 0, ext_ebx = 0, ext_ecx = 0, ext_edx = 0;
    if (__get_cpuid(0x80000000, &ext_eax, &ext_ebx, &ext_ecx, &ext_edx)) {
        features.max_extended_leaf = ext_eax;

        if (features.max_extended_leaf >= 0x80000001) {
            uint32_t feat7_eax = 0, feat7_ebx = 0, feat7_ecx = 0, feat7_edx = 0;
            if (__get_cpuid(0x80000001, &feat7_eax, &feat7_ebx, &feat7_ecx, &feat7_edx)) {
                features.ext_feature_ecx = feat7_ecx;
                features.ext_feature_edx = feat7_edx;
            }
        }
    }

    return features;
}

void print_cpu_info(const CpuFeatures& features) {
    const char* vendor_str = "Unknown";
    switch (features.vendor) {
        case CpuVendor::Intel: vendor_str = "GenuineIntel"; break;
        case CpuVendor::AMD: vendor_str = "AuthenticAMD"; break;
        case CpuVendor::Hygon: vendor_str = "HygonGenuine"; break;
        default: break;
    }

    fprintf(stderr, "CPU Vendor: %s\n", vendor_str);

    if (features.max_extended_leaf >= 0x80000004) {
        uint32_t brand[12];
        for (uint32_t i = 0; i < 3; i++) {
            __get_cpuid(0x80000002 + i, &brand[i * 4], &brand[i * 4 + 1],
                        &brand[i * 4 + 2], &brand[i * 4 + 3]);
        }
        char brand_str[49];
        memcpy(brand_str, brand, 48);
        brand_str[48] = '\0';
        fprintf(stderr, "CPU Brand: %s\n", brand_str);
    }

    fprintf(stderr, "CPU Feature Support:\n");
    fprintf(stderr, "  SSE3:    %s\n", features.has_sse3() ? "yes" : "no");
    fprintf(stderr, "  SSSE3:   %s\n", features.has_ssse3() ? "yes" : "no");
    fprintf(stderr, "  SSE4.1:  %s\n", features.has_sse41() ? "yes" : "no");
    fprintf(stderr, "  SSE4.2:  %s\n", features.has_sse42() ? "yes" : "no");
    fprintf(stderr, "  POPCNT:  %s\n", features.has_popcnt() ? "yes" : "no");
    fprintf(stderr, "  AVX:     %s\n", features.has_avx() ? "yes" : "no");
    fprintf(stderr, "  AVX2:    %s\n", features.has_avx2() ? "yes" : "no");
}

} // namespace badcpu
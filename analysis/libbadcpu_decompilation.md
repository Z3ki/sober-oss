# Issue #4: libbadcpu.so Decompilation Report

## Binary Info
- **File**: `libbadcpu.so`
- **Size**: 376 KB
- **Language**: Rust (confirmed by panic strings, allocators)
- **Format**: ELF 64-bit LSB shared object, x86-64, stripped
- **Ghidra Coverage**: 446 functions, ~54K lines of decompiled C pseudocode

## Entry Point & Initialization

The binary installs itself via `DT_INIT`/`constructor` hooks. We identified the init sequence:

- `_DT_INIT @ 00115c5c`: Standard ELF init (calls `__gmon_start__` if present)
- `_INIT_0 / _INIT_1 / _INIT_2`: Rust runtime initialization, thread-local storage setup
- **Runtime startup**: `FUN_00141060` / `FUN_001411b0` - Appears to be Rust `std::rt::lang_start` entry with panic handling infrastructure
- `FUN_00141710` / `FUN_001417c0` - Additional runtime setup / TLS finalizer

## CPUID Feature Detection (`FUN_00117920`)

Function at `00117920` queries CPU features via the `cpuid` instruction:

```c
undefined4 * FUN_00117920(undefined4 *param_1)
{
    // Calls cpuid_basic_info(0) - get vendor ID
    puVar1 = (undefined4 *)cpuid_basic_info(0);  // EAX=0
    // Stores vendor string (GenuineIntel / AuthenticAMD / HygonGenuine)
    // Calls cpuid(0x80000000) - extended features
    // Returns vendor ID, max standard leaf, max extended leaf
    *param_1 = uVar7;     // vendor enum (0=Intel, 1=AMD, 2=Hygon)
    param_1[1] = uVar5;   // max standard leaf
    param_1[2] = uVar6;   // feature bits ECX
    param_1[3] = uVar4;   // feature bits EDX
    param_1[4] = uVar2;   // max standard leaf from basic
    param_1[5] = uVar3;   // max extended leaf
    return param_1;
}
```

**Vendor Detection Logic**:
- Compares EBX+ECX+EDX against known vendor strings
- Intel: `GenuineIntel`
- AMD: `AuthenticAMD`
- Hygon: `HygonGenuine`

## The Instruction Emulator Core (`FUN_00152e80`)

This is the heart of libbadcpu - a ~1500-line function implementing the signal-handler based instruction emulator.

**Architecture**:
State machine driven by lookup tables. Uses a sliding window / ring buffer pattern for the copy loop (likely a `memmove` or `memcpy` with overlap handling).

```
Key data structures (inferred):
- param_2 at offsets 0x27b4, 0x210, 0xa10: lookup tables for instruction decode
- `local_68` / `uStack_58`: bit buffer (holds instruction bitstream)
- `local_70` / `local_78`: source / destination pointers for copy operations
- `uStack_5c`: state machine control variable (0x0c, 0x13, 0x14, 0x15, 0x17, etc.)
```

**State Machine States** (from switch/case analysis):
- `0x0c`: Copy/memmove state - transfers decoded instruction bytes
- `0x13`: Overlapping copy with wraparound (ring buffer)
- `0x14`: Bit extraction / alignment
- `0x15`: Error path - outputs "FATAL: Unrecognized Instruction" message
- `0x16`: Ring buffer boundary check
- `0x17`: Byte-by-byte copy fallback with bit shifting
- `0x18+`: Default/error branches

**Lookup Tables**:
- `param_2 + 0x210 + (bits & 0x3ff) * 2`: primary decode table (10-bit index, 16-bit entries)
- `param_2 + 0xa10 + index * 2`: secondary decode table (for extended opcodes)
- Entries encode `length | (destination << 9)` or negative values for multi-level decode

**The "FATAL: Unrecognized Instruction" Path** (case 0x15):
At offset `00154a65`, when the decoder reaches an unhandled opcode pattern, it constructs the error message by indexing into a string table:

```
bStack_54 = "---------------------------------------------------------\nFATAL: Unrecognized Instruction\n" [(ulong)uVar19 + 0x5a];
```

This extracts instruction bytes from the bitbuffer and prints them alongside the error.

## Memory Copy Helpers

### `FUN_00155300` - Overlapping Memory Copy
Implements `memmove` with overlap detection:
- Checks if source overlaps destination
- If overlap: copies backward byte-by-byte
- If no overlap: delegates to forward copy

### `FUN_001558f0` - Forward Byte Copy
Simple `memcpy`-like function with alignment checks.

### `FUN_00155440` - Extended Decode + Copy
340-line function combining the lookup table decode with the copy engine. Appears to handle variable-length instruction encoding (x86 prefix + opcode + modr/m + SIB + displacement).

## Signal Handler Setup

The SIGILL handler registration is implemented via a Rust signal hook. We identified references to:
- `sigaction` string in the binary (imported from libc)
- `Error setting SIGILL handler` - error message if handler install fails
- Handler receives `siginfo_t` with fault address and instruction bytes

However, the actual `signal_action` / `ucontext_t` manipulation is buried in the Rust runtime and was not cleanly decompiled by Ghidra (appears as indirect function calls through vtables).

## Error Dialog / User Facing Strings

**From strings analysis**:
```
CPU Brand:
CPU Vendor:
CPU Feature Support:
This program requires a CPU with x86-64-v2 features.
We attempted to emulate missing features, but encountered an instruction that could not be handled.
Your CPU does not support at least SSE 4.1, which is required for this software to work.
While your CPU supports SSE 4.1, illegal instruction errors can also be thrown due to the program encountering something it didn't expect as a way to quickly stop itself. Check your configuration, verify file integrity, and try again. If nothing works, you should probably report this as a bug.
```

## Translation Layer Behavior (Inferred)

Based on the decompiled structures, libbadcpu implements what appears to be a **x86 instruction stream decoder + interpreter**:

1. **Install phase**: Registers SIGILL handler via `sigaction`
2. **Query phase**: Uses CPUID to detect host CPU features (SSE4.1, SSE4.2, SSSE3, AVX, POPCNT)
3. **Runtime phase**:
   - When SIGILL fires, handler receives fault address
   - Reads raw instruction bytes from fault address
   - Decodes using multi-level lookup tables (10-bit primary + overflow tables)
   - If recognized: emulates behavior by modifying `ucontext_t` registers
   - If unrecognized: prints "FATAL: Unrecognized Instruction" with hex dump, then aborts

## Static Analysis Limitations

- The Rust `panic_handler` and `#[no_mangle]` exports complicate symbol recovery
- Signal handler manipulation of `ucontext_t` is done through libc wrapper functions that Ghidra cannot resolve (indirect calls)
- The actual x86 instruction decode tables are initialized at runtime, making static analysis of the tables incomplete
- Thread-local storage access patterns (`in_FS_OFFSET`) are Rust-specific and obfuscate data flow

## Reimplementation Notes

To reimplement libbadcpu:
1. Parse CPUID leaf 0, 1, and 0x80000000+ to get vendor/feature bits
2. Install SIGILL handler with `SA_SIGINFO` flag to receive `siginfo_t` and `ucontext_t`
3. In handler: read `si_addr` (fault address) and `ucontext->uc_mcontext.gregs[REG_RIP]`
4. Disassemble bytes at RIP using an x86 decoder (e.g. Zydis, XED, or custom tables)
5. For supported instructions (SSE4.1, AVX, etc.), emulate by writing to `gregs[]` and advancing RIP
6. For unsupported: show error dialog and abort

---
*Report generated from Ghidra 11.2 decompilation of libbadcpu.so (376KB, stripped Rust ELF)*
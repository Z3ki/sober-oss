#!/usr/bin/env python3
"""Automated Ghidra headless decompilation of Sober binaries.

Usage:
    python ghidra_decompile.py [--ghidra-dir /snap/ghidra/35/ghidra_12.0_PUBLIC] [--binary-dir ./binaries] [--output-dir ./decompiled]

Requires Ghidra 12.0+ with JDK 21+.
"""

import argparse
import os
import subprocess
import sys

GHIDRA_SCRIPT = """
// Decompile all functions and write to file
//@category Decompile
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;
import java.io.*;

public class ExportAllDecompiled extends ghidra.app.script.GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);
        
        FunctionManager funcMgr = currentProgram.getFunctionManager();
        String outDir = System.getProperty("decompile.output.dir", "/tmp/sober-oss/output/decompiled");
        new File(outDir).mkdirs();
        
        String binaryName = currentProgram.getName();
        FileWriter writer = new FileWriter(new File(outDir, binaryName + ".c"));
        
        int count = 0;
        java.util.Iterator<Function> iter = funcMgr.getFunctions(true);
        while (iter.hasNext()) {
            Function func = iter.next();
            DecompileResults results = decompiler.decompileFunction(func, 60, monitor);
            if (results != null && results.getDecompiledFunction() != null) {
                String cCode = results.getDecompiledFunction().getC();
                String name = func.getName();
                String addr = func.getEntryPoint().toString();
                writer.write("// ===== " + name + " @ " + addr + " =====\\n");
                writer.write(cCode);
                writer.write("\\n\\n");
                count++;
            }
        }
        writer.close();
        decompiler.dispose();
        println("Decompiled " + count + " functions from " + binaryName);
    }
}
"""

BINARIES = {
    "sober": {"name": "SoberAnalysis", "output": "sober"},
    "sober_services": {"name": "SoberServices", "output": "sober_services"},
    "libloader.so": {"name": "SoberLoader", "output": "libloader"},
    "libbadcpu.so": {"name": "SoberBadcpu", "output": "libbadcpu"},
}

def main():
    parser = argparse.ArgumentParser(description="Decompile Sober binaries with Ghidra")
    parser.add_argument("--ghidra-dir", default="/snap/ghidra/35/ghidra_12.0_PUBLIC",
                        help="Path to Ghidra installation")
    parser.add_argument("--java-home", default="/snap/ghidra/35/usr/lib/jvm/java-21-openjdk-amd64",
                        help="Path to JDK")
    parser.add_argument("--binary-dir", default=".", help="Directory containing binaries")
    parser.add_argument("--output-dir", default="./decompiled", help="Output directory")
    parser.add_argument("--project-dir", default="/tmp/sober-decompile/ghidra-project",
                        help="Ghidra project directory")
    args = parser.parse_args()

    analyze = os.path.join(args.ghidra_dir, "support", "analyzeHeadless")
    if not os.path.exists(analyze):
        print(f"ERROR: analyzeHeadless not found at {analyze}")
        sys.exit(1)

    script_path = os.path.join(args.output_dir, "ExportAllDecompiled.java")
    os.makedirs(args.output_dir, exist_ok=True)
    with open(script_path, "w") as f:
        f.write(GHIDRA_SCRIPT)

    env = os.environ.copy()
    env["JAVA_HOME"] = args.java_home
    env["PATH"] = f"{args.java_home}/bin:{env.get('PATH', '')}"

    results = {}
    for binary, config in BINARIES.items():
        binary_path = os.path.join(args.binary_dir, binary)
        if not os.path.exists(binary_path):
            print(f"SKIP: {binary} not found at {binary_path}")
            continue

        print(f"\nDecompiling {binary}...")
        cmd = [
            analyze,
            args.project_dir,
            config["name"],
            "-import", binary_path,
            "-overwrite",
            "-postScript", script_path,
            "-scriptPath", args.output_dir,
        ]

        try:
            result = subprocess.run(cmd, env=env, capture_output=True, text=True, timeout=600)
            output = result.stdout + result.stderr
            
            # Extract function count from Ghidra output
            for line in output.split("\n"):
                if "Decompiled" in line and "functions" in line:
                    print(f"  {line.strip()}")
            
            # Move output file to proper location
            src = os.path.join(args.output_dir, f"{binary}.c")
            dst_dir = os.path.join(args.output_dir, config["output"])
            os.makedirs(dst_dir, exist_ok=True)
            dst = os.path.join(dst_dir, f"{config['output']}.c")
            if os.path.exists(src):
                os.rename(src, dst)
                results[binary] = dst
                print(f"  Output: {dst}")
        except subprocess.TimeoutExpired:
            print(f"  TIMEOUT: {binary} decompilation timed out")
        except Exception as e:
            print(f"  ERROR: {binary}: {e}")

    print(f"\nResults: {len(results)}/{len(BINARIES)} binaries decompiled")
    return results

if __name__ == "__main__":
    main()
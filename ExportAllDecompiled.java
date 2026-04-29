
//Decompile all functions and write to file
//@category Decompile
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.address.Address;
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
                writer.write("// ===== " + name + " @ " + addr + " =====\n");
                writer.write(cCode);
                writer.write("\n\n");
                count++;
            }
        }
        writer.close();
        decompiler.dispose();
        println("Decompiled " + count + " functions from " + binaryName);
    }
}

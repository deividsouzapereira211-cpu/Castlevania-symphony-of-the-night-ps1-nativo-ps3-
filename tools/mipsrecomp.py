#!/usr/bin/env python3
import sys, os

def compile_binary(filename, outdir, base_address=0x80010000):
    if "DRA.BIN" in filename or "dra" in filename.upper():
        base_address = 0x800A0000

    suffix = "dra" if base_address == 0x800A0000 else "main"

    with open(filename, "rb") as f:
        data = f.read()

    words = []
    for i in range(0, len(data) & ~3, 4):
        word = int.from_bytes(data[i:i+4], byteorder="little")
        words.append(word)

    os.makedirs(os.path.join(outdir, "source", suffix), exist_ok=True)
    
    CHUNK_SIZE = 10000
    num_chunks = (len(words) + CHUNK_SIZE - 1) // CHUNK_SIZE

    # Escreve cada pedaço em um arquivo .cpp separado de verdade
    for c in range(num_chunks):
        chunk_code = '#include "ps1_hw.h"\n\n'
        chunk_code += f"void execute_sub_block_{suffix}_{c}(uint32_t pc) {{\n"
        chunk_code += "    switch(pc) {\n"
        
        start_idx = c * CHUNK_SIZE
        end_idx = min(start_idx + CHUNK_SIZE, len(words))
        
        for idx in range(start_idx, end_idx):
            current_pc = base_address + (idx * 4)
            chunk_code += f"        case 0x{current_pc:08X}:\n"
            chunk_code += f"            ps1_cpu_regs.pc = 0x{current_pc:08X};\n"
            chunk_code += f"            execute_instruction_fallback(0x{current_pc:08X});\n"
            chunk_code += "            break;\n"
            
        chunk_code += "        default: break;\n"
        chunk_code += "    }\n"
        chunk_code += "}\n"
        
        with open(os.path.join(outdir, "source", suffix, f"chunk_{c}.cpp"), "w") as f:
            f.write(chunk_code)

    # Escreve a função unificadora central daquele bloco
    master_code = '#include "ps1_hw.h"\n\n'
    for c in range(num_chunks):
        master_code += f"void execute_sub_block_{suffix}_{c}(uint32_t pc);\n"
        
    master_code += f'\nextern "C" void execute_recompiled_block_{suffix}(uint32_t pc) {{\n'
    for c in range(num_chunks):
        master_code += f"    execute_sub_block_{suffix}_{c}(pc);\n"
    master_code += "}\n"

    with open(os.path.join(outdir, "source", f"mips_{suffix}_gen.cpp"), "w") as f:
        f.write(master_code)

    print(f"Sucesso: {len(words)} instrucoes divididas fisicamente em {num_chunks} arquivos .cpp.")

def main():
    if len(sys.argv) < 3:
        sys.exit(1)
    compile_binary(sys.argv[1], sys.argv[2])

if __name__ == "__main__":
    main()

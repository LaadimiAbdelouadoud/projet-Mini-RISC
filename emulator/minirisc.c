#include "minirisc.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Helper function to sign-extend a value
static int32_t sign_extend(uint32_t value, int bits)
{
    int32_t result = (int32_t)value;
    if (value & (1 << (bits - 1))) {
        result |= (-1 << bits);
    }
    return result;
}

minirisc_t* minirisc_new(uint32_t initial_PC, platform_t *platform)
{
    minirisc_t *mr = (minirisc_t *)malloc(sizeof(minirisc_t));
    if (!mr) {
        fprintf(stderr, "Error: Failed to allocate processor\n");
        return NULL;
    }

    mr->PC = initial_PC;
    mr->IR = 0;
    mr->next_PC = initial_PC + 4;
    mr->platform = platform;
    mr->halt = 0;

    for (int i = 0; i < 32; i++) {
        mr->regs[i] = 0;
    }

    return mr;
}

void minirisc_free(minirisc_t *mr)
{
    if (mr) {
        free(mr);
    }
}

void minirisc_fetch(minirisc_t *mr)
{
    uint32_t instr = 0;
    if (platform_read(mr->platform, ACCESS_WORD, mr->PC, &instr) != 0) {
        fprintf(stderr, "Error: Failed to fetch instruction at PC 0x%08x\n", mr->PC);
        mr->halt = 1;
        return;
    }
    mr->IR = instr;
}

void minirisc_decode_and_execute(minirisc_t *mr)
{
    uint32_t ir = mr->IR;
    uint8_t opcode = ir & 0x7F; // bits [6:0]
    uint8_t rd = (ir >> 7) & 0x1F;   // bits [11:7]
    uint8_t rs1 = (ir >> 15) & 0x1F;  // bits [19:15]
    uint8_t rs2 = (ir >> 20) & 0x1F;  // bits [24:20]
    uint8_t funct3 = (ir >> 12) & 0x07;  // bits [14:12]
    uint8_t funct7 = (ir >> 25) & 0x7F;  // bits [31:25]

    // Sign-extend immediate values
    int32_t imm_i = sign_extend((ir >> 20), 12);    // I-type immediate
    int32_t imm_s = sign_extend(((ir >> 25) << 5) | ((ir >> 7) & 0x1F), 12);  // S-type immediate
    int32_t imm_b = sign_extend(  
        ((ir >> 31) << 12) | (((ir >> 7) & 0x01) << 11) |
        (((ir >> 25) & 0x3F) << 5) | (((ir >> 8) & 0x0F) << 1), 13);  // B-type immediate
    int32_t imm_u = sign_extend(ir & 0xFFFFF000, 32);   // U-type immediate
    int32_t imm_j = sign_extend(
        ((ir >> 31) << 20) | (((ir >> 12) & 0xFF) << 12) |
        (((ir >> 20) & 0x01) << 11) | (((ir >> 21) & 0x3FF) << 1), 21);  // J-type immediate

    int32_t rs1_val = (int32_t)mr->regs[rs1];   // For signed comparisons
    int32_t rs2_val = (int32_t)mr->regs[rs2];  // For signed comparisons

    mr->next_PC = mr->PC + 4;

    switch (opcode) {
    case 0x37:  // LUI
    {
        mr->regs[rd] = imm_u;
        break;
    }
    case 0x17:  // AUIPC
    {
        mr->regs[rd] = mr->PC + imm_u;
        break;
    }
    case 0x6F:  // JAL
    {
        mr->regs[rd] = mr->PC + 4;
        mr->next_PC = mr->PC + imm_j;
        break;
    }
    case 0x67:  // JALR
    {
        if (funct3 != 0) {
            fprintf(stderr, "Error: Invalid JALR funct3\n");
            mr->halt = 1;
            break;
        }
        mr->regs[rd] = mr->PC + 4;
        mr->next_PC = (mr->regs[rs1] + imm_i) & ~1;
        break;
    }
    case 0x63:  // Branch
    {
        int branch_taken = 0;
        switch (funct3) {
        case 0x0:  // BEQ
            branch_taken = (mr->regs[rs1] == mr->regs[rs2]);
            break;
        case 0x1:  // BNE
            branch_taken = (mr->regs[rs1] != mr->regs[rs2]);
            break;
        case 0x4:  // BLT
            branch_taken = (rs1_val < rs2_val);
            break;
        case 0x5:  // BGE
            branch_taken = (rs1_val >= rs2_val);
            break;
        case 0x6:  // BLTU
            branch_taken = (mr->regs[rs1] < mr->regs[rs2]);
            break;
        case 0x7:  // BGEU
            branch_taken = (mr->regs[rs1] >= mr->regs[rs2]);
            break;
        default:
            fprintf(stderr, "Error: Invalid Branch funct3: %d\n", funct3);
            mr->halt = 1;
        }
        if (branch_taken) {
            mr->next_PC = mr->PC + imm_b;
        }
        break;
    }
    case 0x03:  // Load
    {
        uint32_t addr = mr->regs[rs1] + imm_i;
        uint32_t value = 0;
        int32_t signed_value = 0;

        switch (funct3) {
        case 0x0:  // LB (Load Byte)
            if (platform_read(mr->platform, ACCESS_BYTE, addr, &value) != 0) {
                mr->halt = 1;
                break;
            }
            signed_value = sign_extend(value & 0xFF, 8);
            mr->regs[rd] = (uint32_t)signed_value;
            break;
        case 0x1:  // LH (Load Half)
            if (platform_read(mr->platform, ACCESS_HALF, addr, &value) != 0) {
                mr->halt = 1;
                break;
            }
            signed_value = sign_extend(value & 0xFFFF, 16);
            mr->regs[rd] = (uint32_t)signed_value;
            break;
        case 0x2:  // LW (Load Word)
            if (platform_read(mr->platform, ACCESS_WORD, addr, &value) != 0) {
                mr->halt = 1;
                break;
            }
            mr->regs[rd] = value;
            break;
        case 0x4:  // LBU (Load Byte Unsigned)
            if (platform_read(mr->platform, ACCESS_BYTE, addr, &value) != 0) {
                mr->halt = 1;
                break;
            }
            mr->regs[rd] = value & 0xFF;
            break;
        case 0x5:  // LHU (Load Half Unsigned)
            if (platform_read(mr->platform, ACCESS_HALF, addr, &value) != 0) {
                mr->halt = 1;
                break;
            }
            mr->regs[rd] = value & 0xFFFF;
            break;
        default:
            fprintf(stderr, "Error: Invalid Load funct3: %d\n", funct3);
            mr->halt = 1;
        }
        break;
    }
    case 0x23:  // Store
    {
        uint32_t addr = mr->regs[rs1] + imm_s;
        switch (funct3) {
        case 0x0:  // SB
            if (platform_write(mr->platform, ACCESS_BYTE, addr, mr->regs[rs2]) != 0) {
                mr->halt = 1;
            }
            break;
        case 0x1:  // SH
            if (platform_write(mr->platform, ACCESS_HALF, addr, mr->regs[rs2]) != 0) {
                mr->halt = 1;
            }
            break;
        case 0x2:  // SW
            if (platform_write(mr->platform, ACCESS_WORD, addr, mr->regs[rs2]) != 0) {
                mr->halt = 1;
            }
            break;
        default:
            fprintf(stderr, "Error: Invalid Store funct3: %d\n", funct3);
            mr->halt = 1;
        }
        break;
    }
    case 0x13:  // Arithmetic Immediate
    {
        switch (funct3) {
        case 0x0:  // ADDI
            mr->regs[rd] = mr->regs[rs1] + imm_i;
            break;
        case 0x2:  // SLTI
            mr->regs[rd] = (rs1_val < imm_i) ? 1 : 0;
            break;
        case 0x3:  // SLTIU
            mr->regs[rd] = (mr->regs[rs1] < (uint32_t)imm_i) ? 1 : 0;
            break;
        case 0x4:  // XORI
            mr->regs[rd] = mr->regs[rs1] ^ imm_i;
            break;
        case 0x6:  // ORI
            mr->regs[rd] = mr->regs[rs1] | imm_i;
            break;
        case 0x7:  // ANDI
            mr->regs[rd] = mr->regs[rs1] & imm_i;
            break;
        case 0x1:  // SLLI
        {
            uint32_t shamt = imm_i & 0x1F;
            mr->regs[rd] = mr->regs[rs1] << shamt;
            break;
        }
        case 0x5:  // SRLI or SRAI
        {
            uint32_t shamt = imm_i & 0x1F;
            if ((ir >> 30) & 0x01) {
                // SRAI
                mr->regs[rd] = (uint32_t)(rs1_val >> shamt);
            } else {
                // SRLI
                mr->regs[rd] = mr->regs[rs1] >> shamt;
            }
            break;
        }
        default:
            fprintf(stderr, "Error: Invalid Arithmetic Immediate funct3: %d\n", funct3);
            mr->halt = 1;
        }
        break;
    }
    case 0x33:  // Arithmetic Register + M extension
    {
        switch (funct3) {
        case 0x0:  // ADD or SUB or MUL
            if (funct7 == 0x00) {
                // ADD
                mr->regs[rd] = mr->regs[rs1] + mr->regs[rs2];
            } else if (funct7 == 0x20) {
                // SUB
                mr->regs[rd] = mr->regs[rs1] - mr->regs[rs2];
            } else if (funct7 == 0x01) {
                // MUL (M extension)
                mr->regs[rd] = mr->regs[rs1] * mr->regs[rs2];
            } else {
                fprintf(stderr, "Error: Invalid funct7 for ADD/SUB/MUL: %d\n", funct7);
                mr->halt = 1;
            }
            break;
        case 0x1:  // SLL or MULH
            if (funct7 == 0x00) {
                // SLL
                mr->regs[rd] = mr->regs[rs1] << (mr->regs[rs2] & 0x1F);
            } else if (funct7 == 0x01) {
                // MULH (M extension)
                int64_t result = (int64_t)rs1_val * (int64_t)rs2_val;
                mr->regs[rd] = (uint32_t)(result >> 32);
            } else {
                fprintf(stderr, "Error: Invalid funct7 for SLL/MULH: %d\n", funct7);
                mr->halt = 1;
            }
            break;
        case 0x2:  // SLT or MULHSU
            if (funct7 == 0x00) {
                // SLT
                mr->regs[rd] = (rs1_val < rs2_val) ? 1 : 0;
            } else if (funct7 == 0x01) {
                // MULHSU (M extension)
                int64_t result = (int64_t)rs1_val * (uint64_t)mr->regs[rs2];
                mr->regs[rd] = (uint32_t)(result >> 32);
            } else {
                fprintf(stderr, "Error: Invalid funct7 for SLT/MULHSU: %d\n", funct7);
                mr->halt = 1;
            }
            break;
        case 0x3:  // SLTU or MULHU
            if (funct7 == 0x00) {
                // SLTU
                mr->regs[rd] = (mr->regs[rs1] < mr->regs[rs2]) ? 1 : 0;
            } else if (funct7 == 0x01) {
                // MULHU (M extension)
                uint64_t result = (uint64_t)mr->regs[rs1] * (uint64_t)mr->regs[rs2];
                mr->regs[rd] = (uint32_t)(result >> 32);
            } else {
                fprintf(stderr, "Error: Invalid funct7 for SLTU/MULHU: %d\n", funct7);
                mr->halt = 1;
            }
            break;
        case 0x4:  // XOR or DIV
            if (funct7 == 0x00) {
                // XOR
                mr->regs[rd] = mr->regs[rs1] ^ mr->regs[rs2];
            } else if (funct7 == 0x01) {
                // DIV (M extension)
                if (rs2_val == 0) {
                    mr->regs[rd] = 0xFFFFFFFF;  // Division by zero
                } else if (rs1_val == INT32_MIN && rs2_val == -1) {
                    mr->regs[rd] = (uint32_t)INT32_MIN;  // Overflow case
                } else {
                    mr->regs[rd] = (uint32_t)(rs1_val / rs2_val);
                }
            } else {
                fprintf(stderr, "Error: Invalid funct7 for XOR/DIV: %d\n", funct7);
                mr->halt = 1;
            }
            break;
        case 0x5:  // SRL or SRA or DIVU
            if (funct7 == 0x00) {
                // SRL
                mr->regs[rd] = mr->regs[rs1] >> (mr->regs[rs2] & 0x1F);
            } else if (funct7 == 0x20) {
                // SRA
                mr->regs[rd] = (uint32_t)(rs1_val >> (mr->regs[rs2] & 0x1F));
            } else if (funct7 == 0x01) {
                // DIVU (M extension)
                if (mr->regs[rs2] == 0) {
                    mr->regs[rd] = 0xFFFFFFFF;  // Division by zero
                } else {
                    mr->regs[rd] = mr->regs[rs1] / mr->regs[rs2];
                }
            } else {
                fprintf(stderr, "Error: Invalid funct7 for SRL/SRA/DIVU: %d\n", funct7);
                mr->halt = 1;
            }
            break;
        case 0x6:  // OR or REM
            if (funct7 == 0x00) {
                // OR
                mr->regs[rd] = mr->regs[rs1] | mr->regs[rs2];
            } else if (funct7 == 0x01) {
                // REM (M extension)
                if (rs2_val == 0) {
                    mr->regs[rd] = mr->regs[rs1];
                } else if (rs1_val == INT32_MIN && rs2_val == -1) {
                    mr->regs[rd] = 0;  // Overflow case
                } else {
                    mr->regs[rd] = (uint32_t)(rs1_val % rs2_val);
                }
            } else {
                fprintf(stderr, "Error: Invalid funct7 for OR/REM: %d\n", funct7);
                mr->halt = 1;
            }
            break;
        case 0x7:  // AND or REMU
            if (funct7 == 0x00) {
                // AND
                mr->regs[rd] = mr->regs[rs1] & mr->regs[rs2];
            } else if (funct7 == 0x01) {
                // REMU (M extension)
                if (mr->regs[rs2] == 0) {
                    mr->regs[rd] = mr->regs[rs1];
                } else {
                    mr->regs[rd] = mr->regs[rs1] % mr->regs[rs2];
                }
            } else {
                fprintf(stderr, "Error: Invalid funct7 for AND/REMU: %d\n", funct7);
                mr->halt = 1;
            }
            break;
        default:
            fprintf(stderr, "Error: Invalid Arithmetic Register funct3: %d\n", funct3);
            mr->halt = 1;
        }
        break;
    }
    default:
        fprintf(stderr, "Error: Unknown opcode 0x%02x (IR: 0x%08x)\n", opcode, ir);
        mr->halt = 1;
    }

    // r0 is always 0
    mr->regs[0] = 0;

    mr->PC = mr->next_PC;
}

void minirisc_run(minirisc_t *mr)
{
    while (!mr->halt) {
        minirisc_fetch(mr);
        minirisc_decode_and_execute(mr);
    }
}

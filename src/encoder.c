#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include"encoder.h"
#include"symbol_table.h"

const EncodeInstr encode_table[] = {
    {"ADD", R_TYPE, 0x33, 0x0, 0x00},
    {"SUB", R_TYPE, 0x33, 0x0, 0x20},
    {"ADDI", I_TYPE, 0x13, 0x0, 0x00},

    {"LW", I_TYPE, 0x03, 0x2, 0x00},
    {"SW", S_TYPE, 0x23, 0x2, 0x00},

    {"BEQ", B_TYPE, 0x63, 0x0, 0x00},
    {"BNE", B_TYPE, 0x63, 0x1, 0x00},

    {"JAL", J_TYPE, 0x6F, 0x0, 0x00},

    {"HLT", R_TYPE, 0x00, 0x0, 0x00}
};

#define ENC_COUNT (sizeof(encode_table) / sizeof(encode_table[0]))
#define HLT_INSTR 0xFFFFFFFF

int reg_number(char *reg){

    if(reg[0] != 'R' || !isdigit(reg[1])){
        printf("Invalid register: %s\n", reg);
        exit(1);
    }
    int r = atoi(reg + 1);

    if(r<0 || r>31){
        printf("\nInvalid register %s\n", reg);
        exit(1);
    }

    return r;
}

unsigned int encode_S(int rs1, int rs2, int imm, int funct3, int opcode){

    unsigned int inst = 0;
    unsigned int uimm = imm;        // because 'C' does not gurrantees right shifting for signed integers

    int imm11_5 = (uimm >> 5) & 0x7F;
    int imm4_0 = uimm & 0x1F;

    inst |= (imm11_5 << 25);
    inst |= (rs2 << 20);
    inst |= (rs1 << 15);
    inst |= (funct3 << 12);
    inst |= (imm4_0 << 7);
    inst |= opcode;

    return inst;
}

unsigned int encode_J(int rd, int imm, int opcode){

    unsigned int inst = 0;
    unsigned int uimm = imm;        // because 'C' does not gurrantees right shifting for signed integers

    int imm20 = (uimm >> 20) & 0x1;
    int imm10_1 = (uimm >> 1) & 0x3FF;
    int imm11 = (uimm >> 11) & 0x1;
    int imm19_12 = (uimm >> 12) & 0xFF;

    inst |= imm20 << 31;
    inst |= imm10_1 << 21;
    inst |= imm11 << 20;
    inst |= imm19_12 << 12;
    inst |= rd << 7;
    inst |= opcode;

    return inst;
}

unsigned int encode_B(int rs1, int rs2, int imm, int funct3, int opcode){

    unsigned int inst = 0;
    unsigned int uimm = imm;        // because 'C' does not gurrantees right shifting for signed integers

    // We have recieved an immediate value (13 bit) from the function call
    // Now we will extract those divided immediate bit values from those 12 bits.

    // Conceptually, there are 12 bits, but there is one more hidden bit. 
    // Offsets are always multiples of 2, so, in RISC V architecture, we follow (offset >> 1) convention.
    // So, the last bit is always 0.
    // So, actaully there are 13 bits, out of which only 12 appear in the instruction field.

    int imm12 = (uimm >> 12) & 0x1;     // extracting bit - 12 only
    int imm10_5 = (uimm >> 5) & 0x3F;   // extracting bit - 5 to bit - 10
    int imm4_1 = (uimm >> 1) & 0xF;     // extracting bit - 1 to bit - 4
    int imm11 = (uimm >> 11) & 0x1;     // extracting bit - 11 only

    inst |= (imm12 << 31);
    inst |= (imm10_5 << 25);
    inst |= (rs2 << 20);
    inst |= (rs1 << 15);
    inst |= (funct3 << 12);
    inst |= (imm4_1 << 8);
    inst |= (imm11 << 7);
    inst |= opcode;

    return inst;
}

unsigned int encode_R(int rd, int rs1, int rs2, int funct3, int funct7, int opcode){

    unsigned int inst = 0;

    inst |= (funct7 << 25);
    inst |= (rs2 << 20);
    inst |= (rs1 << 15);
    inst |= (funct3 << 12);
    inst |= (rd << 7);
    inst |= opcode;

    return inst;
}

unsigned int encode_I(int rd, int rs1, int imm, int funct3, int opcode){

    unsigned int inst = 0;
    unsigned int uimm = imm;        // because 'C' does not gurrantees right shifting for signed integers

    inst |= ((uimm & 0xFFF) << 20);
    inst |= (rs1 << 15);
    inst |= (funct3 << 12);
    inst |= (rd << 7);
    inst |= opcode;

    return inst;
}

unsigned int encode(Token tokens[], int count, int pc){

    for(int i = 0; i<ENC_COUNT; i++){           
        // this linear search can be optimized using hashmaps, for O(1) lookup
         
        if( strcmp(tokens[0].value, encode_table[i].name) == 0 ){

            if(encode_table[i].format == R_TYPE){           // for R-Type instructions

                // Since we considered 'HLT' to be R-Type instruction.
                if( strcmp(encode_table[i].name, "HLT") == 0 ){
                    return HLT_INSTR;       // special instruction value of halt
                }

                int rd = reg_number(tokens[1].value);
                int rs1 = reg_number(tokens[3].value);
                int rs2 = reg_number(tokens[5].value);

                return encode_R(
                    rd, rs1, rs2, 
                    encode_table[i].funct3,     // Function-03 value for this instruction is predefined in the system
                    encode_table[i].funct7,     // Function-07 value for this instruction is predefined in the system
                    encode_table[i].opcode      // Opcode is for this instruction also predefined and stored in the encode table
                );
            }
            else if (encode_table[i].format == I_TYPE){     // for I-Type instructions

                int rd = reg_number(tokens[1].value);

                if( strcmp(encode_table[i].name, "LW") == 0 ){

                    int imm = atoi(tokens[3].value);
                    int rs1 = reg_number(tokens[5].value);
                    
                    if(imm<-2048 || imm>2047){
                        printf("Error: Immediate out of range for LW\n");
                        exit(1);
                    }

                    return encode_I(
                        rd, rs1, imm,
                        encode_table[i].funct3,
                        encode_table[i].opcode
                    );
                }
                else{
                    
                    int rs1 = reg_number(tokens[3].value);
                    int imm = atoi(tokens[5].value);

                    if(imm<-2048 || imm>2047){
                        printf("Error: Immediate out of range for ADDI\n");
                        exit(1);
                    }

                    return encode_I(
                        rd, rs1, imm,
                        encode_table[i].funct3,
                        encode_table[i].opcode
                    );
                }
            }
            else if(encode_table[i].format == B_TYPE){      // for B-Type instructions

                int rs1 = reg_number(tokens[1].value);
                int rs2 = reg_number(tokens[3].value);
                
                char *label = tokens[5].value;
                int label_address = find_symbol(label);
                if( label_address == -1){
                    printf("Error: Undefined label: %s\n", label);
                    exit(1);
                }

                int offset = label_address - (pc + 4);  
                //RISC-V calculates branch relative to next instruction, not current.
                
                if( offset%2 != 0 ){
                    printf("Error: Branch target misaligned!\n");
                    return 0;
                }
                if(offset > 4094 || offset < -4096){
                    printf("Error: Branch target out of range\n");
                    exit(1);
                }

                return encode_B(
                    rs1, rs2, offset,
                    encode_table[i].funct3,
                    encode_table[i].opcode
                );

            }
            else if(encode_table[i].format == J_TYPE){      // for J-Type instructions

                int rd = reg_number(tokens[1].value);

                char* label = tokens[3].value;
                int label_address = find_symbol(label);

                if(label_address == -1){
                    printf("Error! Undefined label: %s", label);
                    return 0;
                }

                int offset = label_address - (pc + 4);
                //RISC-V calculates branch relative to next instruction, not current.

                if(offset % 2 != 0){
                    printf("Branch target misaligned!");
                    return 0;
                }
                if(offset < -1048576 || offset > 1048574){
                    printf("Error: Jump target out of range\n");
                    exit(1);
                }

                return encode_J(
                    rd, offset,
                    encode_table[i].opcode
                );
            }
            else if(encode_table[i].format == S_TYPE){      // for S-Type instructions

                int rs2 = reg_number(tokens[1].value);      // value register
                int imm = atoi(tokens[3].value);
                int rs1 = reg_number(tokens[5].value);      // base register
                
                if(imm<-2048 || imm>2047){
                    printf("Error: Immediate out of range for SW\n");
                    exit(1);
                }

                return encode_S(
                    rs1, rs2, imm,
                    encode_table[i].funct3,
                    encode_table[i].opcode
                );
            }
        }
    }
    
    printf("\nError: Unknown instruction %s\n", tokens[0].value);
    exit(1);
}

#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>
#include"lexer.h"
#include"parser.h"
#include"encoder.h"
#include"symbol_table.h"

#define MAX_LINES 200

int is_blank(char* str){
    
    while(*str){
        if(!isspace(*str)){
            return 0;
        }
        str++;
    }    
    return 1;
}


// For intermediate lines:
typedef struct {
    char instruction[100];
    int address;
} Intermediate_Lines;

Intermediate_Lines intermediate[MAX_LINES];
int line_count = 0;                             // For counting intermediate lines


int main(){
    FILE *fp;
    char line[MAX];
    int line_num = 1;
    int pc = 0;

    Token tokens[100];
    int tok_count;

    fp = fopen("test/assembly.txt", "r");

    // Error Management:
    {
        if(fp == NULL){
            printf("Error in opening the file.\n");
            return 1;
        }
    }

    // Printing original file:
    {   printf("\nOriginal file: \n");
        while (fgets(line, sizeof(line), fp) != NULL){
            
            printf("%d. %s", line_num, line);
            line_num++;
        }
        printf("\n");
        line_num = 1;
        rewind(fp);
    }


    // PASS - 01:
    {// Removing comments, blank lines and labels, and then tokenizing the string:

        printf("\n\n----------------- PASS - 01 -----------------");
        while (fgets(line, sizeof(line), fp) != NULL){
            
        // Removing comments:
            
            line[strcspn(line, ";")] = '\0';
            line[strcspn(line, "\n")] = '\0';

            trim(line);

            if(is_blank(line)){
                continue;
            }

        // Detecting Labels:
            
            add_symbol(line, pc);
            if(is_blank(line)){
                line_num++;
                continue;
            }
            
            printf("\n%d. %-14s", line_num, line);

        // Tokenization:
            tok_count = lexer(line, tokens);
            printf("   (Number of tokens: %d)", tok_count);
            
        // Calling Parser and building an intermediate program listing.
        // (Parser basically validates the syntax of the program)
            if(parser(tokens, tok_count, line_num)){
                
                strcpy(intermediate[line_count].instruction, line);
                intermediate[line_count++].address = pc;
                pc += 4;
            }
            
            line_num++;

        }
        printf("\n");
        line_num = 1;
        rewind(fp); 
        pc = 0;
    }

    // INTERMEDIATE CODE:
    { // Printing the Intermediate code with Addresses:

        printf("\n");
        printf("\n\n------- Intermediate Code -----------");
        printf("\n Address \t Instruction");
        printf("\n-------------------------------------");

        for(int i = 0; i<line_count; i++){
            printf("\n 0x%04x \t %s", intermediate[i].address, intermediate[i].instruction);
        }
    }


    FILE *bin_out, *hex_out;
    bin_out = fopen("output/program.bin", "wb");
    hex_out = fopen("output/program.hex", "w");
    if(bin_out == NULL){
        printf("\nError in opening the file.\n");
        exit(1);
    }
    if(hex_out == NULL){
        printf("\nError in opening the file.\n");
        exit(1);
    }

    // PASS - 02:
    {// MACHINE CODE GENERATION:

        printf("\n\n----------------- PASS - 02 -----------------");
        printf("\n---- MACHINE CODE  AND ADDRESS GENERATION ----\n");
        for(int i = 0; i<line_count; i++){
            
            tok_count = lexer(intermediate[i].instruction, tokens); 
            
            printf("\n 0x%04x \t %-15s", 
                intermediate[i].address, 
                intermediate[i].instruction
            );

            if(parser(tokens, tok_count, i+1)){
                // here (i+1) is sent, because, line_num = i+1

                // MACHINE CODE GENERATION
                unsigned int machine_code = encode(tokens, tok_count, intermediate[i].address);

                // WRITTING MACINE CODE IN A FILE:
                printf(" | Machine Code: 0x%08X", machine_code);
                fwrite(&machine_code, sizeof(machine_code), 1, bin_out);        // To be later used by CPU simulator
                
                // CLEANER/READABLE HEX OUTPUT:
                fprintf(hex_out, "%04X : %08X\n", intermediate[i].address,machine_code);        // Human readable hex file
                
            }
            
        }
    }

    // Symbol Table Printing:
    print_symbol_table();

    // Closing the file:
    fclose(fp);
    fclose(bin_out);
    fclose(hex_out);
    return 0;
}

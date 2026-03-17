// '.c' file for symbol table

#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>
#include"symbol_table.h"

// Removes the leading and trailing whitespaces from the string.
void trim (char*str){
    char *start = str;

    // Remove leading spaces:
    while (isspace(*start)){
        start++;
    }

    // Shift string left:
    if(start != str){
        memmove(str, start, strlen(start)+1 );
    }

    // Remove trailing spaces:
    int len = strlen(str);
    while( len>0 && isspace(str[len-1]) ){
        str[len-1] = '\0';
        len--;
    }
}



// Array named 'symbol_table' of type 'Symbol'
Symbol symbol_table[MAX_SYMBOLS];
int symbol_count = 0;



// Add Symbol (In symbol table)
void add_symbol(char *line, int pc){

    // Check for overflow:
    if(symbol_count >= MAX_SYMBOLS){
        printf("Error: Symbol table overflow.\nReduce the number of symbols used.\n");
        exit(1);
    }

    char *colon = strchr(line, ':');

    if(colon == NULL){
        return;
    }
    *colon = '\0';           // split label and instruction

    trim(line);

    // to convert lower case symbols to upper case, in order to prevent case sensitivity
    for(int i = 0; line[i]; i++){
        line[i] = toupper(line[i]);
    }

    if(find_symbol(line) != -1){
        printf("Error: Duplicate label: %s\n", line);
        exit(1);
    }

    strcpy(symbol_table[symbol_count].label, line);
    symbol_table[symbol_count].address = pc;
    symbol_count++;

    // move instruction part to beginning
    memmove(line, colon + 1, strlen(colon + 1) + 1);

    trim(line);              // clean remaining instruction
}



// FINDING LABELS AND CALCULATING OFFSETS:
int find_symbol(char* label){

    for(int i = 0; i<symbol_count; i++){
        if( strcmp(label, symbol_table[i].label) == 0 ){
            return symbol_table[i].address;
        }
    }
    return -1;
}



// Symbol Table printing:
void print_symbol_table(){

    printf("\n\n------ Symbol Table ------\n");
    printf(" Label \t Address\n");
    printf("---------------------------");
    for(int i = 0; i<symbol_count; i++){
        printf("\n %s \t 0x%04X", symbol_table[i].label, symbol_table[i].address);
    }
    printf("\n");

    /*In above for-loop, the '%04x' is used to print the integer in hex-format.
        %x is format specifier for hex in lower-case
        %X is format specifier for HEX in Upper-case
        '04' is used to reserve 4 characters for the printed output.

        so for int = 8,
        '%04X' prints '0008'
    */
    
}

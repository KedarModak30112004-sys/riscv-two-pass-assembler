#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>
#include"lexer.h"

void add_token(Token tokens[], int* count, TokenType type, const char* value){

    if(*count >= MAX_TOKENS){
        printf("Error: Too many tokens.\n");
        exit(1);
    }

    tokens[*count].type = type;
    strcpy(tokens[*count].value, value);
    (*count)++;
}

// Lexer tokenizes the input string:
int lexer(char* line, Token tokens[]){

    int count = 0;
    char* ptr = line;

    while(*ptr){

        // Handling whitespaces (per line):
        while(isspace(*ptr)){                   // Skipping the blank spaces
            ptr++;
        }

        // End of string:
        if(*ptr == '\0')        break;

        // Handling comma:
        if(*ptr == ','){

            add_token(tokens, &count, COMMA, ",");

            ptr++;
            continue;
        }

        // Handling Left Parantheses:
        if(*ptr == '('){

            char temp[2] = {*ptr, '\0'};
            add_token(tokens, &count, LPAREN, temp);
            ptr++;
            continue;
        }

        // Handling Right Parantheses:
        if(*ptr == ')'){
            
            char temp[2] = {*ptr, '\0'};
            add_token(tokens, &count, RPAREN, temp);
            ptr++;
            continue;
        }

     // Extracting words:
        char buffer[50];
        int i = 0;

        // Word Extracting Loop:
        while( (*ptr)&&(!isspace(*ptr))&&(*ptr != ',')&&(*ptr != '(')&&(*ptr != ')') ){
            buffer[i++] = *ptr;
            ptr++;
        }
        buffer[i] = '\0';

        if(i == 0){
            continue;
        }

        // Preventing Case Sensitivity:
        for(int j = 0; buffer[j]; j++){
            buffer[j] = toupper(buffer[j]);     // toupper function converts lower case to upper case. 
        }


        // Classifying words as tokens:

        TokenType type; 

        if( (buffer[0] == 'R') && isdigit(buffer[1]) ){
            type = REGISTER;
        }
        else if( isdigit(buffer[0]) || ((buffer[0] == '-')&&isdigit(buffer[1]) ) ){
            type = IMMEDIATE;
        }
        else if( isalpha(buffer[0]) ){
            type = IDENTIFIER;
        }
        else{
            type = UNKNOWN;
        }

        add_token(tokens, &count, type, buffer);
    }

    return count;
}

#include <stdio.h>
#include <string.h>

void printIntroduction();

int main(int argc, char ** argv){

    // setear socket UDP 

    //interface para el usuario

    printIntroduction();

    char input[3];

    while (1){
        scanf("%2s", input); // Read up to two characters and store in input

        printf("%s\n", input);

        if (strcmp(input, "BT") == 0) {
            printf("Bytes transferred: ...\n");
        }
        else if (strcmp(input, "BR") == 0) {
            printf("Bytes received: ...\n");
        }
        else if (strcmp(input, "CC") == 0) {
            printf("Current connections: ...\n");
        }
        else if (strcmp(input, "HC") == 0) {
            printf("History connections: ...\n");
        }
        else if (strcmp(input, "q") == 0) {
            printf("Quitting...\n");
            return 0;
        }
        else {
            printf("Invalid option. Please try again.\n");
        }
        printf("> ");
    }
}

void printIntroduction(){
    printf("Welcome! The commands available are:\n");
    printf("1. BT: To see the bytes that were transfered\n");
    printf("2. BR: To see the amount of bytes received \n");
    printf("3. CC: To see the current connections\n");
    printf("4. HC: To see the history connections\n");
    printf("> ");
}
#include <stdio.h>
#include <string.h>
#include "../src/logger/logger.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#define INPSIZE 128


void printIntroduction();
int udpClientSocket(const char *host, const char *service, struct addrinfo **servAddr);

int main(int argc, char ** argv){

    //Receive the port to program
    if(argc != 4){
        // log(ERROR,"Error: Invalid number of arguments\n");
        fprintf(stderr,"Usage: <IPv4|IPv6> <Server IP> <Server Port/Service>\n");
        exit(1);
    }

    if ( strcmp(argv[1],"IPv6") != 0 && strcmp(argv[1],"IPv4") != 0){
        fprintf(stderr,"INVALID IP VERSION, use IPv6 or IPv4\n");
        exit(1);
    }

    // setear socket UDP -------------------------------------

    // A diferencia de TCP, guardamos a que IP/puerto se envia la data, para verificar
    // que la respuesta sea del mismo host
    struct addrinfo *servAddr; 
    
    char *server = argv[1];  

    // char *servPort = (argc == 3) ? argv[2] : "echo";
    char *servPort = "echo"; //hace falta?


    errno = 0;
    int sock = udpClientSocket(server, servPort, &servAddr);
    if (sock < 0)
        log(FATAL, "socket() failed: %s", strerror(errno));


    // client interface -------------------------------------

    printIntroduction();

    char input[INPSIZE];

    while (1){
        scanf("%[^\n]", input); // Read up to a new line and store in input

        // printf("%s\n", input);
        // printf("%d\n", strlen(input));

        if (strncmp(input, "BT", 2) == 0) {
            printf("Bytes transferred: ...\n");
        }
        else if (strncmp(input, "BR", 2) == 0) {
            printf("Bytes received: ...\n");
        }
        else if (strncmp(input, "CC", 2) == 0) {
            printf("Current connections: ...\n");
        }
        else if (strncmp(input, "HC", 2) == 0) {
            printf("History connections: ...\n");
        }
        else if (strncmp(input, "AU", 2) == 0) {
            if(strlen(input) < 4){
                printf("Usage: AU <USER>.\n");
            } else{
                printf("Adding user...\n");
            }
        }
        else if (strncmp(input, "CA", 2) == 0) {
            if(strlen(input) < 4){
                printf("Usage: CA <KEY>.\n");
            } else{
                printf("Creating a new key...\n");
            }
        }
        else if (strncmp(input, "DM", 2) == 0) {
            if(strlen(input) < 4){
                printf("Usage: DM <METHOD>.\n");
            } else{
                printf("Disabling a specific method...\n");
            }
        }
        else if (strncmp(input, "MP", 2) == 0) {
            if(strlen(input) < 6){
                printf("Usage: MP <USER> <NEW PASSWORD>.\n");
            } else{
                printf("Modifying password...\n");
            }
        }
        else if (strncmp(input, "DU", 2) == 0) {
            if(strlen(input) < 4){
                printf("Usage: DU <USER>.\n");
            } else{
                printf("Deleting user...\n");
            }
        }
        else if (strncmp(input, "LU", 2) == 0) {
            printf("Listing users...\n");
        }
        else if (strncmp(input, "HELP", 4) == 0) {
            printIntroduction();
        }
        else if (strncmp(input, "q") == 0) {
            printf("Quitting...\n");
            return 0;
        }
        else {
            printf("Invalid option. Please try again.\n");
        }
        getchar(); // Consume the newline character from the buffer
        printf("> ");
    }
}

/* En esta version no iteramos por las posibles IPs del servidor Echo, como se hizo para TCP
** Realizar las modificaciones necesarias para que intente por todas las IPs
*/
int udpClientSocket(const char *host, const char *service, struct addrinfo **servAddr) {
  // Pedimos solamente para UDP, pero puede ser IPv4 o IPv6
  struct addrinfo addrCriteria;                   
  memset(&addrCriteria, 0, sizeof(addrCriteria)); 
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
  addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol

  // Tomamos la primera de la lista
  int rtnVal = getaddrinfo(host, service, &addrCriteria, servAddr);
  if (rtnVal != 0) {
    log(FATAL, "getaddrinfo() failed: %s", gai_strerror(rtnVal));
	return -1;
  }

  // Socket cliente UDP
  return socket((*servAddr)->ai_family, (*servAddr)->ai_socktype, (*servAddr)->ai_protocol); // Socket descriptor for client
  
}

void printIntroduction(){
    printf("Welcome! The commands available are:\n");
    printf("1.  BT                       : To see the bytes that were transfered\n");
    printf("2.  BR                       : To see the amount of bytes received \n");
    printf("3.  CC                       : To see the current connections\n");
    printf("4.  HC                       : To see the history connections\n");
    printf("5.  AU <USER>                : To add a user\n");
    printf("6.  CA <KEY>                 : To create a new key\n");
    printf("7.  DM <METHOD>              : To disable a specific method\n");
    printf("8.  MP <USER> <NEW PASSWORD> : To modify the password\n");
    printf("9.  DU <USER>                : To delete a user\n");
    printf("10. LU                       : To list users\n");
    printf("11. HELP                     : To print options again\n");
    printf("> ");
}
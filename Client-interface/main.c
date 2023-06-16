#include <stdio.h>
#include <string.h>
#include "../src/logger/logger.h"
#include <sys/socket.h>

void printIntroduction();
int udpClientSocket(const char *host, const char *service, struct addrinfo **servAddr);

int main(int argc, char ** argv){

    //Receive the port to program
    if(argc < 1 || argc > 2){
        // log(ERROR,"Error: Invalid number of arguments\n");
        log(ERROR,"Usage: %s <Server Port/Service>", argv[0]);
        // return 1;
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
    printf("1. BT: To see the bytes that were transfered\n");
    printf("2. BR: To see the amount of bytes received \n");
    printf("3. CC: To see the current connections\n");
    printf("4. HC: To see the history connections\n");
    printf("> ");
}
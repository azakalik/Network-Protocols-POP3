#include "mp3pFunctions.h"
#include <string.h>
#include <stdbool.h>
#include "../stats/stats.h"
#include <stdio.h>
#include <ctype.h>
#define MAXDATAGRAMLENGTH 8096
#define VERSION "V1.0"
#define AUTHKEY "AGUANTEPROTOS"


typedef enum {
    START,
    READM,
    READP,
    READ3,
    READSECONDP,
    READVERSIONV,
    READVERSIONNUM,
    READVERSIONDOT,
    READVERSIONSEPARATOR,
    READVERSIONSUBNUM,
    READFIRSTNEWLINE,
    READSECONDNEWLINE,
    READTHIRDNEWLINE,
    READCLIENTDGRAMID,
    READCLIENTPASSWORD,
    READSEPARATOR,
    READNUMBER,
    READDOT,
    READVERSION,
    READNEWLINE,
    READPASS,
    READCOMMAND,
    READB,
    READBT,
    READH,
    READC,
    READHC,
    READCC,
    READBR,
    DGRAM_BT_COMMAND,
    DGRAM_BR_COMMAND,
    DGRAM_HC_COMMAND,
    DGRAM_CC_COMMAND,
    DGRAMERR,
    INVALID_VERSION,
    INVALID_AUTHKEY,
} mp3p_states;



#define UNAUTHORIZED 100
#define WRONG_PROTOCOL_VERSION 101
#define INVALID_METHOD 102


/*
ORDEN:
header
identifDgram
clave
contenido
*/


#define SERVERSTATSMESSAGE "MP3P V1.0 200\n%s\n%ld"
#define SERVERERRORMESSAGE "MP3P V1.0 %d\n%s\n"


#define IDLENGTH 32


static inline int errorDatagramMessage(char * dgramOutput,mp3p_headers_data * data, int error){
    return sprintf(dgramOutput,SERVERERRORMESSAGE,error,data->uniqueID) + 1;//we count null terminated
}


//error type 100, unauthorized
static int unauthorizedStrategy(mp3p_headers_data *data, char * dgramOutput){
    return errorDatagramMessage(dgramOutput,data,UNAUTHORIZED);
}
//error type 101, wrong protocol version
static int versionMismatchStrategy(mp3p_headers_data * data, char * dgramOutput){
    return errorDatagramMessage(dgramOutput,data,WRONG_PROTOCOL_VERSION);
}

static int outputStatisticsMessage(char * dgramOutput, mp3p_headers_data * data ,uint64_t numberData){
    return sprintf(dgramOutput,SERVERSTATSMESSAGE,data->uniqueID,numberData) + 1;//consider null terminated
}

static int bytesTransferedStrategy(mp3p_headers_data * args, char * dgramOutput){
    uint64_t transferedBytes = getBytesRecievedFromStats();
    return outputStatisticsMessage(dgramOutput,args,transferedBytes);
}

static int bytesRecievedStrategy(mp3p_headers_data * args, char * dgramOutput){
    uint64_t transferedBytes = getBytesRecievedFromStats();
    return outputStatisticsMessage(dgramOutput,args,transferedBytes);
}


static int historicConnectionsStrategy(mp3p_headers_data * args,char * dgramOutput){
    uint64_t historicConnections = getHistoricConnectionsFromStats();
    return outputStatisticsMessage(dgramOutput,args,historicConnections);
}


static int concurrentConnectionsStrategy(mp3p_headers_data * args, char * dgramOutput){
    uint64_t concurrentConnections = getConcurrentConnectionsFromStats();
    return outputStatisticsMessage(dgramOutput,args,concurrentConnections);
}





static mp3p_states parseMp3pCharacter(char c, mp3p_states prevState, int * length){
    if ( prevState == START){
        if (c == 'M')
            return READM;
        return DGRAMERR;
    }

    if (prevState == READM){
        if ( c == 'P')
            return READP;
        return DGRAMERR;
    }

    if (prevState == READP){
        if ( c == '3')
            return READ3;
        return DGRAMERR;
    }

    if (prevState == READ3){
        if ( c == 'P'){
            return READSECONDP;
        }
        return DGRAMERR;
    }

    if (prevState == READSECONDP){
        if (c == ' '){
            return READVERSIONSEPARATOR;
        }
        return DGRAMERR;
    }

    if (prevState == READVERSIONSEPARATOR){
        if ( c == 'V')
            return READVERSIONV;
        return DGRAMERR;
    }

    if (prevState == READVERSIONV){
        if ( c >= '0' && c <= '9'){
            return READVERSIONNUM;
        }
        return DGRAMERR;
    }

    if (prevState == READVERSIONNUM){
        if ( c >= '0' && c <= '9'){
            return READVERSIONNUM;
        } else if ( c == '.'){
            return READVERSIONDOT;
        }
        return DGRAMERR;
    }

    if (prevState == READVERSIONDOT){
        if ( c >= '0' && c <= '9'){
            return READVERSIONSUBNUM;
        }
        return DGRAMERR;
    }

    if (prevState == READVERSIONSUBNUM){
        if ( c == '\n'){
            return READFIRSTNEWLINE;
        }
    }

    if (prevState == READFIRSTNEWLINE){
        if (isalnum(c)){
            *length += 1;
            return READCLIENTDGRAMID;
        }
        return DGRAMERR;
    }

    if (prevState == READCLIENTDGRAMID){
        if (c == '\n' && *length == IDLENGTH){
            return READSECONDNEWLINE;
        }
        if ( *length < IDLENGTH && isalnum(c)){
            *length += 1;
            return READCLIENTDGRAMID;
        }
        return DGRAMERR;
    }


    if (prevState == READSECONDNEWLINE){
        if (c != '\n'){
            return READCLIENTPASSWORD;
        }
        return DGRAMERR;
    }

    if (prevState == READCLIENTPASSWORD){
        if (c == '\n')
            return READTHIRDNEWLINE;
        return READCLIENTPASSWORD;
    }

    if (prevState == READTHIRDNEWLINE){
        if (c == 'B'){
            return READB;
        } else if ( c == 'H'){
            return READH;
        } else if ( c == 'C'){
            return READC;
        }
        return DGRAMERR;
    }

    if ( prevState == READC){
        if (c == 'C'){
            return READCC;
        }
        return DGRAMERR;
    }

    if ( prevState == READH){
        if (c == 'C'){
            return READHC;
        }
        return DGRAMERR;
    }

    if (prevState == READB){
        if (c == 'R'){
            return READBR;
        }
        if (c == 'T'){
            return READBT;
        }
        return DGRAMERR;
    }

    if (prevState == READBT){
        if ( c == '\0'){
            return DGRAM_BT_COMMAND;
        }
        return DGRAMERR;
    }

    if ( prevState == READBR){
        if (c == '\0'){
            return DGRAM_BR_COMMAND;
        }
        return DGRAMERR;
    }

    if ( prevState == READHC){
        if (c == '\0'){
            return DGRAM_HC_COMMAND;
        }
        return DGRAM_CC_COMMAND;
    }

    if (prevState == READCC){
        if (c == '\0'){
            return DGRAM_CC_COMMAND;
        }
        return DGRAMERR;
    }
    return DGRAMERR;

}


static inline bool checkVersion(char * version){
    return strcmp(version,VERSION) == 0;
}

static inline bool checkAuthentication(char * auth){
    return strcmp(auth,AUTHKEY) == 0;
}

//returns end of line, more efficient
char * copyVersion(char * dgram, char * dest){
    char * versionPtr = strchr(dgram,'V');
    while (*versionPtr != '\n')
    {
        *dest++ = *versionPtr++;
    }

    *dest = 0;

    return versionPtr;
    
}

char * copyLine(char * line, char * dest){
    while ( *line != '\n'){
        *dest++ = *line++;
    }
    *dest = 0;
    return line;
}


void copyDgramData(char * dgram, mp3p_data * dest){
    char * currentDgramPosition = copyVersion(dgram,dest->headers.version);
    //we skip the newline
    currentDgramPosition++;
    // we copy the packet id
    currentDgramPosition = copyLine(currentDgramPosition,dest->headers.uniqueID);
    //we skip newline
    currentDgramPosition++;
    //we copy the password
    copyLine(currentDgramPosition,dest->headers.authorization);

}



int parseDatagram(char * dgram, int dgramLen,mp3p_data * dest){
    mp3p_states currentState = START;
    int idLength = 0;
    int i;
    for ( i = 0; i < dgramLen && i < MAXDATAGRAMLENGTH; i++){
        currentState = parseMp3pCharacter(dgram[i],currentState,&idLength);
        if ( currentState == DGRAMERR ){
            break;
        } 
    }

    if (i > MAXDATAGRAMLENGTH || currentState == DGRAMERR){
        return DGRAMERROR;
    }

    //datagram is well-formed, we can copy the data
    copyDgramData(dgram,dest);

    if (!checkVersion(dest->headers.version)){
        currentState = INVALID_VERSION;
    } else if (!checkAuthentication(dest->headers.authorization)){
        currentState = INVALID_AUTHKEY;
    }


    switch (currentState)
    {
    case DGRAM_BT_COMMAND:
        dest->commandFunction = bytesTransferedStrategy;
        break;
    case DGRAM_BR_COMMAND:
        dest->commandFunction = bytesRecievedStrategy;
        break;
    case DGRAM_HC_COMMAND:
        dest->commandFunction = historicConnectionsStrategy;
        break;
    case DGRAM_CC_COMMAND:
        dest->commandFunction = concurrentConnectionsStrategy;
        break;
    case INVALID_AUTHKEY:
        dest->commandFunction = unauthorizedStrategy;
        break;
    case INVALID_VERSION:
        dest->commandFunction = versionMismatchStrategy;
    default:
        break;
    }

    return DGRAMSUCCESS;
}


/*
cliente-->servidor
MP3P V1.0\n
IDENTIFICADOR_DATAGRAMA\n
AUTORIZACION\n
COMMANDO'\0'
*/


/*
servidor-->cliente
MP3P V1.0 2xx\n
IDENTIFICADOR_DATAGRAMA\n
DATOS
*/

/*
servidor-->cliente
MP3P V1.0 1xx\n
IDENTIFICADOR_DATAGRAMA\n
*/



/*


CLIENTE-->SERVIDOR

MP3P V1.0\n
CLAVEUSUARIO\n
COMMANDO_USUARIO0

*/



/*

SERVIDOR--->CLIENTE
200 MP3pV1.0\n
UID
DATOS'0'

*/



/*
    BT
    BR 
    CC
    HC
*/

/*
int parseDatagram(char * datagram){

}
*/


// recibir como argumento la clave del usuario.
// >BT
// >BR
// >CC
// >HC
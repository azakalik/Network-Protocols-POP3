#include "mp3pFunctions.h"
#include <string.h>
#include <stdbool.h>
#include "../stats/stats.h"
#include <stdio.h>

#define AUTHTOKEN "aguanteProtos"


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
} mp3p_states;






typedef enum{
    READINGVERSION,
    READINGAUTH,
    READINGPASSWORD,
} proccess_dgram_states;



#define SERVERSTATSMESSAGE "MP3P V1.0 200\n%s\n%ld"
#define SERVERERRORMESSAGE "MP3P V1.0 %d\n%s\n"

//error type 100, bad request

static int errorDatagramMessage(char * dgramOutput,mp3p_headers_data * data, int error){
    return sprintf(dgramOutput,SERVERERRORMESSAGE,error,data->uniqueID) + 1;//we count null terminated
}

//error type 100, malformed datagram
static int malformedDatagramStrategy(mp3p_headers_data *data,char * dgramOutput){
    return errorDatagramMessage(dgramOutput,data,100);
}

//error type 101, unauthorized
static int unauthorizedStrategy(mp3p_headers_data *data, char * dgramOutput){
    return errorDatagramMessage(dgramOutput,data,101);
}
//error type 102, wrong protocol version
static int wrongProtocolVersionStrategy(mp3p_headers_data * data, char * dgramOutput){
    return errorDatagramMessage(dgramOutput,data,102);
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





static mp3p_states parseMp3pCharacter(char c, mp3p_states prevState){
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
        if (c != '\n'){
            return READCLIENTDGRAMID;
        }
        return DGRAMERR;
    }

    if (prevState == READCLIENTDGRAMID){
        if (c == '\n'){
            return READSECONDNEWLINE;
        }
        return READCLIENTDGRAMID;
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
    //we copy the password
    currentDgramPosition = copyLine(currentDgramPosition,dest->headers.authorization);
    //we skip newline
    currentDgramPosition++;
    // we copy the packet id
    copyLine(currentDgramPosition,dest->headers.uniqueID);

}



int parseDatagram(char * dgram, int dgramLen,mp3p_data * dest){
    mp3p_states currentState = START;
    for ( int i = 0; i < dgramLen && currentState != DGRAMERR; i++){
        currentState = parseMp3pCharacter(dgram[i],currentState);
    }

    if ( currentState == DGRAMERR ){
        dest->commandFunction = malformedDatagramStrategy;
        return DGRAMERROR;
    }


    //datagram is well-formed, we can copy the data
    copyDgramData(dgram,dest);


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
    case DGRAM_CC_COMMAND:
        dest->commandFunction = concurrentConnectionsStrategy;
    default:
        break;
    }


    return DGRAMSUCCESS;
}


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
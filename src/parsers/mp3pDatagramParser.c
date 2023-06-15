#include "mp3pDatagramParser.h"
#include <string.h>


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
    READBR,
    DGRAMOK,
    DGRAMERR,

} mp3p_states;






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

    if (prevState == READBT || prevState == READBR){
        if ( c == '\0'){
            return DGRAMOK;
        }
        return DGRAMERR;
    }

    return DGRAMERR;



}



int parseDatagram(char * dgram, int dgramLen,mp3p_data * dest){
    mp3p_states currentState = START;
    for ( int i = 0; i < dgramLen ; i++){
        currentState = parseMp3pCharacter(dgram[i],currentState);
        if ( currentState == DGRAMERR)
            return DGRAMERROR;
    }

    if ( currentState != DGRAMOK){
        return DGRAMERROR;
    }

    return 0;
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
// 
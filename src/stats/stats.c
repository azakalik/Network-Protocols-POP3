#include "stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




static stats_singleton * createStatsInstance(){
    static stats_singleton statistics;
    memset(&statistics,0,sizeof(stats_singleton));
    return &statistics;
}




static stats_singleton * getStatsInstance(){
    static stats_singleton * singletonPtr = NULL;
    if ( singletonPtr == NULL ){
        singletonPtr = createStatsInstance();
    }
    return singletonPtr;
}



void addTransferedBytes(int bytes){
    stats_singleton * stats = getStatsInstance();
    stats->bytesTransfered += bytes;
}

void addConnection(){
    stats_singleton * stats = getStatsInstance();
    stats->concurrentConnections += 1;
    stats->historicConnections += 1;
}

void removeConnection(){
    stats_singleton * stats = getStatsInstance();
    stats->concurrentConnections -= 1;
}
#ifndef STATS
#define STATS

#include <stdint.h>

typedef struct {
    uint64_t historicConnections;
    uint64_t bytesTransfered;
    uint64_t bytesRecieved;
    int concurrentConnections;
} stats_singleton;


void addTransferedBytesToStats(int bytes);
void addConcurrentConnectionToStats();
void removeConcurrentConnectionFromStats();
void addRecievedBytesToStats(int bytes);

#endif
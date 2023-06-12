#ifndef STATS
#define STATS

#include <stdint.h>

typedef struct {
    uint64_t historicConnections;
    uint64_t bytesTransfered;
    int concurrentConnections;
} stats_singleton;


#endif
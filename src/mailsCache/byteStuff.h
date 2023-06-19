#ifndef BYTE_STUFF
#define BYTE_STUFF

typedef struct charactersProcessor charactersProcessor;

charactersProcessor * initCharactersProcessor();
void freeCharactersProcessor(charactersProcessor * charactersProcessor);
int getNProcessedCharacters(charactersProcessor * charactersProcessor, char * buffer, int n);
int addCharactersToProcess(charactersProcessor * charactersProcessor, char * buffer, int n);
int availableCharacters(charactersProcessor * charactersProcessor);
int availableSpace(charactersProcessor * charactersProcessor);

#endif
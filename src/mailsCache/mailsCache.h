#ifndef MAIL_CACHE
#define MAIL_CACHE

typedef struct mailCache mailCache;

mailCache * initCache(char * username);
void freeCache(mailCache * mailCache);
int openMail(mailCache * mailCache, int mailNo);
int markMailToDelete(mailCache * mailCache, int mailNo);
int resetToDelete(mailCache * mailCache);
int deleteMarkedMails(mailCache * mailCache);
int getAmountOfMails(mailCache * mailCache); //excludes mails marked to be deleted
long getSizeOfMails(mailCache * mailCache); //excludes mails marked to be deleted

#endif
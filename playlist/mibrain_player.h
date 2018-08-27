
#ifdef __cplusplus
extern "C" {
#endif


/* interal define */
#define SAFE_MALLOC(len) ((len) ? calloc((len+1),1) : NULL)
#define SAFE_CALLOC(len,count) ((len) && (count) ? calloc((len+1),(count)) : NULL)
#define SAFE_FREE(ptr) \
do{ \
    if((ptr)) { \
        free((ptr)); \
        (ptr) = NULL; \
    } \
}while(0)

#define SAFE_MEMCPY(s1,s2,len) ((s1) && (s2) && (len) ? memcpy((s1),(s2),(len)) : NULL)

#define SAFE_STRLEN(str) ((str) ? strlen((str)): 0)
#define SAFE_STRCPY(s1,s2) ((s1) && (s2) ? strcpy((s1),(s2)) : NULL)
#define SAFE_STRCMP(s1,s2) ((s1) && (s2) ? strcmp((s1),(s2)) : -1)
#define SAFE_STRDUP(str) ((str) ? strdup(str) : NULL)




int miBrainSdkPlayerInit(void);

void miBrainSdkPlayerExit(void);



void miBrainSdkPlayerClearPlayList(void);

int miBrainSdkPlayerStart(void);

void miBrainSdkPlayerResume(void);
void miBrainSdkPlayerPause(void);

int miBrainSdkPlayerNext(void);
int miBrainSdkPlayerPrev(void);

void miBrainSdkPlayerItemfinished(void);


/* only for outside temp */
int miBrainSdkPlayerAddItem(const char *cp, const char *id, const char *url, const char *type);

#include "mibrainsdk.h"
int miBrainSdkPlayerConfig(MiBrainSdkRequestParams *params, char *format, char *xiaomiId, char *albumId);
int miBrainSdkPlayerRefreshConfig(enum MiBrainAuthMode enAuthMode,  char *token);


#ifdef __cplusplus
}
#endif

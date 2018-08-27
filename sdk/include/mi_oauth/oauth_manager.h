#ifndef MIBRAIN_OAUTH_MANAGER_H
#define MIBRAIN_OAUTH_MANAGER_H


#ifdef __cplusplus
extern "C" {
#endif
/**
 * 用于辅助授权过期验证等,一个设备对应一个MiBrainSdkOauthManager,每次只会有一个请求发出,其中有一个未结束其余的都将阻塞
 */
#include <pthread.h>
#include "mibrainsdk.h"

#define MIBRAIN_OAUTHMANAGER_ERROR_NET                  -111
#define MIBRAIN_OAUTHMANAGER_ERROR_CALLBCK_FAILED       -112
#define MIBRAIN_OAUTHMANAGER_ERROR_INTERNAL             -113
#define MIBRAIN_OAUTHMANAGER_ERROR_SERVER               -115
#define MIBRAIN_OAUTHMANAGER_ERROR_PARAMS               -116
#define MIBRAIN_OAUTHMANAGER_MALLOC                     -117
#define MIBRAIN_OAUTHMANAGER_INVALID_CODE               -118
#define MIBRAIN_OAUTHMANAGER_SPECIAL_CODE               -119

#ifndef MIBRAIN_OAUTH_XIAOMIPASSPORT
#define MIBRAIN_OAUTH_XIAOMIPASSPORT 0
#endif
#ifndef MIBRAIN_OAUTH_ENV_PRODUCTION
#define MIBRAIN_OAUTH_ENV_PRODUCTION 0
#endif
#ifndef MIBRAIN_OAUTH_ENV_PREVIEW
#define MIBRAIN_OAUTH_ENV_PREVIEW 1
#endif
#ifndef MIBRAIN_OAUTH_ENV_STAGING
#define MIBRAIN_OAUTH_ENV_STAGING 2
#endif
#ifndef MIBRAIN_OAUTH_PT_SHUIDI
#define MIBRAIN_OAUTH_PT_SHUIDI 1
#endif
#ifndef MIBRAIN_OAUTH_PT_DEV
#define MIBRAIN_OAUTH_PT_DEV 0
#endif

#define AUTH_TYPE_OAUTH 1
#define AUTH_TYPE_ANONYMOUS 5
#define AUTH_TYPE_ANONYMOUS_PROXY 6

typedef int (*MiBrainSdkOauthManagerPut)(const char *scopeData, const char *data, int size, void *pVoid);

typedef int (*MiBrainSdkOauthManagerGet)(const char *scopeData, char *data, int size, void *pVoid);

typedef int (*MiBrainSdkOauthManagerGetCode)(const char *scopeData, char *data, int size, void *pVoid);

typedef struct {
    MiBrainSdkOauthManagerPut put;
    MiBrainSdkOauthManagerGet get;
    MiBrainSdkOauthManagerGetCode getCode;
    int env;
    char *redirectUrl;
    char *clientSecret;
    char *clientId;
    char *scopeData;
    void *pVoid;
    int timeOut;
    pthread_mutex_t mutex;
    int pt;
    int auth_type;
    //if type is AUTH_TYPE_ANONYMOUS, we need below
    char *api_key;
    char *device_id;
    char *app_data;
} MiBrainSdkOauthManager;

AIVS_EXPORT int MiBrainSdkOauthManagerInit(MiBrainSdkOauthManager *manager);

AIVS_EXPORT int MiBrainSdkOauthManagerSetPVoid(MiBrainSdkOauthManager *manager, void *pVoid);

AIVS_EXPORT int MiBrainSdkOauthManagerSetCallBack(MiBrainSdkOauthManager *manager, MiBrainSdkOauthManagerPut put, MiBrainSdkOauthManagerGet get, MiBrainSdkOauthManagerGetCode getCode);

AIVS_EXPORT int MiBrainSdkOauthManagerSetRedirectUrl(MiBrainSdkOauthManager *manager, const char *url, int hasEncoded);

AIVS_EXPORT int MiBrainSdkOauthManagerSetEnv(MiBrainSdkOauthManager *manager, int env);

AIVS_EXPORT int MiBrainSdkOauthManagerSetScope(MiBrainSdkOauthManager *manager, const char *scope);

AIVS_EXPORT int MiBrainSdkOauthManagerSetClientId(MiBrainSdkOauthManager *manager, const char *clientId);

AIVS_EXPORT int MiBrainSdkOauthManagerGetToken(MiBrainSdkOauthManager *manager, char *buf, int size, int forceUseCode, int *isFromCode);

AIVS_EXPORT int MiBrainSdkOauthManagerClean(MiBrainSdkOauthManager *manager);

AIVS_EXPORT int MiBrainSdkOauthManagerSetClientSecret(MiBrainSdkOauthManager *manager, const char *clientSecret);

AIVS_EXPORT int MiBrainSdkOauthManagerUnInit(MiBrainSdkOauthManager *manager);

AIVS_EXPORT int MiBrainSdkOauthManagerSetTimeOut(MiBrainSdkOauthManager *manager, int timeOut);

AIVS_EXPORT int MiBrainSdkOauthManagerSetPt(MiBrainSdkOauthManager *manager, int pt);

AIVS_EXPORT int MiBrainSdkOauthManagerSetAnonymousAuthMode(MiBrainSdkOauthManager *manager, char *api_key, char *device_id, char *app_data, int mode);

AIVS_EXPORT int MiBrainSdkOauthManagerUpdateAnonymousAuthArgs(MiBrainSdkOauthManager *manager, char *clientId,
        char *signSecret, char *apiKey, char *appData);

AIVS_EXPORT char *miBrainSdkGenAnonymousAuthorization(MiBrainSdkOauthManager *manager, const char *clientId, const char *apiKey, const char *token);

AIVS_EXPORT char *miBrainSdkGenAnonymousAuthorizationInternal(MiBrainSdkOauthManager *manager, const char *clientId, const char *apiKey, const char *token);

#ifdef __cplusplus
}
#endif

#endif

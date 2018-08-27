#ifndef MIBRAIN_OAUTH_H
#define MIBRAIN_OAUTH_H

#ifdef __cplusplus
extern "C" {
#endif

#define MIBRAIN_OAUTH_ERROR_NET           -11
#define MIBRAIN_OAUTH_ERROR_RAW_NOT_SET   -12
#define MIBRAIN_OAUTH_ERROR_INTERNAL      -13
#define MIBRAIN_OAUTH_ERROR_SERVER        -15
#define MIBRAIN_OAUTH_ERROR_PARAMS        -16
#define MIBRAIN_OAUTH_ERROR_MALLOC        -17

#ifndef MIBRAIN_OAUTH_XIAOMIPASSPORT
#define MIBRAIN_OAUTH_XIAOMIPASSPORT 0
#endif
#ifndef MIBRAIN_OAUTH_AONOYMOUS
#define MIBRAIN_OAUTH_AONOYMOUS 1
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

#include "./oauth_manager.h"

struct MiBrainSdkOauthResult {
    char *result;
    int retCode;//httpCode
    char *accessToken;
    char *refreshToken;
    long long expiresIn;
    int errorCode;//json错误码
};
/**
 * redirect_uri 需要转义后的
 *
 * authorization_code 当不是第一次请求时authorization_code可以是空
 * 除此其他都不可以为空
 */
struct MibrainSdkOauthParams {
    const char *redirectUri;
    const char *clientSecret;
    const char *authorizationCode;
    const char *clientId;
    const char *refreshToken;
    const char *scopeData;
    int env;
    int type;
    int timeout;
    int pt;
    int auth_type;
    //if type is AUTH_TYPE_ANONYMOUS, we need below
    char *api_key;
    char *device_id;
    char *app_data;
};

struct MiBrainSdkOauthResult *mibrainSdkOauthNew();

void miBrainSdkOauthFree(struct MiBrainSdkOauthResult *miBrainSdkOauthResult);

int miBrainSdkOauthRefreshToken(struct MibrainSdkOauthParams params, struct MiBrainSdkOauthResult *result);

int miBrainSdkAnonymousOauthRefreshToken(struct MibrainSdkOauthParams params, struct MiBrainSdkOauthResult *result);

#ifdef __cplusplus
}
#endif

#endif


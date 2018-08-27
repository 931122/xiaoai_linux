#ifdef __cplusplus
extern "C" {
#endif

#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cJSON.h"
#include "mibrainsdk.h"
#include "player.h"
#include "mibrain_player.h"
#include "mibrain_speak.h"

typedef enum _player_status {
    STOPPED,
    PLAYING,
    PAUSED,
    FINISHED
}player_status;

static volatile int current_play_status = FINISHED;

typedef struct _play_item_t {
    char *cp;
    char *id;
    char *url;
    char *type;  /* play type : music/radio */
    int playoffset;
} play_item_t;

#define MAX_PLAY_LIST   32

typedef struct _play_list_t {
    play_item_t items[MAX_PLAY_LIST];
    int current;  /* 当前播放item */
    int playcount;  /* 播放列表有多少item */
}play_list_t;

static play_list_t g_playList;

/* 新版本的SDK 播放器配置数据 可以不再用MiBrainSdkRequestParams 做中转 */
typedef struct player_resource_enginee{
    enum MiBrainEnv      env;
    enum MiBrainAuthMode authMode;
    char          *appId;
    char          *token;
    char          *deviceId;
    char          *scopeData;
    char          *xiaomiId;
    char          *format;
    char          *albumId;
}player_resource_enginee;

static player_resource_enginee g_aivs_engine;

int miBrainSdkPlayerAddItem(const char *cp, const char *id, const char *url, const char *type)
{
    int ret = 0;
    play_item_t *newItem;

    if ((NULL == cp || NULL == id) && (NULL == url)){
        return -1;
    }

    if (g_playList.playcount + 1 < MAX_PLAY_LIST) {
        newItem = &g_playList.items[g_playList.playcount];
        newItem->cp = SAFE_STRDUP(cp);
        newItem->id = SAFE_STRDUP(id);
        newItem->url = SAFE_STRDUP(url);
        newItem->type = SAFE_STRDUP(type);
        newItem->playoffset = 0;
        g_playList.playcount++;
    } else  {
        /* 播放列表已经满了 */
        printf("Add item failed due to reach max playlist count:%d\n", MAX_PLAY_LIST);
        return -1;
    }

    if (current_play_status == FINISHED) {
        /* report start play event to cloud */
        printf( "item %d. Trigger player start\n", g_playList.current);
        ret = miBrainSdkPlayerStart();
    }

    return ret;
}

void miBrainSdkPlayerClearPlayList(void)
{
    play_item_t *item;

    for (int i = 0; i < g_playList.playcount; i++) {
        item = &g_playList.items[i];
        SAFE_FREE(item->cp);
        SAFE_FREE(item->id);
        SAFE_FREE(item->url);
        SAFE_FREE(item->type);
    }
    g_playList.current = 0;
    g_playList.playcount = 0;
    return;
}

/* aync we need record current value in case flush by user */
static int miBrainSdkGetCurrentPlayURL(int currentItem, char url[])
{
    char *resourceResult;
    int ret = -1;
    play_item_t *item = &g_playList.items[currentItem];
    MiBrainSdkRequestParams params = {0};

    if (0 == g_playList.playcount){
        return -1;
    }

    if (NULL == item->cp || NULL == item->id){
        return -1;
    }

    /* new version: here we got from aivs engin */
    params.deviceId = g_aivs_engine.deviceId;
    params.authMode = g_aivs_engine.authMode;
    params.appId = g_aivs_engine.appId;
    params.appToken = g_aivs_engine.token;
    params.scopeData = g_aivs_engine.scopeData;
    params.env = g_aivs_engine.env;
    params.authMode = g_aivs_engine.authMode;

    printf("###fetch item:%d  status：%d\n", g_playList.current, current_play_status);

    resourceResult = miBrainSdkGetResUrlExt(params, item->cp, item->id, g_aivs_engine.xiaomiId, item->type,
                                         g_aivs_engine.albumId, g_aivs_engine.format);
    if (!resourceResult){
        return -1;
    }

    /* parse resourceResult */
    mibrain_cJSON * resultjson = mibrain_cJSON_Parse((const char *)resourceResult);
    if (resultjson != NULL) {
        mibrain_cJSON *status = mibrain_cJSON_GetObjectItem(resultjson, "status");
        if (status == NULL) {
            ret = -1;
        }
        mibrain_cJSON *code = mibrain_cJSON_GetObjectItem(status, "code");
        if (code->valueint == 200) {
            mibrain_cJSON *data = mibrain_cJSON_GetObjectItem(resultjson, "data");
            if (NULL != data) {
                mibrain_cJSON *content = mibrain_cJSON_GetArrayItem(data, 0);
                if (NULL == content){
                    ret = -1;
                }
                /* 会返回多个 */
                mibrain_cJSON *jsonurl = mibrain_cJSON_GetObjectItem(content, "url");
                if(jsonurl){
                    SAFE_STRCPY(url, jsonurl->valuestring);
                    printf("Get resouce result:%s\n", jsonurl->valuestring);
                    ret = 0;
                }
            }
       }
    }
    if (0 != ret){
        printf("Parse result[%s] failed\n", resourceResult);
    }
    SAFE_FREE(resourceResult);

    return ret;
}

int miBrainSdkPlayerStart(void)
{
    play_item_t *item;
    char url[2048];
    int ret;

    item = &g_playList.items[g_playList.current];

    if ((NULL == item->cp || NULL == item->id) && (NULL == item->url)){
        current_play_status = FINISHED;
        return -1;
    }
    // If high priority channel is playing, pending audio player
    //duer_speech_need_play_internal
    if (aivs_speak_on_work()) {
        current_play_status = PAUSED;
        g_playList.items[g_playList.current].playoffset = 0;
        return 0;
    } else {
        current_play_status = PLAYING;
    }

    /* url 不为空    可能为新闻       */
    if (item->url){
        /* report start play event to cloud */
        /* Call developer playe function */
        miBrain_audio_play_handler(item->url, item->playoffset);
    }else{
        /* get url by cp and id */
        ret = miBrainSdkGetCurrentPlayURL(g_playList.current, url);
        if (0 == ret) {
            /* report start play event to cloud */
            miBrain_audio_play_handler(url, item->playoffset);
            return 0;
        }else {
            printf( "Start player get url failed. \n");
            return -1;
        }
    }
    return 0;
}

void miBrainSdkPlayerPause(void)
{
    if (current_play_status == PLAYING) {
        current_play_status = PAUSED;
        printf( "Player change status to paused.\n");
        g_playList.items[g_playList.current].playoffset = miBrain_audio_pause_handler();
        /* report pause event to cloud */
    }
}

void miBrainSdkPlayerResume(void)
{
    if (current_play_status != PAUSED) {
        return;
    }

    current_play_status = PLAYING;
    printf( "Player resume change status to playing.offset[%u]\n",g_playList.items[g_playList.current].playoffset);
    printf( "player resume %d. Trigger player start\n", g_playList.current);

    miBrainSdkPlayerStart();
}

/* 用户控制跳过当前播放item 播放下一首 */
int miBrainSdkPlayerNext(void)
{
    if (current_play_status != PLAYING)    {
        printf( "Current play status %d \n", current_play_status);
        return -1;
    }
    /* report status to cloud */

    /* stop current */
    miBrain_audio_stop_handler();
    if (g_playList.playcount - 1 == g_playList.current) {
        /* 到达播放列表尾 next重新开始            */
        g_playList.current = 0;
    } else {
        g_playList.current++;
    }

    printf( "item %d. Trigger player next start\n", g_playList.current);

    miBrainSdkPlayerStart();

    return 0;
}

int miBrainSdkPlayerPrev(void)
{
    if (current_play_status != PLAYING)    {
        printf( "Current play status %d \n", current_play_status);
        return -1;
    }
    /* report status to cloud */

    /* stop current */
    miBrain_audio_stop_handler();
    if (g_playList.current == 0){
        /* 已经在第一首,前一首跳到最后 */
        g_playList.current = g_playList.playcount - 1;
    }else {
        g_playList.current--;
    }
    printf( "item %d. Trigger player prev start\n", g_playList.current);
    miBrainSdkPlayerStart();

    return 0;
}

/* 用户调用通知当前歌曲播放完毕 自动播放下一首 */
void miBrainSdkPlayerItemfinished(void)
{
    if (current_play_status != PLAYING) {
        return;
    }

    /* report to cloud. item play FINISH */
    current_play_status = FINISHED;

    if (g_playList.playcount - 1 == g_playList.current)
    {
        /* 到达播放列表尾 清除播放列表 LoadMore 回头重新播放? */
        printf( "play done\n");
    }
    else
    {
        g_playList.current++;
        printf( "Current play status change to %d item[%d] \n", current_play_status, g_playList.current);
        miBrainSdkPlayerStart();
    }
}

int miBrainSdkPlayerRefreshConfig(enum MiBrainAuthMode enAuthMode,  char *token)
{
    /* support authmode dynamic change */
    g_aivs_engine.authMode = enAuthMode;

    if (g_aivs_engine.token){
        SAFE_FREE(g_aivs_engine.token);
        g_aivs_engine.token = NULL;
        g_aivs_engine.token = SAFE_STRDUP(token);
    }else{
        g_aivs_engine.token = SAFE_STRDUP(token);
    }
    return 0;
}

/* 设置一次 mplayer SDK 维护 */
int miBrainSdkPlayerConfig(MiBrainSdkRequestParams *params, char *format, char *xiaomiId, char *albumId)
{
    /* just for in case of */
    g_aivs_engine.deviceId = SAFE_STRDUP(params->deviceId);
    g_aivs_engine.appId = SAFE_STRDUP(params->appId);
    g_aivs_engine.scopeData = SAFE_STRDUP(params->scopeData);
    g_aivs_engine.format = SAFE_STRDUP(format);
    g_aivs_engine.xiaomiId = SAFE_STRDUP(xiaomiId);
    g_aivs_engine.albumId = SAFE_STRDUP(albumId);

    g_aivs_engine.authMode = params->authMode;
    g_aivs_engine.env = params->env;

    return 0;
}

int miBrainSdkPlayerInit(void)
{
    /* 1.prepare playlist */
    memset(&g_playList, 0, sizeof(g_playList));
    current_play_status = FINISHED;

    memset(&g_aivs_engine, 0, sizeof(g_aivs_engine));

    return 0;
}

void miBrainSdkPlayerExit(void)
{
    play_item_t *item;

    for (int i = 0; i < g_playList.playcount; i++) {
        item = &g_playList.items[i];
        SAFE_FREE(item->cp);
        SAFE_FREE(item->id);
        SAFE_FREE(item->url);
        SAFE_FREE(item->type);
    }


    if (g_aivs_engine.deviceId){
        SAFE_FREE(g_aivs_engine.deviceId);
        g_aivs_engine.deviceId = NULL;
    }
    if (g_aivs_engine.appId){
        SAFE_FREE(g_aivs_engine.appId);
        g_aivs_engine.appId = NULL;
    }
    if (g_aivs_engine.token){
        SAFE_FREE(g_aivs_engine.token);
        g_aivs_engine.token = NULL;
    }
    if (g_aivs_engine.scopeData){
        SAFE_FREE(g_aivs_engine.scopeData);
        g_aivs_engine.scopeData = NULL;
    }
    if (g_aivs_engine.xiaomiId){
        SAFE_FREE(g_aivs_engine.xiaomiId);
        g_aivs_engine.xiaomiId = NULL;
    }
    if (g_aivs_engine.format){
        SAFE_FREE(g_aivs_engine.format);
        g_aivs_engine.format = NULL;
    }
}

#ifdef __cplusplus
}
#endif

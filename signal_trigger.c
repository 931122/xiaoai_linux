#ifdef __cplusplus
extern "C" {
#endif /* C++ */

#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <signal.h>
#include "unistd.h"
#include "pthread.h"
#include <alsa/asoundlib.h>
#include <sys/inotify.h>
#include "mibrainsdk.h"
#include "audiorec.h"
#include "player.h"
#include "cJSON.h"
#include "mibrain_player.h"
#include "mibrain_speak.h"

static pthread_mutex_t loadLock = PTHREAD_MUTEX_INITIALIZER;
static long long mTaskId = 0;
#define MAX_MSG_BUFFER  1024 * 64

#define SET_BIT(var, pos)   ((var) |= (1 << (pos)))
#define CHECK_BIT(var, pos)  (var & (((unsigned short)0x01) << (pos)))
#define TEST_BIT(var,pos)   (CHECK_BIT((var), (pos)) >> (pos))

static void onStart(long long taskId, void *userdata)
{
    //可以在此处对传入的自定义指针进行初始化，也可以在传入之前进行初始化
    printf("**********onStart Called********** \n");
}

static void onError(long long taskId, int code, void *userdata, char *msg)
{
    //必要时需要判断当taskId是否是最新的
    printf("**********onError Called %dmsg:%s**********\n", code, (char *)msg);
}


static void onEnd(long long taskId, void *userdata)
{
    //传入的自定义指针可以在此释放，onstart和oneEnd是必定会调用的，除非startRequest时返回了错误1
    printf("**********onEnd Called********** \n");
}

long long static newTaskId()
{
    long long l = 0;
    pthread_mutex_lock(&loadLock);
    l = ++mTaskId;
    pthread_mutex_unlock(&loadLock);
    return l;
}

MiBrainSdkRequestParams params;
long long taskId;
struct recorder *recorder;

static void parseMusicNlpResult(long long taskId, void *result, int len, int eof, int type, void *userContext)
{
    char contentBuffer[MAX_MSG_BUFFER];

    // for debug
    if (type != MIBRAINSDK_RESULT_TYPE_TTS_BINARY)
    {
        /* Rresult java下来是不带 \0 的 */
        memset(contentBuffer, 0, MAX_MSG_BUFFER);
        memcpy(contentBuffer, result, len);
        contentBuffer[len] = '\0';
        printf("****Json     txt:%s\n", (char *)contentBuffer);
    }

    /* warning
        处理请求 等可能耗时较长。

        此处也需要考虑异步处理
    */
    /* Rresult java下来是不带 \0 的 */
    memset(contentBuffer, 0, MAX_MSG_BUFFER);
    memcpy(contentBuffer, result, len);
    contentBuffer[len] = '\0';
    mibrain_cJSON * resultjson = mibrain_cJSON_Parse((const char *)result);
    if (resultjson == NULL || resultjson->type != mibrain_cJSON_Object){
        return;
    }
    mibrain_cJSON *meta = mibrain_cJSON_GetObjectItem(resultjson, "meta");
    if (meta == NULL || meta->type != mibrain_cJSON_Object){
        return;
    }

    mibrain_cJSON *metaType = mibrain_cJSON_GetObjectItem(meta, "type");
    if (metaType == NULL || metaType->type != mibrain_cJSON_String){
        return;
    }
    char *typeStr = metaType->valuestring;

    if (type == MIBRAINSDK_RESULT_TYPE_TTS_TXT)  {
        if (strcmp(typeStr, "RESULT_TTS_URL") == 0){
            mibrain_cJSON *response = mibrain_cJSON_GetObjectItem(resultjson, "response");
            if (NULL == response && response->type != mibrain_cJSON_Object){
                return;
            }
            mibrain_cJSON *file_url = mibrain_cJSON_GetObjectItem(response, "file_url");
            if (NULL == file_url || file_url->type != mibrain_cJSON_String){
                return;
            }

            aivs_speak_start(file_url->valuestring);
        }
    }

    if (type == MIBRAINSDK_RESULT_TYPE_TXT_NLP)  {
        if (strcmp(typeStr, "RESULT_NLP") == 0){
            mibrain_cJSON *response = mibrain_cJSON_GetObjectItem(resultjson, "response");
            if (NULL == response || response->type != mibrain_cJSON_Object){
                return;
            }
            mibrain_cJSON *status = mibrain_cJSON_GetObjectItem(response, "status");
            if (NULL == status){
                return;
            }
            mibrain_cJSON *code = mibrain_cJSON_GetObjectItem(status, "code");
            if (code->type != mibrain_cJSON_Number || 200 != code->valueint){
                /* log */
                return;
            }
            mibrain_cJSON *answer = mibrain_cJSON_GetObjectItem(response, "answer");
            if (NULL == answer){
                return;
            }
            mibrain_cJSON *contentAll = mibrain_cJSON_GetArrayItem(answer, 0);
            if (NULL == contentAll){
                return;
            }
            mibrain_cJSON *domain = mibrain_cJSON_GetObjectItem(contentAll,"domain");
            if(NULL == domain){
                return;
            }
            mibrain_cJSON *content = mibrain_cJSON_GetObjectItem(contentAll, "content");
            if (NULL == contentAll){
                return;
            }

            mibrain_cJSON *tospeak = mibrain_cJSON_GetObjectItem(content,"toSpeak");
            if(tospeak){
                /* TODO 启动 tts */
                printf("##################got to speak##############\n");
            }
            /* process music domain */
            if (0 == strcmp(domain->valuestring, "music")){
                mibrain_cJSON *musics = mibrain_cJSON_GetObjectItem(content,"musics");
                if (NULL == musics){
                    return;
                }
                /* clear current playlist */
                miBrainSdkPlayerClearPlayList();
                int num = mibrain_cJSON_GetArraySize(musics);
                for (int i = 0; i < num; i++) {
                    mibrain_cJSON *song = mibrain_cJSON_GetArrayItem(musics, i);
                    if (!song){
                       return;
                    }
                    mibrain_cJSON *id = mibrain_cJSON_GetObjectItem(song,"id");
                    if(!id){
                       return;
                    }
                    mibrain_cJSON *cp = mibrain_cJSON_GetObjectItem(song,"origin");
                    if(!cp){
                       return;
                    }

                    /* add cp id to array */
                    miBrainSdkPlayerAddItem(cp->valuestring, id->valuestring, NULL, "music");
                }
                return;
            }
            if (0 == strcmp(domain->valuestring, "soundboxControl")){
                mibrain_cJSON *action = mibrain_cJSON_GetObjectItem(contentAll, "action");
                if (0 == strcmp(action->valuestring, "next")){
                    miBrainSdkPlayerNext();
                }
                if (0 == strcmp(action->valuestring, "prev")){
                    miBrainSdkPlayerPrev();
                }
            }

            if (0 == strcmp(domain->valuestring, "soundboxnews")){
                mibrain_cJSON *news = mibrain_cJSON_GetObjectItem(content, "news");
                if (NULL == news){
                    return;
                }
                miBrainSdkPlayerClearPlayList();
                int num = mibrain_cJSON_GetArraySize(news);
                for (int i = 0; i < num; i++) {
                    mibrain_cJSON *newsitem = mibrain_cJSON_GetArrayItem(news, i);
                    if (!newsitem){
                       return;
                    }
                    mibrain_cJSON *playurl = mibrain_cJSON_GetObjectItem(newsitem, "playUrl");
                    if(!playurl){
                       return;
                    }
                    /* add cp id to array */
                    miBrainSdkPlayerAddItem(NULL, NULL, playurl->valuestring, NULL);
                }
                return;
            }
            if (0 == strcmp(domain->valuestring, "station")){
                mibrain_cJSON *stations = mibrain_cJSON_GetObjectItem(content, "stations");
                if (NULL == stations){
                    return;
                }
                miBrainSdkPlayerClearPlayList();
                int num = mibrain_cJSON_GetArraySize(stations);
                for (int i = 0; i < num; i++) {
                    mibrain_cJSON *station = mibrain_cJSON_GetArrayItem(stations, i);
                    if (!station){
                       return;
                    }
                    mibrain_cJSON *id = mibrain_cJSON_GetObjectItem(station, "id");
                    if(!id){
                       return;
                    }
                    /* warning 这里用CP 也可以? */
                    mibrain_cJSON *cp = mibrain_cJSON_GetObjectItem(station, "origin");
                    if(!cp){
                       return;
                    }
                    /* add cp id to array */
                    miBrainSdkPlayerAddItem(cp->valuestring, id->valuestring, NULL, "radio");
                }
                return;
            }

            if (0 == strcmp(domain->valuestring, "joke")){
                mibrain_cJSON *text = mibrain_cJSON_GetObjectItem(content, "text");
                if (NULL == text){
                    return;
                }
                /* tts manual */
                return;
            }
            /* parse default domain */
            if (0 == strcmp(domain->valuestring, "default")){

            }
        }
    }
}


/* after stop_record, there are still some data callbacks */
static void writeToFile(char *fileName, char *buf, int len)
{
    FILE *fpBinary = fopen(fileName, "a");
    if (!fpBinary){
        return;
    }

    if (fwrite(buf, 1, (size_t)len, fpBinary) != len)
    {
        /* here not close file handler ? */
        printf("write buffer error \n");
    }
    fclose(fpBinary);
}

/* the record call back */
static void record_cb(char *data, unsigned long len, void *user_para)
{
    int errcode = 0;
    //struct recorder *recorder = (struct recorder *)user_para;

    if(len == 0 || data == NULL)
        return;

    //printf("###record len :%ld\n", len);

    errcode = miBrainSdkAddData(taskId, data, len, 0);
    //writeToFile("record.wav", data, len);
    /* process audio */
    //errcode = write_audio_data(sr, data, len);
    if (errcode) {
        errcode = stop_record(recorder);
        //printf("###stop record :%ld error %d\n", len, errcode);
        unsigned int timeout_ms = -1;
        while (!is_record_stopped(recorder)) {
            sleep(1);
            //printf("###record stop status %d\n", recorder->state);
            if (timeout_ms != (unsigned int)-1)
                if (0 == timeout_ms--)
                    break;
        }
        return;
    }
}

#define DEFAULT_FORMAT        \
{\
    RAW_FORMAT_PCM,    \
    2,            \
    16000,            \
    32000,            \
    2,            \
    16,            \
    sizeof(PCMFORMATEX)    \
}

static int start_full(void)
{
    PCMFORMATEX wavfmt = DEFAULT_FORMAT;
    int errcode;
    record_dev_id devid;

    errcode = create_recorder(&recorder, record_cb, NULL);
    if (recorder == NULL || errcode != 0) {
        printf("create recorder failed: %s\n", snd_strerror(errcode));
        goto fail;
    }

    devid = get_default_input_dev();
    errcode = open_recorder(recorder, devid, &wavfmt);
    if (errcode != 0) {
        printf("recorder open failed: %s\n", snd_strerror(errcode));
        goto fail;
    }

    errcode = start_record(recorder);
    if (errcode != 0) {
        printf("start record failed: %s\n", snd_strerror(errcode));
        goto fail;
    }

    /* 等待录音结束或者 信号异步等 */
    /* 远场通信模式 如果按键录音 由按键事件和VAD综合判断录音结束 */
    while (!is_record_stopped(recorder)) {
        sleep(1);
    }

    printf("recorder full stop.\n");

    miBrainSdkStopRequest(taskId, 1);

fail:
    if (recorder) {
        if(!is_record_stopped(recorder))
            stop_record(recorder);
        close_recorder(recorder);
        destroy_recorder(recorder);
        recorder = NULL;
    }

    return 0;

}

static int buldParm(void)
{
    params.callbacks.onError = onError;
    params.callbacks.onResults = parseMusicNlpResult;
    params.callbacks.onEnd = onEnd;
    params.callbacks.onStart = onStart;
    params.asrParams.sampleRate = 16000; //当前只支持１６０００
    params.asrParams.useInternalVad = 2; //是否使用内部的vad；
    params.asrParams.channels = 1;
    params.asrParams.audioBitNum = 16;
    params.receiveTimeout = 15;
    params.connectTimeout = 5;
    params.uploadTimeout = 5;
    params.activeTimeout = 15;
    params.deviceId = "EVAL-DEVICE-ID";
    params.userAgent = "Dalvik/2.1.0 (Linux; U; Android 6.0; Redmi Note 4X MIUI/8.5.25-internal";
    params.sdkCmd = MIBRAINSDK_CMD_ASR_NLP_TTS; //当传入tts时
    params.ttsParams.ttsAsFileUrl = 1;
    params.authMode = MIBRAIN_AUTH_TPTOKEN;
    params.codecMode = 0x104;
    params.opusBitRates = MIBRAIN_OPUS_BITRATES_32K;
    params.opusFrameSize = MIBRAIN_OPUS_FRAMESIZE_1280;
    /* 当前仅支持2.2 版本了。parseMusicNlpResult 解析结果有变化 需要重写解析过程 */
    //params.nlpParams.nlpVersion = "2.2";

    /* tp 认证方式 初始profile 生成参考 deviceTokenAuth。后续解析profile 请求token。
       token有效期7天 刷新参考profile.py         改写   c ?   */
    // TODO TOKEN REFRESH
    params.appId = "326766038739850240";
    params.appToken = "RMQ3XqsVzc8.UcQiXr4X20-URMu9VeZFxlzcDyf_I7yWNqMcZ89bvakv8gMM6HP6nnn5L035Q5eNEIcGG_lDl4UV9GIq-ECc2bp51-mgA83qRMQ.KE3F44Slx688FyekDoA7DTrQvbcJo10sb-11fKmxvSQ";

    return 0;
}

void trigger_device_wakeup(void)
{
    taskId = newTaskId();
    int r = miBrainSdkStartRequest(params, taskId);
    if (r != 0)
    {
        return;
    }

    /* 实际打开需要考虑 异步 */
    start_full();

    /* 需要考虑 异步 */

    //pthread_t dothing;
    //pthread_create(&dothing, NULL, start_full, NULL);

    return;
}

#if 0  /* nlp test in case  no voice input */
static int onMiBrainSdkGetToken(long long taskId, int lastIsFailed, char *buffer, int bufferLength, void *userContext)
{
    return 0;
}

int testBananaNlpTts(const char *string, enum MiBrainEnv env)
{
    MiBrainSdkRequestParams params = {0};
    params.callbacks.onResults = parseMusicNlpResult;
    params.deviceId = "EVAL-DEVICE-ID";
    params.sdkCmd = MIBRAINSDK_CMD_NLP_TTS;
    params.nlpParams.nlpText = string;
    //params.nlpParams.nlpVersion ="2.2";
    params.ttsParams.ttsAsFileUrl = 1;

    /* 根据认证类型填入不同参数   TOKEN 在外部控制刷新 */
    params.authMode = MIBRAIN_AUTH_TPTOKEN;
    params.appId = "326766038739850240";
    params.appToken = "RMQ3XqsVzc8.UcQiXr4X20-URMu9VeZFxlzcDyf_I7yWNqMcZ89bvakv8gMM6HP6nnn5L035Q5eNEIcGG_lDl4UV9GIq-ECc2bp51-mgA83qRMQ.KE3F44Slx688FyekDoA7DTrQvbcJo10sb-11fKmxvSQ";

    long long taskId = newTaskId();
    miBrainSdkStartRequest(params, taskId);
    miBrainSdkStopRequest(taskId, MIBRAINSDK_STOP_FLAG_WAIT_RESULT); //同步stop并等待结果，不可以在对应task的回调中调用

    return 0;
}

static void testPlayer(enum MiBrainEnv env)
{
    MiBrainSdkRequestParams params = {0};
    params.callbacks.getToken = onMiBrainSdkGetToken;
    params.env = env;
    params.deviceId = "EVAL-DEVICE-ID";

    params.authMode = MIBRAIN_AUTH_TPTOKEN;
    params.appId = "326766038739850240";
    params.appToken = "z9JCl25o5HU.2tJXl3tq8vUfUr50kJtsfNfKeu46XpUsvbVprgomlBOk5HbFLQ7TJPLvWoQ8Pr43m5Fz0jw-vj-e4hfjPT21YyUBnSFlfuRQz9I.OIMc8uMKmtxqfvkc7b0Yao1rCD3l26nDiqUiqxALdQM";

    player_init();

    miBrainSdkPlayerInit();
    aivs_speak_init();

    /* 播放列表 取资源        参数   token 内部刷新 */
    miBrainSdkPlayerConfig(&params, NULL, "1206359785", NULL);
    miBrainSdkPlayerRefreshConfig(MIBRAIN_AUTH_TPTOKEN, "RMQ3XqsVzc8.UcQiXr4X20-URMu9VeZFxlzcDyf_I7yWNqMcZ89bvakv8gMM6HP6nnn5L035Q5eNEIcGG_lDl4UV9GIq-ECc2bp51-mgA83qRMQ.KE3F44Slx688FyekDoA7DTrQvbcJo10sb-11fKmxvSQ");

    {
    //testBananaNlpTts("播放诗歌", env);
    testBananaNlpTts("播放新闻", env);

    //testBananaNlpTts("讲5个笑话", env);
    //testBananaNlpTts("播放北京电台", env);

    //testBananaNlpTts("播放音乐", env);
    //sleep(15);

    //printf("##############播放下一首###################\n");
    ////testBananaNlpTts("播放下一首", env);

    //sleep(15);

    //printf("##############播放上一首###################\n");
    //testBananaNlpTts("播放上一首", env);

    //printf("status:%d  duration:%d position:%d\n", player_getstatus(), player_getduration(),player_getposition());
    //miBrain_audio_play_handler("chineseMan.mp3", 100);
    }
    while(1){
        sleep(30);
    }
    //testBananaNlpTts("播放下一首", env);
    miBrainSdkPlayerExit();
    aivs_speak_fini();
    player_exit();
}

#endif

/* pkill -10 demonname
 * 信号模拟唤醒或触控
*/
void singalHandle(int signal)
{
    trigger_device_wakeup();
}

static void main_loop(void)
{
    miBrainSdkPlayerInit();
    aivs_speak_init();
    player_init();

    /* 播放列表 取资源        参数   token 内部刷新 for music CP + ID 取资源 */
    miBrainSdkPlayerConfig(&params, NULL, "1206359785", NULL);

    /* TODO  定时器 线程 token刷新时  更新           */

    miBrainSdkPlayerRefreshConfig(MIBRAIN_AUTH_TPTOKEN, "RMQ3XqsVzc8.UcQiXr4X20-URMu9VeZFxlzcDyf_I7yWNqMcZ89bvakv8gMM6HP6nnn5L035Q5eNEIcGG_lDl4UV9GIq-ECc2bp51-mgA83qRMQ.KE3F44Slx688FyekDoA7DTrQvbcJo10sb-11fKmxvSQ");

    /* loop 唤醒 或按键 触发录音事件 */
    signal(SIGUSR1, singalHandle);
    while(1){
        sleep(2);
    }

    miBrainSdkPlayerExit();
    aivs_speak_fini();
    player_exit();
}

static pid_t safe_fork(void)
{
    pid_t result;
    result = fork();

    if (result == -1) {
        exit(1);
    }
    return result;
}


#ifdef ALL_EXAMPLE
int main(int argc, char *argv[])
{
    printf("MibrainSdk version: %s\n", miBrainSdkGetVerison());
    /* 测试账号go     preview */
    enum MiBrainEnv env = MIBRAIN_ENV_PREVIEW;

    if (argc >= 2)
    {
        if (strcmp(argv[1], "3")==0)
        {
            env = MIBRAIN_ENV_PRODUCTION;
        }
        if (strcmp(argv[1], "2")==0)
        {
            env = MIBRAIN_ENV_PREVIEW;
        }
    }
    params.env = env;
    buldParm();

    /* 其它配置   等     */
    miBrainSdkSetDebugLevel(MIBRAINSDK_DEBUG_LEVEL_DEBUG);

    switch (safe_fork()) {
    case 0:                /* child */
        setsid();
        main_loop();
        break;
    default:               /* parent */
        exit(0);
        break;
    }
    return 0;
}
#else


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif


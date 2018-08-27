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

#include "mibrainsdk.h"
#include "audiorec.h"
#include "player.h"
#include "cJSON.h"
#include "mibrain_player.h"
#include "mibrain_speak.h"
#include "xaudio_engine.h"

static pthread_mutex_t loadLock = PTHREAD_MUTEX_INITIALIZER;
static long long mTaskId = 0;
#define MAX_MSG_BUFFER  1024 * 64
MiBrainSdkRequestParams params;
long long taskId;
struct recorder *recorder;


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

    if(len == 0 || data == NULL)
        return;

    /* ignore the data if error/vad happened */

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
    params.userAgent = "dasdlmsakdjasdkas";
    params.sdkCmd = MIBRAINSDK_CMD_ASR_NLP_TTS;
    params.ttsParams.ttsAsFileUrl = 1;
    /* 当前仅支持2.2 版本了。parseMusicNlpResult 解析结果有变化 需要重写解析过程 */
    //params.nlpParams.nlpVersion = "2.2";

    /* tp 认证方式 初始profile 生成参考 deviceTokenAuth。后续解析profile 请求token。
       token有效期7天 刷新参考profile.py         改写   c ?   */
    params.authMode = MIBRAIN_AUTH_TPTOKEN;
    params.appId = "326766038739850240";
    params.appToken = "RMQ3XqsVzc8.UcQiXr4X20-URMu9VeZFxlzcDyf_I7yWNqMcZ89bvakv8gMM6HP6nnn5L035Q5eNEIcGG_lDl4UV9GIq-ECc2bp51-mgA83qRMQ.KE3F44Slx688FyekDoA7DTrQvbcJo10sb-11fKmxvSQ";

    return 0;
}

void trigger_device_wakeup(void)
{
    buldParm();
    taskId = newTaskId();
    int r = miBrainSdkStartRequest(params, taskId);
    if (r != 0)
    {
        return;
    }

    /* 实际打开需要考虑 异步 */
    start_full();

    /* 实际打开需要考虑 异步 */

    //pthread_t dothing;
    //pthread_create(&dothing, NULL, start_full, NULL);

    return;
}

///   wake up
volatile int g_is_exit  = 0;
void sigint_process(int sign_no)
{
     if(sign_no==SIGINT)
     {
        printf("GET INT SIGNAL!\n");
        g_is_exit = 1;
     }
}
void asr_data_callback(void * usr_data_asr, const char *buffer, size_t size)
{
    /* 上传语音数据做 sai 做后期处理 */
    printf("while while while. asr_data_callback ######\n");
    FILE *fp_asr;
    fp_asr = fopen("/tmp/asr_callback.pcm", "ab");
    fwrite(buffer, size, 1, fp_asr);
    fclose(fp_asr);
}

void wakeup_callback(void * usr_data_wk, int wakeup_result, float angle)
{
    printf("while while while. wakeup_callback result %d ######\n", wakeup_result);
    /* can't be blocked */

    /* async data  */
    if(wakeup_result == 1)
    {
        /* paplay  default sound card play we can send to mqtt player server or user play item */
        system("aplay wakeup1.wav");
        /* play wake up musics */
        trigger_device_wakeup();
    }
}

void ivw_data_callback(const char *buffer, size_t size)
{
    /* 上传唤醒数据做 wake-learning */
    printf("while while while. ivw_data_callback ######\n");

    set_wakeup_status();
    FILE *fp_asr;
    fp_asr = fopen("/tmp/ivw.pcm", "ab");
    fwrite(buffer, size, 1, fp_asr);
    fclose(fp_asr);
}

void voip_data_callback(void * usr_data_voip, const char *buffer, size_t size)
{
    printf("while while while. voip_data_callback ######\n");
    /* voip-mode  返回数据做进一步处理 MONO 16K S16_LE */
}

void oneshot_callback(OneShortEventType event)
{
    printf("while while while. oneshot_callback ######\n");
}

void wav_energy_callback(float val)
{
    printf("while while while. wav_energy_callback ######\n");
}

#ifdef WAKEUP_EXAMPLE
int main(int argc, char *argv[])
{
    printf("MibrainSdk version: %s\n", miBrainSdkGetVerison());
    enum MiBrainEnv env = MIBRAIN_ENV_PREVIEW;
    int ret_code;

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

    miBrainSdkSetDebugLevel(MIBRAINSDK_DEBUG_LEVEL_DEBUG);
    player_init();

    /* sdk interal interface */
    miBrainSdkPlayerInit();
    aivs_speak_init();

    miBrainSdkPlayerConfig(&params, NULL, "1206359785", NULL);
    miBrainSdkPlayerRefreshConfig(MIBRAIN_AUTH_TPTOKEN, "RMQ3XqsVzc8.UcQiXr4X20-URMu9VeZFxlzcDyf_I7yWNqMcZ89bvakv8gMM6HP6nnn5L035Q5eNEIcGG_lDl4UV9GIq-ECc2bp51-mgA83qRMQ.KE3F44Slx688FyekDoA7DTrQvbcJo10sb-11fKmxvSQ");

    char *wakeup_conf_path = "/home/pi/gitSrc/aivs-client/command_tool/demo/wakeup/xiaomi_config/xaudio_engine.conf";
    char *usr_data_asr = NULL;
    char *usr_data_wk = NULL;
    char *usr_data_voip = NULL;

    printf("compare to init system >o<!,conf_path %s\n", wakeup_conf_path);
    printf("recorder open failed: %s\n", snd_strerror(-2));
    printf("we got id:%d \n", get_input_dev_num());

    while(g_is_exit != 1){

        ret_code = init_system(NULL, 0, wakeup_conf_path,
                              asr_data_callback,
                              wakeup_callback,
                              ivw_data_callback,
                              voip_data_callback,
                              oneshot_callback,
                              wav_energy_callback,
                              usr_data_asr,
                              usr_data_wk,
                              usr_data_voip);
        if(ret_code == -1)
        {
            printf("system init failed\n");
            return -1;
        }

        printf("init system success! wow^^^^wow\n");
        signal(SIGINT, sigint_process);
        start_service();

        while(g_is_exit != 1)
        {
            sleep(1);
        }
        //delete_xaudio(h_xaudio);
        stop_service();
        terminate_system();
    }

    miBrainSdkPlayerExit();
    aivs_speak_fini();
    player_exit();

    return 0;
}
#else

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif


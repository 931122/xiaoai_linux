#ifndef MI_BRAINSDK_LIB_H
#define MI_BRAINSDK_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __unix__
#define AIVS_NO_EXPORT __attribute__ ((visibility("hidden")))
#define AIVS_EXPORT    __attribute__ ((visibility("default")))
#else
#define AIVS_NO_EXPORT
#define AIVS_EXPORT
#endif


/* errorcodes */
#define MIBRAINSDK_ERR_INIT_FAILED                      -1 //SDK init failed
#define MIBRAINSDK_ERR_UNINITIALED                      -2 //SDK 没有初始化
#define MIBRAINSDK_ERR_HAS_INITIALIZED                  -3//SDK 已经初始化
#define MIBRAINSDK_ERR_MEM_FAILED                       -4//sdl 分配内存失败
#define MIBRAINSDK_ERR_ENDED                            -5//无需再添加数据
#define MIBRAINSDK_ERR_PARAMS                           -6//参数错误
#define MIBRAINSDK_ERR_BUFFERQUEUE                      -7//队列或者音频总达到上限
#define MIBRAINSDK_ERR_UNKNOWN                          -8//未知错误
#define MIBRAINSDK_ERR_RECVDATA                         -9 //接到的服务器的数据异常
#define MIBRAINSDK_ERR_TRANSMISSION_CONNECT_TIMEOUT     -10//连接超时
#define MIBRAINSDK_ERR_TRANSMISSION_READ_DATA_ERROR     -11//数据读取出错
#define MIBRAINSDK_ERR_TRANSMISSION_WRITE_DATA_ERROR    -12//数据发送出错
#define MIBRAINSDK_ERR_TRANSMISSION_HANDSHAKE_ERROR     -13//握手失败
#define MIBRAINSDK_ERR_TRANSMISSION_CRT_ERROR           -14//证书异常
#define MIBRAINSDK_ERR_TRANSMISSION_NETWORK_ERROR       -15//网络出错
#define MIBRAINSDK_ERR_TRANSMISSION_INIT_FAILED_ERROR   -16//传输内容初始化失败
#define MIBRAINSDK_ERR_TRANSMISSION_INIT_STATE_ERROR    -17//传输状态异常
#define MIBRAINSDK_ERR_TRANSMISSION_READ_DATA_TIMEOUT   -18//读取数据超时
#define MIBRAINSDK_ERR_TRANSMISSION_WRITE_DATA_TIMEOUT  -19//发送数据超时
#define MIBRAINSDK_ERR_TRANSMISSION_ACTIVE_TIMEOUT      -20//连接过长时间不活跃
#define MIBRAINSDK_ERR_TRANSMISSION_SYSTEM_TIME_ERROR   -21//系统时间获取失败
#define MIBRAINSDK_ERR_TRANSMISSION_AUTH_FAILED         -22//授权失败
#define MIBRAINSDK_ERR_TRANSMISSION_GET_TOKEN_FAILED    -23//获取token失败

//mibrainsdk_get_resource_url接口的返回值
#define MIBRAINSDK_ERR_GET_RES_URL_PARAMS_ERROR         -30//cp或id为空
#define MIBRAINSDK_ERR_GET_RES_URL_AUTH_ERROR           -31//认证失败
#define MIBRAINSDK_ERR_GET_RES_URL_INITHTTP_ERROR       -32//init http error
#define MIBRAINSDK_ERR_GET_RES_URL_ERROR                -33//获取失败

//track相关错误码
#define MIBRAINSDK_ERR_TRACKMSG_PARAMS_ERROR         -40//
#define MIBRAINSDK_ERR_TRACKMSG_AUTH_ERROR           -41//认证失败
#define MIBRAINSDK_ERR_TRACKMSG_INITHTTP_ERROR       -42//init http error
#define MIBRAINSDK_ERR_TRACKMSG_ERROR                -43//获取失败


//result type
#define MIBRAINSDK_RESULT_TYPE_TXT_ASR                     11
#define MIBRAINSDK_RESULT_TYPE_TXT_ASR_FINAL               12
#define MIBRAINSDK_RESULT_TYPE_TTS_BINARY                  121
#define MIBRAINSDK_RESULT_TYPE_TTS_TXT                     122
#define MIBRAINSDK_RESULT_TYPE_TXT_NLP                     31
#define MIBRAINSDK_RESULT_TYPE_TXT_FLAG_SESSION_BEGIN      41
#define MIBRAINSDK_RESULT_TYPE_TXT_FLAG_SESSION_END        42
#define MIBRAINSDK_RESULT_TYPE_TXT_FLAG_SESSION_TTS_BEGIN  51
#define MIBRAINSDK_RESULT_TYPE_TXT_FLAG_SESSION_TTS_END    52
#define MIBRAINSDK_RESULT_TYPE_TXT_FLAG_OTHER              62
#define MIBRAINSDK_RESULT_TYPE_TXT_FLAG_ERROR              72
#define MIBRAINSDK_RESULT_TYPE_TXT_RESULT_VOR_REC          92
#define MIBRAINSDK_RESULT_TYPE_TXT_RESULT_VOR_REG_FINAL    82
#define MIBRAINSDK_RESULT_TYPE_TXT_RESULT_VOR_REG_PARTIAL  83
#define MIBRAINSDK_RESULT_TYPE_TXT_RESULT_VOR_DEL          84
#define MIBRAINSDK_RESULT_TYPE_TXT_RESULT_VOR_QUERY        85
#define MIBRAINSDK_RESULT_TYPE_TXT_INSTRUCTION             86
//cmds
#define MIBRAINSDK_CMD_ASR_NLP                             0
#define MIBRAINSDK_CMD_ASR                                 1
#define MIBRAINSDK_CMD_ASR_NLP_TTS                         2
#define MIBRAINSDK_CMD_TTS                                 3
#define MIBRAINSDK_CMD_NLP                                 4
#define MIBRAINSDK_CMD_NLP_TTS                             5
#define MIBRAINSDK_CMD_VOR_REG                             6
#define MIBRAINSDK_CMD_VOR                                 7
#define MIBRAINSDK_CMD_VOR_DEL                             8
#define MIBRAINSDK_CMD_VOR_QUERY                           9
#define MIBRAINSDK_CMD_EVENT                               10
#define MIBRAINSDK_CMD_EVENT_TTS                           11

// stop flag
#define MIBRAINSDK_STOP_FLAG_WAIT_RESULT                    0
#define MIBRAINSDK_STOP_FLAG_SYNC_STOP                      1
#define MIBRAINSDK_STOP_FLAG_ASYNC_STOP                     2

//miBrainSdkSetDebugLevel()
#define MIBRAINSDK_DEBUG_LEVEL_ERROR                        3
#define MIBRAINSDK_DEBUG_LEVEL_WARNING                      2
#define MIBRAINSDK_DEBUG_LEVEL_DEBUG                        1

//仅仅在注册声纹的时候有效，否则与BRAIN_ADD_DATA_FLAG_TASK_END同效果
#define MIBRAINSDK_ADD_DATA_FLAG_VOR_REG_DIV                 2
#define MIBRAINSDK_ADD_DATA_FLAG_TASK_END                    1
#define MIBRAINSDK_ADD_DATA_FLAG_CONTINUE                    0

//asrParams.useInternalVad
#define MIBRAINSDK_INTERNAL_VAD_NONE                         0
#define MIBRAINSDK_INTERNAL_VAD_LOCAL                        1
#define MIBRAINSDK_INTERNAL_VAD_CLOUD                        2



//request
typedef void (*OnMiBrainSdkOnStartCallBack)(long long taskId, void *userContext);

typedef void (*onMiBrainSdkResultCallBack)(long long taskId, void *result, int len, int eof, int type, void *userContext);

typedef void (*OnMiBrainSdkErrorCallBack)(long long taskId, int code, void *userContext, char *msg);

typedef void (*OnMiBrainSdkOnEndCallBack)(long long taskId, void *userContext);

typedef void (*OnMiBrainSdkOnEventCallBack)(long long taskId, void *userContext, int code, char *msg);

//当开始请求的时候没填token且授权方式与oauth有关,将会调用这个获取token
typedef int   (*onMiBrainSdkGetTokenCallback)(long long taskId, int lastIsFailed, char *buffer, int bufferLength, void *userContext);


//log
typedef void (*OnMiBrainSdkLog)(int lv, char *msg);


typedef struct _MiBrainSdkCallbacks {
    onMiBrainSdkResultCallBack onResults;
    OnMiBrainSdkErrorCallBack onError;
    OnMiBrainSdkOnEndCallBack onEnd;
    OnMiBrainSdkOnStartCallBack onStart;
    OnMiBrainSdkOnEventCallBack onEvent;
    onMiBrainSdkGetTokenCallback getToken;
} MiBrainSdkCallbacks;


typedef struct _MiBrainSdkNlpParamsUserInfo {
    const char *id;
    const char *idType;
    const char *gender;
    int         age;
    const char *ip;
    const char *extend;//json
} MiBrainSdkNlpParamsUserInfo;

typedef struct _MiBrainSdkNlpParams {
    MiBrainSdkNlpParamsUserInfo userInfo;
    const char *context;
    const char *session;
    const char *ip;
    const char *speechData;
    const char *nlpText;
    const char *nlpVersion;
    const char *customNlpApi;
    const char *customAppId;
    const char *customToken;
} MiBrainSdkNlpParams;

typedef struct _MiBrainSdkTtsParams {
    const char *ttsText;
    const char *ttsVendor;
    const char *ttsCodec;
    int         ttsVolume;
    int         ttsAsFileUrl;
    int         ttsTone;
    int         ttsSpeed;
    int         ttsSpeaker;
    int         ttsSuperaddition;//tts流式追加
    int         ttsTimeout;
} MiBrainSdkTtsParams;

typedef struct _MiBrainSdkAsrParams {
    const char *asrVendor;
    const char *asrLang;
    int         maxEmptyAudioTime;
    int         useInternalVad;//是否使用内部的vad检测
    int         sampleRate;
    int         audioBitNum;
    int         channels;
    int         isForSai;
    int         preAsrTrack; //新增pre_asr字段
    int         max_audio_seconds; //默认30s
    int         disable_timeout;
} MiBrainSdkAsrParams;

typedef struct _MiBrainSdkEventContextParams {
    const char *nameSpace;
    const char *name;
    const char *payload; // json字符串
} MiBrainSdkEventContextParams;

typedef struct _MiBrainSdkEventParams {
    MiBrainSdkEventContextParams *contexts;   //context数组
    int                           contextSize;//context数组长度
    const char                   *nameSpace;
    const char                   *name;
    const char                   *payload;    // json字符串
} MiBrainSdkEventParams;

typedef struct _MiBrainSdkVoiceprintParams {
    const char *familyId;
    const char *userId;
    const char *deleteIds;
    const char *queries;    //此字段作废
    const char *vendor;
    const char *query;      //新增字段
    int         voice_id;   //新增字段
    int         disable_text_dependent;
} MiBrainSdkVoiceprintParams;

enum MiBrainEnv {
    MIBRAIN_ENV_PRODUCTION,
    MIBRAIN_ENV_PREVIEW,
    MIBRAIN_ENV_STAGING,
    MIBRAIN_ENV_PRODUCTION_SGP,
};

enum MiBrainAuthMode {
    MIBRAIN_AUTH_APPIDTOKEN,
    MIBRAIN_AUTH_MIAITOKEN,
    MIBRAIN_AUTH_MIOTTOKEN,
    MIBRAIN_AUTH_TPTOKEN,
    MIBRAIN_AUTH_MIAOTOKEN,
    MIBRAIN_AUTH_ANONYMOUS = 5,
    MIBRAIN_AUTH_ANONYMOUS_PROXY = 6,
    MIBRAIN_AUTH_DS_SIGNATURE = 7,
};

typedef void* SpeechHeader;

#define MIBRAIN_CODEC_PCM   0
#define MIBRAIN_CODEC_BV32  1
#define MIBRAIN_CODEC_OPUS  2

#define MIBRAIN_OPUS_BITRATES_32K  32000
#define MIBRAIN_OPUS_BITRATES_64K  64000
#define MIBRAIN_OPUS_FRAMESIZE_640  640
#define MIBRAIN_OPUS_FRAMESIZE_1280 1280

typedef struct _MiBrainSdkRequestParams {
    MiBrainSdkCallbacks callbacks;
    MiBrainSdkNlpParams nlpParams;//可选的nlp的参数
    MiBrainSdkTtsParams ttsParams;
    MiBrainSdkAsrParams asrParams;
    MiBrainSdkVoiceprintParams voiceprintParams;
    MiBrainSdkEventParams*     events;    //event数组
    int                        eventSize; //event数组长度
    const char                *eventSessionId; //event的session id，字符串格式的id。
    enum MiBrainEnv     env;
    const char         *appId;
    const char         *appToken;//如果不填token则会回调; 在服务器对接模式下（MIBRAIN_SERVER_AUTH_DS_SIGNATURE），标识Client Key
    const char         *deviceId;
    const char         *device;//告诉云端是什么设备
    const char         *userAgent;
    const char         *scopeData;
    enum MiBrainAuthMode authMode;
    void               *userContext;
    int                 receiveTimeout;//请求发送完毕但是没收到数据的超时
    int                 connectTimeout;//连接超时默认为5秒
    int                 uploadTimeout;//单个数据包上传超时默认为5秒
    int                 activeTimeout;//长时间不发送或者没接收数据的超时限制
    int                 sdkCmd;
    const char         *apiKey;//在服务器对接模式下（MIBRAIN_SERVER_AUTH_DS_SIGNATURE），标识Client Secret
    int                 eventLevel;
    unsigned short       codecMode;  /* bit 8位: 0 表示APP自己编码.1表示需要SDK编码
                                        bit 0位:  pcm格式
                                        bit 1位: bv32float
                                        bit 2位: opus格式
                                        bit 3位: bv32fixed(未支持)
                                      */
    int                 opusFrameSize;  /* MIBRAIN_OPUS_FRAMESIZE_1280(Default) or MIBRAIN_OPUS_FRAMESIZE_640 */
    int                 opusBitRates;   /* MIBRAIN_OPUS_BITRATES_32K(Default) or MIBRAIN_OPUS_BITRATES_64K */
    int                 isPendingSend;
    SpeechHeader        customHeader; //外部可添加网络请求的header字段
    const char          *customSpeechUrl; //自定义speech服务URL，外部根据国家代码来进行配置。若不配置，则使用内部默认地址
} MiBrainSdkRequestParams;

AIVS_EXPORT int miBrainSdkSetLogHookMethod(OnMiBrainSdkLog log);

AIVS_EXPORT int miBrainSdkStartRequest(MiBrainSdkRequestParams params, long long taskId);

AIVS_EXPORT int miBrainSdkAddData(long long taskId, void *buffer, int len, int flag);//buffer大小最大不能超过16000即半秒的音频

AIVS_EXPORT int miBrainSdkStopRequest(long long taskId, int stopFlag);

AIVS_EXPORT void miBrainSdkSetDebugLevel(int level);

AIVS_EXPORT SpeechHeader miBrainSdkCreateSpeechHeader();

AIVS_EXPORT int miBrainSdkAddSpeechHeader(SpeechHeader header, const char* key, const char* value);

//需要自己释放
AIVS_EXPORT char* miBrainSdkGetResUrl(MiBrainSdkRequestParams params, const char* cp, const char* id);

AIVS_EXPORT int miBrainSdkStartNewSession();

/*
* 通过CP、ID获取包含资源URL的JSON
* params：参数
* cp：nlp结果中获取的资源cp
* id:nlp结果中获取的id 资源ID
* xiaomiId 小米ID，任何需要经鉴权的CP均必须提供此参数 如：xiaowei qingting等
* type: 资源类型 如: music/radio/on_demand
* albumId 专辑ID，目前针对CP为qingting使用
* format: 有的 cp 方对同一个 id 可以返回多个不同格式的文件。用词参数表面可接受的格式。以逗号分割。如：mp3
* response: 返回参数，获取的云端响应
* response_len：返回参数，获取的云端响应的长度
*/
AIVS_EXPORT char* miBrainSdkGetResUrlExt(MiBrainSdkRequestParams params, const char* cp, const char* id, char *xiaomiId,
                                                  char *type, char *albumId, char *format);

AIVS_EXPORT int miBrainSdkSendPendingData(MiBrainSdkRequestParams initParams, long long taskId);

AIVS_EXPORT const char* miBrainSdkGetVerison();

//---------------------------------------------------------------------------
// track message相关type和参数
//---------------------------------------------------------------------------
//#define MIBRAIN_MSG_MASK_TRACK                            0x04000000U
#define MIBRAIN_MSG_TYPE_TRACK                              0x04000000U
#define MIBRAIN_MSG_KEY_TRACK_APPID                        0x04000001U
#define MIBRAIN_MSG_KEY_TRACK_SID                           0x04000002U
#define MIBRAIN_MSG_KEY_TRACK_DID                           0x04000003U
#define MIBRAIN_MSG_KEY_TRACK_UID                           0x04000004U
#define MIBRAIN_MSG_KEY_TRACK_DATA_REQUESTID                0x04000005U
#define MIBRAIN_MSG_KEY_TRACK_DATA_RESOURCEID               0x04000006U
#define MIBRAIN_MSG_KEY_TRACK_DATA_ALBUMID                  0x04000007U
#define MIBRAIN_MSG_KEY_TRACK_DATA_EPISODE                  0x04000008U
#define MIBRAIN_MSG_KEY_TRACK_DATA_CP                       0x04000009U
#define MIBRAIN_MSG_KEY_TRACK_DATA_DOMAIN                   0x0400000aU
#define MIBRAIN_MSG_KEY_TRACK_DATA_ACTIONTYPE               0x0400000bU

//action 0,1,2,3时，设置如下几个参数
#define MIBRAIN_MSG_KEY_TRACK_DATA_STARTTIME                0x0400000cU
#define MIBRAIN_MSG_KEY_TRACK_DATA_ENDTIME                  0x0400000dU
#define MIBRAIN_MSG_KEY_TRACK_DATA_POSITION                 0x0400000eU
#define MIBRAIN_MSG_KEY_TRACK_DATA_OFFSET                   0x0400000fU

//action 4时，设置如下几个参数
#define MIBRAIN_MSG_KEY_TRACK_DATA_LOADTIME                 0x04000010U
#define MIBRAIN_MSG_KEY_TRACK_DATA_SUCCEED                  0x04000011U
#define MIBRAIN_MSG_KEY_TRACK_DATA_TIMESTAMP                0x04000012U

typedef void mibrain_track_message;
typedef void mibrain_track_result;


typedef enum _trackmsg_type {
    MIBRAIN_TRACK_MSG_PAUSE             = 0, ///< 暂停事件
    MIBRAIN_TRACK_MSG_MANSWITCH         = 1, ///< 人工切换资源事件
    MIBRAIN_TRACK_MSG_AUTOSWITCH        = 2, ///< 自动切换资源事件
    MIBRAIN_TRACK_MSG_MEDIALISTSWITCH   = 3, ///< 列表替换
    MIBRAIN_TRACK_MSG_LOADRESOURCE      = 4  ///< 加载资源成功 or 失败
} trackmsg_type;

AIVS_EXPORT mibrain_track_message* miBrainSdkCreateTrackMsg(trackmsg_type type);

AIVS_EXPORT int miBrainSdkSetTrackMsgParam(mibrain_track_message* msg, int key, const void* value, int value_len);

AIVS_EXPORT void miBrainSdkReleaseTrackMsg(mibrain_track_message* msg);

AIVS_EXPORT mibrain_track_result* miBrainSdkPostTrackMsg(MiBrainSdkRequestParams params, mibrain_track_message* msg);

AIVS_EXPORT int miBrainSdkGetTrackResultCode(mibrain_track_result* result);

AIVS_EXPORT void miBrainSdkReleaseTrackResult(mibrain_track_result* result);

#ifdef __cplusplus
}
#endif

#endif


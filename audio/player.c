#include <err.h>
#include <pthread.h>
#include <unistd.h>
#include <libavformat/avformat.h>
#include <libavresample/avresample.h>
#include <libavutil/opt.h>
#include <SDL.h>

#include "player.h"

/*
TODO:
1  framelist --> blockqueue ?? 控制解码速度防止读取过多占用较多内存
    https://github.com/vonnyfly/BlockingQueue  SDLQueue

2  AVresample  ---->   swresample

3  SDL_CreateThread replace pthread_create ?

4. 支持维护多个播放资源列表 方便从pause快速恢复 不用seek？
   SDL_OpenAudio受限 考虑混音方式mixer

*/

/*
 * Increasing the SDL audio buffer reduces risk of dropouts,
 * but increases response time.  SDL_OpenAudio(3) uses 8192 as an
 * example value, which seems to be reasonable.
 */
#define SDL_AUDIO_BUFFER_SIZE    8192

enum player_states {
    STOPPED,
    PLAYING,
    PAUSED
};

struct frame {
    Uint8    *data;    /* decoded audio data */
    size_t     size;    /* data size */
    int64_t     pts;    /* timestamp */
};

struct framelist {
    struct    frame *frame;
    struct    framelist *next;
};

static pthread_t    decodetid;
static pthread_t    watchtid;

/* framelist info */
static pthread_mutex_t    flmutex;
static pthread_cond_t    flcond;
static struct    framelist *flhead;

/* playback info */
static AVFormatContext    *avfmt;
static int    sti;    /* best audio stream index */
static enum    player_states state;
static int    duration;    /* in seconds */
static int    position;    /* in seconds */

static int    debugflag  = 1;    /* enables/disables debug messages */

/* flags */
static volatile int    abortflag;    /* signals abort to all threads */
static volatile int    eofflag;    /* reached end of file indicator */

/* resample info */
static uint64_t    out_channel_layout;
static int    out_channels;
static int    out_sample_rate;
static enum    AVSampleFormat out_sample_fmt;

/* sdl_audio_callback variables */
static Uint8    *abuf;
static size_t    abuflen;
static volatile int    abufi;

static volatile int bSpeak = 0;
static volatile int playoffset = 0;

static void *xmalloc(size_t size)
{
    void *p;

    p = malloc(size);
    if (p == NULL)
        err(1, NULL);
    return p;
}

static void xprintf(char *fmt, ...)
{
    va_list args;

    if (!debugflag)
        return;

    fflush(stdout);
    va_start(args, fmt);
    //printf(fmt, args);
    vfprintf(stderr, fmt, args);
    va_end(args);
    //printf("\n");
    fprintf(stderr, "\n");
}

static char *avstrerror(int error)
{
    static char errorstr[512];

    av_strerror(error, errorstr, sizeof(errorstr));
    return errorstr;
}

/*
 * Destroys the framelist, mutex and condition.
 */
static void fldestroy(void)
{
    struct framelist *cur, *tmp;

    cur = flhead;
    while (cur != NULL) {
        tmp = cur->next;
        if (cur->frame)
            free(cur->frame->data);
        free(cur->frame);
        free(cur);
        cur = tmp;
    }

    pthread_mutex_destroy(&flmutex);
    pthread_cond_destroy(&flcond);
}

/*
 * Initializes the framelist and creates a mutex and condition.
 */
static void flinit(void)
{
    flhead = NULL;
    pthread_mutex_init(&flmutex, NULL);
    pthread_cond_init(&flcond, NULL);
}

/*
 * Retrieves a frame from the head of the framelist.
 */
static struct frame *flpop(void)
{
    struct frame *frame;
    struct framelist *next;

    pthread_mutex_lock(&flmutex);
    if (flhead == NULL)
        pthread_cond_wait(&flcond, &flmutex);
    next = flhead->next;
    frame = flhead->frame;
    free(flhead);
    flhead = next;
    pthread_mutex_unlock(&flmutex);
    return frame;
}

/*
 * Adds a frame to the end of the framelist.
 */
static void flpush(struct frame *frame)
{
    struct framelist *cur, *tmp;

    tmp = xmalloc(sizeof(struct framelist));
    tmp->frame = frame;
    tmp->next = NULL;

    pthread_mutex_lock(&flmutex);
    if (flhead == NULL)
        flhead = tmp;
    else {
        for (cur = flhead; cur->next != NULL; cur = cur->next)
            continue;
        cur->next = tmp;
    }
    pthread_cond_signal(&flcond);
    pthread_mutex_unlock(&flmutex);
}

/*
 * 关闭ffmpeg codec.
 */
static void close_decoder(void)
{
    avcodec_close(avfmt->streams[sti]->codec);
    avformat_close_input(&avfmt);
    avfmt = NULL;
}


/*
 * 打开播放流 并取对应的解码器
 * Opens a url, reads its stream info, locates its audio stream index, and
 * loads the corresponding codec.
 */
static int open_decoder(const char *url)
{
    AVCodec *codec = NULL;
    int nr;

    if (url == NULL)
        return -1;

    nr = avformat_open_input(&avfmt, url, NULL, NULL);
    if (nr < 0) {
        xprintf("(%s:%d) avformat_open_input, error: %s",
            __FILE__, __LINE__, avstrerror(nr));
        goto error;
    }
    nr = avformat_find_stream_info(avfmt, NULL);
    if (nr < 0) {
        xprintf("(%s:%d) avformat_find_stream, error: %s",
            __FILE__, __LINE__, avstrerror(nr));
        goto error;
    }
    sti = av_find_best_stream(avfmt, AVMEDIA_TYPE_AUDIO, -1, -1,
        &codec, 0);
    if (sti < 0) {
        xprintf("(%s:%d) av_find_best_stream, error: %s",
            __FILE__, __LINE__, avstrerror(nr));
        goto error;
    }
    nr = avcodec_open2(avfmt->streams[sti]->codec, codec, NULL);
    if (nr < 0) {
        xprintf("(%s:%d) avcodec_open2, error: %s",
            __FILE__, __LINE__, avstrerror(nr));
        goto error;
    }
    duration = (int)(av_q2d(avfmt->streams[sti]->time_base) *
        (double)avfmt->streams[sti]->duration);
    return 0;
error:
    if (codec)
        avcodec_close(avfmt->streams[sti]->codec);
    if (avfmt) {
        avformat_close_input(&avfmt);
        avfmt = NULL;
    }
    return -1;
}

/*
 * 重新采样
 * Resamples a frame if needed and pushes the audio data along
 * with a timestamp onto the framelist.
 */
static void resampleframe(AVFrame *avframe)
{
    AVAudioResampleContext *avr;
    struct frame *frame;
    uint8_t *output;
    size_t outputlen;
    int linesize, nr, samples;

    if (avframe->format != out_sample_fmt ||
        avframe->channel_layout != out_channel_layout ||
        avframe->sample_rate != out_sample_rate) {
        avr = avresample_alloc_context();
        if (avr == NULL) {
           xprintf("(%s:%d) avresample_alloc_context",
                __FILE__, __LINE__);
        }
        av_opt_set_int(avr, "in_channel_layout",
            avframe->channel_layout, 0);
        av_opt_set_int(avr, "out_channel_layout", out_channel_layout,
            0);
        av_opt_set_int(avr, "in_sample_rate", avframe->sample_rate, 0);
        av_opt_set_int(avr, "out_sample_rate", out_sample_rate, 0);
        av_opt_set_int(avr, "in_sample_fmt", avframe->format, 0);
        av_opt_set_int(avr, "out_sample_fmt", out_sample_fmt, 0);
        nr = avresample_open(avr);
        if (nr < 0) {
            avresample_free(&avr);
            return;
        }
        outputlen = av_samples_get_buffer_size(&linesize, out_channels,
            avframe->nb_samples, out_sample_fmt, 0);
        output = xmalloc(outputlen);
        samples = avresample_convert(avr, &output, linesize,
            avframe->nb_samples, avframe->data, avframe->linesize[0],
            avframe->nb_samples);
        outputlen = samples * out_channels *
            av_get_bytes_per_sample(out_sample_fmt);
        avresample_close(avr);
        avresample_free(&avr);
    } else {
        outputlen = av_samples_get_buffer_size(NULL,
            avfmt->streams[sti]->codec->channels,
            avframe->nb_samples, avframe->format, 1);
        output = xmalloc(outputlen);
        memcpy(output, avframe->data[0], outputlen);
    }
    frame = xmalloc(sizeof(struct frame));
    frame->data = output;
    frame->size = outputlen;
    frame->pts = avframe->pkt_pts;
    flpush(frame);
}

static void sdl_audio_callback(void *userdata, Uint8 *stream, int streamlen)
{
    struct frame *frame;
    size_t len;

    while (streamlen > 0 && !eofflag) {
        if (abufi >= abuflen) {
            free(abuf);
            frame = flpop();
            /* A NULL frame means that we reached the end. */
            if (frame == NULL) {
                /* Fill the remaining buffer with silence */
                memset(stream, 0, streamlen);
                eofflag = 1;
                break;
            }
            abufi = 0;
            abuflen = frame->size;
            abuf = frame->data;

            position = (int)(av_q2d(
                avfmt->streams[sti]->time_base) *
                (double)frame->pts);
            free(frame);
        }
        len = abuflen - abufi;
        if ((int)len > streamlen){
            len = streamlen;
        }

        SDL_memset(stream, 0, len);
        //SDL_MixAudio(stream, abuf + abufi, len, SDL_MIX_MAXVOLUME);
        stream = memcpy(stream, abuf + abufi, len);
        streamlen -= len;
        stream += len;
        abufi += len;
    }
}


/*
 * 重采样 Splits a packet into frames and sends each frame to the resampler.
 */
static void decodepacket(AVPacket *packet)
{
    AVFrame *frame;
    int got_frame, nr;

    frame = av_frame_alloc();
    if (frame == NULL)
        xprintf("(%s:%d) av_frame_alloc failed", __FILE__, __LINE__);
    while (packet->size > 0) {
        got_frame = 0;
        nr = avcodec_decode_audio4(avfmt->streams[sti]->codec, frame,
            &got_frame, packet);
        if (nr < 0)
            break;
        packet->size -= nr;
        packet->data += nr;
        if (got_frame)
            resampleframe(frame);
    }
    av_frame_free(&frame);
}

/*
 * 解码thread
 * 结束发送null frame 标志结束
 */
static void *decodethread(void *arg)
{
    AVPacket packet;
    int nr;

    //av_seek_frame(avfmt, -1, playoffset * AV_TIME_BASE, AVSEEK_FLAG_ANY);
    while (!abortflag) {
        nr = av_read_frame(avfmt, &packet);
        if (nr < 0) {
            xprintf("(%s:%d) av_read_frame, warning: %s",
                __FILE__, __LINE__, avstrerror(nr));
            break;
        }

        /* discard unwanted packets */
        if (packet.stream_index != sti) {
            av_free_packet(&packet);
            continue;
        }
        decodepacket(&packet);
        av_free_packet(&packet);
    }
    flpush(NULL);
    pthread_exit(NULL);
}


/*
 * Opens the SDL audio device and sets channel_layout, channels, sample_rate
 * and sample_fmt output information that we use for the resample section
 * in the decoder.
 * This function should be called after openfile, because we need its
 * channel, channel_layout, and sample_rate information.
 *
 * On error it returns -1, on success 0;
 */
static int open_sdl_audio(void)
{
    AVCodecContext *codec;
    SDL_AudioSpec *desired, *obtained, *hwspec;

    if (avfmt == NULL) {
        xprintf("(%s:%d) AVFormatContext was not set",
            __FILE__, __LINE__);
        return -1;
    }

    desired = xmalloc(sizeof(SDL_AudioSpec));
    obtained = xmalloc(sizeof(SDL_AudioSpec));

    codec = avfmt->streams[sti]->codec;
    if (!codec->channel_layout) {
        codec->channel_layout = av_get_default_channel_layout(
            codec->channels);
    }
    if (!codec->channel_layout) {
        xprintf("(%s:%d) Unable to guess channel layout",
            __FILE__, __LINE__);
        goto error;
    }

    desired->callback = sdl_audio_callback;
    if (codec->channels == 1)
        desired->channels = 1;    /* mono */
    else
        desired->channels = 2;    /* stereo */
    desired->format = AUDIO_S16SYS;
    desired->freq = codec->sample_rate;
    desired->samples = SDL_AUDIO_BUFFER_SIZE;
    desired->silence = 0;
    desired->userdata = NULL;
    if (SDL_OpenAudio(desired, obtained) < 0) {
        xprintf("(%s:%d) SDL_OpenAudio: %s", __FILE__, __LINE__,
            SDL_GetError());
        goto error;
    }
    if (obtained == NULL)
        hwspec = desired;
    else
        hwspec = obtained;

    if (hwspec->channels == 1)
        out_channel_layout = AV_CH_LAYOUT_MONO;
    else
        out_channel_layout = AV_CH_LAYOUT_STEREO;
    out_channels = hwspec->channels;
    out_sample_rate = hwspec->freq;
    out_sample_fmt = AV_SAMPLE_FMT_S16;

    free(desired);
    free(obtained);
    return 0;
error:
    free(desired);
    free(obtained);
    return -1;
}

/*
 * 监控播放状态 线程
 */
static void *watchthread(void *arg)
{
    while (!abortflag && !eofflag){
        SDL_Delay(100);
    }

    pthread_join(decodetid, NULL);
    SDL_CloseAudio();
    close_decoder();
    fldestroy();

    xprintf("(%s:%d) watch thread exit, bspeak: %d abortflag:%d eofflag:%d\n",
                __FILE__, __LINE__, bSpeak, abortflag, eofflag);

    /* reset playack related variables */
    abortflag = 0;
    duration = 0;
    position = 0;
    abuf = NULL;
    abuflen = 0;
    abufi = 0;
    state = STOPPED;

    /* notify sdk play item done */
    if (bSpeak){
        avis_speak_on_finished();
    }else{
        if (eofflag){
            miBrainSdkPlayerItemfinished();
        }
    }

    eofflag = 0;
    pthread_exit(NULL);
}

/*
 * Loads url and starts playback.
 * On success returns 0, on failure -1.
 */
int player_play(const char *url)
{
    if (state != STOPPED){
        player_stop();
    }
    if (open_decoder(url) < 0){
        return -1;
    }
    if (open_sdl_audio() < 0) {
        close_decoder();
        return -1;
    }
    flinit();    /* initialize the framelist */

    /*
     * Create two threads: one for reading and decoding the url,
     * the other one for cleaning up after the track has finished or
     * after an abort signal.
     */

    abortflag = 0;
    eofflag = 0;
    pthread_create(&watchtid, NULL, watchthread, NULL);
    pthread_detach(watchtid);
    pthread_create(&decodetid, NULL, decodethread, NULL);

    SDL_PauseAudio(0);    /* start playback (1 = Pause, 0 = Play) */
    state = PLAYING;
    return 0;
}

/*
 * Returns duration in seconds.
 */
int player_getduration(void)
{
    return duration;
}

/*
 * Returns position in seconds.
 */
int player_getposition(void)
{
    return position;
}

int player_getstatus(void)
{
    return state;
}

/*
 * Enables debug messages.
 */
void player_setdebugmode(void)
{
    debugflag = 1;
}

/*
 * Stops the current playback.
 * This function waits until the playback has gracefully stopped.
 */
void player_stop(void)
{
    if (state == STOPPED)
        return;

    abortflag = 1;
    while (state != STOPPED)
        SDL_Delay(100);
}

/*
 * Pauses the SDL callback function.
 * The decodethread does not pause.
 */
void player_togglepause(void)
{
    if (state == PLAYING) {
        SDL_PauseAudio(1);
        state = PAUSED;
    } else if (state == PAUSED) {
        SDL_PauseAudio(0);
        state = PLAYING;
    }
}

void player_init(void)
{
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    if (debugflag)
        av_log_set_level(AV_LOG_VERBOSE);
    else
        av_log_set_level(AV_LOG_QUIET);
    avfmt = NULL;
    abortflag = 0;
    eofflag = 0;
    duration = 0;
    position = 0;
    abuf = NULL;
    abuflen = 0;
    abufi = 0;
    state = STOPPED;
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
        xprintf("Unable to initialize SDL: %s", SDL_GetError());
}

void player_exit(void)
{
    if (state != STOPPED)
        player_stop();

    avformat_network_deinit();
    SDL_Quit();
}



/* TODO 控制列表由 SDK实现时 APP实现这个weak 函数 */

/* 播放器独立实现 此处通过mqtt public 消息控制 播放器 并接受回应 */
int miBrain_audio_pause_handler(void){
    int pos = player_getposition();
    xprintf("miBrain_audio_pause_handler  offset:%d\n",  pos);
    player_stop();
    return pos;
}

void miBrain_audio_stop_handler(void){
    player_stop();
    xprintf("#############miBrain_audio_stop_handler ###################:\n");
}
void miBrain_audio_play_handler(const char *url, int offset){
    bSpeak = 0;
    playoffset = offset;
    xprintf("miBrain_audio_play_handler play url:%s, offset:%d\n", url, offset);
    player_play(url);
}

void aivs_speak_handler(const char *url){
    bSpeak = 1;
    xprintf("aivs_speak_handler play url:%s\n", url);
    player_play(url);
}


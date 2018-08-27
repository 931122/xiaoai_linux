
#ifdef __cplusplus
extern "C" {
#endif

#include <err.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "cJSON.h"
#include "player.h"
#include "mibrain_player.h"

static int s_is_speech_supported = 0;

enum _play_state {
    PLAYING,
    FINISHED,
    MAX_PLAY_STATE,
};

//static const char* const s_player_activity[MAX_PLAY_STATE] = {"PLAYING", "FINISHED"};

static volatile int s_play_state = FINISHED;

static int aivs_report_speak_started_event()
{
    return 0;
}

static int aivs_report_speak_finished_event()
{
    return 0;
}

int aivs_speak_on_work(void)
{
    if (s_play_state == PLAYING) {
        return 1;
    }
    return 0;
}

void avis_speak_on_finished(void)
{
    printf("Speak done on %d trigger player resum\n", s_play_state);

    if (s_play_state == PLAYING) {
        s_play_state = FINISHED;

        aivs_report_speak_finished_event();
        if (1/*!duer_is_multiple_round_dialogue()*/) {
            miBrainSdkPlayerResume();
        }
    }
}

int aivs_speak_start(char *url){
    s_play_state = PLAYING;
    printf("Speak start on %d \n", s_play_state);

    // Pause audio player
    miBrainSdkPlayerPause();
    aivs_report_speak_started_event();

    aivs_speak_handler(url);

    return 0;
}

int aivs_get_speak_state(void)
{
    return 0;
}

void aivs_speak_init(void)
{
    static int is_first_time = 1;

    if (!is_first_time) {
        return;
    }

    is_first_time = 0;

    s_is_speech_supported = 1;
}

void aivs_speak_fini(void)
{

}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdio.h>
/**
 * DESC:
 * Developer needs to implement this interface to play audio.
 *
 * PARAM:
 * @param[in] url: the url of the audio need to play
 *
 * @RETURN: none.
 */
__attribute__((weak)) void miBrain_audio_play_handler(const char *url, int offset)
{
    printf( "Please implement audio_play_handler interface to play audio");
}

/**
 * DESC:
 * Developer needs to implement this interface to stop audio player.
 *
 * PARAM: none
 *
 * @RETURN: none.
 */
__attribute__((weak)) void miBrain_audio_stop_handler(void)
{
    printf( "Please implement audio_stop_handler interface to stop audio player");
}

/**
 * DESC:
 * Developer needs to implement this interface to pause audio play.
 *
 * PARAM: none
 *
 * @RETURN: the play position of the current audio.
 */
__attribute__((weak)) int miBrain_audio_pause_handler(void)
{
    printf( "Please implement audio_pause_handler interface to pause audio player");
    return 0;
}

/**
 * DESC:
 * Developer needs to implement this interface to play speech.
 *
 * PARAM:
 * @param[in] url: the url of the speech need to play
 *
 * @RETURN: none.
 */
__attribute__((weak)) void aivs_speak_handler(const char *url)
{
    printf( "Please implement aivs_speak_handler interface to play speech");
}
#if 0

/**
 * DESC:
 * Developer needs to implement this interface, it is used to get volume state.
 *
 * @param[out] volume: current volume value.
 * @param[out] is_mute: current mute state.
 *
 * @RETURN: none.
 */
__attribute__((weak)) void duer_dcs_get_speaker_state(int *volume, bool *is_mute)
{
    DUER_LOGW("Please implement this interface to get speaker state");
}

/**
 * DESC:
 * Developer needs to implement this interface to set volume.
 *
 * PARAM:
 * @param[in] volume: the value of the volume need to set
 *
 * @RETURN: none.
 */
__attribute__((weak)) void duer_dcs_volume_set_handler(int volume)
{
    DUER_LOGW("Please implement this interface to set volume");
}

/**
 * DESC:
 * Developer needs to implement this interface to adjust volume.
 *
 * PARAM:
 * @param[in] volume: the value need to adjusted.
 *
 * @RETURN: none.
 */
__attribute__((weak)) void duer_dcs_volume_adjust_handler(int volume)
{
    DUER_LOGW("Please implement this interface to adjust volume");
}

/**
 * DESC:
 * Developer needs to implement this interface to change mute state.
 *
 * PARAM:
 * @param[in] is_mute: set/discard mute.
 *
 * @RETURN: none.
 */
__attribute__((weak)) void duer_dcs_mute_handler(bool is_mute)
{
    DUER_LOGW("Please implement this interface to set mute state");
}

/**
 * DESC:
 * Developer needs to implement this interface to start recording.
 *
 * PARAM: none
 *
 * @RETURN: none.
 */
__attribute__((weak)) void duer_dcs_listen_handler(void)
{
    DUER_LOGW("Please implement this interface to start recording");
}

/**
 * DESC:
 * Developer needs to implement this interface to stop recording.
 *
 * PARAM: none
 *
 * @RETURN: none.
 */
__attribute__((weak)) void duer_dcs_stop_listen_handler(void)
{
    DUER_LOGW("Please implement this interface to stop recording");
}

/**
 * DESC:
 * Developer needs to implement this interface to set alert.
 *
 * PARAM:
 * @param[in] token: the token of the alert, it's the ID of the alert.
 * @param[in] time: the schedule time of the alert, ISO 8601 format.
 * @param[in] type: the type of the alert, TIMER or ALARM.
 *
 * @RETURN: none.
 */
__attribute__((weak)) void duer_dcs_alert_set_handler(const char *token,
                                                      const char *time,
                                                      const char *type)
{
    DUER_LOGW("Please implement this interface to set alert");
}

/**
 * DESC:
 * Developer needs to implement this interface to delete alert.
 *
 * PARAM:
 * @param[in] token: the token of the alert need to delete.
 *
 * @RETURN: none.
 */
__attribute__((weak)) void duer_dcs_alert_delete_handler(const char *token)
{
    DUER_LOGW("Please implement this interface to delete alert");
}

/**
 * DESC:
 * It's used to get all alert info by DCS.
 * Developer can implement this interface by call duer_dcs_report_alert to report all alert info.
 *
 * PARAM:
 * @param[in] token: the token of the alert need to delete.
 *
 * @RETURN: none.
 */
__attribute__((weak)) void duer_dcs_get_all_alert(baidu_json *alert_array)
{
    DUER_LOGW("Please implement this interface to report all alert info");
}
#endif

#ifdef __cplusplus
}
#endif



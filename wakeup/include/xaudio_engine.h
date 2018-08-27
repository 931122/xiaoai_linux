/*
 * xaudio_engine.h
 *
 *  Created on: Apr 10, 2017
 *      Author: xaudio
 */

#ifndef API_XAUDIO_ENGINE_H_
#define API_XAUDIO_ENGINE_H_

/*
 * @brief: Call-back functional handler
 * @param val: voice energy value
 *
 */
typedef void (*mi_wav_energy_callback)(float val);

/*
 * Extension of the following `oneshot_callback`
 */
typedef enum {
    EVENT_WAKE_NORMAL,//normal wake
    EVENT_WAKE_ONESHOT,//one-shot wake
    EVENT_VAD_NORMAL,//vad middle status
    EVENT_VAD_BEGIN,//vad begin flag
    EVENT_VAD_END,//vad end edge flag
    EVENT_VAD_TIMEOUT//vad end flag due to timeout
} OneShortEventType;

/*
 * @brief: Call-back functional handler
 * @param event: denoting the event corresponding to events defined by OneShortEventType
 *
 */
typedef void (*mi_oneshot_callback)(OneShortEventType event);

/*
 * @brief: Call-back functional handler
 * @param buffer: voip data (currently mono-channel, 16KHz)
 * @param size: buffer length
 * @Note: When set up voip-mode, you would be feed with this voip data, copy it for further usage
 *
 */
typedef void (*mi_voip_data_callback)(void * usr_data_voip, const char *buffer, size_t size);

/*
 * @brief: Call-back functional handler
 * @param buffer: wakeup data (currently mono-channel, 16KHz) for transfering
 * @param size: buffer length
 * @Note: When you got this wakeup data, copy it for transfering to sai wake-learning online server
 *
 */
typedef void (*mi_ivw_data_callback)(const char *buffer, size_t size);

/*
 * @brief: Call-back functional handler
 * @param buffer: preprocessed asr data for transfering
 * @param size: buffer length
 * @Note: When you got data, copy it for transfering to sai online server for postprocessing
 *
 */
typedef void (*mi_asr_data_callback)(void * usr_data_asr, const char *buffer, size_t size);


/*
 * @brief: Call-back functional handler
 * @param wakeup_result: wake keyword index
 * @param angle: angle value
 * @Note: When input voice wake up devices, this functional handler is called back.
 * 		  This function cannot be blocked.
 *
 */
typedef void (*mi_wakeup_callback)(void * usr_data_wk, int wakeup_result, float angle);

/*
 * @brief :Initialize data system
 * @param info: client information text-string, got from sai provisioner
 * @param threshold: wakeup threshold (0~1)
 * @param config_file_path: resource file path, e.g., "/etc/sai_config"
 * @param asr_cbf, w_cbf, voip_cbf: call-back handlers for asr data, wakeup, voip data, others likewise
 * @return 0 successful, error code otherwise
 */
int init_system(const char * info, double threshold, const char * config_file_path,
		mi_asr_data_callback asr_cbf, mi_wakeup_callback w_cbf, mi_ivw_data_callback ivw_cbf,
		mi_voip_data_callback voip_cbf, mi_oneshot_callback one_shot_cbf, mi_wav_energy_callback energy_cbf,
                void * usr_data_asr, void * usr_data_wk, void * usr_data_voip);

/*
 * @brief: Start service
 * @return 0 successful, error code otherwise
 */
int start_service();

/*
 * @brief: Stop service
 * @return 0 successful, error code otherwise
 */
int stop_service();

/*
 * @brief: Terminate & release system resources
 */
void terminate_system();

/*
 * @brief: Tell sdk that its waken-status must stop right now.
 *
 */
void set_unwakeup_status();

/*
 * @brief: Tell sdk that it has to be waken right now
 *
 */
void set_wakeup_status();

/*
 * @brief: set the volume.
 * @note:	need to set volume before start service and when volume has change.
 *
 */
void set_volume(int volume);

/*
 * @brief: Set prefix & suffix (ms) for wakeup pcm audio recording
 *
 */
void set_wakeup_prefix_suffix(size_t prefix, size_t suffix);

/*
 * @brief: Set voip mode
 * @param flag: 0: normal mode, 1: voip mode
 *
 */
void set_voip_flag(int flag);

/*
 * @brief: Set voip beam pattern
 * @param flag: 1: fixed direction; 0: non-fixed direction
 *
 */
void set_voip_beam(int flag);

/*
 * @brief: Shut wake-up key word: @word: 0: none; 1: xiao ai tong xue; 2: xiao mi dian shi
 *
 */
void shut_wake_key_word(int word);

/*
 * @brief: Enable or disable the function of providing wake-up data
 * @param en: 1(enable), 0(disable)
 *
 */
void enable_wake_data(int en);

/*
 * @brief: Enable or disable the function of providing wake-up data
 * @param en: 1(enable), 0(disable)
 *
 */
void enable_energy_provision(int en);

/*
 * @brief: Get wakeup energy value
 * @return energy value
 *
 */
float get_wake_word_energy();

/*
 * @brief: Set time out (s) of vad process
 * @param time: timeout threshold
 *
 */
void set_vad_timeout(float time);

/*
 * @brief: Enable or disable local vad
 * @param val: 1(enable), 0(disable)
 *
 */
void enable_local_vad(int val);

/*
 * @brief: Obtain driver data
 * @param buf: buffer pointer (has to be allocated and released by the user)
 * @param len_limit: maximum length of the buffer
 * @return actual bytes of the buffer
 *
 */
size_t get_local_data(char *buffer, size_t len_limit);

/*
 * @brief: Obtain size of the wake word audio
 * @return length(ms) of the wake word pcm
 *
 */
size_t get_wake_word_length();

/*
 * @brief: Try to set led with element-wise led-light-control
 * @return 0 successful
 *
 */
int set_led_lights(unsigned int leds);

/*
 * @brief: Try to set led with angle
 * @param angle: angle value
 * @return 0 successful
 *
 */
int set_led_angle(float angle);

/*
 * @brief: Try to set led with pre-defined led-light-mode
 * @param mode:
 *       - 0 all blue (default)
 *       - 1 all dimmed
 *       - 2 all green
 *       - 3 all red
 * @return 0 successful
 *
 */
int set_led_mode(int mode);

#endif /* API_XAUDIO_ENGINE_H_ */
